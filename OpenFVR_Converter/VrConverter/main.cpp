#include <iostream>

#include <boost/program_options.hpp>

#include "filevr.h"
#include "fnxvr.h"

int vrConvert(const std::string &vrFileName, const bool toCubemap)
{
    std::cout << "Converting " << vrFileName << " to PNG" << std::endl;

    FileVr vr;
    if (!vr.load(vrFileName))
    {
        std::cout << "Error loading VR file" << std::endl;
        return 1;
    }

    if (!vr.isValid())
    {
        std::cout << "VR file is not valid" << std::endl;
        return 2;
    }

    std::vector<uint8_t> frameData;
    if (!vr.getRawFrameRgb565(frameData))
    {
        std::cout << "Error unpacking VR frame" << std::endl;
        return 3;
    }

    Frame frame = Frame::rgb565ToFrame(frameData, vr.getFrameWidth(), vr.getFrameHeight());
    if (!frame.isValid())
    {
        std::cout << "Error converting to frame" << std::endl;
        return 4;
    }

    if (toCubemap && vr.getType() == FileVr::Type::VR_STATIC_VR)
    {
        Frame tmpFrame = frame;
        Frame::vrToCubicMap(tmpFrame, frame);
    }

    std::string baseFileName = vrFileName.substr(0, vrFileName.length() - 3);
    if (!frame.saveToPng(baseFileName + ".png"))
    {
        std::cout << "Error saving to image" << std::endl;
        return 5;
    }

    return 0;
}

int vrAnimation(const std::string &vrFileName, const bool toCubemap)
{
    std::cout << "Exporting " << vrFileName << " animations" << std::endl;

    FileVr vr;
    if (!vr.load(vrFileName))
    {
        std::cout << "Error loading VR file" << std::endl;
        return 1;
    }

    if (!vr.isValid())
    {
        std::cout << "VR file is not valid" << std::endl;
        return 2;
    }

    std::vector<FileVr::Animation> animationList;
    if (!vr.getRawAnimationsRgba32(animationList))
    {
        std::cout << "Error unpacking VR animations" << std::endl;
        return 3;
    }

    for (FileVr::Animation &animation : animationList)
    {
        if (toCubemap && vr.getType() == FileVr::Type::VR_STATIC_VR)
        {
            for (Frame &frame : animation.frameList)
            {
                Frame tmpFrame = frame;
                Frame::vrToCubicMap(tmpFrame, frame);
            }
        }

        std::string baseFileName = vrFileName.substr(0, vrFileName.length() - 3);
        if (!Frame::framesToWebP(animation.frameList, baseFileName + "_" + animation.name + ".webp"))
        {
            std::cout << "Error saving to image" << std::endl;
            return 4;
        }
    }

    return -1;
}

int vrViewer(const std::string &vrFileName)
{
    FnxVr fnxVr;
    if (!fnxVr.isValid())
    {
        std::cout << "Failed to initialise FnxVr" << std::endl;
        return 1;
    }

    if (!fnxVr.loadFile(vrFileName))
    {
        std::cout << "Failed to load VR file" << std::endl;
        return 2;
    }

    if (!fnxVr.loop())
    {
        std::cout << "Failed to execute viewer" << std::endl;
        return 3;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    boost::program_options::options_description optGeneric("Program options");
    optGeneric.add_options()("help,h", "help message");
    optGeneric.add_options()("vr-convert", boost::program_options::value<std::string>(), "converts the given VR file to a PNG image");
    optGeneric.add_options()("vr-convert-cubemap", boost::program_options::value<std::string>(), "converts the given VR file to a cubemap PNG image");
    optGeneric.add_options()("vr-animation", boost::program_options::value<std::string>(), "converts the given VR file animations to a WebP animated image");
    optGeneric.add_options()("vr-animation-cubemap", boost::program_options::value<std::string>(), "converts the given VR file to a cubemap WebP animated image");
    optGeneric.add_options()("vr-viewer", boost::program_options::value<std::string>(), "view the given VR file");

    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(optGeneric).run(), vm);
    boost::program_options::notify(vm);

    if (vm.count("help"))
    {
        std::cout << optGeneric << std::endl;
        return 0;
    }

    if (vm.count("vr-convert"))
    {
        return vrConvert(vm["vr-convert"].as<std::string>(), false);
    }

    if (vm.count("vr-convert-cubemap"))
    {
        return vrConvert(vm["vr-convert-cubemap"].as<std::string>(), true);
    }

    if (vm.count("vr-animation"))
    {
        return vrAnimation(vm["vr-animation"].as<std::string>(), false);
    }

    if (vm.count("vr-animation-cubemap"))
    {
        return vrAnimation(vm["vr-animation-cubemap"].as<std::string>(), true);
    }

    if (vm.count("vr-viewer"))
    {
        return vrViewer(vm["vr-viewer"].as<std::string>());
    }

    std::cout << "No valid action specified. Type \"" << argv[0] << " -h\" for help" << std::endl;

    return 1;
}
