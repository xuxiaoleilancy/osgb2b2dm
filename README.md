# OSGB 转 B3DM 转换器

一个用于将 OSGB 格式的 3D 模型转换为 B3DM 格式的工具。B3DM 是 3D Tiles 规范中的一种瓦片格式，适用于大规模 3D 场景的流式加载。

## 功能特点

- 支持 OSGB 到 B3DM 的转换
- 保留模型的几何数据（顶点、法线、纹理坐标）
- 支持材质和纹理信息
- 正确处理变换矩阵
- 生成符合 glTF 2.0 规范的 JSON
- 优化二进制数据打包

## 依赖项

- OpenSceneGraph (OSG)
- nlohmann/json
- zlib

## 编译和安装

1. 确保已安装所有依赖项：
```bash
# macOS (使用 Homebrew)
brew install open-scene-graph nlohmann-json

# Ubuntu/Debian
sudo apt-get install libopenscenegraph-dev nlohmann-json3-dev zlib1g-dev
```

2. 克隆仓库并编译：
```bash
git clone https://github.com/yourusername/osgb2b3dm.git
cd osgb2b3dm
mkdir build && cd build
cmake ..
make
```

## 使用方法

```bash
./osgb2b3dm input.osgb output.b3dm
```

## 实现的功能

- [x] 基本几何数据提取
- [x] 材质和纹理处理
- [x] 变换矩阵应用
- [x] glTF JSON 生成
- [x] 二进制数据打包
- [x] B3DM 文件写入

## TODO 列表

### 功能改进
- [ ] 实现完整的材质属性支持
- [ ] 添加纹理压缩和优化
- [ ] 支持动画数据
- [ ] 添加批处理功能
- [ ] 实现命令行参数解析

### 代码优化
- [ ] 添加并行处理支持
- [ ] 优化内存使用
- [ ] 改进错误处理机制
- [ ] 添加单元测试
- [ ] 实现日志系统

### 文档完善
- [ ] 添加详细的 API 文档
- [ ] 编写使用示例
- [ ] 添加性能测试报告
- [ ] 完善错误代码说明
- [ ] 添加贡献指南

## 贡献

欢迎提交 Issue 和 Pull Request 来帮助改进这个项目。

## 许可证

MIT License 