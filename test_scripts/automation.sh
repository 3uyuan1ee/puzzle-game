#!/bin/bash

# Qt拼图游戏自动化构建和测试脚本
# 集成构建、测试、部署的完整流程

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR/.."

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m'

# 配置
BUILD_TYPES=("debug" "release")
PLATFORMS=("native")
TEST_TYPES=("functionality" "performance")
DEPLOY_TARGETS=("local" "archive")

# 结果追踪
BUILD_RESULTS=()
TEST_RESULTS=()
DEPLOY_RESULTS=()

# 日志文件
MAIN_LOG="$PROJECT_DIR/automation_$(date +%Y%m%d_%H%M%S).log"
SUMMARY_FILE="$PROJECT_DIR/automation_summary.txt"

# 初始化日志
init_logging() {
    echo "Qt Puzzle Game Automation Build & Test" > "$MAIN_LOG"
    echo "Started at: $(date)" >> "$MAIN_LOG"
    echo "========================================" >> "$MAIN_LOG"
}

# 日志函数
log() {
    local level=$1
    shift
    local message="$*"
    local timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    echo "[$timestamp] [$level] $message" | tee -a "$MAIN_LOG"
}

log_step() {
    echo -e "\n${BLUE}========================================${NC}"
    echo -e "${BLUE}  $1${NC}"
    echo -e "${BLUE}========================================${NC}"
    log "STEP" "$1"
}

# 环境检查
check_environment() {
    log_step "Environment Check"
    
    local issues=0
    
    # 检查Qt安装
    if ! command -v qmake >/dev/null 2>&1; then
        log "ERROR" "Qt qmake not found in PATH"
        issues=$((issues + 1))
    else
        local qt_version=$(qmake --version | head -1)
        log "INFO" "Qt found: $qt_version"
    fi
    
    # 检查编译器
    if ! command -v clang++ >/dev/null 2>&1 && ! command -v g++ >/dev/null 2>&1; then
        log "ERROR" "C++ compiler not found"
        issues=$((issues + 1))
    else
        if command -v clang++ >/dev/null 2>&1; then
            local clang_version=$(clang++ --version | head -1)
            log "INFO" "Compiler found: $clang_version"
        else
            local gcc_version=$(g++ --version | head -1)
            log "INFO" "Compiler found: $gcc_version"
        fi
    fi
    
    # 检查项目文件
    if [ ! -f "$PROJECT_DIR/PuzzleGame.pro" ]; then
        log "ERROR" "PuzzleGame.pro not found"
        issues=$((issues + 1))
    fi
    
    # 检查源代码目录
    if [ ! -d "$PROJECT_DIR/src" ]; then
        log "ERROR" "src directory not found"
        issues=$((issues + 1))
    fi
    
    # 检查资源文件
    if [ ! -d "$PROJECT_DIR/res" ]; then
        log "ERROR" "res directory not found"
        issues=$((issues + 1))
    fi
    
    # 检查必要的工具
    local required_tools=("make" "git" "find" "grep")
    for tool in "${required_tools[@]}"; do
        if ! command -v "$tool" >/dev/null 2>&1; then
            log "ERROR" "Required tool '$tool' not found"
            issues=$((issues + 1))
        fi
    done
    
    if [ $issues -gt 0 ]; then
        log "ERROR" "Environment check failed with $issues issues"
        return 1
    fi
    
    log "INFO" "Environment check passed"
    return 0
}

# 清理构建环境
clean_build_environment() {
    log_step "Cleaning Build Environment"
    
    # 清理项目目录
    cd "$PROJECT_DIR"
    
    # 清理构建文件
    make clean >/dev/null 2>&1 || true
    
    # 清理临时文件
    rm -rf temp/ build/ ../debug/ ../release/
    
    # 清理日志文件（保留最近的）
    find . -name "*.log" -mtime +7 -delete 2>/dev/null || true
    
    # 清理临时文件
    find . -name "*.o" -delete 2>/dev/null || true
    find . -name "moc_*" -delete 2>/dev/null || true
    find . -name "ui_*" -delete 2>/dev/null || true
    
    log "INFO" "Build environment cleaned"
}

# 构建函数
build_project() {
    local build_type=$1
    local platform=$2
    
    log_step "Building $build_type for $platform"
    
    cd "$PROJECT_DIR"
    
    # 配置构建参数
    local config_args=""
    local output_dir=""
    
    case $build_type in
        "debug")
            config_args="CONFIG+=debug"
            output_dir="../debug"
            ;;
        "release")
            config_args="CONFIG+=release"
            output_dir="../release"
            ;;
        *)
            log "ERROR" "Unknown build type: $build_type"
            return 1
            ;;
    esac
    
    # 创建输出目录
    mkdir -p "$output_dir"
    
    # 运行qmake
    log "INFO" "Running qmake with config: $config_args"
    if ! qmake $config_args PuzzleGame.pro; then
        log "ERROR" "qmake failed for $build_type build"
        BUILD_RESULTS+=("BUILD:$platform:$build_type:FAILED")
        return 1
    fi
    
    # 编译
    log "INFO" "Compiling with make"
    local make_start_time=$(date +%s)
    
    if ! make -j$(nproc); then
        log "ERROR" "Compilation failed for $build_type build"
        BUILD_RESULTS+=("BUILD:$platform:$build_type:FAILED")
        return 1
    fi
    
    local make_end_time=$(date +%s)
    local build_time=$((make_end_time - make_start_time))
    
    # 检查输出文件
    local success=0
    if [ -f "$output_dir/PuzzleGame.app/Contents/MacOS/PuzzleGame" ] || [ -f "$output_dir/PuzzleGame" ]; then
        log "INFO" "$build_type build successful in ${build_time}s"
        BUILD_RESULTS+=("BUILD:$platform:$build_type:SUCCESS:${build_time}s")
        success=1
    else
        log "ERROR" "Build output not found in $output_dir"
        BUILD_RESULTS+=("BUILD:$platform:$build_type:FAILED")
    fi
    
    return $success
}

# 运行功能测试
run_functionality_tests() {
    local build_type=$1
    local platform=$2
    
    log_step "Running Functionality Tests for $build_type ($platform)"
    
    local test_script="$PROJECT_DIR/test_scripts/functionality_test.sh"
    if [ ! -f "$test_script" ]; then
        log "WARNING" "Functionality test script not found: $test_script"
        TEST_RESULTS+=("TEST:functionality:$build_type:$platform:SKIPPED")
        return 0
    fi
    
    # 确保脚本可执行
    chmod +x "$test_script"
    
    # 运行测试
    local test_log="$PROJECT_DIR/test_functionality_${build_type}_${platform}_$(date +%Y%m%d_%H%M%S).log"
    
    log "INFO" "Running functionality tests (log: $test_log)"
    
    if timeout 300 "$test_script" > "$test_log" 2>&1; then
        local result=$(grep "Success Rate:" "$test_log" | tail -1 | grep -o '[0-9]*%')
        log "INFO" "Functionality tests passed with $result success rate"
        TEST_RESULTS+=("TEST:functionality:$build_type:$platform:SUCCESS:$result")
        return 0
    else
        local exit_code=$?
        if [ $exit_code -eq 124 ]; then
            log "ERROR" "Functionality tests timed out (5 minutes)"
            TEST_RESULTS+=("TEST:functionality:$build_type:$platform:TIMEOUT")
        else
            log "ERROR" "Functionality tests failed with exit code $exit_code"
            TEST_RESULTS+=("TEST:functionality:$build_type:$platform:FAILED")
        fi
        return 1
    fi
}

# 运行性能测试
run_performance_tests() {
    local build_type=$1
    local platform=$2
    
    log_step "Running Performance Tests for $build_type ($platform)"
    
    # 性能测试通常只在release版本上运行
    if [ "$build_type" != "release" ]; then
        log "INFO" "Skipping performance tests for $build_type build"
        TEST_RESULTS+=("TEST:performance:$build_type:$platform:SKIPPED")
        return 0
    fi
    
    local test_script="$PROJECT_DIR/test_scripts/performance_test.sh"
    if [ ! -f "$test_script" ]; then
        log "WARNING" "Performance test script not found: $test_script"
        TEST_RESULTS+=("TEST:performance:$build_type:$platform:SKIPPED")
        return 0
    fi
    
    # 确保脚本可执行
    chmod +x "$test_script"
    
    # 运行性能测试
    local test_log="$PROJECT_DIR/test_performance_${build_type}_${platform}_$(date +%Y%m%d_%H%M%S).log"
    
    log "INFO" "Running performance tests (log: $test_log)"
    
    if timeout 600 "$test_script" > "$test_log" 2>&1; then
        log "INFO" "Performance tests completed"
        TEST_RESULTS+=("TEST:performance:$build_type:$platform:SUCCESS")
        return 0
    else
        local exit_code=$?
        if [ $exit_code -eq 124 ]; then
            log "ERROR" "Performance tests timed out (10 minutes)"
            TEST_RESULTS+=("TEST:performance:$build_type:$platform:TIMEOUT")
        else
            log "ERROR" "Performance tests failed with exit code $exit_code"
            TEST_RESULTS+=("TEST:performance:$build_type:$platform:FAILED")
        fi
        return 1
    fi
}

# 静态代码分析
run_static_analysis() {
    log_step "Running Static Code Analysis"
    
    cd "$PROJECT_DIR"
    
    # 检查代码风格
    log "INFO" "Checking code style..."
    local style_issues=0
    
    # 检查常见的代码问题
    local files_to_check=$(find src/ -name "*.cpp" -o -name "*.h")
    
    for file in $files_to_check; do
        # 检查文件编码
        if file "$file" | grep -q "CRLF"; then
            log "WARNING" "File $file uses CRLF line endings"
            style_issues=$((style_issues + 1))
        fi
        
        # 检查空行过多
        local consecutive_empty_lines=$(grep -c "^$" "$file")
        if [ $consecutive_empty_lines -gt 5 ]; then
            log "WARNING" "File $file has excessive empty lines ($consecutive_empty_lines)"
            style_issues=$((style_issues + 1))
        fi
    done
    
    # 检查include guards
    local header_files=$(find src/ -name "*.h")
    for header in $header_files; do
        if ! grep -q "#ifndef.*_H" "$header"; then
            log "WARNING" "Header file $header missing include guard"
            style_issues=$((style_issues + 1))
        fi
    done
    
    if [ $style_issues -eq 0 ]; then
        log "INFO" "Static code analysis passed"
        return 0
    else
        log "WARNING" "Static code analysis found $style_issues issues"
        return 1
    fi
}

# 生成构建信息
generate_build_info() {
    local build_type=$1
    local platform=$2
    local info_file="$PROJECT_DIR/build_info.txt"
    
    cat > "$info_file" << EOF
Qt Puzzle Game Build Information
================================
Build Date: $(date)
Build Type: $build_type
Platform: $platform
Git Commit: $(git rev-parse HEAD 2>/dev/null || echo "N/A")
Git Branch: $(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo "N/A")
Qt Version: $(qmake --version | head -1)
Compiler: $(clang++ --version | head -1 2>/dev/null || g++ --version | head -1)
Build System: qmake + Make
EOF
    
    log "INFO" "Build info generated: $info_file"
}

# 创建部署包
create_deployment_package() {
    local build_type=$1
    local target=$2
    
    log_step "Creating Deployment Package for $build_type ($target)"
    
    local package_name="PuzzleGame-${build_type}-$(date +%Y%m%d-%H%M%S)"
    local package_dir="$PROJECT_DIR/deploy/$package_name"
    
    # 创建包目录
    mkdir -p "$package_dir"
    
    # 复制可执行文件
    local output_dir="../$build_type"
    if [ -f "$output_dir/PuzzleGame.app" ]; then
        cp -R "$output_dir/PuzzleGame.app" "$package_dir/"
    elif [ -f "$output_dir/PuzzleGame" ]; then
        cp "$output_dir/PuzzleGame" "$package_dir/"
    else
        log "ERROR" "No executable found for deployment"
        DEPLOY_RESULTS+=("DEPLOY:$build_type:$target:FAILED")
        return 1
    fi
    
    # 复制资源文件
    cp -R "$PROJECT_DIR/res" "$package_dir/"
    
    # 复制文档
    cp "$PROJECT_DIR/README.md" "$package_dir/" 2>/dev/null || true
    cp "$PROJECT_DIR/测试文档.md" "$package_dir/" 2>/dev/null || true
    
    # 复制构建信息
    cp "$PROJECT_DIR/build_info.txt" "$package_dir/" 2>/dev/null || true
    
    # 创建启动脚本
    cat > "$package_dir/launch.sh" << 'EOF'
#!/bin/bash
cd "$(dirname "$0")"
if [ -f "PuzzleGame.app/Contents/MacOS/PuzzleGame" ]; then
    open PuzzleGame.app
elif [ -f "PuzzleGame" ]; then
    ./PuzzleGame
else
    echo "Executable not found"
    exit 1
fi
EOF
    
    chmod +x "$package_dir/launch.sh"
    
    # 创建压缩包
    cd "$PROJECT_DIR/deploy"
    if command -v tar >/dev/null 2>&1; then
        tar -czf "${package_name}.tar.gz" "$package_name"
        log "INFO" "Package created: ${package_name}.tar.gz"
        DEPLOY_RESULTS+=("DEPLOY:$build_type:$target:SUCCESS:${package_name}.tar.gz")
    else
        log "ERROR" "tar command not found for package creation"
        DEPLOY_RESULTS+=("DEPLOY:$build_type:$target:FAILED")
        return 1
    fi
    
    return 0
}

# 生成汇总报告
generate_summary_report() {
    log_step "Generating Summary Report"
    
    {
        echo "Qt Puzzle Game Automation Summary"
        echo "=================================="
        echo "Started at: $(head -n 2 "$MAIN_LOG" | tail -n 1)"
        echo "Completed at: $(date)"
        echo ""
        
        echo "Build Results:"
        echo "-------------"
        for result in "${BUILD_RESULTS[@]}"; do
            IFS=':' read -r type platform build_type status time <<< "$result"
            echo "  $platform $build_type: $status ${time:-}"
        done
        
        echo ""
        echo "Test Results:"
        echo "------------"
        for result in "${TEST_RESULTS[@]}"; do
            IFS=':' read -r type test_type build_type platform status details <<< "$result"
            echo "  $test_type test ($build_type, $platform): $status ${details:-}"
        done
        
        echo ""
        echo "Deployment Results:"
        echo "------------------"
        for result in "${DEPLOY_RESULTS[@]}"; do
            IFS=':' read -r type build_type target status package <<< "$result"
            echo "  $build_type ($target): $status ${package:-}"
        done
        
        echo ""
        echo "Log Files:"
        echo "----------"
        echo "  Main log: $MAIN_LOG"
        find "$PROJECT_DIR" -name "test_*.log" -o -name "automation_*.log" | while read log_file; do
            echo "  Test log: $log_file"
        done
        
    } > "$SUMMARY_FILE"
    
    log "INFO" "Summary report generated: $SUMMARY_FILE"
}

# 主要流程
main() {
    echo -e "${PURPLE}===========================================${NC}"
    echo -e "${PURPLE}  Qt Puzzle Game Automation Build & Test    ${NC}"
    echo -e "${PURPLE}===========================================${NC}"
    
    # 创建测试脚本目录
    mkdir -p "$PROJECT_DIR/test_scripts"
    
    # 初始化日志
    init_logging
    
    # 环境检查
    if ! check_environment; then
        log "ERROR" "Environment check failed"
        exit 1
    fi
    
    # 清理环境
    clean_build_environment
    
    # 静态代码分析
    run_static_analysis
    local analysis_result=$?
    
    # 构建所有配置
    for build_type in "${BUILD_TYPES[@]}"; do
        for platform in "${PLATFORMS[@]}"; do
            if build_project "$build_type" "$platform"; then
                # 构建成功，运行测试
                run_functionality_tests "$build_type" "$platform"
                run_performance_tests "$build_type" "$platform"
                
                # 创建部署包（仅release版本）
                if [ "$build_type" == "release" ]; then
                    for target in "${DEPLOY_TARGETS[@]}"; do
                        create_deployment_package "$build_type" "$target"
                    done
                fi
            fi
        done
    done
    
    # 生成汇总报告
    generate_summary_report
    
    # 显示汇总
    log_step "Automation Complete"
    echo -e "${GREEN}✅ Automation process completed!${NC}"
    echo -e "${CYAN}Summary report: $SUMMARY_FILE${NC}"
    echo -e "${CYAN}Main log: $MAIN_LOG${NC}"
    
    # 检查是否有失败
    local failed_builds=$(printf '%s\n' "${BUILD_RESULTS[@]}" | grep -c "FAILED" || true)
    local failed_tests=$(printf '%s\n' "${TEST_RESULTS[@]}" | grep -c "FAILED" || true)
    local failed_deploys=$(printf '%s\n' "${DEPLOY_RESULTS[@]}" | grep -c "FAILED" || true)
    
    if [ $failed_builds -gt 0 ] || [ $failed_tests -gt 0 ] || [ $failed_deploys -gt 0 ]; then
        echo -e "\n${YELLOW}⚠️  Some issues detected:${NC}"
        [ $failed_builds -gt 0 ] && echo -e "  ${RED}• $failed_builds build(s) failed${NC}"
        [ $failed_tests -gt 0 ] && echo -e "  ${RED}• $failed_tests test(s) failed${NC}"
        [ $failed_deploys -gt 0 ] && echo -e "  ${RED}• $failed_deploys deployment(s) failed${NC}"
        exit 1
    else
        echo -e "\n${GREEN}🎉 All operations completed successfully!${NC}"
        exit 0
    fi
}

# 运行主函数
main "$@"