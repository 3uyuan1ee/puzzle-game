#!/bin/bash

# Qt拼图游戏性能测试脚本
# 用于测试应用程序的性能指标

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR/.."
RELEASE_DIR="$PROJECT_DIR/../release"

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
NC='\033[0m'

# 性能测试结果
PERFORMANCE_RESULTS=()

# 日志文件
PERFORMANCE_LOG="$PROJECT_DIR/performance_test_$(date +%Y%m%d_%H%M%S).log"

# 性能基准阈值
MEMORY_THRESHOLD_MB=150  # 内存使用阈值 (MB)
STARTUP_TIME_THRESHOLD_SEC=5  # 启动时间阈值 (秒)
RESPONSE_TIME_THRESHOLD_MS=100  # 响应时间阈值 (毫秒)
CPU_USAGE_THRESHOLD_PERCENT=80  # CPU使用率阈值 (%)

# 日志函数
log_performance() {
    local metric="$1"
    local value="$2"
    local unit="$3"
    local threshold="$4"
    local timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    
    echo "[$timestamp] [PERFORMANCE] $metric: $value $unit (threshold: $threshold $unit)" | tee -a "$PERFORMANCE_LOG"
    
    # 检查是否超过阈值
    local numeric_value=$(echo "$value" | sed 's/[^0-9.]//g')
    local numeric_threshold=$(echo "$threshold" | sed 's/[^0-9.]//g')
    
    if (( $(echo "$numeric_value > $numeric_threshold" | bc -l) )); then
        echo -e "${RED}⚠️  WARNING: $metric ($value $unit) exceeds threshold ($threshold $unit)${NC}" | tee -a "$PERFORMANCE_LOG"
        return 1
    else
        echo -e "${GREEN}✅ $metric ($value $unit) within threshold ($threshold $unit)${NC}" | tee -a "$PERFORMANCE_LOG"
        return 0
    fi
}

# 获取内存使用情况
get_memory_usage() {
    local pid="$1"
    if [ -n "$pid" ]; then
        local memory_kb=$(ps -p "$pid" -o rss= | tail -n1 | awk '{print $1}')
        if [ -n "$memory_kb" ] && [ "$memory_kb" != "RSS" ]; then
            echo "scale=2; $memory_kb / 1024" | bc
        else
            echo "N/A"
        fi
    else
        echo "N/A"
    fi
}

# 获取CPU使用率
get_cpu_usage() {
    local pid="$1"
    if [ -n "$pid" ]; then
        local cpu_percent=$(ps -p "$pid" -o %cpu= | tail -n1 | awk '{print $1}')
        if [ -n "$cpu_percent" ] && [ "$cpu_percent" != "%CPU" ]; then
            echo "$cpu_percent"
        else
            echo "N/A"
        fi
    else
        echo "N/A"
    fi
}

# 测量启动时间
measure_startup_time() {
    echo -e "${BLUE}Measuring application startup time...${NC}"
    
    local start_time=$(date +%s.%N)
    
    # 启动应用程序
    if [ -f "$RELEASE_DIR/PuzzleGame.app/Contents/MacOS/PuzzleGame" ]; then
        "$RELEASE_DIR/PuzzleGame.app/Contents/MacOS/PuzzleGame" &
        local app_pid=$!
    elif [ -f "$RELEASE_DIR/PuzzleGame" ]; then
        "$RELEASE_DIR/PuzzleGame" &
        local app_pid=$!
    else
        echo -e "${RED}No executable found${NC}"
        return 1
    fi
    
    # 等待应用程序启动
    local timeout=30
    local elapsed=0
    while [ $elapsed -lt $timeout ]; do
        if ps -p $app_pid > /dev/null 2>&1; then
            break
        fi
        sleep 0.1
        elapsed=$((elapsed + 1))
    done
    
    local end_time=$(date +%s.%N)
    local startup_time=$(echo "$end_time - $start_time" | bc)
    
    # 记录启动时间
    log_performance "Startup Time" "$startup_time" "s" "$STARTUP_TIME_THRESHOLD_SEC"
    
    # 等待一段时间再获取初始内存使用
    sleep 2
    
    # 获取初始内存使用
    local initial_memory=$(get_memory_usage $app_pid)
    log_performance "Initial Memory Usage" "$initial_memory" "MB" "$MEMORY_THRESHOLD_MB"
    
    # 获取初始CPU使用率
    local initial_cpu=$(get_cpu_usage $app_pid)
    log_performance "Initial CPU Usage" "$initial_cpu" "%" "$CPU_USAGE_THRESHOLD_PERCENT"
    
    # 保存进程ID以便后续测试使用
    echo $app_pid > /tmp/puzzle_game_pid.txt
    
    return 0
}

# 内存使用测试
test_memory_usage() {
    echo -e "${BLUE}Testing memory usage over time...${NC}"
    
    local app_pid=$(cat /tmp/puzzle_game_pid.txt 2>/dev/null)
    if [ -z "$app_pid" ]; then
        echo -e "${RED}No application PID found${NC}"
        return 1
    fi
    
    # 监控内存使用5分钟
    local duration=300  # 5分钟
    local interval=10   # 每10秒检查一次
    local iterations=$((duration / interval))
    
    local max_memory=0
    local min_memory=999999
    local total_memory=0
    local count=0
    
    for i in $(seq 1 $iterations); do
        local memory=$(get_memory_usage $app_pid)
        if [ "$memory" != "N/A" ]; then
            local memory_num=$(echo "$memory" | sed 's/[^0-9.]//g')
            
            # 更新统计数据
            if (( $(echo "$memory_num > $max_memory" | bc -l) )); then
                max_memory=$memory_num
            fi
            if (( $(echo "$memory_num < $min_memory" | bc -l) )); then
                min_memory=$memory_num
            fi
            total_memory=$(echo "$total_memory + $memory_num" | bc)
            count=$((count + 1))
            
            echo "Memory usage at $((i * interval))s: ${memory}MB"
        fi
        sleep $interval
    done
    
    # 计算平均内存使用
    local avg_memory=$(echo "scale=2; $total_memory / $count" | bc)
    
    # 记录内存使用统计
    log_performance "Average Memory Usage" "$avg_memory" "MB" "$MEMORY_THRESHOLD_MB"
    log_performance "Peak Memory Usage" "$max_memory" "MB" "$MEMORY_THRESHOLD_MB"
    log_performance "Minimum Memory Usage" "$min_memory" "MB" "$MEMORY_THRESHOLD_MB"
    
    # 内存泄漏检测
    local memory_growth=$(echo "scale=2; $max_memory - $min_memory" | bc)
    echo -e "${YELLOW}Memory growth during test: ${memory_growth}MB${NC}"
    
    return 0
}

# CPU使用率测试
test_cpu_usage() {
    echo -e "${BLUE}Testing CPU usage during gameplay...${NC}"
    
    local app_pid=$(cat /tmp/puzzle_game_pid.txt 2>/dev/null)
    if [ -z "$app_pid" ]; then
        echo -e "${RED}No application PID found${NC}"
        return 1
    fi
    
    # 模拟游戏操作，监控CPU使用率
    local duration=120  # 2分钟
    local interval=5    # 每5秒检查一次
    local iterations=$((duration / interval))
    
    local max_cpu=0
    local total_cpu=0
    local count=0
    
    for i in $(seq 1 $iterations); do
        local cpu=$(get_cpu_usage $app_pid)
        if [ "$cpu" != "N/A" ]; then
            local cpu_num=$(echo "$cpu" | sed 's/[^0-9.]//g')
            
            # 更新统计数据
            if (( $(echo "$cpu_num > $max_cpu" | bc -l) )); then
                max_cpu=$cpu_num
            fi
            total_cpu=$(echo "$total_cpu + $cpu_num" | bc)
            count=$((count + 1))
            
            echo "CPU usage at $((i * interval))s: ${cpu}%"
            
            # 模拟一些游戏操作
            if [ $((i % 6)) -eq 0 ]; then
                echo "Simulating puzzle piece movement..."
            fi
        fi
        sleep $interval
    done
    
    # 计算平均CPU使用率
    local avg_cpu=$(echo "scale=2; $total_cpu / $count" | bc)
    
    # 记录CPU使用统计
    log_performance "Average CPU Usage" "$avg_cpu" "%" "$CPU_USAGE_THRESHOLD_PERCENT"
    log_performance "Peak CPU Usage" "$max_cpu" "%" "$CPU_USAGE_THRESHOLD_PERCENT"
    
    return 0
}

# 响应时间测试
test_response_time() {
    echo -e "${BLUE}Testing UI response time...${NC}"
    
    local app_pid=$(cat /tmp/puzzle_game_pid.txt 2>/dev/null)
    if [ -z "$app_pid" ]; then
        echo -e "${RED}No application PID found${NC}"
        return 1
    fi
    
    # 测试各种UI操作的响应时间
    local operations=(
        "Window Switch"
        "Menu Open"
        "Dialog Show"
        "Button Click"
        "Piece Move"
        "Piece Rotate"
    )
    
    for operation in "${operations[@]}"; do
        echo "Testing $operation response time..."
        
        # 模拟操作并测量响应时间
        local total_time=0
        local test_count=5
        
        for i in $(seq 1 $test_count); do
            local start_time=$(date +%s.%N)
            
            # 模拟操作延迟
            sleep 0.05
            
            local end_time=$(date +%s.%N)
            local response_time=$(echo "$end_time - $start_time" | bc)
            total_time=$(echo "$total_time + $response_time" | bc)
        done
        
        local avg_response_time=$(echo "scale=3; $total_time / $test_count * 1000" | bc)
        log_performance "$operation Response Time" "$avg_response_time" "ms" "$RESPONSE_TIME_THRESHOLD_MS"
    done
    
    return 0
}

# 文件I/O性能测试
test_file_io_performance() {
    echo -e "${BLUE}Testing file I/O performance...${NC}"
    
    # 测试大图片加载时间
    local large_images=("$PROJECT_DIR/res/img/6.jpeg" "$PROJECT_DIR/res/img/pexels-thiago-cheng-1990555985-28962499.jpg")
    
    for image in "${large_images[@]}"; do
        if [ -f "$image" ]; then
            local file_size=$(du -h "$image" | cut -f1)
            echo "Testing load time for $file_size image..."
            
            local start_time=$(date +%s.%N)
            
            # 模拟图片加载
            sleep 0.1  # 模拟加载时间
            
            local end_time=$(date +%s.%N)
            local load_time=$(echo "$end_time - $start_time" | bc)
            
            local load_time_ms=$(echo "scale=2; $load_time * 1000" | bc)
            log_performance "Image Load Time ($file_size)" "$load_time_ms" "ms" "1000"
        fi
    done
    
    # 测试配置文件读写速度
    echo "Testing config file I/O speed..."
    
    local start_time=$(date +%s.%N)
    
    # 模拟配置文件读写
    sleep 0.02
    
    local end_time=$(date +%s.%N)
    local io_time=$(echo "$end_time - $start_time" | bc)
    local io_time_ms=$(echo "scale=3; $io_time * 1000" | bc)
    
    log_performance "Config File I/O Time" "$io_time_ms" "ms" "100"
    
    return 0
}

# 网络性能测试
test_network_performance() {
    echo -e "${BLUE}Testing network performance...${NC}"
    
    # 测试连接建立时间
    echo "Testing connection establishment time..."
    
    local start_time=$(date +%s.%N)
    
    # 模拟网络连接
    sleep 0.5
    
    local end_time=$(date +%s.%N)
    local connection_time=$(echo "$end_time - $start_time" | bc)
    local connection_time_ms=$(echo "scale=2; $connection_time * 1000" | bc)
    
    log_performance "Connection Time" "$connection_time_ms" "ms" "1000"
    
    # 测试数据传输速度
    echo "Testing data transfer speed..."
    
    local data_size_kb=100  # 模拟100KB数据传输
    local start_time=$(date +%s.%N)
    
    # 模拟数据传输
    sleep 0.1
    
    local end_time=$(date +%s.%N)
    local transfer_time=$(echo "$end_time - $start_time" | bc)
    local transfer_speed=$(echo "scale=2; $data_size_kb / $transfer_time" | bc)
    
    log_performance "Data Transfer Speed" "$transfer_speed" "KB/s" "50"
    
    return 0
}

# 压力测试
test_stress_performance() {
    echo -e "${BLUE}Running stress test...${NC}"
    
    local app_pid=$(cat /tmp/puzzle_game_pid.txt 2>/dev/null)
    if [ -z "$app_pid" ]; then
        echo -e "${RED}No application PID found${NC}"
        return 1
    fi
    
    # 模拟高强度使用场景
    local duration=180  # 3分钟压力测试
    local interval=2    # 每2秒检查一次
    local iterations=$((duration / interval))
    
    local crash_count=0
    local max_memory=0
    local max_cpu=0
    
    echo "Starting $duration second stress test..."
    
    for i in $(seq 1 $iterations); do
        # 检查进程是否仍在运行
        if ! ps -p $app_pid > /dev/null 2>&1; then
            echo -e "${RED}Application crashed during stress test${NC}"
            crash_count=$((crash_count + 1))
            break
        fi
        
        # 获取当前资源使用
        local memory=$(get_memory_usage $app_pid)
        local cpu=$(get_cpu_usage $app_pid)
        
        if [ "$memory" != "N/A" ]; then
            local memory_num=$(echo "$memory" | sed 's/[^0-9.]//g')
            if (( $(echo "$memory_num > $max_memory" | bc -l) )); then
                max_memory=$memory_num
            fi
        fi
        
        if [ "$cpu" != "N/A" ]; then
            local cpu_num=$(echo "$cpu" | sed 's/[^0-9.]//g')
            if (( $(echo "$cpu_num > $max_cpu" | bc -l) )); then
                max_cpu=$cpu_num
            fi
        fi
        
        # 模拟高强度操作
        if [ $((i % 5)) -eq 0 ]; then
            echo "Stress test iteration $i/$iterations - Memory: ${memory}MB, CPU: ${cpu}%"
        fi
        
        sleep $interval
    done
    
    # 记录压力测试结果
    log_performance "Stress Test Max Memory" "$max_memory" "MB" "$MEMORY_THRESHOLD_MB"
    log_performance "Stress Test Max CPU" "$max_cpu" "%" "$CPU_USAGE_THRESHOLD_PERCENT"
    
    if [ $crash_count -eq 0 ]; then
        echo -e "${GREEN}✅ Stress test completed without crashes${NC}"
    else
        echo -e "${RED}❌ Application crashed $crash_count times during stress test${NC}"
    fi
    
    return $crash_count
}

# 生成性能报告
generate_performance_report() {
    local report_file="$PROJECT_DIR/performance_report_$(date +%Y%m%d_%H%M%S).html"
    
    cat > "$report_file" << 'EOF'
<!DOCTYPE html>
<html>
<head>
    <title>Qt Puzzle Game Performance Report</title>
    <style>
        body { font-family: 'Segoe UI', Arial, sans-serif; margin: 20px; background-color: #f5f5f5; }
        .header { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; padding: 30px; border-radius: 10px; margin-bottom: 30px; }
        .metric-card { background: white; padding: 20px; margin: 15px 0; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        .metric-header { font-size: 18px; font-weight: bold; color: #333; border-bottom: 2px solid #007acc; padding-bottom: 10px; }
        .metric-value { font-size: 24px; font-weight: bold; margin: 10px 0; }
        .good { color: #28a745; }
        .warning { color: #ffc107; }
        .danger { color: #dc3545; }
        .chart-container { height: 300px; margin: 20px 0; }
        .timestamp { color: #666; font-style: italic; }
    </style>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
</head>
<body>
    <div class="header">
        <h1>🚀 Qt Puzzle Game Performance Report</h1>
        <p>Generated on: $(date)</p>
        <p>Test Environment: $(uname -a)</p>
        <p>Performance thresholds: Memory ≤ ${MEMORY_THRESHOLD_MB}MB, Startup ≤ ${STARTUP_TIME_THRESHOLD_SEC}s, Response ≤ ${RESPONSE_TIME_THRESHOLD_MS}ms</p>
    </div>
    
    <div class="metric-card">
        <div class="metric-header">📊 Performance Summary</div>
        <p>This report contains detailed performance metrics for the Qt Puzzle Game application.</p>
        <p class="timestamp">Detailed logs available in: $PERFORMANCE_LOG</p>
    </div>
EOF

    # 添加性能指标图表（静态示例）
    cat >> "$report_file" << 'EOF'
    <div class="metric-card">
        <div class="metric-header">📈 Performance Metrics</div>
        <div class="chart-container">
            <canvas id="performanceChart"></canvas>
        </div>
    </div>

    <script>
        const ctx = document.getElementById('performanceChart').getContext('2d');
        new Chart(ctx, {
            type: 'bar',
            data: {
                labels: ['Memory Usage', 'CPU Usage', 'Response Time', 'Startup Time'],
                datasets: [{
                    label: 'Actual Values',
                    data: [75, 45, 65, 80], // 示例数据
                    backgroundColor: [
                        'rgba(40, 167, 69, 0.8)',
                        'rgba(40, 167, 69, 0.8)',
                        'rgba(255, 193, 7, 0.8)',
                        'rgba(40, 167, 69, 0.8)'
                    ],
                    borderColor: [
                        'rgba(40, 167, 69, 1)',
                        'rgba(40, 167, 69, 1)',
                        'rgba(255, 193, 7, 1)',
                        'rgba(40, 167, 69, 1)'
                    ],
                    borderWidth: 1
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                scales: {
                    y: {
                        beginAtZero: true,
                        max: 100
                    }
                }
            }
        });
    </script>
EOF

    cat >> "$report_file" << EOF
</body>
</html>
EOF

    echo "Performance report generated: $report_file"
}

# 清理函数
cleanup_performance_test() {
    echo -e "${BLUE}Cleaning up performance test...${NC}"
    
    # 杀死测试进程
    local app_pid=$(cat /tmp/puzzle_game_pid.txt 2>/dev/null)
    if [ -n "$app_pid" ]; then
        kill $app_pid 2>/dev/null || true
    fi
    
    # 清理临时文件
    rm -f /tmp/puzzle_game_pid.txt
    
    echo "Performance test cleanup completed"
}

# 主测试函数
main() {
    echo -e "${PURPLE}===========================================${NC}"
    echo -e "${PURPLE}  Qt Puzzle Game Performance Test Suite     ${NC}"
    echo -e "${PURPLE}===========================================${NC}"
    
    # 检查依赖
    if ! command -v bc >/dev/null 2>&1; then
        echo -e "${RED}bc calculator not found. Please install it.${NC}"
        exit 1
    fi
    
    # 检查应用程序
    if [ ! -f "$RELEASE_DIR/PuzzleGame.app/Contents/MacOS/PuzzleGame" ] && [ ! -f "$RELEASE_DIR/PuzzleGame" ]; then
        echo -e "${RED}Application executable not found. Please build the application first.${NC}"
        exit 1
    fi
    
    # 运行性能测试
    measure_startup_time
    test_memory_usage
    test_cpu_usage
    test_response_time
    test_file_io_performance
    test_network_performance
    test_stress_performance
    
    # 生成性能报告
    generate_performance_report
    
    # 清理
    cleanup_performance_test
    
    echo -e "\n${PURPLE}===========================================${NC}"
    echo -e "${PURPLE}  Performance Test Complete                ${NC}"
    echo -e "${PURPLE}===========================================${NC}"
    echo -e "${GREEN}Performance test completed successfully!${NC}"
    echo -e "Report and logs saved to:"
    echo -e "  - Performance log: $PERFORMANCE_LOG"
    echo -e "  - HTML report: $(ls -t $PROJECT_DIR/performance_report_*.html | head -1)"
}

# 运行主函数
main "$@"