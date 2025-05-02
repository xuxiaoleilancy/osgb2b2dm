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
    
    # 确保测试可执行文件存在
    if [ ! -f "tests/test_osgb2b3dm" ]; then
        echo -e "${RED}Test executable not found. Please build the project first.${NC}"
        return 1
    fi

    # 运行测试并捕获输出
    if [ "$VERBOSE" = true ]; then
        ctest --output-on-failure -j$NUM_JOBS -V
    else
        ctest -j$NUM_JOBS
    fi
    
    # 检查测试结果
    if [ $? -ne 0 ]; then
        echo -e "${RED}Some tests failed${NC}"
        return 1
    fi
    
    cd ..
    return 0
}

# 生成测试报告
generate_report() {
    echo -e "${YELLOW}Generating test report...${NC}"
    create_report_dir
    cd "$BUILD_DIR"
    
    # 确保测试日志目录存在
    mkdir -p "$REPORT_DIR"
    
    # 运行测试并生成详细日志
    ctest --output-on-failure -j$NUM_JOBS -V > "$REPORT_DIR/test_results.log" 2>&1
    
    # 使用Python生成HTML报告
    python3 << EOF
import os
import datetime
import re

# 读取测试日志
with open('$REPORT_DIR/test_results.log', 'r') as f:
    log_content = f.read()

# 解析测试结果
test_results = []
current_test = None
current_output = []
test_pattern = re.compile(r'Test #\d+: (.*?) \.\.\.')

# 首先尝试解析CTest输出
for line in log_content.split('\n'):
    # 检查是否是新测试的开始
    test_match = test_pattern.match(line)
    if test_match:
        if current_test:
            # 保存前一个测试的结果
            test_results.append({
                'name': current_test,
                'status': 'passed' if any('Passed' in l for l in current_output) else 'failed',
                'output': '\n'.join(current_output)
            })
        current_test = test_match.group(1).strip()
        current_output = [line]
    elif current_test:
        current_output.append(line)

# 添加最后一个测试
if current_test:
    test_results.append({
        'name': current_test,
        'status': 'passed' if any('Passed' in l for l in current_output) else 'failed',
        'output': '\n'.join(current_output)
    })

# 如果没有找到测试结果，尝试从Google Test输出中提取
if not test_results:
    # 尝试从Google Test输出中提取测试信息
    test_lines = []
    in_test = False
    current_test = None
    current_output = []
    
    for line in log_content.split('\n'):
        if '[ RUN      ]' in line:
            in_test = True
            current_test = line.split('[ RUN      ]')[1].strip()
            current_output = [line]
        elif '[       OK ]' in line and in_test:
            test_results.append({
                'name': current_test,
                'status': 'passed',
                'output': '\n'.join(current_output + [line])
            })
            in_test = False
        elif '[  FAILED  ]' in line and in_test:
            test_results.append({
                'name': current_test,
                'status': 'failed',
                'output': '\n'.join(current_output + [line])
            })
            in_test = False
        elif in_test:
            current_output.append(line)

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
        pre {{ white-space: pre-wrap; word-wrap: break-word; }}
        .no-tests {{ color: #856404; background-color: #fff3cd; padding: 10px; }}
        .error {{ color: #721c24; background-color: #f8d7da; padding: 10px; }}
    </style>
</head>
<body>
    <h1>Test Report</h1>
    <div class="summary">
        <p>Generated on: {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}</p>
        <p>Total tests: {len(test_results)}</p>
        <p>Passed: {sum(1 for t in test_results if t['status'] == 'passed')}</p>
        <p>Failed: {sum(1 for t in test_results if t['status'] == 'failed')}</p>
    </div>
'''

if not test_results:
    html += '''
    <div class="no-tests">
        <h3>No tests found in the log file</h3>
        <p>This could be because:</p>
        <ul>
            <li>No tests were run</li>
            <li>The test output format has changed</li>
            <li>There was an error running the tests</li>
        </ul>
        <p>Please check the test log file for more information.</p>
    </div>
    <div class="error">
        <h3>Test Log Content:</h3>
        <pre>''' + log_content + '''</pre>
    </div>
'''
else:
    for test in test_results:
        html += f'''
        <div class="test {'passed' if test['status'] == 'passed' else 'failed'}">
            <h3>{test['name']}</h3>
            <p>Status: {test['status']}</p>
            <pre>{test['output']}</pre>
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
        if ! run_tests; then
            echo -e "${RED}Test execution failed${NC}"
            exit 1
        fi
    fi

    # 生成报告
    if [ "$GENERATE_REPORT" = true ]; then
        generate_report
    fi

    echo -e "${GREEN}Build completed successfully${NC}"
}

# 执行主函数
main "$@" 