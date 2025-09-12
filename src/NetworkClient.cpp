#include "NetworkClient.h"
#include <QDataStream>
#include <QTimer>
#include <QDebug>

NetworkClient::NetworkClient(QObject *parent)
    : QObject(parent)
    , socket(new QTcpSocket(this))
    , reconnect_timer(new QTimer(this))
    , server_port(8080)
{
    // 连接socket信号
    connect(socket, &QTcpSocket::connected, this, &NetworkClient::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &NetworkClient::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &NetworkClient::onReadyRead);
    connect(socket, &QTcpSocket::errorOccurred, this, &NetworkClient::onError);

    // 重连定时器
    reconnect_timer->setInterval(5000); // 5秒重连
    connect(reconnect_timer, &QTimer::timeout, this, [this]() {
        if (!isConnected()) {
            connectToServer(server_host, server_port);
        }
    });
}

NetworkClient::~NetworkClient()
{
    disconnectFromServer();
}

bool NetworkClient::connectToServer(const QString &host, int port)
{
    server_host = host;
    server_port = port;
    
    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->disconnectFromHost();
    }
    
    socket->connectToHost(host, port);
    return socket->waitForConnected(5000); // 5秒超时
}

void NetworkClient::disconnectFromServer()
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->disconnectFromHost();
    }
    reconnect_timer->stop();
}

bool NetworkClient::isConnected() const
{
    return socket->state() == QAbstractSocket::ConnectedState;
}

void NetworkClient::registerUser(const QString &username, const QString &password, const QString &nickname)
{
    if (!isConnected()) {
        emit registerFinished(NetworkResponse(false, "未连接到服务器"));
        return;
    }

    QJsonObject data;
    data["username"] = username;
    data["password"] = password;
    data["nickname"] = nickname;

    QJsonObject request = createRequest("register", data);
    sendJson(request);
}

void NetworkClient::loginUser(const QString &username, const QString &password)
{
    if (!isConnected()) {
        emit loginFinished(NetworkResponse(false, "未连接到服务器"));
        return;
    }

    QJsonObject data;
    data["username"] = username;
    data["password"] = password;

    QJsonObject request = createRequest("login", data);
    sendJson(request);
}

void NetworkClient::logout()
{
    current_user = UserInfo();
    emit logoutFinished();
}

void NetworkClient::getLevelRankings(int limit)
{
    if (!isConnected()) {
        emit levelRankingsFinished(NetworkResponse(false, "未连接到服务器"), QList<LevelRankingInfo>());
        return;
    }

    QJsonObject data;
    data["limit"] = limit;

    QJsonObject request = createRequest("get_level_rankings", data);
    sendJson(request);
}

void NetworkClient::getTimeRankings(int grid_size, bool used_undo, int limit)
{
    if (!isConnected()) {
        emit timeRankingsFinished(NetworkResponse(false, "未连接到服务器"), QList<TimeRankingInfo>());
        return;
    }

    QJsonObject data;
    data["grid_size"] = grid_size;
    data["used_undo"] = used_undo;
    data["limit"] = limit;

    QJsonObject request = createRequest("get_time_rankings", data);
    sendJson(request);
}

void NetworkClient::getStepRankings(int grid_size, bool used_undo, int limit)
{
    if (!isConnected()) {
        emit stepRankingsFinished(NetworkResponse(false, "未连接到服务器"), QList<StepRankingInfo>());
        return;
    }

    QJsonObject data;
    data["grid_size"] = grid_size;
    data["used_undo"] = used_undo;
    data["limit"] = limit;

    QJsonObject request = createRequest("get_step_rankings", data);
    sendJson(request);
}

void NetworkClient::submitGameResult(const QString &game_type, int grid_size, 
                                     int max_level, int time_seconds, 
                                     int step_count, bool used_undo)
{
    if (!isConnected() || !isLoggedIn()) {
        emit submitGameResultFinished(NetworkResponse(false, "未连接到服务器或未登录"));
        return;
    }

    QJsonObject data;
    data["user_id"] = current_user.user_id;
    data["session_id"] = current_user.session_id;
    data["game_type"] = game_type;
    
    if (game_type == "level") {
        data["max_level"] = max_level;
    } else if (game_type == "time") {
        data["grid_size"] = grid_size;
        data["time_seconds"] = time_seconds;
        data["used_undo"] = used_undo;
    } else if (game_type == "step") {
        data["grid_size"] = grid_size;
        data["step_count"] = step_count;
        data["used_undo"] = used_undo;
    }

    QJsonObject request = createRequest("submit_game_result", data);
    sendJson(request);
}

UserInfo NetworkClient::getCurrentUser() const
{
    return current_user;
}

bool NetworkClient::isLoggedIn() const
{
    return current_user.isValid();
}

void NetworkClient::onConnected()
{
    qDebug() << "Connected to server";
    buffer.clear();
    reconnect_timer->stop();
    emit connected();
}

void NetworkClient::onDisconnected()
{
    qDebug() << "Disconnected from server";
    buffer.clear();
    emit disconnected();
    
    // 启动重连定时器
    reconnect_timer->start();
}

void NetworkClient::onReadyRead()
{
    QByteArray data = socket->readAll();
    qDebug() << "NetworkClient: onReadyRead received" << data.size() << "bytes, buffer now" << buffer.size() + data.size() << "bytes";
    buffer.append(data);

    // 处理完整的消息
    while (buffer.size() >= 4) {
        // 读取消息长度
        QDataStream stream(buffer);
        stream.setByteOrder(QDataStream::BigEndian);
        
        quint32 length;
        stream >> length;
        
        // 检查是否有完整的消息
        if (buffer.size() < 4 + length) {
            qDebug() << "NetworkClient: Incomplete message, need" << (4 + length) << "bytes, have" << buffer.size() << "bytes";
            break;
        }
        
        // 提取消息内容
        QByteArray message = buffer.mid(4, length);
        qDebug() << "NetworkClient: Processing message of length" << length << ", message size:" << message.size();
        buffer.remove(0, 4 + length);
        
        // 处理消息
        processResponse(message);
    }
}

void NetworkClient::onError(QAbstractSocket::SocketError socketError)
{
    QString error = socket->errorString();
    qDebug() << "Socket error:" << error;
    emit connectionError(error);
}

bool NetworkClient::sendJson(const QJsonObject &json)
{
    if (!isConnected()) {
        return false;
    }

    QJsonDocument doc(json);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    // 创建数据包: 长度(4字节) + 数据
    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    
    quint32 length = data.size();
    stream << length;
    packet.append(data);

    qint64 bytesWritten = socket->write(packet);
    return bytesWritten == packet.size();
}

void NetworkClient::processResponse(const QByteArray &data)
{
    qDebug() << "NetworkClient: processResponse received" << data.size() << "bytes";
    qDebug() << "NetworkClient: Raw data:" << data;
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (doc.isNull()) {
        qWarning() << "Invalid JSON response";
        qDebug() << "NetworkClient: JSON parse error:" << parseError.errorString();
        return;
    }

    QJsonObject response = doc.object();
    qDebug() << "NetworkClient: Parsed JSON object:" << QJsonDocument(response).toJson(QJsonDocument::Compact);
    handleResponse(response);
}

void NetworkClient::handleResponse(const QJsonObject &response)
{
    QString type = response["type"].toString();
    bool success = response["success"].toBool();
    QString message = response["message"].toString();
    QJsonValue dataValue = response["data"];
    QString error_code = response["error_code"].toString();

    NetworkResponse network_response(success, message, dataValue.toObject(), error_code);

    if (type == "register_response") {
        emit registerFinished(network_response);
    }
    else if (type == "login_response") {
        if (success) {
            QJsonObject dataObj = dataValue.toObject();
            current_user = UserInfo(
                dataObj["user_id"].toInt(),
                dataObj["username"].toString(),
                dataObj["nickname"].toString(),
                dataObj["session_id"].toString()
            );
        }
        emit loginFinished(network_response);
    }
    else if (type == "level_rankings_response") {
        QList<LevelRankingInfo> rankings;
        if (success && dataValue.isArray()) {
            rankings = parseLevelRankings(dataValue.toArray());
        }
        emit levelRankingsFinished(network_response, rankings);
    }
    else if (type == "time_rankings_response") {
        QList<TimeRankingInfo> rankings;
        qDebug() << "NetworkClient: time_rankings_response, success:" << success << "dataValue is array:" << dataValue.isArray();
        if (success && dataValue.isArray()) {
            QJsonArray dataArray = dataValue.toArray();
            qDebug() << "NetworkClient: data array size:" << dataArray.size();
            rankings = parseTimeRankings(dataArray);
        }
        qDebug() << "NetworkClient: Received time_rankings_response, success:" << success << "rankings.size():" << rankings.size();
        emit timeRankingsFinished(network_response, rankings);
    }
    else if (type == "step_rankings_response") {
        QList<StepRankingInfo> rankings;
        if (success && dataValue.isArray()) {
            rankings = parseStepRankings(dataValue.toArray());
        }
        emit stepRankingsFinished(network_response, rankings);
    }
    else if (type == "submit_result_response") {
        emit submitGameResultFinished(network_response);
    }
    else if (type == "error") {
        qWarning() << "Server error:" << message << "(" << error_code << ")";
    }
    else {
        qWarning() << "Unknown response type:" << type;
    }
}

QList<LevelRankingInfo> NetworkClient::parseLevelRankings(const QJsonArray &array)
{
    QList<LevelRankingInfo> rankings;
    
    for (const QJsonValue &value : array) {
        QJsonObject obj = value.toObject();
        LevelRankingInfo info;
        info.id = obj["id"].toInt();
        info.user_id = obj["user_id"].toInt();
        info.username = obj["username"].toString();
        info.nickname = obj["nickname"].toString();
        info.max_level = obj["max_level"].toInt();
        info.update_time = obj["update_time"].toString();
        rankings.append(info);
    }
    
    return rankings;
}

QList<TimeRankingInfo> NetworkClient::parseTimeRankings(const QJsonArray &array)
{
    QList<TimeRankingInfo> rankings;
    qDebug() << "parseTimeRankings: array size =" << array.size();
    
    for (const QJsonValue &value : array) {
        QJsonObject obj = value.toObject();
        TimeRankingInfo info;
        info.id = obj["id"].toInt();
        info.user_id = obj["user_id"].toInt();
        info.username = obj["username"].toString();
        info.nickname = obj["nickname"].toString();
        info.grid_size = obj["grid_size"].toInt();
        info.time_seconds = obj["time_seconds"].toInt();
        info.used_undo = obj["used_undo"].toBool();
        info.create_time = obj["create_time"].toString();
        rankings.append(info);
    }
    
    return rankings;
}

QList<StepRankingInfo> NetworkClient::parseStepRankings(const QJsonArray &array)
{
    QList<StepRankingInfo> rankings;
    
    for (const QJsonValue &value : array) {
        QJsonObject obj = value.toObject();
        StepRankingInfo info;
        info.id = obj["id"].toInt();
        info.user_id = obj["user_id"].toInt();
        info.username = obj["username"].toString();
        info.nickname = obj["nickname"].toString();
        info.grid_size = obj["grid_size"].toInt();
        info.step_count = obj["step_count"].toInt();
        info.used_undo = obj["used_undo"].toBool();
        info.create_time = obj["create_time"].toString();
        rankings.append(info);
    }
    
    return rankings;
}

QString NetworkClient::formatTime(int seconds) const
{
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;
    
    if (hours > 0) {
        return QString("%1:%2:%3")
            .arg(hours, 2, 10, QChar('0'))
            .arg(minutes, 2, 10, QChar('0'))
            .arg(secs, 2, 10, QChar('0'));
    } else {
        return QString("%1:%2")
            .arg(minutes, 2, 10, QChar('0'))
            .arg(secs, 2, 10, QChar('0'));
    }
}

QJsonObject NetworkClient::createRequest(const QString &type, const QJsonObject &data)
{
    QJsonObject request;
    request["type"] = type;
    request["data"] = data;
    return request;
}