#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QTimer>
#include <QNetworkReply>

// 网络响应数据结构
struct NetworkResponse {
    bool success;
    QString message;
    QJsonObject data;
    QString error_code;
    
    NetworkResponse() : success(false) {}
    NetworkResponse(bool s, const QString& msg, const QJsonObject& d = QJsonObject(), const QString& err = QString())
        : success(s), message(msg), data(d), error_code(err) {}
};

// 用户信息结构
struct UserInfo {
    int user_id;
    QString username;
    QString nickname;
    QString session_id;
    
    UserInfo() : user_id(0) {}
    UserInfo(int id, const QString& uname, const QString& nick, const QString& sid)
        : user_id(id), username(uname), nickname(nick), session_id(sid) {}
    
    bool isValid() const {
        return user_id > 0 && !session_id.isEmpty();
    }
};

// 排行榜数据结构
struct LevelRankingInfo {
    int id;
    int user_id;
    QString username;
    QString nickname;
    int max_level;
    QString update_time;
    
    LevelRankingInfo() : id(0), user_id(0), max_level(0) {}
};

struct TimeRankingInfo {
    int id;
    int user_id;
    QString username;
    QString nickname;
    int grid_size;
    int time_seconds;
    bool used_undo;
    QString create_time;
    
    TimeRankingInfo() : id(0), user_id(0), grid_size(0), time_seconds(0), used_undo(false) {}
};

struct StepRankingInfo {
    int id;
    int user_id;
    QString username;
    QString nickname;
    int grid_size;
    int step_count;
    bool used_undo;
    QString create_time;
    
    StepRankingInfo() : id(0), user_id(0), grid_size(0), step_count(0), used_undo(false) {}
};

class NetworkClient : public QObject
{
    Q_OBJECT

public:
    explicit NetworkClient(QObject *parent = nullptr);
    ~NetworkClient();

    // 连接服务器
    bool connectToServer(const QString &host = "localhost", int port = 8080);
    void disconnectFromServer();
    bool isConnected() const;

    // 用户相关
    void registerUser(const QString &username, const QString &password, const QString &nickname);
    void loginUser(const QString &username, const QString &password);
    void logout();

    // 排行榜相关
    void getLevelRankings(int limit = 50);
    void getTimeRankings(int grid_size, bool used_undo, int limit = 50);
    void getStepRankings(int grid_size, bool used_undo, int limit = 50);

    // 游戏数据提交
    void submitGameResult(const QString &game_type, int grid_size = 0, 
                         int max_level = 0, int time_seconds = 0, 
                         int step_count = 0, bool used_undo = false);

    // 获取当前用户信息
    UserInfo getCurrentUser() const;
    bool isLoggedIn() const;

signals:
    // 连接状态信号
    void connected();
    void disconnected();
    void connectionError(const QString &error);

    // 用户相关信号
    void registerFinished(const NetworkResponse &response);
    void loginFinished(const NetworkResponse &response);
    void logoutFinished();

    // 排行榜信号
    void levelRankingsFinished(const NetworkResponse &response, const QList<LevelRankingInfo> &rankings);
    void timeRankingsFinished(const NetworkResponse &response, const QList<TimeRankingInfo> &rankings);
    void stepRankingsFinished(const NetworkResponse &response, const QList<StepRankingInfo> &rankings);

    // 游戏数据提交信号
    void submitGameResultFinished(const NetworkResponse &response);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket *socket;
    UserInfo current_user;
    QByteArray buffer;
    QTimer *reconnect_timer;
    QString server_host;
    int server_port;

    // 发送和接收数据
    bool sendJson(const QJsonObject &json);
    void processResponse(const QByteArray &data);
    void handleResponse(const QJsonObject &response);

    // 解析响应数据
    QList<LevelRankingInfo> parseLevelRankings(const QJsonArray &array);
    QList<TimeRankingInfo> parseTimeRankings(const QJsonArray &array);
    QList<StepRankingInfo> parseStepRankings(const QJsonArray &array);

    // 工具函数
    QString formatTime(int seconds) const;
    QJsonObject createRequest(const QString &type, const QJsonObject &data = QJsonObject());
};

#endif // NETWORKCLIENT_H