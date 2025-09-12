#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <thread>
#include <mutex>
#include <chrono>
#include <ctime>
#include <csignal>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <mysql/mysql.h>

// JSON库使用nlohmann/json
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// 服务器配置
struct ServerConfig {
    int port = 8080;
    std::string db_host = "localhost";
    int db_port = 3306;
    std::string db_user = "puzzle_admin";
    std::string db_password = "123456";
    std::string db_name = "puzzle_game";
    int max_connections = 100;
};

// 用户会话信息
struct Session {
    int user_id;
    std::string username;
    std::string nickname;
    std::chrono::system_clock::time_point create_time;
    
    Session(int id, const std::string& uname, const std::string& nick)
        : user_id(id), username(uname), nickname(nick),
          create_time(std::chrono::system_clock::now()) {}
};

// 数据库连接类
class Database {
private:
    MYSQL* mysql;
    ServerConfig config;
    
public:
    Database(const ServerConfig& cfg) : config(cfg) {
        mysql = mysql_init(nullptr);
        if (!mysql) {
            throw std::runtime_error("MySQL初始化失败");
        }
        
        if (!mysql_real_connect(mysql, config.db_host.c_str(), config.db_user.c_str(),
                               config.db_password.c_str(), config.db_name.c_str(),
                               config.db_port, nullptr, 0)) {
            std::string error = mysql_error(mysql);
            mysql_close(mysql);
            throw std::runtime_error("数据库连接失败: " + error);
        }
        
        // 设置字符集
        mysql_set_character_set(mysql, "utf8mb4");
    }
    
    ~Database() {
        if (mysql) {
            mysql_close(mysql);
        }
    }
    
    // 用户注册
    bool registerUser(const std::string& username, const std::string& password, 
                     const std::string& nickname, int& user_id) {
        std::string query = "INSERT INTO users (username, password, nickname) VALUES (?, ?, ?)";
        
        MYSQL_STMT* stmt = mysql_stmt_init(mysql);
        if (!stmt) return false;
        
        if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
            mysql_stmt_close(stmt);
            return false;
        }
        
        MYSQL_BIND bind[3];
        memset(bind, 0, sizeof(bind));
        
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (void*)username.c_str();
        bind[0].buffer_length = username.length();
        
        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = (void*)password.c_str();
        bind[1].buffer_length = password.length();
        
        bind[2].buffer_type = MYSQL_TYPE_STRING;
        bind[2].buffer = (void*)nickname.c_str();
        bind[2].buffer_length = nickname.length();
        
        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            mysql_stmt_close(stmt);
            return false;
        }
        
        if (mysql_stmt_execute(stmt) != 0) {
            mysql_stmt_close(stmt);
            return false;
        }
        
        user_id = mysql_insert_id(mysql);
        mysql_stmt_close(stmt);
        return true;
    }
    
    // 用户登录
    bool loginUser(const std::string& username, const std::string& password,
                  int& user_id, std::string& nickname) {
        std::string query = "SELECT id, nickname FROM users WHERE username = ? AND password = ?";
        
        MYSQL_STMT* stmt = mysql_stmt_init(mysql);
        if (!stmt) return false;
        
        if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
            mysql_stmt_close(stmt);
            return false;
        }
        
        MYSQL_BIND bind[2];
        memset(bind, 0, sizeof(bind));
        
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (void*)username.c_str();
        bind[0].buffer_length = username.length();
        
        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = (void*)password.c_str();
        bind[1].buffer_length = password.length();
        
        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            mysql_stmt_close(stmt);
            return false;
        }
        
        if (mysql_stmt_execute(stmt) != 0) {
            mysql_stmt_close(stmt);
            return false;
        }
        
        MYSQL_RES* result = mysql_stmt_result_metadata(stmt);
        if (!result) {
            mysql_stmt_close(stmt);
            return false;
        }
        
        MYSQL_BIND result_bind[2];
        memset(result_bind, 0, sizeof(result_bind));
        
        result_bind[0].buffer_type = MYSQL_TYPE_LONG;
        result_bind[0].buffer = &user_id;
        
        char nickname_buf[256];
        unsigned long nickname_length;
        result_bind[1].buffer_type = MYSQL_TYPE_STRING;
        result_bind[1].buffer = nickname_buf;
        result_bind[1].buffer_length = sizeof(nickname_buf);
        result_bind[1].length = &nickname_length;
        
        if (mysql_stmt_bind_result(stmt, result_bind) != 0) {
            mysql_free_result(result);
            mysql_stmt_close(stmt);
            return false;
        }
        
        if (mysql_stmt_fetch(stmt) != 0) {
            mysql_free_result(result);
            mysql_stmt_close(stmt);
            return false;
        }
        
        nickname = std::string(nickname_buf, nickname_length);
        
        mysql_free_result(result);
        mysql_stmt_close(stmt);
        
        // 更新最后登录时间
        std::string update_query = "UPDATE users SET last_login_time = NOW() WHERE id = ?";
        MYSQL_STMT* update_stmt = mysql_stmt_init(mysql);
        if (update_stmt) {
            mysql_stmt_prepare(update_stmt, update_query.c_str(), update_query.length());
            MYSQL_BIND update_bind;
            memset(&update_bind, 0, sizeof(update_bind));
            update_bind.buffer_type = MYSQL_TYPE_LONG;
            update_bind.buffer = &user_id;
            mysql_stmt_bind_param(update_stmt, &update_bind);
            mysql_stmt_execute(update_stmt);
            mysql_stmt_close(update_stmt);
        }
        
        return true;
    }
    
    // 获取关卡排行榜
    json getLevelRankings(int limit = 50) {
        std::string query = "SELECT r.id, r.user_id, u.username, u.nickname, r.max_level, r.update_time "
                           "FROM level_rankings r JOIN users u ON r.user_id = u.id "
                           "ORDER BY r.max_level DESC, r.update_time ASC LIMIT ?";
        
        MYSQL_STMT* stmt = mysql_stmt_init(mysql);
        if (!stmt) return json::array();
        
        if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
            mysql_stmt_close(stmt);
            return json::array();
        }
        
        MYSQL_BIND bind;
        memset(&bind, 0, sizeof(bind));
        bind.buffer_type = MYSQL_TYPE_LONG;
        bind.buffer = &limit;
        
        if (mysql_stmt_bind_param(stmt, &bind) != 0) {
            mysql_stmt_close(stmt);
            return json::array();
        }
        
        if (mysql_stmt_execute(stmt) != 0) {
            mysql_stmt_close(stmt);
            return json::array();
        }
        
        MYSQL_RES* result = mysql_stmt_result_metadata(stmt);
        if (!result) {
            mysql_stmt_close(stmt);
            return json::array();
        }
        
        json rankings = json::array();
        
        int id, user_id, max_level;
        char username[256], nickname[256], update_time[256];
        unsigned long username_len, nickname_len, update_time_len;
        
        MYSQL_BIND result_bind[6];
        memset(result_bind, 0, sizeof(result_bind));
        
        result_bind[0].buffer_type = MYSQL_TYPE_LONG;
        result_bind[0].buffer = &id;
        
        result_bind[1].buffer_type = MYSQL_TYPE_LONG;
        result_bind[1].buffer = &user_id;
        
        result_bind[2].buffer_type = MYSQL_TYPE_STRING;
        result_bind[2].buffer = username;
        result_bind[2].buffer_length = sizeof(username);
        result_bind[2].length = &username_len;
        
        result_bind[3].buffer_type = MYSQL_TYPE_STRING;
        result_bind[3].buffer = nickname;
        result_bind[3].buffer_length = sizeof(nickname);
        result_bind[3].length = &nickname_len;
        
        result_bind[4].buffer_type = MYSQL_TYPE_LONG;
        result_bind[4].buffer = &max_level;
        
        result_bind[5].buffer_type = MYSQL_TYPE_STRING;
        result_bind[5].buffer = update_time;
        result_bind[5].buffer_length = sizeof(update_time);
        result_bind[5].length = &update_time_len;
        
        if (mysql_stmt_bind_result(stmt, result_bind) == 0) {
            while (mysql_stmt_fetch(stmt) == 0) {
                json ranking;
                ranking["id"] = id;
                ranking["user_id"] = user_id;
                ranking["username"] = std::string(username, username_len);
                ranking["nickname"] = std::string(nickname, nickname_len);
                ranking["max_level"] = max_level;
                ranking["update_time"] = std::string(update_time, update_time_len);
                rankings.push_back(ranking);
            }
        }
        
        mysql_free_result(result);
        mysql_stmt_close(stmt);
        
        return rankings;
    }
    
    // 获取时间排行榜
    json getTimeRankings(int grid_size, bool used_undo, int limit = 50) {
        std::cout << "getTimeRankings called with grid_size=" << grid_size << ", used_undo=" << used_undo << ", limit=" << limit << std::endl;
        
        std::string query = "SELECT r.id, r.user_id, u.username, u.nickname, r.grid_size, r.time_seconds, r.used_undo, r.create_time "
                           "FROM time_rankings r JOIN users u ON r.user_id = u.id "
                           "WHERE r.grid_size = ? AND r.used_undo = ? "
                           "ORDER BY r.time_seconds ASC, r.create_time ASC LIMIT ?";
        
        MYSQL_STMT* stmt = mysql_stmt_init(mysql);
        if (!stmt) {
            std::cout << "Failed to initialize statement" << std::endl;
            return json::array();
        }
        
        if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
            std::cout << "Failed to prepare statement: " << mysql_stmt_error(stmt) << std::endl;
            mysql_stmt_close(stmt);
            return json::array();
        }
        
        MYSQL_BIND bind[3];
        memset(bind, 0, sizeof(bind));
        
        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &grid_size;
        
        bind[1].buffer_type = MYSQL_TYPE_TINY;
        bind[1].buffer = &used_undo;
        
        bind[2].buffer_type = MYSQL_TYPE_LONG;
        bind[2].buffer = &limit;
        
        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            mysql_stmt_close(stmt);
            return json::array();
        }
        
        if (mysql_stmt_execute(stmt) != 0) {
            mysql_stmt_close(stmt);
            return json::array();
        }
        
        MYSQL_RES* result = mysql_stmt_result_metadata(stmt);
        if (!result) {
            mysql_stmt_close(stmt);
            return json::array();
        }
        
        json rankings = json::array();
        
        int id, user_id, gs, time_seconds;
        bool undo;
        char username[256], nickname[256], create_time[256];
        unsigned long username_len, nickname_len, create_time_len;
        
        MYSQL_BIND result_bind[8];
        memset(result_bind, 0, sizeof(result_bind));
        
        result_bind[0].buffer_type = MYSQL_TYPE_LONG;
        result_bind[0].buffer = &id;
        
        result_bind[1].buffer_type = MYSQL_TYPE_LONG;
        result_bind[1].buffer = &user_id;
        
        result_bind[2].buffer_type = MYSQL_TYPE_STRING;
        result_bind[2].buffer = username;
        result_bind[2].buffer_length = sizeof(username);
        result_bind[2].length = &username_len;
        
        result_bind[3].buffer_type = MYSQL_TYPE_STRING;
        result_bind[3].buffer = nickname;
        result_bind[3].buffer_length = sizeof(nickname);
        result_bind[3].length = &nickname_len;
        
        result_bind[4].buffer_type = MYSQL_TYPE_LONG;
        result_bind[4].buffer = &gs;
        
        result_bind[5].buffer_type = MYSQL_TYPE_LONG;
        result_bind[5].buffer = &time_seconds;
        
        result_bind[6].buffer_type = MYSQL_TYPE_TINY;
        result_bind[6].buffer = &undo;
        
        result_bind[7].buffer_type = MYSQL_TYPE_STRING;
        result_bind[7].buffer = create_time;
        result_bind[7].buffer_length = sizeof(create_time);
        result_bind[7].length = &create_time_len;
        
        if (mysql_stmt_bind_result(stmt, result_bind) == 0) {
            while (mysql_stmt_fetch(stmt) == 0) {
                json ranking;
                ranking["id"] = id;
                ranking["user_id"] = user_id;
                ranking["username"] = std::string(username, username_len);
                ranking["nickname"] = std::string(nickname, nickname_len);
                ranking["grid_size"] = gs;
                ranking["time_seconds"] = time_seconds;
                ranking["used_undo"] = undo;
                ranking["create_time"] = std::string(create_time, create_time_len);
                rankings.push_back(ranking);
            }
        }
        
        mysql_free_result(result);
        mysql_stmt_close(stmt);
        
        std::cout << "getTimeRankings returning " << rankings.size() << " results" << std::endl;
        return rankings;
    }
    
    // 获取步数排行榜
    json getStepRankings(int grid_size, bool used_undo, int limit = 50) {
        std::string query = "SELECT r.id, r.user_id, u.username, u.nickname, r.grid_size, r.step_count, r.used_undo, r.create_time "
                           "FROM step_rankings r JOIN users u ON r.user_id = u.id "
                           "WHERE r.grid_size = ? AND r.used_undo = ? "
                           "ORDER BY r.step_count ASC, r.create_time ASC LIMIT ?";
        
        MYSQL_STMT* stmt = mysql_stmt_init(mysql);
        if (!stmt) return json::array();
        
        if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
            mysql_stmt_close(stmt);
            return json::array();
        }
        
        MYSQL_BIND bind[3];
        memset(bind, 0, sizeof(bind));
        
        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &grid_size;
        
        bind[1].buffer_type = MYSQL_TYPE_TINY;
        bind[1].buffer = &used_undo;
        
        bind[2].buffer_type = MYSQL_TYPE_LONG;
        bind[2].buffer = &limit;
        
        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            mysql_stmt_close(stmt);
            return json::array();
        }
        
        if (mysql_stmt_execute(stmt) != 0) {
            mysql_stmt_close(stmt);
            return json::array();
        }
        
        MYSQL_RES* result = mysql_stmt_result_metadata(stmt);
        if (!result) {
            mysql_stmt_close(stmt);
            return json::array();
        }
        
        json rankings = json::array();
        
        int id, user_id, gs, step_count;
        bool undo;
        char username[256], nickname[256], create_time[256];
        unsigned long username_len, nickname_len, create_time_len;
        
        MYSQL_BIND result_bind[8];
        memset(result_bind, 0, sizeof(result_bind));
        
        result_bind[0].buffer_type = MYSQL_TYPE_LONG;
        result_bind[0].buffer = &id;
        
        result_bind[1].buffer_type = MYSQL_TYPE_LONG;
        result_bind[1].buffer = &user_id;
        
        result_bind[2].buffer_type = MYSQL_TYPE_STRING;
        result_bind[2].buffer = username;
        result_bind[2].buffer_length = sizeof(username);
        result_bind[2].length = &username_len;
        
        result_bind[3].buffer_type = MYSQL_TYPE_STRING;
        result_bind[3].buffer = nickname;
        result_bind[3].buffer_length = sizeof(nickname);
        result_bind[3].length = &nickname_len;
        
        result_bind[4].buffer_type = MYSQL_TYPE_LONG;
        result_bind[4].buffer = &gs;
        
        result_bind[5].buffer_type = MYSQL_TYPE_LONG;
        result_bind[5].buffer = &step_count;
        
        result_bind[6].buffer_type = MYSQL_TYPE_TINY;
        result_bind[6].buffer = &undo;
        
        result_bind[7].buffer_type = MYSQL_TYPE_STRING;
        result_bind[7].buffer = create_time;
        result_bind[7].buffer_length = sizeof(create_time);
        result_bind[7].length = &create_time_len;
        
        if (mysql_stmt_bind_result(stmt, result_bind) == 0) {
            while (mysql_stmt_fetch(stmt) == 0) {
                json ranking;
                ranking["id"] = id;
                ranking["user_id"] = user_id;
                ranking["username"] = std::string(username, username_len);
                ranking["nickname"] = std::string(nickname, nickname_len);
                ranking["grid_size"] = gs;
                ranking["step_count"] = step_count;
                ranking["used_undo"] = undo;
                ranking["create_time"] = std::string(create_time, create_time_len);
                rankings.push_back(ranking);
            }
        }
        
        mysql_free_result(result);
        mysql_stmt_close(stmt);
        
        return rankings;
    }
    
    // 提交游戏结果
    bool submitGameResult(int user_id, const std::string& game_type, int grid_size, 
                         int max_level, int time_seconds, int step_count, bool used_undo) {
        try {
            if (game_type == "level") {
                // 更新或插入关卡排行榜
                std::string query = "INSERT INTO level_rankings (user_id, max_level) VALUES (?, ?) "
                                   "ON DUPLICATE KEY UPDATE max_level = GREATEST(max_level, ?), update_time = NOW()";
                
                MYSQL_STMT* stmt = mysql_stmt_init(mysql);
                if (!stmt) return false;
                
                if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
                    mysql_stmt_close(stmt);
                    return false;
                }
                
                MYSQL_BIND bind[3];
                memset(bind, 0, sizeof(bind));
                
                bind[0].buffer_type = MYSQL_TYPE_LONG;
                bind[0].buffer = &user_id;
                
                bind[1].buffer_type = MYSQL_TYPE_LONG;
                bind[1].buffer = &max_level;
                
                bind[2].buffer_type = MYSQL_TYPE_LONG;
                bind[2].buffer = &max_level;
                
                bool result = mysql_stmt_bind_param(stmt, bind) == 0 && mysql_stmt_execute(stmt) == 0;
                mysql_stmt_close(stmt);
                return result;
            }
            else if (game_type == "time") {
                // 更新或插入时间排行榜
                std::string query = "INSERT INTO time_rankings (user_id, grid_size, time_seconds, used_undo) VALUES (?, ?, ?, ?) "
                                   "ON DUPLICATE KEY UPDATE time_seconds = LEAST(time_seconds, ?), create_time = IF(time_seconds > ?, NOW(), create_time)";
                
                MYSQL_STMT* stmt = mysql_stmt_init(mysql);
                if (!stmt) return false;
                
                if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
                    mysql_stmt_close(stmt);
                    return false;
                }
                
                MYSQL_BIND bind[5];
                memset(bind, 0, sizeof(bind));
                
                bind[0].buffer_type = MYSQL_TYPE_LONG;
                bind[0].buffer = &user_id;
                
                bind[1].buffer_type = MYSQL_TYPE_LONG;
                bind[1].buffer = &grid_size;
                
                bind[2].buffer_type = MYSQL_TYPE_LONG;
                bind[2].buffer = &time_seconds;
                
                bind[3].buffer_type = MYSQL_TYPE_TINY;
                bind[3].buffer = &used_undo;
                
                bind[4].buffer_type = MYSQL_TYPE_LONG;
                bind[4].buffer = &time_seconds;
                
                bool result = mysql_stmt_bind_param(stmt, bind) == 0 && mysql_stmt_execute(stmt) == 0;
                mysql_stmt_close(stmt);
                return result;
            }
            else if (game_type == "step") {
                // 更新或插入步数排行榜
                std::string query = "INSERT INTO step_rankings (user_id, grid_size, step_count, used_undo) VALUES (?, ?, ?, ?) "
                                   "ON DUPLICATE KEY UPDATE step_count = LEAST(step_count, ?), create_time = IF(step_count > ?, NOW(), create_time)";
                
                MYSQL_STMT* stmt = mysql_stmt_init(mysql);
                if (!stmt) return false;
                
                if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
                    mysql_stmt_close(stmt);
                    return false;
                }
                
                MYSQL_BIND bind[5];
                memset(bind, 0, sizeof(bind));
                
                bind[0].buffer_type = MYSQL_TYPE_LONG;
                bind[0].buffer = &user_id;
                
                bind[1].buffer_type = MYSQL_TYPE_LONG;
                bind[1].buffer = &grid_size;
                
                bind[2].buffer_type = MYSQL_TYPE_LONG;
                bind[2].buffer = &step_count;
                
                bind[3].buffer_type = MYSQL_TYPE_TINY;
                bind[3].buffer = &used_undo;
                
                bind[4].buffer_type = MYSQL_TYPE_LONG;
                bind[4].buffer = &step_count;
                
                bool result = mysql_stmt_bind_param(stmt, bind) == 0 && mysql_stmt_execute(stmt) == 0;
                mysql_stmt_close(stmt);
                return result;
            }
        }
        catch (...) {
            return false;
        }
        return false;
    }
};

// 客户端连接类
class ClientConnection {
private:
    int socket_fd;
    std::string client_ip;
    std::chrono::system_clock::time_point connect_time;
    
public:
    ClientConnection(int fd, const std::string& ip) 
        : socket_fd(fd), client_ip(ip), connect_time(std::chrono::system_clock::now()) {}
    
    ~ClientConnection() {
        if (socket_fd != -1) {
            close(socket_fd);
        }
    }
    
    int getSocket() const { return socket_fd; }
    std::string getIp() const { return client_ip; }
    
    bool sendData(const std::string& data) {
        size_t total_sent = 0;
        size_t data_size = data.size();
        
        // 先发送数据长度
        uint32_t length = htonl(data_size);
        if (::send(socket_fd, &length, sizeof(length), 0) != sizeof(length)) {
            return false;
        }
        
        // 再发送数据
        while (total_sent < data_size) {
            ssize_t sent = ::send(socket_fd, data.c_str() + total_sent, data_size - total_sent, 0);
            if (sent <= 0) {
                return false;
            }
            total_sent += sent;
        }
        
        return true;
    }
    
    int receiveData(std::string& data) {
        // 先接收数据长度
        uint32_t length;
        ssize_t received = ::recv(socket_fd, &length, sizeof(length), 0);
        if (received <= 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 非阻塞模式下没有数据可读
     
                return -1;
            }
            std::cout << "接收长度失败，连接断开" << std::endl;
            return 0; // 连接断开
        }
        
        if (received != sizeof(length)) {
            std::cout << "接收长度不完整，连接断开" << std::endl;
            return 0; // 连接断开
        }
        
        length = ntohl(length);
        std::cout << "接收到数据长度: " << length << std::endl;
        
        if (length > 1024 * 1024) { // 限制最大1MB
            std::cout << "数据长度超过限制，连接断开" << std::endl;
            return 0; // 连接断开
        }
        
        // 再接收数据
        data.resize(length);
        size_t total_received = 0;
        
        while (total_received < length) {
            ssize_t rec = ::recv(socket_fd, &data[total_received], length - total_received, 0);
            if (rec <= 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // 非阻塞模式下没有数据可读，稍后重试
                    std::cout << "接收数据时非阻塞模式，稍后重试" << std::endl;
                    continue;
                }
                std::cout << "接收数据失败，连接断开" << std::endl;
                return 0; // 连接断开
            }
            total_received += rec;
        }
        
        std::cout << "成功接收数据，长度: " << total_received << std::endl;
        return 1; // 成功接收数据
    }
    
    void setNonBlocking() {
        int flags = fcntl(socket_fd, F_GETFL, 0);
        fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
    }
};

// 服务器类
class PuzzleGameServer {
private:
    ServerConfig config;
    int server_fd;
    std::map<int, std::shared_ptr<ClientConnection>> clients;
    std::map<std::string, std::shared_ptr<Session>> sessions;
    std::mutex clients_mutex;
    std::mutex sessions_mutex;
    std::unique_ptr<Database> db;
    bool running;
    
public:
    PuzzleGameServer(const ServerConfig& cfg) : config(cfg), running(false) {}
    
    ~PuzzleGameServer() {
        stop();
    }
    
    bool start() {
        // 创建服务器socket
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1) {
            std::cerr << "创建socket失败" << std::endl;
            return false;
        }
        
        // 设置socket选项
        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            std::cerr << "设置socket选项失败" << std::endl;
            close(server_fd);
            return false;
        }
        
        // 绑定地址和端口
        sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(config.port);
        
        if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "绑定失败" << std::endl;
            close(server_fd);
            return false;
        }
        
        // 开始监听
        if (listen(server_fd, config.max_connections) < 0) {
            std::cerr << "监听失败" << std::endl;
            close(server_fd);
            return false;
        }
        
        // 初始化数据库
        try {
            db = std::make_unique<Database>(config);
        }
        catch (const std::exception& e) {
            std::cerr << "数据库连接失败: " << e.what() << std::endl;
            close(server_fd);
            return false;
        }
        
        running = true;
        std::cout << "服务器启动成功，监听端口: " << config.port << std::endl;
        
        // 设置非阻塞
        int flags = fcntl(server_fd, F_GETFL, 0);
        fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);
        
        return true;
    }
    
    void stop() {
        if (!running) return;
        
        running = false;
        
        // 关闭所有客户端连接
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.clear();
        
        if (server_fd != -1) {
            close(server_fd);
            server_fd = -1;
        }
        
        std::cout << "服务器已停止" << std::endl;
    }
    
    void run() {
        while (running) {
            // 接受新连接
            acceptNewConnections();
            
            // 处理客户端消息
            handleClientMessages();
            
            // 清理过期会话
            cleanupExpiredSessions();
            
            // 短暂休眠
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
private:
    void acceptNewConnections() {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                std::cerr << "接受连接失败: " << strerror(errno) << std::endl;
            }
            return;
        }
        
        // 获取客户端IP
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        
        std::cout << "新连接来自: " << client_ip << std::endl;
        
        // 创建客户端连接
        auto client = std::make_shared<ClientConnection>(client_fd, client_ip);
        client->setNonBlocking();
        
        // 添加到客户端列表
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients[client_fd] = client;
    }
    
    void handleClientMessages() {
        std::lock_guard<std::mutex> lock(clients_mutex);
        
        auto it = clients.begin();
        while (it != clients.end()) {
            auto client = it->second;
            std::string message;
            
            int result = client->receiveData(message);
            if (result > 0) {
                // 成功接收到数据，处理消息
                handleClientMessage(client, message);
                ++it;
            }
            else if (result == 0) {
                // 连接已断开
                std::cout << "客户端断开连接: " << client->getIp() << std::endl;
                it = clients.erase(it);
            }
            else {
                // 暂时没有数据，继续下一个客户端
                ++it;
            }
        }
    }
    
    void handleClientMessage(std::shared_ptr<ClientConnection> client, const std::string& message) {
        try {
            json request = json::parse(message);
            std::string type = request["type"];
            
            json response;
            
            if (type == "register") {
                response = handleRegister(request);
            }
            else if (type == "login") {
                response = handleLogin(request);
            }
            else if (type == "get_level_rankings") {
                response = handleGetLevelRankings(request);
            }
            else if (type == "get_time_rankings") {
                std::cout << "Processing get_time_rankings request" << std::endl;
                response = handleGetTimeRankings(request);
                std::cout << "get_time_rankings response prepared" << std::endl;
            }
            else if (type == "get_step_rankings") {
                response = handleGetStepRankings(request);
            }
            else if (type == "submit_game_result") {
                response = handleSubmitGameResult(request);
            }
            else {
                response = {
                    {"type", "error"},
                    {"success", false},
                    {"message", "未知请求类型"},
                    {"error_code", "INVALID_REQUEST"}
                };
            }
            
            // 发送响应
            std::cout << "Sending response for type: " << type << ", size: " << response.dump().length() << " bytes" << std::endl;
            client->sendData(response.dump());
        }
        catch (const std::exception& e) {
            json error_response = {
                {"type", "error"},
                {"success", false},
                {"message", "消息解析失败"},
                {"error_code", "INVALID_REQUEST"}
            };
            client->sendData(error_response.dump());
        }
    }
    
    json handleRegister(const json& request) {
        try {
            std::string username = request["data"]["username"];
            std::string password = request["data"]["password"];
            std::string nickname = request["data"]["nickname"];
            
            int user_id = 0;
            if (db->registerUser(username, password, nickname, user_id)) {
                return {
                    {"type", "register_response"},
                    {"success", true},
                    {"message", "注册成功"},
                    {"data", {
                        {"user_id", user_id},
                        {"username", username},
                        {"nickname", nickname}
                    }}
                };
            }
            else {
                return {
                    {"type", "register_response"},
                    {"success", false},
                    {"message", "注册失败"}
                };
            }
        }
        catch (const std::exception& e) {
            return {
                {"type", "register_response"},
                {"success", false},
                {"message", "注册失败: " + std::string(e.what())}
            };
        }
    }
    
    json handleLogin(const json& request) {
        try {
            std::string username = request["data"]["username"];
            std::string password = request["data"]["password"];
            
            int user_id = 0;
            std::string nickname;
            
            if (db->loginUser(username, password, user_id, nickname)) {
                // 创建会话
                std::string session_id = std::to_string(user_id) + "_" + 
                                       std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
                
                auto session = std::make_shared<Session>(user_id, username, nickname);
                
                {
                    std::lock_guard<std::mutex> lock(sessions_mutex);
                    sessions[session_id] = session;
                }
                
                return {
                    {"type", "login_response"},
                    {"success", true},
                    {"message", "登录成功"},
                    {"data", {
                        {"user_id", user_id},
                        {"username", username},
                        {"nickname", nickname},
                        {"session_id", session_id}
                    }}
                };
            }
            else {
                return {
                    {"type", "login_response"},
                    {"success", false},
                    {"message", "用户名或密码错误"}
                };
            }
        }
        catch (const std::exception& e) {
            return {
                {"type", "login_response"},
                {"success", false},
                {"message", "登录失败: " + std::string(e.what())}
            };
        }
    }
    
    json handleGetLevelRankings(const json& request) {
        try {
            int limit = 50;
            if (request["data"].contains("limit")) {
                limit = request["data"]["limit"];
            }
            
            json rankings = db->getLevelRankings(limit);
            
            return {
                {"type", "level_rankings_response"},
                {"success", true},
                {"data", rankings}
            };
        }
        catch (const std::exception& e) {
            return {
                {"type", "level_rankings_response"},
                {"success", false},
                {"message", "获取关卡排行榜失败: " + std::string(e.what())}
            };
        }
    }
    
    json handleGetTimeRankings(const json& request) {
        try {
            int grid_size = request["data"]["grid_size"];
            bool used_undo = request["data"]["used_undo"];
            int limit = 50;
            
            if (request["data"].contains("limit")) {
                limit = request["data"]["limit"];
            }
            
            json rankings = db->getTimeRankings(grid_size, used_undo, limit);
            
            std::cout << "Returning time_rankings_response with data size: " << rankings.size() << std::endl;
            std::cout << "JSON response: " << json({
                {"type", "time_rankings_response"},
                {"success", true},
                {"data", rankings}
            }).dump() << std::endl;
            
            return {
                {"type", "time_rankings_response"},
                {"success", true},
                {"data", rankings}
            };
        }
        catch (const std::exception& e) {
            std::cout << "Exception in handleGetTimeRankings: " << e.what() << std::endl;
            return {
                {"type", "time_rankings_response"},
                {"success", false},
                {"message", "获取时间排行榜失败: " + std::string(e.what())}
            };
        }
    }
    
    json handleGetStepRankings(const json& request) {
        try {
            int grid_size = request["data"]["grid_size"];
            bool used_undo = request["data"]["used_undo"];
            int limit = 50;
            
            if (request["data"].contains("limit")) {
                limit = request["data"]["limit"];
            }
            
            json rankings = db->getStepRankings(grid_size, used_undo, limit);
            
            return {
                {"type", "step_rankings_response"},
                {"success", true},
                {"data", rankings}
            };
        }
        catch (const std::exception& e) {
            return {
                {"type", "step_rankings_response"},
                {"success", false},
                {"message", "获取步数排行榜失败: " + std::string(e.what())}
            };
        }
    }
    
    json handleSubmitGameResult(const json& request) {
        try {
            int user_id = request["data"]["user_id"];
            std::string session_id = request["data"]["session_id"];
            std::string game_type = request["data"]["game_type"];
            
            // 验证会话
            {
                std::lock_guard<std::mutex> lock(sessions_mutex);
                if (sessions.find(session_id) == sessions.end()) {
                    return {
                        {"type", "submit_result_response"},
                        {"success", false},
                        {"message", "会话无效"}
                    };
                }
            }
            
            int grid_size = 0;
            int max_level = 0;
            int time_seconds = 0;
            int step_count = 0;
            bool used_undo = false;
            
            if (game_type == "level") {
                max_level = request["data"]["max_level"];
            }
            else if (game_type == "time") {
                grid_size = request["data"]["grid_size"];
                time_seconds = request["data"]["time_seconds"];
                used_undo = request["data"]["used_undo"];
            }
            else if (game_type == "step") {
                grid_size = request["data"]["grid_size"];
                step_count = request["data"]["step_count"];
                used_undo = request["data"]["used_undo"];
            }
            else {
                return {
                    {"type", "submit_result_response"},
                    {"success", false},
                    {"message", "无效的游戏类型"}
                };
            }
            
            if (db->submitGameResult(user_id, game_type, grid_size, max_level, time_seconds, step_count, used_undo)) {
                return {
                    {"type", "submit_result_response"},
                    {"success", true},
                    {"message", "数据提交成功"}
                };
            }
            else {
                return {
                    {"type", "submit_result_response"},
                    {"success", false},
                    {"message", "数据提交失败"}
                };
            }
        }
        catch (const std::exception& e) {
            return {
                {"type", "submit_result_response"},
                {"success", false},
                {"message", "数据提交失败: " + std::string(e.what())}
            };
        }
    }
    
    void cleanupExpiredSessions() {
        auto now = std::chrono::system_clock::now();
        auto expire_time = std::chrono::hours(24); // 24小时过期
        
        std::lock_guard<std::mutex> lock(sessions_mutex);
        auto it = sessions.begin();
        while (it != sessions.end()) {
            if (now - it->second->create_time > expire_time) {
                it = sessions.erase(it);
            }
            else {
                ++it;
            }
        }
    }
};

int main() {
    ServerConfig config;
    config.port = 8080;
    config.db_host = "localhost";
    config.db_port = 3306;
    config.db_user = "root";
    config.db_password = "password";
    config.db_name = "puzzle_game";
    config.max_connections = 100;
    
    PuzzleGameServer server(config);
    
    if (!server.start()) {
        std::cerr << "服务器启动失败" << std::endl;
        return 1;
    }
    
    std::cout << "拼图游戏服务器正在运行..." << std::endl;
    std::cout << "按 Ctrl+C 停止服务器" << std::endl;
    
    // 设置信号处理
    signal(SIGINT, [](int) {
        std::cout << "\n正在停止服务器..." << std::endl;
        exit(0);
    });
    
    server.run();
    
    return 0;
}