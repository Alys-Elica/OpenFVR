#ifndef FVRRENDERER_H
#define FVRRENDERER_H

#if defined(_MSC_VER) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define FVRRENDERER_DECL_EXPORT __declspec(dllexport)
#define FVRRENDERER_DECL_IMPORT __declspec(dllimport)
#else
#define FVRRENDERER_DECL_EXPORT __attribute__((visibility("default")))
#define FVRRENDERER_DECL_IMPORT __attribute__((visibility("default")))
#endif

#if defined(FVRRENDERER_LIBRARY)
#define FVRRENDERER_EXPORT FVRRENDERER_DECL_EXPORT
#else
#define FVRRENDERER_EXPORT FVRRENDERER_DECL_IMPORT
#endif

/**
 * @class FvrRenderer
 * @brief This class is responsible for rendering panoramic VR files.
 *
 * The FvrRenderer class provides the functionality to render panoramic VR files.
 * This is a basic and inefficient software renderer, and is used as a "working" placeholder replacement for the DLL version.
 * It takes about 10ms to render a single 1920*1080 frame on a Ryzen 9 3900XT CPU.
 *
 * Feel free to optimize it as you see fit.
 */
class FVRRENDERER_EXPORT FvrRenderer {
public:
    FvrRenderer();
    ~FvrRenderer();

    /**
     * @brief Retrieves the internal cubemap buffer.
     *
     * This method is used to get the current cubemap buffer.
     * The cubemap buffer is a 256*6144*2 bytes array (allocated interenally).
     * It stores the 256*6144 pixel data in 16bit RGB565 format.
     *
     * All 6 faces are stored top to bottom in the following order:
     *     - Down
     *     - Left
     *     - Up
     *     - Right
     *     - Front
     *     - Back
     * Each face is a 512*512 image divided in 4 256*256 sub-images stored in the following order:
     *     - Top-left
     *     - Top-right
     *     - Bottom-right
     *     - Bottom-left
     *
     * @return The pointer to the internal cubemap buffer.
     */
    unsigned short* cubemapBuffer();

    /**
     * @brief Sets the resolution for the VR projection.
     *
     * @param width The width of the VR projection.
     * @param height The height of the VR projection.
     */
    void setResolution(
        float width,
        float height);

    /**
     * @brief Renders a frame.
     *
     * This method is responsible for drawing a single frame. It can be called
     * repeatedly by the main loop.
     *
     * @param outputBuffer The RGB565 buffer where the rendered frame will be stored.
     * @param yaw The yaw rotation angle (radian).
     * @param pitch The pitch rotation angle (radian).
     * @param roll The roll rotation angle (radian).
     * @param fov The field of view.
     */
    void render(
        unsigned short* outputBuffer,
        float yaw,
        float pitch,
        float roll,
        float fov);

private:
    class FvrRendererPrivate;
    FvrRendererPrivate* d_ptr;
};

#endif // FVRRENDERER_H
