-- 为拼图游戏生成更多测试数据
-- 这个脚本会在现有数据的基础上添加更多用户和游戏记录

-- 清空现有测试数据（保留表结构）
DELETE FROM level_rankings;
DELETE FROM time_rankings;
DELETE FROM step_rankings;
DELETE FROM users WHERE username IN ('testuser', 'player1', 'player2', 'player3', 'player4', 'player5', 'gamer1', 'gamer2', 'pro1', 'pro2', 'noob1', 'expert1', 'master1', 'champion1', 'legend1');

-- 添加更多测试用户
INSERT INTO users (username, password, nickname, create_time) VALUES
('testuser', 'password123', '测试用户', NOW()),
('player1', 'pass123', '勇敢的冒险者', NOW()),
('player2', 'pass123', '智慧之剑', NOW()),
('player3', 'pass123', '闪电侠', NOW()),
('player4', 'pass123', '策略大师', NOW()),
('player5', 'pass123', '拼图新手', NOW()),
('gamer1', 'pass123', '游戏达人', NOW()),
('gamer2', 'pass123', '休闲玩家', NOW()),
('pro1', 'pass123', '专业选手', NOW()),
('pro2', 'pass123', '职业玩家', NOW()),
('noob1', 'pass123', '初学者小王', NOW()),
('expert1', 'pass123', '专家李', NOW()),
('master1', 'pass123', '大师张', NOW()),
('champion1', 'pass123', '冠军王', NOW()),
('legend1', 'pass123', '传奇玩家', NOW());

-- 获取用户ID以便后续插入
SET @testuser_id = (SELECT id FROM users WHERE username = 'testuser');
SET @player1_id = (SELECT id FROM users WHERE username = 'player1');
SET @player2_id = (SELECT id FROM users WHERE username = 'player2');
SET @player3_id = (SELECT id FROM users WHERE username = 'player3');
SET @player4_id = (SELECT id FROM users WHERE username = 'player4');
SET @player5_id = (SELECT id FROM users WHERE username = 'player5');
SET @gamer1_id = (SELECT id FROM users WHERE username = 'gamer1');
SET @gamer2_id = (SELECT id FROM users WHERE username = 'gamer2');
SET @pro1_id = (SELECT id FROM users WHERE username = 'pro1');
SET @pro2_id = (SELECT id FROM users WHERE username = 'pro2');
SET @noob1_id = (SELECT id FROM users WHERE username = 'noob1');
SET @expert1_id = (SELECT id FROM users WHERE username = 'expert1');
SET @master1_id = (SELECT id FROM users WHERE username = 'master1');
SET @champion1_id = (SELECT id FROM users WHERE username = 'champion1');
SET @legend1_id = (SELECT id FROM users WHERE username = 'legend1');

-- 插入关卡排行榜数据（从高到低）
INSERT INTO level_rankings (user_id, max_level) VALUES
(@legend1_id, 50),    -- 传奇玩家，最高50关
(@champion1_id, 48),   -- 冠军，48关
(@master1_id, 45),    -- 大师，45关
(@expert1_id, 42),    -- 专家，42关
(@pro1_id, 38),      -- 专业选手，38关
(@pro2_id, 35),      -- 职业玩家，35关
(@player1_id, 32),    -- 勇敢的冒险者，32关
(@gamer1_id, 28),     -- 游戏达人，28关
(@player2_id, 25),    -- 智慧之剑，25关
(@player3_id, 22),    -- 闪电侠，22关
(@gamer2_id, 18),     -- 休闲玩家，18关
(@player4_id, 15),    -- 策略大师，15关
(@testuser_id, 12),   -- 测试用户，12关
(@player5_id, 8),     -- 拼图新手，8关
(@noob1_id, 5);      -- 初学者小王，5关

-- 插入时间排行榜数据 - 3x3 网格（未使用撤销）
INSERT INTO time_rankings (user_id, grid_size, time_seconds, used_undo) VALUES
(@legend1_id, 3, 45, 0),     -- 传奇玩家，3x3，45秒，未使用撤销
(@champion1_id, 3, 52, 0),   -- 冠军，3x3，52秒，未使用撤销
(@master1_id, 3, 58, 0),     -- 大师，3x3，58秒，未使用撤销
(@expert1_id, 3, 65, 0),     -- 专家，3x3，65秒，未使用撤销
(@pro1_id, 3, 72, 0),       -- 专业选手，3x3，72秒，未使用撤销
(@pro2_id, 3, 78, 0),       -- 职业玩家，3x3，78秒，未使用撤销
(@player1_id, 3, 85, 0),     -- 勇敢的冒险者，3x3，85秒，未使用撤销
(@gamer1_id, 3, 92, 0),      -- 游戏达人，3x3，92秒，未使用撤销
(@player2_id, 3, 98, 0),     -- 智慧之剑，3x3，98秒，未使用撤销
(@player3_id, 3, 105, 0);    -- 闪电侠，3x3，105秒，未使用撤销

-- 插入时间排行榜数据 - 3x3 网格（使用撤销）
INSERT INTO time_rankings (user_id, grid_size, time_seconds, used_undo) VALUES
(@champion1_id, 3, 38, 1),   -- 冠军，3x3，38秒，使用撤销
(@legend1_id, 3, 42, 1),     -- 传奇玩家，3x3，42秒，使用撤销
(@master1_id, 3, 48, 1),     -- 大师，3x3，48秒，使用撤销
(@expert1_id, 3, 55, 1),     -- 专家，3x3，55秒，使用撤销
(@pro1_id, 3, 62, 1),       -- 专业选手，3x3，62秒，使用撤销
(@player1_id, 3, 68, 1),     -- 勇敢的冒险者，3x3，68秒，使用撤销
(@gamer1_id, 3, 75, 1),      -- 游戏达人，3x3，75秒，使用撤销
(@player2_id, 3, 82, 1),     -- 智慧之剑，3x3，82秒，使用撤销
(@player3_id, 3, 88, 1),     -- 闪电侠，3x3，88秒，使用撤销
(@gamer2_id, 3, 95, 1);      -- 休闲玩家，3x3，95秒，使用撤销

-- 插入时间排行榜数据 - 4x4 网格（未使用撤销）
INSERT INTO time_rankings (user_id, grid_size, time_seconds, used_undo) VALUES
(@legend1_id, 4, 120, 0),    -- 传奇玩家，4x4，2分钟，未使用撤销
(@champion1_id, 4, 135, 0),  -- 冠军，4x4，2分15秒，未使用撤销
(@master1_id, 4, 150, 0),    -- 大师，4x4，2分30秒，未使用撤销
(@expert1_id, 4, 165, 0),    -- 专家，4x4，2分45秒，未使用撤销
(@pro1_id, 4, 180, 0),      -- 专业选手，4x4，3分钟，未使用撤销
(@pro2_id, 4, 195, 0),      -- 职业玩家，4x4，3分15秒，未使用撤销
(@player1_id, 4, 210, 0),    -- 勇敢的冒险者，4x4，3分30秒，未使用撤销
(@gamer1_id, 4, 225, 0),     -- 游戏达人，4x4，3分45秒，未使用撤销
(@player2_id, 4, 240, 0),    -- 智慧之剑，4x4，4分钟，未使用撤销
(@player3_id, 4, 255, 0);    -- 闪电侠，4x4，4分15秒，未使用撤销

-- 插入时间排行榜数据 - 5x5 网格（未使用撤销）
INSERT INTO time_rankings (user_id, grid_size, time_seconds, used_undo) VALUES
(@legend1_id, 5, 300, 0),    -- 传奇玩家，5x5，5分钟，未使用撤销
(@champion1_id, 5, 330, 0),  -- 冠军，5x5，5分30秒，未使用撤销
(@master1_id, 5, 360, 0),    -- 大师，5x5，6分钟，未使用撤销
(@expert1_id, 5, 390, 0),    -- 专家，5x5，6分30秒，未使用撤销
(@pro1_id, 5, 420, 0),      -- 专业选手，5x5，7分钟，未使用撤销
(@pro2_id, 5, 450, 0),      -- 职业玩家，5x5，7分30秒，未使用撤销
(@player1_id, 5, 480, 0),    -- 勇敢的冒险者，5x5，8分钟，未使用撤销
(@gamer2_id, 5, 510, 0),     -- 休闲玩家，5x5，8分30秒，未使用撤销
(@player2_id, 5, 540, 0),    -- 智慧之剑，5x5，9分钟，未使用撤销
(@player3_id, 5, 570, 0);    -- 闪电侠，5x5，9分30秒，未使用撤销

-- 插入步数排行榜数据 - 3x3 网格（未使用撤销）
INSERT INTO step_rankings (user_id, grid_size, step_count, used_undo) VALUES
(@legend1_id, 3, 28, 0),     -- 传奇玩家，3x3，28步，未使用撤销
(@champion1_id, 3, 32, 0),   -- 冠军，3x3，32步，未使用撤销
(@master1_id, 3, 35, 0),     -- 大师，3x3，35步，未使用撤销
(@expert1_id, 3, 38, 0),     -- 专家，3x3，38步，未使用撤销
(@pro1_id, 3, 42, 0),       -- 专业选手，3x3，42步，未使用撤销
(@pro2_id, 3, 45, 0),       -- 职业玩家，3x3，45步，未使用撤销
(@player1_id, 3, 48, 0),     -- 勇敢的冒险者，3x3，48步，未使用撤销
(@gamer1_id, 3, 52, 0),      -- 游戏达人，3x3，52步，未使用撤销
(@player2_id, 3, 55, 0),     -- 智慧之剑，3x3，55步，未使用撤销
(@player3_id, 3, 58, 0);     -- 闪电侠，3x3，58步，未使用撤销

-- 插入步数排行榜数据 - 3x3 网格（使用撤销）
INSERT INTO step_rankings (user_id, grid_size, step_count, used_undo) VALUES
(@champion1_id, 3, 25, 1),   -- 冠军，3x3，25步，使用撤销
(@legend1_id, 3, 28, 1),     -- 传奇玩家，3x3，28步，使用撤销
(@master1_id, 3, 30, 1),     -- 大师，3x3，30步，使用撤销
(@expert1_id, 3, 33, 1),     -- 专家，3x3，33步，使用撤销
(@pro1_id, 3, 36, 1),       -- 专业选手，3x3，36步，使用撤销
(@player1_id, 3, 40, 1),     -- 勇敢的冒险者，3x3，40步，使用撤销
(@gamer1_id, 3, 43, 1),      -- 游戏达人，3x3，43步，使用撤销
(@player2_id, 3, 46, 1),     -- 智慧之剑，3x3，46步，使用撤销
(@player3_id, 3, 50, 1),     -- 闪电侠，3x3，50步，使用撤销
(@gamer2_id, 3, 53, 1);      -- 休闲玩家，3x3，53步，使用撤销

-- 插入步数排行榜数据 - 4x4 网格（未使用撤销）
INSERT INTO step_rankings (user_id, grid_size, step_count, used_undo) VALUES
(@legend1_id, 4, 65, 0),     -- 传奇玩家，4x4，65步，未使用撤销
(@champion1_id, 4, 72, 0),   -- 冠军，4x4，72步，未使用撤销
(@master1_id, 4, 80, 0),     -- 大师，4x4，80步，未使用撤销
(@expert1_id, 4, 88, 0),     -- 专家，4x4，88步，未使用撤销
(@pro1_id, 4, 95, 0),       -- 专业选手，4x4，95步，未使用撤销
(@pro2_id, 4, 102, 0),      -- 职业玩家，4x4，102步，未使用撤销
(@player1_id, 4, 110, 0),    -- 勇敢的冒险者，4x4，110步，未使用撤销
(@gamer1_id, 4, 118, 0),     -- 游戏达人，4x4，118步，未使用撤销
(@player2_id, 4, 125, 0),    -- 智慧之剑，4x4，125步，未使用撤销
(@player3_id, 4, 132, 0);    -- 闪电侠，4x4，132步，未使用撤销

-- 插入步数排行榜数据 - 5x5 网格（未使用撤销）
INSERT INTO step_rankings (user_id, grid_size, step_count, used_undo) VALUES
(@legend1_id, 5, 120, 0),    -- 传奇玩家，5x5，120步，未使用撤销
(@champion1_id, 5, 135, 0),  -- 冠军，5x5，135步，未使用撤销
(@master1_id, 5, 150, 0),    -- 大师，5x5，150步，未使用撤销
(@expert1_id, 5, 165, 0),    -- 专家，5x5，165步，未使用撤销
(@pro1_id, 5, 180, 0),      -- 专业选手，5x5，180步，未使用撤销
(@pro2_id, 5, 195, 0),      -- 职业玩家，5x5，195步，未使用撤销
(@player1_id, 5, 210, 0),    -- 勇敢的冒险者，5x5，210步，未使用撤销
(@gamer2_id, 5, 225, 0),     -- 休闲玩家，5x5，225步，未使用撤销
(@player2_id, 5, 240, 0),    -- 智慧之剑，5x5，240步，未使用撤销
(@player3_id, 5, 255, 0);    -- 闪电侠，5x5，255步，未使用撤销

-- 插入一些中等水平的玩家数据
INSERT INTO step_rankings (user_id, grid_size, step_count, used_undo) VALUES
(@player4_id, 3, 85, 0),     -- 策略大师，3x3，85步
(@testuser_id, 3, 92, 0),   -- 测试用户，3x3，92步
(@player5_id, 3, 110, 0),    -- 拼图新手，3x3，110步
(@noob1_id, 3, 125, 0),     -- 初学者小王，3x3，125步
(@player4_id, 4, 180, 0),    -- 策略大师，4x4，180步
(@testuser_id, 4, 220, 0),   -- 测试用户，4x4，220步
(@player5_id, 4, 280, 0),    -- 拼图新手，4x4，280步
(@noob1_id, 4, 320, 0);     -- 初学者小王，4x4，320步

-- 查看数据统计
SELECT '=== 用户统计 ===' as category;
SELECT COUNT(*) as total_users FROM users;

SELECT '=== 关卡排行榜统计 ===' as category;
SELECT COUNT(*) as total_records FROM level_rankings;
SELECT 
    MIN(max_level) as min_level,
    MAX(max_level) as max_level,
    ROUND(AVG(max_level), 1) as avg_level
FROM level_rankings;

SELECT '=== 时间排行榜统计 ===' as category;
SELECT COUNT(*) as total_records FROM time_rankings;
SELECT 
    grid_size,
    MIN(time_seconds) as best_time,
    MAX(time_seconds) as worst_time,
    ROUND(AVG(time_seconds), 1) as avg_time,
    COUNT(*) as count
FROM time_rankings 
GROUP BY grid_size;

SELECT '=== 步数排行榜统计 ===' as category;
SELECT COUNT(*) as total_records FROM step_rankings;
SELECT 
    grid_size,
    MIN(step_count) as best_steps,
    MAX(step_count) as worst_steps,
    ROUND(AVG(step_count), 1) as avg_steps,
    COUNT(*) as count
FROM step_rankings 
GROUP BY grid_size;

SELECT '=== 撤销功能使用统计 ===' as category;
SELECT 
    used_undo,
    COUNT(*) as count
FROM time_rankings 
GROUP BY used_undo;

SELECT 
    used_undo,
    COUNT(*) as count
FROM step_rankings 
GROUP BY used_undo;

-- 显示前5名玩家（按各种指标）
SELECT '=== 关卡排行榜前5名 ===' as category;
SELECT 
    u.nickname,
    r.max_level,
    r.update_time
FROM level_rankings r
JOIN users u ON r.user_id = u.id
ORDER BY r.max_level DESC
LIMIT 5;

SELECT '=== 3x3时间排行榜前5名（未使用撤销）===' as category;
SELECT 
    u.nickname,
    r.time_seconds,
    r.used_undo
FROM time_rankings r
JOIN users u ON r.user_id = u.id
WHERE r.grid_size = 3 AND r.used_undo = 0
ORDER BY r.time_seconds ASC
LIMIT 5;

SELECT '=== 3x3步数排行榜前5名（未使用撤销）===' as category;
SELECT 
    u.nickname,
    r.step_count,
    r.used_undo
FROM step_rankings r
JOIN users u ON r.user_id = u.id
WHERE r.grid_size = 3 AND r.used_undo = 0
ORDER BY r.step_count ASC
LIMIT 5;

-- 完成！
SELECT '=== 测试数据生成完成！===' as category;