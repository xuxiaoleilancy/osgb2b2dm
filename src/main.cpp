#include "osgb2b3dm.h"
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <input.osgb> <output.b3dm>" << std::endl;
        return 1;
    }

    Osgb2B3dm converter;
    if (converter.convert(argv[1], argv[2])) {
        std::cout << "Conversion successful!" << std::endl;
        return 0;
    } else {
        std::cout << "Conversion failed!" << std::endl;
        return 1;
    }
}