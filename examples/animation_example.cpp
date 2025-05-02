#include "osgb2b3dm.h"
#include <iostream>
#include <string>
#include <filesystem>

using namespace osgb2b3dm;
namespace fs = std::filesystem;

/**
 * 动画处理示例
 * 展示了如何处理带有动画的OSGB模型
 */

// 打印动画信息的辅助函数
void printAnimationInfo(const std::string& path) {
    Osgb2B3dm converter;
    converter.setVerbose(true);

    std::cout << "Analyzing animations in: " << path << std::endl;
    std::cout << "-------------------------" << std::endl;

    // 创建临时输出路径
    fs::path temp_output = fs::temp_directory_path() / "temp.b3dm";
    
    if (!converter.convert(path, temp_output.string())) {
        std::cerr << "Error: " << converter.lastErrorMessage() << std::endl;
        return;
    }

    // 删除临时文件
    fs::remove(temp_output);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.osgb>" << std::endl;
        return 1;
    }

    printAnimationInfo(argv[1]);
    return 0;
} 