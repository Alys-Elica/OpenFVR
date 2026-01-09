#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

#include <ofnx/files/tst.h>
#include <ofnx/files/vr.h>
#include <ofnx/ofnxmanager.h>

#define FNXVR_WINDOW_WIDTH 640
#define FNXVR_WINDOW_HEIGHT 480
#define FNXVR_WINDOW_FOV 1.0f
#define FNXVR_MOUSE_SENSITIVITY 0.1f
#define FNXVR_FPS 30.0
#define FNXVR_FRAMETIME 1000.0 / FNXVR_FPS

std::string removeExtension(const std::string& str)
{
    size_t lastindex = str.find_last_of(".");
    return str.substr(0, lastindex);
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <vr_file_vr>" << std::endl;
        return 1;
    }

    const std::string vrFileName(argv[1]);

    std::cout << "Opening VR file " << vrFileName << std::endl;

    // Load VR file VR
    ofnx::files::Vr vrFile;
    if (!vrFile.load(vrFileName)) {
        std::cerr << "VR failed to load" << std::endl;
        return 1;
    }

    std::vector<uint16_t> imageData;
    if (!vrFile.getDataRgb565(imageData)) {
        std::cerr << "Failed to load VR image data" << std::endl;
        return 1;
    }

    // Load TST file if present
    std::string tstFileName = removeExtension(vrFileName) + ".tst";
    ofnx::files::Tst tstFile;
    if (tstFile.loadFile(tstFileName)) {
        std::cout << "Corresponding TST file found" << std::endl;
    }

    // Init Ofnx manager
    ofnx::OfnxManager ofnx;
    if (!ofnx.init(FNXVR_WINDOW_WIDTH, FNXVR_WINDOW_HEIGHT)) {
        std::cerr << "Failed to init Ofnx manager" << std::endl;
        return 1;
    }

    // Main loop
    bool isVr = true;
    // TODO: VR2 files seems to have a different cube face structure
    if (vrFile.getType() == ofnx::files::Vr::Type::VR_STATIC_VR
        || vrFile.getType() == ofnx::files::Vr::Type::VR2_STATIC_VR) {
        ofnx.renderer().updateVr(imageData.data());
        ofnx.renderer().setCursorSettings(true, true);
        isVr = true;
    } else {
        ofnx.renderer().updateFrame(imageData.data());
        ofnx.renderer().setCursorSettings(true, false);
        isVr = false;
    }

    bool isRunning = true;

    float yawDeg = 270.0f; // Horizontal (left/right)
    float pitchDeg = 90.0f; // Vertical (up/down) - 0.0f: down, 90.0f: straight, 180.0f: up
    float rollDeg = 0.0f; // Tilt (left/right)
    float fov = FNXVR_WINDOW_FOV;

    while (isRunning) {
        const std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();

        // Handle events
        for (ofnx::OfnxManager::Event event : ofnx.getEvents()) {
            switch (event.type) {
            case ofnx::OfnxManager::Event::Quit:
                isRunning = false;
                break;

            case ofnx::OfnxManager::Event::MouseMove: {
                yawDeg += event.xRel * FNXVR_MOUSE_SENSITIVITY;
                pitchDeg -= event.yRel * FNXVR_MOUSE_SENSITIVITY;

                if (yawDeg < 0.0f) {
                    yawDeg = 360.0f;
                } else if (yawDeg > 360.0f) {
                    yawDeg = 0.0f;
                }

                if (pitchDeg < 0.0f) {
                    pitchDeg = 0.0f;
                } else if (pitchDeg > 360.0f) {
                    pitchDeg = 0.0f;
                }

                pitchDeg = std::clamp(pitchDeg, 0.0f, 180.0f);

                // TST zone
                int tstZone = -1;
                if (isVr) {
                    tstZone = tstFile.checkZoneVr(yawDeg, pitchDeg);
                } else {
                    tstZone = tstFile.checkZoneStatic(event.x, event.y);
                }

                if (tstZone == -1) {
                    ofnx.renderer()
                        .setCursorSystem(ofnx::graphics::RendererOpenGL::CursorSystem::Default);
                } else {
                    ofnx.renderer()
                        .setCursorSystem(ofnx::graphics::RendererOpenGL::CursorSystem::Pointer);
                }
            } break;

            case ofnx::OfnxManager::Event::MouseWheel:
                fov -= event.y * 0.1f;
                fov = std::clamp(fov, 0.5f, 2.0f);
                break;

            case ofnx::OfnxManager::Event::MouseClickLeft:
                break;
            }
        }

        // Render
        if (isVr) {
            // vrFile.applyAnimationFrameRgb565("torche", imageData.data());
            ofnx.renderer().updateVr(imageData.data());
            ofnx.renderer().renderVr(yawDeg, pitchDeg, rollDeg, fov);
        } else {
            ofnx.renderer().renderFrame();
        }

        const std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now();
        double elapsedTime = (double)std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0;

        if (FNXVR_FRAMETIME > elapsedTime) {
            std::this_thread::sleep_for(
                std::chrono::microseconds((int)((FNXVR_FRAMETIME - elapsedTime) * 1000.0)));
        }
    }

    // Clean up
    ofnx.deinit();

    return 0;
}
