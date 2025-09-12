-- 拼图游戏数据库结构
-- 创建数据库
CREATE DATABASE IF NOT EXISTS puzzle_game CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE puzzle_game;

-- 用户表
CREATE TABLE IF NOT EXISTS users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    password VARCHAR(255) NOT NULL,
    nickname VARCHAR(50) NOT NULL,
    create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    last_login_time DATETIME,
    INDEX idx_username (username),
    INDEX idx_nickname (nickname)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 关卡排行榜表
CREATE TABLE IF NOT EXISTS level_rankings (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    max_level INT NOT NULL DEFAULT 1,
    update_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    INDEX idx_user_id (user_id),
    INDEX idx_max_level (max_level),
    INDEX idx_update_time (update_time)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 时间排行榜表
CREATE TABLE IF NOT EXISTS time_rankings (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    grid_size INT NOT NULL,  -- 3=3x3, 4=4x4, 5=5x5, 6=6x6, 7=7x7, 8=8x8
    time_seconds INT NOT NULL,
    used_undo BOOLEAN NOT NULL DEFAULT FALSE,
    create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    INDEX idx_user_id (user_id),
    INDEX idx_grid_size (grid_size),
    INDEX idx_time_seconds (time_seconds),
    INDEX idx_used_undo (used_undo),
    UNIQUE KEY unique_user_grid_undo (user_id, grid_size, used_undo)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 步数排行榜表
CREATE TABLE IF NOT EXISTS step_rankings (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    grid_size INT NOT NULL,  -- 3=3x3, 4=4x4, 5=5x5, 6=6x6, 7=7x7, 8=8x8
    step_count INT NOT NULL,
    used_undo BOOLEAN NOT NULL DEFAULT FALSE,
    create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    INDEX idx_user_id (user_id),
    INDEX idx_grid_size (grid_size),
    INDEX idx_step_count (step_count),
    INDEX idx_used_undo (used_undo),
    UNIQUE KEY unique_user_grid_undo (user_id, grid_size, used_undo)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 插入一些测试数据（可选）
INSERT IGNORE INTO users (username, password, nickname) VALUES 
('admin', 'admin123', '管理员'),
('player1', '123456', '玩家1'),
('player2', '123456', '玩家2');

-- 如果插入测试用户，同时插入一些测试数据
INSERT IGNORE INTO level_rankings (user_id, max_level) VALUES 
(1, 8),
(2, 5),
(3, 3);

INSERT IGNORE INTO time_rankings (user_id, grid_size, time_seconds, used_undo) VALUES 
(1, 3, 120, FALSE),
(1, 4, 300, FALSE),
(2, 3, 150, FALSE),
(2, 4, 400, TRUE),
(3, 3, 180, FALSE);

INSERT IGNORE INTO step_rankings (user_id, grid_size, step_count, used_undo) VALUES 
(1, 3, 50, FALSE),
(1, 4, 120, FALSE),
(2, 3, 65, FALSE),
(2, 4, 150, TRUE),
(3, 3, 80, FALSE);