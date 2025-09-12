#ifndef LEVELCONFIG_H
#define LEVELCONFIG_H

#include <QString>

// 关卡配置结构
struct LevelConfig {
    int level;
    int rows;
    int cols;
    QString name;
    bool unlocked;
    bool completed;
    int bestTime;  // 最佳时间（秒）
    int bestMoves; // 最佳步数
};

#endif // LEVELCONFIG_H