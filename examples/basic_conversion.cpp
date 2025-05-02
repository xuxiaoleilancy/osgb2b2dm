#include "osgb2b3dm.h"
#include <iostream>
#include <string>

using namespace osgb2b3dm;

/**
 * 基本的OSGB到B3DM转换示例
 * 展示了最基本的转换功能
 */
int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input.osgb> <output.b3dm>" << std::endl;
        return 1;
    }

    // 创建转换器实例
    Osgb2B3dm converter;

    // 设置选项
    converter.setVerbose(true);
    converter.setValidateInput(true);
    converter.setValidateOutput(true);

    // 执行转换
    if (!converter.convert(argv[1], argv[2])) {
        std::cerr << "Error: " << converter.lastErrorMessage() << std::endl;
        return 1;
    }

    std::cout << "Conversion successful!" << std::endl;
    return 0;
} 