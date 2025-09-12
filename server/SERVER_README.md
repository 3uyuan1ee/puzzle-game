# 拼图游戏服务器编译和运行说明

## 系统要求
- Linux系统 (Ubuntu/CentOS等)
- C++17或更高版本
- MySQL 5.7+
- CMake 3.10+
- Git

## 安装依赖

### Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential cmake libmysqlclient-dev libssl-dev
```

### CentOS/RHEL
```bash
sudo yum install gcc-c++ cmake mysql-devel openssl-devel
```

## 编译服务器

### 1. 下载nlohmann/json库
```bash
cd ~
git clone https://github.com/nlohmann/json.git
cd json
mkdir build && cd build
cmake ..
make -j4
sudo make install
```

### 2. 编译服务器
```bash
cd /path/to/puzzle_server
g++ -std=c++17 -o puzzle_server puzzle_server.cpp -lmysqlclient -pthread
```

### 3. 或者使用Makefile
创建 Makefile:
```makefile
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
LDFLAGS = -lmysqlclient -pthread
TARGET = puzzle_server
SOURCES = puzzle_server.cpp

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)

clean:
	rm -f $(TARGET)

install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/
	sudo chmod +x /usr/local/bin/$(TARGET)
```

编译:
```bash
make
```

## 数据库配置

### 1. 创建数据库
```bash
mysql -u root -p < database_schema.sql
```

### 2. 修改服务器配置
在 puzzle_server.cpp 中修改数据库连接配置:
```cpp
ServerConfig config;
config.port = 8080;
config.db_host = "localhost";
config.db_port = 3306;
config.db_user = "root";
config.db_password = "your_password";
config.db_name = "puzzle_game";
config.max_connections = 100;
```

## 运行服务器

### 1. 直接运行
```bash
./puzzle_server
```

### 2. 后台运行
```bash
nohup ./puzzle_server > server.log 2>&1 &
```

### 3. 使用systemd服务
创建服务文件 /etc/systemd/system/puzzle-server.service:
```ini
[Unit]
Description=Puzzle Game Server
After=network.target mysql.service

[Service]
Type=simple
User=your_user
ExecStart=/path/to/puzzle_server
Restart=always
RestartSec=5
Environment=LD_LIBRARY_PATH=/usr/local/lib

[Install]
WantedBy=multi-user.target
```

启动服务:
```bash
sudo systemctl daemon-reload
sudo systemctl enable puzzle-server
sudo systemctl start puzzle-server
```

## 防火墙配置

### Ubuntu (ufw)
```bash
sudo ufw allow 8080/tcp
```

### CentOS (firewalld)
```bash
sudo firewall-cmd --permanent --add-port=8080/tcp
sudo firewall-cmd --reload
```

## 测试服务器

### 1. 使用telnet测试连接
```bash
telnet localhost 8080
```

### 2. 使用curl测试API
```bash
# 测试注册
curl -X POST http://localhost:8080 -H "Content-Type: application/json" -d '{
    "type": "register",
    "data": {
        "username": "testuser",
        "password": "testpass",
        "nickname": "测试用户"
    }
}'

# 测试登录
curl -X POST http://localhost:8080 -H "Content-Type: application/json" -d '{
    "type": "login",
    "data": {
        "username": "testuser",
        "password": "testpass"
    }
}'
```

## 日志监控

### 1. 查看日志
```bash
tail -f server.log
```

### 2. 系统日志
```bash
journalctl -u puzzle-server -f
```

## 性能优化

### 1. 数据库连接池
- 修改代码实现连接池
- 设置合适的连接数

### 2. 内存管理
- 监控内存使用
- 定期清理过期会话

### 3. 网络优化
- 调整TCP参数
- 使用负载均衡

## 安全配置

### 1. 数据库安全
- 使用强密码
- 限制数据库访问IP
- 定期备份

### 2. 网络安全
- 使用SSL/TLS加密
- 配置防火墙规则
- 监控异常连接

### 3. 应用安全
- 输入验证
- SQL注入防护
- 限流控制

## 故障排除

### 1. 数据库连接失败
- 检查数据库服务状态
- 验证连接参数
- 检查防火墙设置

### 2. 端口占用
```bash
sudo netstat -tulpn | grep :8080
sudo lsof -i :8080
```

### 3. 内存泄漏
- 使用valgrind检测
- 检查连接和会话管理

### 4. 性能问题
- 监控CPU和内存使用
- 检查数据库查询性能
- 分析网络延迟