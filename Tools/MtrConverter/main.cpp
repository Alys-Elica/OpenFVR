#include <fstream>
#include <iostream>

#include <fvr_files/fvr_mtr.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <mtr file> [mtr file] ..." << std::endl;
        return false;
    }

    for (int i = 1; i < argc; i++)
    {
        std::string mtrFileName = argv[i];

        FvrMtr mtr;
        if (!mtr.open(mtrFileName))
        {
            std::cerr << "Failed to open " << mtrFileName << std::endl;
            return false;
        }

        std::string pngFileName = mtrFileName + ".png";

        if (!mtr.image().savePng(pngFileName))
        {
            std::cerr << "Failed to save " << pngFileName << std::endl;
            return false;
        }
    }
}
