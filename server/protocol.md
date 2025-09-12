# 拼图游戏客户端-服务器通信协议

## 通信格式
- 使用JSON格式进行数据交换
- 每个消息包含 `type` 字段标识消息类型
- 使用TCP Socket进行通信

## 消息类型定义

### 1. 用户注册 (register)
**客户端 → 服务器**
```json
{
    "type": "register",
    "data": {
        "username": "用户名",
        "password": "密码",
        "nickname": "昵称"
    }
}
```

**服务器 → 客户端 (成功)**
```json
{
    "type": "register_response",
    "success": true,
    "message": "注册成功",
    "data": {
        "user_id": 1,
        "username": "用户名",
        "nickname": "昵称"
    }
}
```

**服务器 → 客户端 (失败)**
```json
{
    "type": "register_response",
    "success": false,
    "message": "用户名已存在"
}
```

### 2. 用户登录 (login)
**客户端 → 服务器**
```json
{
    "type": "login",
    "data": {
        "username": "用户名",
        "password": "密码"
    }
}
```

**服务器 → 客户端 (成功)**
```json
{
    "type": "login_response",
    "success": true,
    "message": "登录成功",
    "data": {
        "user_id": 1,
        "username": "用户名",
        "nickname": "昵称",
        "session_id": "session_token"
    }
}
```

**服务器 → 客户端 (失败)**
```json
{
    "type": "login_response",
    "success": false,
    "message": "用户名或密码错误"
}
```

### 3. 获取关卡排行榜 (get_level_rankings)
**客户端 → 服务器**
```json
{
    "type": "get_level_rankings",
    "data": {
        "limit": 50  // 可选，限制返回数量
    }
}
```

**服务器 → 客户端**
```json
{
    "type": "level_rankings_response",
    "success": true,
    "data": [
        {
            "id": 1,
            "user_id": 1,
            "username": "用户名",
            "nickname": "昵称",
            "max_level": 8,
            "update_time": "2024-01-01 12:00:00"
        }
    ]
}
```

### 4. 获取时间排行榜 (get_time_rankings)
**客户端 → 服务器**
```json
{
    "type": "get_time_rankings",
    "data": {
        "grid_size": 4,
        "used_undo": false,
        "limit": 50
    }
}
```

**服务器 → 客户端**
```json
{
    "type": "time_rankings_response",
    "success": true,
    "data": [
        {
            "id": 1,
            "user_id": 1,
            "username": "用户名",
            "nickname": "昵称",
            "grid_size": 4,
            "time_seconds": 300,
            "used_undo": false,
            "create_time": "2024-01-01 12:00:00"
        }
    ]
}
```

### 5. 获取步数排行榜 (get_step_rankings)
**客户端 → 服务器**
```json
{
    "type": "get_step_rankings",
    "data": {
        "grid_size": 4,
        "used_undo": false,
        "limit": 50
    }
}
```

**服务器 → 客户端**
```json
{
    "type": "step_rankings_response",
    "success": true,
    "data": [
        {
            "id": 1,
            "user_id": 1,
            "username": "用户名",
            "nickname": "昵称",
            "grid_size": 4,
            "step_count": 120,
            "used_undo": false,
            "create_time": "2024-01-01 12:00:00"
        }
    ]
}
```

### 6. 提交游戏数据 (submit_game_result)
**客户端 → 服务器**
```json
{
    "type": "submit_game_result",
    "data": {
        "user_id": 1,
        "session_id": "session_token",
        "game_type": "level",  // "level", "time", "step"
        "grid_size": 4,        // 对于时间和步数排行榜
        "max_level": 8,        // 对于关卡排行榜
        "time_seconds": 300,   // 对于时间排行榜
        "step_count": 120,     // 对于步数排行榜
        "used_undo": false     // 对于时间和步数排行榜
    }
}
```

**服务器 → 客户端**
```json
{
    "type": "submit_result_response",
    "success": true,
    "message": "数据提交成功"
}
```

### 7. 错误响应
**服务器 → 客户端**
```json
{
    "type": "error",
    "success": false,
    "message": "错误信息",
    "error_code": "INVALID_SESSION"
}
```

## 错误码定义
- `INVALID_SESSION`: 会话无效
- `USER_NOT_FOUND`: 用户不存在
- `INVALID_PASSWORD`: 密码错误
- `USERNAME_EXISTS`: 用户名已存在
- `DATABASE_ERROR`: 数据库错误
- `INVALID_REQUEST`: 无效请求

## 时间格式
- 所有时间字段使用ISO 8601格式: `YYYY-MM-DD HH:MM:SS`