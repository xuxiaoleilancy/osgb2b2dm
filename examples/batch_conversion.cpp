#include "osgb2b3dm.h"
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <filesystem>

using namespace osgb2b3dm;
namespace fs = std::filesystem;

/**
 * 批量转换示例
 * 展示了如何并行处理多个OSGB文件的转换
 */

// 线程安全的输出
std::mutex cout_mutex;
void safe_cout(const std::string& msg) {
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cout << msg << std::endl;
}

// 转换单个文件
void convert_file(const fs::path& input, const fs::path& output) {
    Osgb2B3dm converter;
    converter.setVerbose(false);

    safe_cout("Converting: " + input.string());
    
    if (!converter.convert(input.string(), output.string())) {
        safe_cout("Error: " + converter.lastErrorMessage());
    } else {
        safe_cout("Success: " + input.filename().string());
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_dir> <output_dir>" << std::endl;
        return 1;
    }

    fs::path input_dir(argv[1]);
    fs::path output_dir(argv[2]);

    if (!fs::exists(input_dir) || !fs::is_directory(input_dir)) {
        std::cerr << "Error: Input directory does not exist or is not a directory" << std::endl;
        return 1;
    }

    if (!fs::exists(output_dir)) {
        fs::create_directories(output_dir);
    }

    std::vector<std::thread> threads;
    const size_t max_threads = std::thread::hardware_concurrency();

    for (const auto& entry : fs::directory_iterator(input_dir)) {
        if (entry.path().extension() == ".osgb") {
            fs::path output_path = output_dir / entry.path().filename().replace_extension(".b3dm");
            
            // 等待线程数量低于最大值
            while (threads.size() >= max_threads) {
                for (auto it = threads.begin(); it != threads.end();) {
                    if (it->joinable()) {
                        it->join();
                        it = threads.erase(it);
                    } else {
                        ++it;
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            threads.emplace_back(convert_file, entry.path(), output_path);
        }
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    std::cout << "All conversions completed!" << std::endl;
    return 0;
} 