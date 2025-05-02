# OSGB2B3DM API 文档

## 主要类

### Osgb2B3dm

主要的转换类，负责将OSGB格式转换为B3DM格式。

#### 构造函数

```cpp
Osgb2B3dm();
```

创建一个新的转换器实例。

#### 公共方法

##### convert

```cpp
bool convert(const std::string& inputPath, const std::string& outputPath);
```

将OSGB文件转换为B3DM文件。

**参数:**
- `inputPath`: 输入OSGB文件的路径
- `outputPath`: 输出B3DM文件的路径

**返回值:**
- `true`: 转换成功
- `false`: 转换失败

## 数据结构

### Material

```cpp
struct Material {
    std::vector<float> ambient;       // 环境光颜色 (RGBA)
    std::vector<float> diffuse;       // 漫反射颜色 (RGBA)
    std::vector<float> specular;      // 镜面反射颜色 (RGBA)
    std::vector<float> emission;      // 自发光颜色 (RGBA)
    float metallic;                   // 金属度 [0,1]
    float roughness;                  // 粗糙度 [0,1]
    float occlusion;                  // 环境光遮蔽 [0,1]
    float emissiveIntensity;         // 自发光强度
    float opacity;                    // 透明度 [0,1]
    std::string blendMode;           // 混合模式 ("OPAQUE", "MASK", "BLEND")
    bool doubleSided;                // 是否双面渲染
    TextureInfo baseColorTexture;    // 基础颜色纹理
    TextureInfo normalTexture;       // 法线贴图
    TextureInfo metallicRoughnessTexture; // 金属度/粗糙度纹理
    TextureInfo occlusionTexture;    // 环境光遮蔽纹理
    TextureInfo emissiveTexture;     // 自发光纹理
    float normalScale;               // 法线贴图强度
    std::string name;                // 材质名称
};
```

### Animation

```cpp
struct Animation {
    std::string name;                // 动画名称
    double duration;                 // 动画持续时间
    std::vector<std::string> channels;  // 动画通道列表
    std::vector<std::string> samplers;  // 动画采样器列表
};
```

### Instance

```cpp
struct Instance {
    std::string name;               // 实例名称
    osg::Matrix transform;          // 变换矩阵
    int meshIndex;                  // 网格索引
    int materialIndex;              // 材质索引
    std::vector<int> children;      // 子节点列表
    std::vector<std::string> userData;  // 用户数据
};
```

### InstanceGroup

```cpp
struct InstanceGroup {
    std::string name;               // 实例组名称
    std::vector<Instance> instances;  // 实例列表
    osg::Matrix baseTransform;      // 基础变换矩阵
    std::vector<std::string> userData;  // 用户数据
};
```

### Joint

```cpp
struct Joint {
    std::string name;               // 关节名称
    int parentIndex;                // 父关节索引
    osg::Matrix inverseBindMatrix;  // 逆绑定矩阵
    osg::Matrix localMatrix;        // 局部变换矩阵
    std::vector<int> children;      // 子关节列表
    osgAnimation::Bone* bone;       // OSG骨骼节点
};
```

### Skeleton

```cpp
struct Skeleton {
    std::string name;               // 骨骼名称
    std::vector<Joint> joints;      // 关节列表
    int rootJoint;                  // 根关节索引
    std::vector<osg::Matrix> inverseBindMatrices;  // 逆绑定矩阵列表
    osgAnimation::Skeleton* osgSkeleton;  // OSG骨骼
};
```

## 使用示例

### 基本转换

```cpp
#include "osgb2b3dm.h"

int main() {
    Osgb2B3dm converter;
    bool success = converter.convert("input.osgb", "output.b3dm");
    return success ? 0 : 1;
}
```

### 错误处理

```cpp
#include "osgb2b3dm.h"
#include <iostream>

int main() {
    Osgb2B3dm converter;
    if (!converter.convert("input.osgb", "output.b3dm")) {
        std::cerr << "转换失败" << std::endl;
        return 1;
    }
    return 0;
}
```

## 注意事项

1. 内存使用
   - 转换大文件时需要足够的内存
   - 建议对大文件进行分块处理

2. 纹理处理
   - 确保纹理文件路径正确
   - 支持相对路径和绝对路径
   - 纹理文件需要可访问

3. 动画处理
   - 支持多种动画类型
   - 动画数据需要符合规范
   - 注意动画时间轴的同步

4. 实例化处理
   - 实例化数据需要正确组织
   - 注意实例化组的层级关系
   - 确保变换矩阵正确

## 扩展支持

支持以下glTF扩展：

1. `KHR_materials_pbrSpecularGlossiness`
   - 用于PBR材质
   - 支持镜面反射工作流

2. `KHR_texture_transform`
   - 支持纹理变换
   - 包括缩放、偏移和旋转

3. `KHR_animation_events`
   - 支持动画事件系统
   - 可添加自定义事件数据

4. `KHR_animation_mixer`
   - 支持动画混合
   - 控制多个动画的权重

5. `EXT_mesh_gpu_instancing`
   - 支持GPU实例化
   - 优化渲染性能 