#!/bin/bash

# 设置颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 设置变量
BUILD_DIR="build"
REPORT_DIR="reports"
CMAKE_OPTIONS="-DCMAKE_BUILD_TYPE=Release"
NUM_JOBS=$(nproc)

# 打印帮助信息
print_help() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  -h, --help          Show this help message"
    echo "  -c, --clean         Clean build directory"
    echo "  -r, --rebuild       Clean and rebuild"
    echo "  -t, --test          Run tests"
    echo "  -p, --report        Generate test report"
    echo "  -j, --jobs NUM      Set number of parallel jobs (default: nproc)"
    echo "  -d, --debug         Build in debug mode"
    echo "  -v, --verbose       Enable verbose output"
}

# 检查命令是否存在
check_command() {
    if ! command -v $1 &> /dev/null; then
        echo -e "${RED}Error: $1 is not installed${NC}"
        exit 1
    fi
}

# 检查依赖
check_dependencies() {
    echo -e "${YELLOW}Checking dependencies...${NC}"
    check_command cmake
    check_command make
    check_command g++
    check_command python3
}

# 创建报告目录
create_report_dir() {
    if [ ! -d "$REPORT_DIR" ]; then
        mkdir -p "$REPORT_DIR"
    fi
}

# 清理构建目录
clean_build() {
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
    fi
}

# 配置CMake
configure_cmake() {
    echo -e "${YELLOW}Configuring CMake...${NC}"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    cmake $CMAKE_OPTIONS ..
    cd ..
}

# 编译项目
build_project() {
    echo -e "${YELLOW}Building project...${NC}"
    cd "$BUILD_DIR"
    make -j$NUM_JOBS
    if [ $? -ne 0 ]; then
        echo -e "${RED}Build failed${NC}"
        exit 1
    fi
    cd ..
}

# 运行测试
run_tests() {
    echo -e "${YELLOW}Running tests...${NC}"
    cd "$BUILD_DIR"
    if [ "$VERBOSE" = true ]; then
        ctest --output-on-failure -j$NUM_JOBS
    else
        ctest -j$NUM_JOBS
    fi
    cd ..
}

# 生成测试报告
generate_report() {
    echo -e "${YELLOW}Generating test report...${NC}"
    create_report_dir
    cd "$BUILD_DIR"
    
    # 生成XML格式的测试报告
    ctest --output-on-failure -j$NUM_JOBS --output-log "$REPORT_DIR/test_results.xml"
    
    # 使用Python生成HTML报告
    python3 << EOF
import xml.etree.ElementTree as ET
import os
import datetime

# 解析XML测试结果
tree = ET.parse('$REPORT_DIR/test_results.xml')
root = tree.getroot()

# 生成HTML报告
html = f'''<!DOCTYPE html>
<html>
<head>
    <title>Test Report</title>
    <style>
        body {{ font-family: Arial, sans-serif; margin: 20px; }}
        .test {{ margin: 10px 0; padding: 10px; border: 1px solid #ccc; }}
        .passed {{ background-color: #d4edda; }}
        .failed {{ background-color: #f8d7da; }}
        .summary {{ margin: 20px 0; padding: 10px; background-color: #f8f9fa; }}
    </style>
</head>
<body>
    <h1>Test Report</h1>
    <div class="summary">
        <p>Generated on: {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}</p>
        <p>Total tests: {len(root.findall('.//test'))}</p>
        <p>Passed: {len(root.findall('.//test[@status="passed"]'))}</p>
        <p>Failed: {len(root.findall('.//test[@status="failed"]'))}</p>
    </div>
'''

for test in root.findall('.//test'):
    status = test.get('status')
    name = test.get('name')
    output = test.findtext('output', '')
    
    html += f'''
    <div class="test {'passed' if status == 'passed' else 'failed'}">
        <h3>{name}</h3>
        <p>Status: {status}</p>
        <pre>{output}</pre>
    </div>
'''

html += '''
</body>
</html>
'''

with open('$REPORT_DIR/test_report.html', 'w') as f:
    f.write(html)
EOF

    echo -e "${GREEN}Test report generated: $REPORT_DIR/test_report.html${NC}"
    cd ..
}

# 主函数
main() {
    # 检查依赖
    check_dependencies

    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                print_help
                exit 0
                ;;
            -c|--clean)
                CLEAN=true
                shift
                ;;
            -r|--rebuild)
                REBUILD=true
                shift
                ;;
            -t|--test)
                RUN_TESTS=true
                shift
                ;;
            -p|--report)
                GENERATE_REPORT=true
                shift
                ;;
            -j|--jobs)
                NUM_JOBS=$2
                shift 2
                ;;
            -d|--debug)
                CMAKE_OPTIONS="-DCMAKE_BUILD_TYPE=Debug"
                shift
                ;;
            -v|--verbose)
                VERBOSE=true
                shift
                ;;
            *)
                echo -e "${RED}Unknown option: $1${NC}"
                print_help
                exit 1
                ;;
        esac
    done

    # 执行清理
    if [ "$CLEAN" = true ] || [ "$REBUILD" = true ]; then
        clean_build
    fi

    # 配置和构建
    if [ ! -d "$BUILD_DIR" ] || [ "$REBUILD" = true ]; then
        configure_cmake
    fi
    build_project

    # 运行测试
    if [ "$RUN_TESTS" = true ] || [ "$GENERATE_REPORT" = true ]; then
        run_tests
    fi

    # 生成报告
    if [ "$GENERATE_REPORT" = true ]; then
        generate_report
    fi

    echo -e "${GREEN}Build completed successfully${NC}"
}

# 执行主函数
main "$@" 