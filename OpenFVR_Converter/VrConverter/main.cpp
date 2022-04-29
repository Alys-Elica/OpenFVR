#include <QDir>

#include "vrfile.h"

int main(int argc, char *argv[])
{
    if (argc == 2) {
        VrFile vr(argv[1]);
    }

    return 0;
}
