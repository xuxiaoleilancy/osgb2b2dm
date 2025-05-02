#include "osgb2b3dm.h"
#include <iostream>
#include <string>
#include <osg/Node>
#include <osgDB/ReadFile>
#include <osgAnimation/BasicAnimationManager>
#include <osgAnimation/Animation>

/**
 * 动画处理示例
 * 展示了如何处理带有动画的OSGB模型
 */

// 打印动画信息的辅助函数
void printAnimationInfo(osg::Node* node) {
    if (!node) return;

    // 获取动画管理器
    osgAnimation::BasicAnimationManager* animManager = 
        dynamic_cast<osgAnimation::BasicAnimationManager*>(
            node->getUpdateCallback());

    if (animManager) {
        const osgAnimation::AnimationList& animations = 
            animManager->getAnimationList();

        std::cout << "\n找到 " << animations.size() << " 个动画:" << std::endl;
        
        // 遍历所有动画
        for (const auto& anim : animations) {
            std::cout << "\n动画名称: " << anim->getName() << std::endl;
            std::cout << "持续时间: " << anim->getDuration() << " 秒" << std::endl;
            
            // 获取动画通道
            const osgAnimation::ChannelList& channels = anim->getChannels();
            std::cout << "通道数量: " << channels.size() << std::endl;
            
            // 打印每个通道的信息
            for (const auto& channel : channels) {
                std::cout << "- 通道: " << channel->getName() 
                         << " (目标: " << channel->getTargetName() << ")" 
                         << std::endl;
            }
        }
    } else {
        std::cout << "未找到动画管理器" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "用法: " << argv[0] << " <input.osgb> <output.b3dm>" << std::endl;
        return 1;
    }

    std::string inputPath = argv[1];
    std::string outputPath = argv[2];

    // 首先读取OSGB文件以检查动画
    std::cout << "读取OSGB文件: " << inputPath << std::endl;
    osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(inputPath);
    
    if (!node) {
        std::cerr << "无法读取OSGB文件" << std::endl;
        return 1;
    }

    // 打印动画信息
    printAnimationInfo(node.get());

    // 创建转换器实例
    Osgb2B3dm converter;

    // 执行转换
    std::cout << "\n开始转换..." << std::endl;
    bool success = converter.convert(inputPath, outputPath);

    if (success) {
        std::cout << "转换成功!" << std::endl;
        std::cout << "注意: B3DM文件中的动画可以在Cesium中使用 ModelAnimationCollection 播放" << std::endl;
        return 0;
    } else {
        std::cerr << "转换失败!" << std::endl;
        return 1;
    }
} 