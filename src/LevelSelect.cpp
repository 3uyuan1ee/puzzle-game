#include "LevelSelect.h"
#include "ui_LevelSelect.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QGroupBox>
#include <QSettings>
#include <QDebug>
#include <QMessageBox>
#include <QRandomGenerator>

LevelSelect::LevelSelect(int maxUnlockedLevel, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LevelSelect)
    , m_maxUnlockedLevel(maxUnlockedLevel)
{
    ui->setupUi(this);
    
    // 设置窗口标题
    setWindowTitle("关卡选择");
    setFixedSize(800, 600);
    
    // 初始化关卡
    initLevels();
    loadProgress();
    
    // 创建关卡按钮
    createLevelButtons();
    
    // 连接信号
    connect(ui->btnBack, &QPushButton::clicked, this, &LevelSelect::on_btnBack_clicked);
    connect(ui->btnResetProgress, &QPushButton::clicked, this, &LevelSelect::on_btnResetProgress_clicked);
}

LevelSelect::~LevelSelect()
{
    delete ui;
}

void LevelSelect::initLevels()
{
    m_levels.clear();
    
    // 预定义一些基础关卡
    m_levels.append({1, 2, 2, "入门 2x2", true, false, 0, 0});
    m_levels.append({2, 3, 3, "简单 3x3", false, false, 0, 0});
    m_levels.append({3, 3, 4, "初级 3x4", false, false, 0, 0});
    m_levels.append({4, 4, 4, "标准 4x4", false, false, 0, 0});
    m_levels.append({5, 4, 5, "进阶 4x5", false, false, 0, 0});
    
    // 第6关开始随机生成
    for (int i = 6; i <= 50; i++) {
        int rows, cols;
        
        // 根据关卡数增加难度
        if (i <= 10) {
            // 6-10关：3x3 到 5x5
            rows = QRandomGenerator::global()->bounded(3, 6);
            cols = QRandomGenerator::global()->bounded(3, 6);
        } else if (i <= 20) {
            // 11-20关：4x4 到 6x6
            rows = QRandomGenerator::global()->bounded(4, 7);
            cols = QRandomGenerator::global()->bounded(4, 7);
        } else if (i <= 30) {
            // 21-30关：5x5 到 7x7
            rows = QRandomGenerator::global()->bounded(5, 8);
            cols = QRandomGenerator::global()->bounded(5, 8);
        } else {
            // 30关以后：6x6 到 8x8
            rows = QRandomGenerator::global()->bounded(6, 9);
            cols = QRandomGenerator::global()->bounded(6, 9);
        }
        
        // 避免过于极端的比例
        if (rows * cols > 64) {
            rows = 8;
            cols = 8;
        }
        
        m_levels.append({i, rows, cols, QString("挑战 %1x%2").arg(rows).arg(cols), false, false, 0, 0});
    }
    
    // 根据最大解锁关卡设置解锁状态
    for (int i = 0; i < m_levels.size() && i < m_maxUnlockedLevel; i++) {
        m_levels[i].unlocked = true;
    }
}

void LevelSelect::createLevelButtons()
{
    // 清除现有按钮和布局
    qDeleteAll(m_levelButtons);
    m_levelButtons.clear();
    
    // 清除现有布局
    if (ui->scrollAreaWidgetContents->layout()) {
        QLayout *oldLayout = ui->scrollAreaWidgetContents->layout();
        delete oldLayout;
    }
    
    // 创建新的网格布局
    QGridLayout *gridLayout = new QGridLayout(ui->scrollAreaWidgetContents);
    gridLayout->setSpacing(10);
    gridLayout->setContentsMargins(10, 10, 10, 10);
    
    // 创建关卡按钮
    for (int i = 0; i < m_levels.size(); i++) {
        const LevelConfig& level = m_levels[i];
        
        QPushButton *btn = new QPushButton();
        btn->setFixedSize(120, 80);
        btn->setObjectName(QString("levelBtn_%1").arg(i + 1));
        
        // 设置按钮文本
        QString btnText = QString("第%1关\n%2").arg(level.level).arg(level.name);
        if (level.completed) {
            btnText += QString("\n⭐ %1步 %2秒").arg(level.bestMoves).arg(level.bestTime);
        }
        btn->setText(btnText);
        
        // 设置按钮样式
        updateButtonStyle(btn, level);
        
        // 连接点击信号
        connect(btn, &QPushButton::clicked, this, &LevelSelect::onLevelButtonClicked);
        
        // 添加到布局
        int row = i / 6;
        int col = i % 6;
        gridLayout->addWidget(btn, row, col);
        
        m_levelButtons.append(btn);
    }
    
    // 设置滚动区域的内容
    ui->scrollAreaWidgetContents->setLayout(gridLayout);
}

void LevelSelect::updateButtonStyle(QPushButton* button, const LevelConfig& level)
{
    QString style;
    
    if (!level.unlocked) {
        // 未解锁的关卡
        style = "QPushButton { "
                "background-color: #cccccc; "
                "color: #666666; "
                "border: 2px solid #999999; "
                "border-radius: 8px; "
                "font: bold 12px; "
                "}"
                "QPushButton:disabled { "
                "background-color: #cccccc; "
                "color: #666666; "
                "}";
        button->setEnabled(false);
    } else if (level.completed) {
        // 已完成的关卡
        style = "QPushButton { "
                "background-color: #90EE90; "
                "color: #006400; "
                "border: 2px solid #32CD32; "
                "border-radius: 8px; "
                "font: bold 12px; "
                "}"
                "QPushButton:hover { "
                "background-color: #7FDD7F; "
                "border: 2px solid #228B22; "
                "}";
        button->setEnabled(true);
    } else {
        // 已解锁但未完成的关卡
        style = "QPushButton { "
                "background-color: #87CEEB; "
                "color: #000080; "
                "border: 2px solid #4682B4; "
                "border-radius: 8px; "
                "font: bold 12px; "
                "}"
                "QPushButton:hover { "
                "background-color: #6BB6D6; "
                "border: 2px solid #4169E1; "
                "}";
        button->setEnabled(true);
    }
    
    button->setStyleSheet(style);
}

void LevelSelect::onLevelButtonClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    
    // 获取关卡索引
    QString btnName = btn->objectName();
    int index = btnName.split("_").last().toInt() - 1;
    
    if (index >= 0 && index < m_levels.size()) {
        const LevelConfig& level = m_levels[index];
        if (level.unlocked) {
            emit levelSelected(level.level, level.rows, level.cols,this);
        }
    }
}

void LevelSelect::on_btnBack_clicked()
{
    emit backToMenu();
    close();
}

void LevelSelect::on_btnResetProgress_clicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "重置进度", "确定要重置所有关卡进度吗？这将清除所有解锁记录和最佳成绩。",
        QMessageBox::Yes|QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // 重置所有关卡进度
        for (auto& level : m_levels) {
            if (level.level == 1) {
                level.unlocked = true;
                level.completed = false;
                level.bestTime = 0;
                level.bestMoves = 0;
            } else {
                level.unlocked = false;
                level.completed = false;
                level.bestTime = 0;
                level.bestMoves = 0;
            }
        }
        
        m_maxUnlockedLevel = 1;
        saveProgress();
        createLevelButtons();
        
        // 通知主菜单进度已重置
        emit progressReset();
        
        QMessageBox::information(this, "重置完成", "所有关卡进度已重置！");
    }
}

void LevelSelect::loadProgress()
{
    QSettings settings("PuzzleGame", "Progress");
    m_maxUnlockedLevel = settings.value("maxUnlockedLevel", 1).toInt();
    
    // 加载每个关卡的进度
    for (auto& level : m_levels) {
        QString key = QString("level_%1").arg(level.level);
        level.completed = settings.value(key + "_completed", false).toBool();
        level.bestTime = settings.value(key + "_bestTime", 0).toInt();
        level.bestMoves = settings.value(key + "_bestMoves", 0).toInt();
    }
    
    // 更新解锁状态
    for (int i = 0; i < m_levels.size() && i < m_maxUnlockedLevel; i++) {
        m_levels[i].unlocked = true;
    }
}

void LevelSelect::saveProgress()
{
    QSettings settings("PuzzleGame", "Progress");
    settings.setValue("maxUnlockedLevel", m_maxUnlockedLevel);
    
    // 保存每个关卡的进度
    for (const auto& level : m_levels) {
        QString key = QString("level_%1").arg(level.level);
        settings.setValue(key + "_completed", level.completed);
        settings.setValue(key + "_bestTime", level.bestTime);
        settings.setValue(key + "_bestMoves", level.bestMoves);
    }
}

void LevelSelect::setLevels(const QVector<LevelConfig>& levels)
{
    m_levels = levels;
    createLevelButtons();
}

void LevelSelect::updateLevelButtons()
{
    for (int i = 0; i < m_levelButtons.size() && i < m_levels.size(); i++) {
        updateButtonStyle(m_levelButtons[i], m_levels[i]);
        
        // 更新按钮文本
        QString btnText = QString("第%1关\n%2").arg(m_levels[i].level).arg(m_levels[i].name);
        if (m_levels[i].completed) {
            btnText += QString("\n⭐ %1步 %2秒").arg(m_levels[i].bestMoves).arg(m_levels[i].bestTime);
        }
        m_levelButtons[i]->setText(btnText);
    }
}
