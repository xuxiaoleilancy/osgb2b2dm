# 设置CMake最低版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(osgb2b3dm_examples)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖
find_package(OpenSceneGraph REQUIRED COMPONENTS
    osg
    osgDB
    osgAnimation
    osgUtil
)
find_package(nlohmann_json REQUIRED)

# 添加示例可执行文件
add_executable(basic_conversion basic_conversion.cpp)
add_executable(batch_conversion batch_conversion.cpp)
add_executable(animation_example animation_example.cpp)
add_executable(instancing_example instancing_example.cpp)

# 设置包含目录
foreach(target basic_conversion batch_conversion animation_example instancing_example)
    target_include_directories(${target} PRIVATE
        ${CMAKE_SOURCE_DIR}/../include
        ${OPENSCENEGRAPH_INCLUDE_DIRS}
    )
endforeach()

# 链接库
foreach(target basic_conversion batch_conversion animation_example instancing_example)
    target_link_libraries(${target} PRIVATE
        osgb2b3dm
        ${OPENSCENEGRAPH_LIBRARIES}
        nlohmann_json::nlohmann_json
        z
    )
endforeach()

# 添加编译选项
foreach(target basic_conversion batch_conversion animation_example instancing_example)
    target_compile_definitions(${target} PRIVATE
        OSG_ANIMATION_SUPPORT=1
    )
endforeach() 