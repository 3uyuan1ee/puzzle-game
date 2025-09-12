#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <cstring>

enum ActionType {
    ACTION_LOGIN,
    ACTION_REGISTER,
    ACTION_GET_RANKING,
    ACTION_RESPONSE_SUCCESS,
    ACTION_RESPONSE_ERROR
};

enum Difficulty {
    DIFFICULTY_EASY,
    DIFFICULTY_MEDIUM,
    DIFFICULTY_HARD
};

#pragma pack(push, 1)
// 请求头
struct RequestHeader {
    int action;
    int dataLength;
};

// 响应头
struct ResponseHeader {
    int action;
    int statusCode; // 0=成功, 其他=错误码
    int dataLength;
};

// 登录请求
struct LoginRequest {
    char username[50];
    char password[50];
};

// 注册请求
struct RegisterRequest {
    char username[50];
    char password[50];
};

// 排行榜请求
struct RankingRequest {
    int difficulty; // Difficulty枚举值
};

// 排行榜条目
struct RankingItem {
    int rank;
    char username[50];
    float completionTime;
    int moves;
    char difficulty[10]; // "easy", "medium", "hard"
};
#pragma pack(pop)

#endif // PROTOCOL_H
