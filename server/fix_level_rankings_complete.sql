-- 完整修复level_rankings表的脚本
-- 1. 清理重复数据
-- 2. 添加唯一键约束

-- 步骤1：清理重复数据，为每个用户只保留最高关卡数的记录
CREATE TEMPORARY TABLE temp_level_rankings AS
SELECT 
    MIN(id) as id,
    user_id,
    MAX(max_level) as max_level,
    NOW() as update_time
FROM level_rankings
GROUP BY user_id;

-- 删除原有数据
DELETE FROM level_rankings;

-- 插入清理后的数据
INSERT INTO level_rankings (id, user_id, max_level, update_time)
SELECT id, user_id, max_level, update_time
FROM temp_level_rankings;

-- 删除临时表
DROP TEMPORARY TABLE temp_level_rankings;

-- 步骤2：添加唯一键约束
ALTER TABLE level_rankings 
ADD UNIQUE KEY unique_user_id (user_id);

-- 验证修复结果
SELECT user_id, MAX(max_level) as max_level, COUNT(*) as record_count
FROM level_rankings
GROUP BY user_id
HAVING COUNT(*) > 1;