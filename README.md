# OSGB 转 B3DM 转换器

一个用于将 OSGB (OpenSceneGraph Binary) 格式转换为 B3DM (Batched 3D Model) 格式的工具。B3DM 是 Cesium 3D Tiles 规范中的一种瓦片格式，用于高效地传输和渲染大规模 3D 模型。

## 功能特点

- 支持 OSGB 到 B3DM 的完整转换
- 保留模型的几何结构、材质和纹理
- 支持 PBR (Physically Based Rendering) 材质
- 支持多种纹理类型：
  - 基础颜色纹理
  - 法线贴图
  - 金属度/粗糙度纹理
  - 环境光遮蔽纹理
  - 自发光纹理
- 支持材质属性：
  - 环境光、漫反射、镜面反射、自发光颜色
  - 金属度/粗糙度
  - 透明度控制
  - 双面渲染
- 支持纹理变换：
  - 缩放
  - 偏移
  - 旋转
- 支持节点层次结构和变换矩阵
- 支持动画功能：
  - 骨骼动画和蒙皮
  - 变形目标动画
  - 动画混合和过渡
  - 动画事件系统
  - 自定义用户数据
  - 动画关键帧插值

## 依赖项

- OpenSceneGraph (OSG) - 用于读取和处理 OSGB 文件
  - 需要启用动画支持
  - 需要支持 osgAnimation
- nlohmann/json - 用于生成 glTF JSON
- zlib - 用于数据压缩

## 安装

### 依赖安装

```bash
# macOS
brew install openscenegraph nlohmann-json zlib

# Ubuntu/Debian
sudo apt-get install libopenscenegraph-dev nlohmann-json3-dev zlib1g-dev
```

### 编译

```bash
mkdir build
cd build
cmake ..
make
```

## 使用方法

```bash
./osgb2b3dm input.osgb output.b3dm
```

### 参数说明

- `input.osgb` - 输入的 OSGB 文件路径
- `output.b3dm` - 输出的 B3DM 文件路径

## 实现细节

### 材质处理

- 从 OSG 的 `StateSet` 中提取材质属性
- 支持 PBR 材质工作流
- 自动计算金属度和粗糙度
- 支持透明度和混合模式

### 纹理处理

- 支持多种纹理类型
- 保留纹理变换信息
- 自动处理纹理坐标

### 几何处理

- 保留顶点、法线和纹理坐标
- 支持三角形索引
- 应用节点变换矩阵
- 计算正确的边界框
- 支持实例化渲染

### 实例化处理

- 支持实例化组
  - 提取实例组变换矩阵
  - 支持实例组层级结构
  - 支持实例组用户数据
- 支持单个实例
  - 提取实例变换矩阵
  - 支持实例材质
  - 支持实例用户数据
- 支持的glTF扩展
  - EXT_mesh_gpu_instancing

### 动画处理

- 骨骼动画系统
  - 支持骨骼层级结构
  - 支持蒙皮变形
  - 支持骨骼权重
  - 支持逆向绑定矩阵
- 变形目标动画
  - 提取变形目标几何体
  - 保留权重信息
  - 支持多目标混合
- 动画混合系统
  - 支持多动画混合
  - 支持动画过渡
  - 支持权重控制
  - 支持叠加和覆盖模式
- 动画事件系统
  - 支持自定义事件数据
  - 事件时间轴集成
  - 与glTF动画系统兼容
- 支持的glTF扩展
  - KHR_materials_pbrSpecularGlossiness
  - KHR_texture_transform
  - KHR_animation_events
  - KHR_animation_mixer

## TODO

### 功能改进
- [x] 实现完整的材质属性支持
  - [x] 环境光/漫反射/镜面反射/自发光颜色
  - [x] 金属度/粗糙度
  - [x] 透明度控制
  - [x] 双面渲染
- [x] 实现纹理支持
  - [x] 基础颜色纹理
  - [x] 法线贴图
  - [x] 金属度/粗糙度纹理
  - [x] 环境光遮蔽纹理
  - [x] 自发光纹理
- [x] 实现纹理变换
  - [x] 缩放
  - [x] 偏移
  - [x] 旋转
- [x] 实现动画支持
  - [x] 骨骼动画
  - [x] 变形动画
  - [x] 动画混合
  - [x] 动画事件
- [x] 实现实例化支持
  - [x] 实例化几何体
  - [x] 实例化材质
  - [x] 实例化变换

### 代码优化
- [ ] 优化内存使用
  - [ ] 使用内存池
  - [ ] 优化数据结构
  - [ ] 减少临时对象
- [ ] 优化性能
  - [ ] 多线程处理
  - [ ] SIMD 优化
  - [ ] 缓存优化
- [ ] 优化错误处理
  - [ ] 详细的错误信息
  - [ ] 错误恢复机制
  - [ ] 日志系统

### 文档完善
- [x] 添加详细的使用文档
- [ ] 添加 API 文档
- [ ] 添加示例代码
- [ ] 添加测试用例

## 已知问题

- 大文件转换可能需要较大内存
- 某些复杂的动画可能不完全支持
- 需要正确配置纹理路径
- 动画事件数据需要符合JSON格式

## 许可证

MIT License

## 贡献

欢迎提交 Issue 和 Pull Request 来改进这个项目。如果您发现任何问题或有改进建议，请随时提出。
