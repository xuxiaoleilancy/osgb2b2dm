#include "osgb2b3dm.h"
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <thread>
#include <future>

namespace fs = std::filesystem;

/**
 * 批量转换示例
 * 展示了如何并行处理多个OSGB文件的转换
 */

// 转换单个文件的函数
bool convertFile(const fs::path& input, const fs::path& output) {
    try {
        Osgb2B3dm converter;
        return converter.convert(input.string(), output.string());
    } catch (const std::exception& e) {
        std::cerr << "转换文件 " << input << " 时发生错误: " << e.what() << std::endl;
        return false;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "用法: " << argv[0] << " <input_directory> <output_directory>" << std::endl;
        return 1;
    }

    fs::path inputDir = argv[1];
    fs::path outputDir = argv[2];

    // 检查输入目录是否存在
    if (!fs::exists(inputDir)) {
        std::cerr << "输入目录不存在: " << inputDir << std::endl;
        return 1;
    }

    // 创建输出目录（如果不存在）
    fs::create_directories(outputDir);

    // 获取可用的CPU核心数
    unsigned int numThreads = std::thread::hardware_concurrency();
    std::cout << "使用 " << numThreads << " 个线程进行并行转换" << std::endl;

    // 存储所有转换任务
    std::vector<std::future<bool>> tasks;
    int fileCount = 0;

    // 遍历输入目录
    for (const auto& entry : fs::recursive_directory_iterator(inputDir)) {
        if (entry.path().extension() == ".osgb") {
            // 构建输出文件路径
            fs::path relativePath = fs::relative(entry.path(), inputDir);
            fs::path outputPath = outputDir / relativePath;
            outputPath.replace_extension(".b3dm");

            // 创建必要的子目录
            fs::create_directories(outputPath.parent_path());

            // 启动异步转换任务
            tasks.push_back(std::async(std::launch::async, 
                convertFile, entry.path(), outputPath));
            
            fileCount++;
            std::cout << "添加转换任务: " << entry.path() << " -> " << outputPath << std::endl;
        }
    }

    // 等待所有任务完成并收集结果
    int successCount = 0;
    for (auto& task : tasks) {
        if (task.get()) {
            successCount++;
        }
    }

    // 输出统计信息
    std::cout << "\n转换完成!" << std::endl;
    std::cout << "总文件数: " << fileCount << std::endl;
    std::cout << "成功转换: " << successCount << std::endl;
    std::cout << "失败转换: " << (fileCount - successCount) << std::endl;

    return (successCount == fileCount) ? 0 : 1;
} 