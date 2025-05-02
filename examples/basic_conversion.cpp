#include "osgb2b3dm.h"
#include <iostream>
#include <string>

/**
 * 基本的OSGB到B3DM转换示例
 * 展示了最基本的转换功能
 */
int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "用法: " << argv[0] << " <input.osgb> <output.b3dm>" << std::endl;
        return 1;
    }

    std::string inputPath = argv[1];
    std::string outputPath = argv[2];

    // 创建转换器实例
    Osgb2B3dm converter;

    // 执行转换
    std::cout << "开始转换..." << std::endl;
    std::cout << "输入文件: " << inputPath << std::endl;
    std::cout << "输出文件: " << outputPath << std::endl;

    bool success = converter.convert(inputPath, outputPath);

    if (success) {
        std::cout << "转换成功!" << std::endl;
        return 0;
    } else {
        std::cerr << "转换失败!" << std::endl;
        return 1;
    }
} 