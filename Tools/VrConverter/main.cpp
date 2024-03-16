#include <filesystem>
#include <iostream>

#include <SDL.h>
#include <boost/program_options.hpp>

#include "fvr_files/fvr_vr.h"

#include "fnxvr.h"

int vrViewer(const std::string& vrFileName)
{
    FnxVr fnxVr;
    if (!fnxVr.isValid()) {
        std::cerr << "Failed to initialise FnxVr" << std::endl;
        return 1;
    }

    if (!fnxVr.loadFile(vrFileName)) {
        std::cerr << "Failed to load VR file" << std::endl;
        return 2;
    }

    // Get VR base name
    std::string baseFileName = vrFileName.substr(0, vrFileName.length() - 3);
    // Check if equivalent TST file exists
    std::string tstFileName = baseFileName + ".tst";
    std::cout << "Checking for TST file: " << tstFileName << std::endl;
    if (std::filesystem::exists(tstFileName)) {
        std::cout << "Loading corresponding TST file" << std::endl;
        if (!fnxVr.loadTstFile(tstFileName)) {
            std::cerr << "Failed to load TST file" << std::endl;
            return 3;
        }
    }

    if (!fnxVr.loop()) {
        std::cerr << "Failed to execute viewer" << std::endl;
        return 3;
    }

    return 0;
}

int vrConvert(const std::string& vrFileName, const bool toCubemap)
{
    std::cout << "Converting " << vrFileName << " to PNG" << std::endl;

    FvrVr vr;
    if (!vr.open(vrFileName)) {
        std::cerr << "Error loading VR file" << std::endl;
        return 1;
    }

    if (!vr.isOpen()) {
        std::cerr << "VR file is not open" << std::endl;
        return 2;
    }

    Image image;
    bool ret;
    if (toCubemap && vr.getType() == FvrVr::Type::VR_STATIC_VR) {
        ret = vr.getImageCubemap(image);
    } else {
        ret = vr.getImage(image);
    }

    if (!ret || !image.isValid()) {
        std::cerr << "VR image is not valid" << std::endl;
        return 3;
    }

    std::string baseFileName = vrFileName.substr(0, vrFileName.length() - 3);
    if (!image.savePng(baseFileName + ".png")) {
        std::cerr << "Error saving to image" << std::endl;
        return 5;
    }

    return 0;
}

int vrAnimation(const std::string& vrFileName, const bool toCubemap)
{
    std::cout << "Exporting " << vrFileName << " animations" << std::endl;

    // TODO: Export animations

    return -1;
}

int main(int argc, char* argv[])
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

    if (vm.count("help")) {
        std::cout << optGeneric << std::endl;
        return 0;
    }

    if (vm.count("vr-convert")) {
        return vrConvert(vm["vr-convert"].as<std::string>(), false);
    }

    if (vm.count("vr-convert-cubemap")) {
        return vrConvert(vm["vr-convert-cubemap"].as<std::string>(), true);
    }

    if (vm.count("vr-animation")) {
        return vrAnimation(vm["vr-animation"].as<std::string>(), false);
    }

    if (vm.count("vr-animation-cubemap")) {
        return vrAnimation(vm["vr-animation-cubemap"].as<std::string>(), true);
    }

    if (vm.count("vr-viewer")) {
        return vrViewer(vm["vr-viewer"].as<std::string>());
    }

    std::cerr << "No valid action specified. Type \"" << argv[0] << " -h\" for help" << std::endl;

    return 1;
}
