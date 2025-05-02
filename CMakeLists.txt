cmake_minimum_required(VERSION 3.10)
project(osgb2b3dm)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖
find_package(OpenSceneGraph REQUIRED)
find_package(nlohmann_json REQUIRED)

# 添加可执行文件
add_executable(osgb2b3dm
    src/main.cpp
    src/osgb2b3dm.cpp
)

# 包含目录
target_include_directories(osgb2b3dm PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OPENSCENEGRAPH_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(osgb2b3dm PRIVATE
    /opt/homebrew/Cellar/open-scene-graph/3.6.5_2/lib/libosgDB.dylib
    /opt/homebrew/Cellar/open-scene-graph/3.6.5_2/lib/libosg.dylib
    /opt/homebrew/Cellar/open-scene-graph/3.6.5_2/lib/libOpenThreads.dylib
    nlohmann_json::nlohmann_json
    z
)