#include "osgb2b3dm.h"
#include <iostream>
#include <string>

using namespace osgb2b3dm;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input.osgb> <output.b3dm>" << std::endl;
        return 1;
    }

    Osgb2B3dm converter;
    converter.setVerbose(true);

    if (!converter.convert(argv[1], argv[2])) {
        std::cerr << "Error: " << converter.lastErrorMessage() << std::endl;
        return 1;
    }

    std::cout << "Conversion successful!" << std::endl;
    return 0;
}