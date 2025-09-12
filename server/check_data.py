#!/usr/bin/env python3
import mysql.connector
import sys

def check_database():
    try:
        # 连接到云服务器数据库
        conn = mysql.connector.connect(
            host="43.143.238.230",
            user="root",
            password="123456",
            database="puzzle_game"
        )
        
        cursor = conn.cursor(dictionary=True)
        
        # 检查用户表
        print("=== 用户表数据 ===")
        cursor.execute("SELECT COUNT(*) as user_count FROM users")
        user_count = cursor.fetchone()
        print(f"用户总数: {user_count['user_count']}")
        
        cursor.execute("SELECT id, username, nickname FROM users LIMIT 5")
        users = cursor.fetchall()
        for user in users:
            print(f"ID: {user['id']}, 用户名: {user['username']}, 昵称: {user['nickname']}")
        
        # 检查关卡排行榜
        print("\n=== 关卡排行榜数据 ===")
        cursor.execute("SELECT COUNT(*) as level_count FROM level_rankings")
        level_count = cursor.fetchone()
        print(f"关卡排行榜记录数: {level_count['level_count']}")
        
        cursor.execute("""
            SELECT r.max_level, u.nickname, r.update_time 
            FROM level_rankings r 
            JOIN users u ON r.user_id = u.id 
            ORDER BY r.max_level DESC 
            LIMIT 5
        """)
        levels = cursor.fetchall()
        for level in levels:
            print(f"玩家: {level['nickname']}, 最高关卡: {level['max_level']}, 更新时间: {level['update_time']}")
        
        # 检查时间排行榜
        print("\n=== 时间排行榜数据 ===")
        cursor.execute("SELECT COUNT(*) as time_count FROM time_rankings")
        time_count = cursor.fetchone()
        print(f"时间排行榜记录数: {time_count['time_count']}")
        
        cursor.execute("""
            SELECT r.grid_size, r.time_seconds, r.used_undo, u.nickname, r.create_time
            FROM time_rankings r 
            JOIN users u ON r.user_id = u.id 
            WHERE r.grid_size = 3 AND r.used_undo = 0
            ORDER BY r.time_seconds ASC 
            LIMIT 5
        """)
        times = cursor.fetchall()
        for time in times:
            undo_status = "使用撤销" if time['used_undo'] else "未使用撤销"
            print(f"玩家: {time['nickname']}, 网格: {time['grid_size']}x{time['grid_size']}, 时间: {time['time_seconds']}秒, {undo_status}")
        
        # 检查步数排行榜
        print("\n=== 步数排行榜数据 ===")
        cursor.execute("SELECT COUNT(*) as step_count FROM step_rankings")
        step_count = cursor.fetchone()
        print(f"步数排行榜记录数: {step_count['step_count']}")
        
        cursor.execute("""
            SELECT r.grid_size, r.step_count, r.used_undo, u.nickname, r.create_time
            FROM step_rankings r 
            JOIN users u ON r.user_id = u.id 
            WHERE r.grid_size = 3 AND r.used_undo = 0
            ORDER BY r.step_count ASC 
            LIMIT 5
        """)
        steps = cursor.fetchall()
        for step in steps:
            undo_status = "使用撤销" if step['used_undo'] else "未使用撤销"
            print(f"玩家: {step['nickname']}, 网格: {step['grid_size']}x{step['grid_size']}, 步数: {step['step_count']}, {undo_status}")
        
        cursor.close()
        conn.close()
        
    except mysql.connector.Error as err:
        print(f"数据库错误: {err}")
        sys.exit(1)

if __name__ == "__main__":
    check_database()