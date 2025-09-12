#include "DlgMenu.h"
#include "play4x4.h"
#include "IrregularPuzzle.h"
#include "ui_DlgMenu.h"
#include <QTimer>
#include <QImage>
#include <QPixmap>
#include <QFileDialog>
#include <QMessageBox>
#include <QScreen>
#include <qpushbutton.h>
#include <QGuiApplication>
#include <QStandardPaths>
#include <QDebug>
#include <QSettings>
#include <QRandomGenerator>
#include <QCoreApplication>
#include <QPointer>

int save=1;
int state=3;
QString m_customImagePath;
QString _strPos;

QString DlgMenu::diyImagePath = "";

void DlgMenu::connectGameSignals()
{
    connect(_dlgPlay4, &play4x4::sig_back, this, [this]() {
        this->show();
    });
    
    connect(_dlgPlay4, &play4x4::sig_restart, this, [this]() {
        // 重新开始当前关卡
        LevelConfig current = levels[currentLevel - 1];
        _dlgPlay4 = new play4x4(current.rows, current.cols, this, network_client, musicPlayer);
        
        // 重新连接信号
        connect(_dlgPlay4, &play4x4::sig_successRestart, this, [this]() {
            // 通关成功，解锁下一关
            if (currentLevel == maxUnlockedLevel) {
                maxUnlockedLevel++;
                currentLevel = maxUnlockedLevel;
                saveProgress();
            } else {
                currentLevel++;
                if (currentLevel > levels.size()) {
                    LevelConfig newLevel = generateRandomLevel();
                    newLevel.level = levels.size() + 1;
                    levels.append(newLevel);
                    currentLevel = levels.size();
                }
            }
            updateUI();
        });
        
        connectGameSignals();
        
        _dlgPlay4->start();
        _dlgPlay4->show();
    });
}

DlgMenu::DlgMenu(QWidget *parent) :
    ui(new Ui::dlgMenu),
    _dlgPlay4(nullptr),
    _dlgIrregular(nullptr),
    m_levelSelect(nullptr),
    m_customMode(nullptr),
    network_client(nullptr),
    ranking_dialog(nullptr),
    currentLevel(1),
    maxUnlockedLevel(1),
    musicPlayer(nullptr),
    audioOutput(nullptr),
    volumeSlider(nullptr),
    musicStatusLabel(nullptr),
    btnImportMusic(nullptr),
    btnPlayPause(nullptr),
    btnStopMusic(nullptr),
    musicControlWidget(nullptr),
    isMusicPlaying(false)
{
    Q_UNUSED(parent)
    ui->setupUi(this);
    
    // 连接按钮信号
    connect(ui->btStart, &QPushButton::clicked, this, &DlgMenu::on_btStart_clicked);
    connect(ui->btnExit, &QPushButton::clicked, this, &DlgMenu::on_btnExit_clicked);
    connect(ui->btnLevelSelect, &QPushButton::clicked, this, &DlgMenu::on_btnLevelSelect_clicked);
    connect(ui->btnCustomMode, &QPushButton::clicked, this, &DlgMenu::on_btnCustomMode_clicked);
    connect(ui->btnRanking, &QPushButton::clicked, this, &DlgMenu::on_btnRanking_clicked);
    
    // 初始化网络客户端
    initNetwork();
    
    // 初始化关卡系统
    initLevels();
    loadProgress();
    updateUI();
    
    // 初始化音乐播放器
    initMusicPlayer();
    createMusicControls();
    loadMusicSettings();
}

DlgMenu::~DlgMenu()
{
    // 保存音乐设置
    saveMusicSettings();
    
    // 清理音乐播放器
    cleanupMusicPlayer();
    
    if (network_client) {
        network_client->disconnectFromServer();
        delete network_client;
    }
    if (ranking_dialog) {
        delete ranking_dialog;
    }
    delete ui;//删除了ui指针所指向的Ui::dlgMenu对象
}
// 初始化关卡系统
void DlgMenu::initLevels()
{
    levels.clear();
    
    // 预定义一些基础关卡
    levels.append({1, 2, 2, "入门 2x2", true, false, 0, 0});
    levels.append({2, 3, 3, "简单 3x3", false, false, 0, 0});
    levels.append({3, 3, 4, "初级 3x4", false, false, 0, 0});
    levels.append({4, 4, 4, "标准 4x4", false, false, 0, 0});
    levels.append({5, 4, 5, "进阶 4x5", false, false, 0, 0});
    
    // 第6关开始随机生成
    for (int i = 6; i <= 50; i++) {
        LevelConfig level = generateRandomLevel();
        level.level = i;
        levels.append(level);
    }
}

// 生成随机关卡配置
LevelConfig DlgMenu::generateRandomLevel()
{
    int rows, cols;
    
    // 根据关卡数增加难度
    if (currentLevel <= 10) {
        // 6-10关：3x3 到 5x5
        rows = QRandomGenerator::global()->bounded(3, 6);
        cols = QRandomGenerator::global()->bounded(3, 6);
    } else if (currentLevel <= 20) {
        // 11-20关：4x4 到 6x6
        rows = QRandomGenerator::global()->bounded(4, 7);
        cols = QRandomGenerator::global()->bounded(4, 7);
    } else if (currentLevel <= 30) {
        // 21-30关：5x5 到 7x7
        rows = QRandomGenerator::global()->bounded(5, 8);
        cols = QRandomGenerator::global()->bounded(5, 8);
    } else {
        // 30关以后：6x6 到 8x8
        rows = QRandomGenerator::global()->bounded(6, 9);
        cols = QRandomGenerator::global()->bounded(6, 9);
    }
    
    // 避免过于极端的比例
    if (rows * cols > 64) {  // 最大64块
        if (rows > cols) {
            rows = 8;
            cols = 8;
        } else {
            rows = 8;
            cols = 8;
        }
    }
    
    LevelConfig config;
    config.rows = rows;
    config.cols = cols;
    config.name = QString("挑战 %1x%2").arg(rows).arg(cols);
    
    return config;
}

// 保存进度
void DlgMenu::saveProgress()
{
    QSettings settings("PuzzleGame", "Progress");
    settings.setValue("maxUnlockedLevel", maxUnlockedLevel);
    settings.setValue("currentLevel", currentLevel);
}

// 加载进度
void DlgMenu::loadProgress()
{
    QSettings settings("PuzzleGame", "Progress");
    maxUnlockedLevel = settings.value("maxUnlockedLevel", 1).toInt();
    currentLevel = settings.value("currentLevel", 1).toInt();
    
    // 确保当前关卡不超过最大解锁关卡
    if (currentLevel > maxUnlockedLevel) {
        currentLevel = maxUnlockedLevel;
    }
}

// 更新UI显示
void DlgMenu::updateUI()
{
    LevelConfig current = levels[currentLevel - 1];
    
    // 更新开始按钮文本
    if (currentLevel == 1) {
        ui->btStart->setText("快速开始");
    } else {
        ui->btStart->setText(QString("第 %1 关").arg(currentLevel));
    }
}

// 快速开始游戏
void DlgMenu::on_btStart_clicked()
{
    this->hide();
    
    // 快速开始使用当前已解锁的最高关卡
    LevelConfig current = levels[currentLevel - 1];
    
    // 如果已有游戏实例，先安全删除
    if (_dlgPlay4) {
        // 使用 QPointer 确保指针安全
        QPointer<play4x4> safeGame = _dlgPlay4;
        _dlgPlay4 = nullptr;
        
        if (safeGame) {
            // 断开所有信号连接
            safeGame->disconnect();
            safeGame->close();
            safeGame->deleteLater();
        }
        
        // 等待对象被删除
        QCoreApplication::processEvents();
    }
    
    // 创建游戏
    _dlgPlay4 = new play4x4(current.rows, current.cols, this, network_client, musicPlayer);
    
    // 连接游戏信号
    connectGameSignals();
    
    // 连接信号以处理通关后的逻辑
    connect(_dlgPlay4, &play4x4::sig_successRestart, this, [this]() {
        // 通关成功，解锁下一关
        if (currentLevel == maxUnlockedLevel) {
            maxUnlockedLevel++;
            currentLevel = maxUnlockedLevel;
            saveProgress();
        } else {
            currentLevel++;
            if (currentLevel > levels.size()) {
                // 所有关卡都通过了，生成新的随机关卡
                LevelConfig newLevel = generateRandomLevel();
                newLevel.level = levels.size() + 1;
                levels.append(newLevel);
                currentLevel = levels.size();
            }
        }
        updateUI();
        
        // 开始下一关
        LevelConfig next = levels[currentLevel - 1];
        _dlgPlay4 = new play4x4(next.rows, next.cols, this, network_client);
        connectGameSignals();
        
        connect(_dlgPlay4, &play4x4::sig_successRestart, this, [this]() {
            // 通关成功，解锁下一关
            if (currentLevel == maxUnlockedLevel) {
                maxUnlockedLevel++;
                currentLevel = maxUnlockedLevel;
                saveProgress();
            } else {
                currentLevel++;
                if (currentLevel > levels.size()) {
                    LevelConfig newLevel = generateRandomLevel();
                    newLevel.level = levels.size() + 1;
                    levels.append(newLevel);
                    currentLevel = levels.size();
                }
            }
            updateUI();
            
            // 开始下一关
            LevelConfig next = levels[currentLevel - 1];
            _dlgPlay4 = new play4x4(next.rows, next.cols, this, network_client);
            connectGameSignals();
            _dlgPlay4->start();
            _dlgPlay4->show();
        });
        
        _dlgPlay4->start();
        _dlgPlay4->show();
    });
    
    _dlgPlay4->start();
    _dlgPlay4->show();
}

// 打开关卡选择
void DlgMenu::on_btnLevelSelect_clicked()
{
    if (!m_levelSelect) {
        m_levelSelect = new LevelSelect(maxUnlockedLevel, this);
        connect(m_levelSelect, &LevelSelect::levelSelected, this, &DlgMenu::onLevelSelected);
        connect(m_levelSelect, &LevelSelect::backToMenu, this, &DlgMenu::onBackToMenu);
        connect(m_levelSelect, &LevelSelect::progressReset, this, &DlgMenu::onProgressReset);
    }
    
    this->hide();
    m_levelSelect->show();
}

// 打开自定义模式
void DlgMenu::on_btnCustomMode_clicked()
{
    if (!m_customMode) {
        m_customMode = new CustomMode(this);
        connect(m_customMode, &CustomMode::startCustomGame, this, &DlgMenu::onStartCustomGame);
        connect(m_customMode, &CustomMode::backToMenu, this, &DlgMenu::onBackToMenu);
    }
    
    this->hide();
    m_customMode->show();
}

// 关卡选择后的处理
void DlgMenu::onLevelSelected(int level, int rows, int cols,LevelSelect *widget)
{
    this->hide();
    
    // 更新当前关卡
    currentLevel = level;
    updateUI();
    
    // 如果已有游戏实例，先安全删除
    if (_dlgPlay4) {
        // 使用 QPointer 确保指针安全
        QPointer<play4x4> safeGame = _dlgPlay4;
        _dlgPlay4 = nullptr;
        
        if (safeGame) {
            // 断开所有信号连接
            safeGame->disconnect();
            safeGame->close();
            safeGame->deleteLater();
        }
        
        // 等待对象被删除
        QCoreApplication::processEvents();
    }
    
    // 创建游戏
    _dlgPlay4 = new play4x4(rows, cols, widget, network_client, nullptr);
    
    // 连接游戏信号
    connectGameSignals();
    
    // 连接信号以处理通关后的逻辑
    connect(_dlgPlay4, &play4x4::sig_successRestart, this, [this]() {
        // 通关成功，解锁下一关
        if (currentLevel == maxUnlockedLevel) {
            maxUnlockedLevel++;
            currentLevel = maxUnlockedLevel;
            saveProgress();
        } else {
            currentLevel++;
            if (currentLevel > levels.size()) {
                // 所有关卡都通过了，生成新的随机关卡
                LevelConfig newLevel = generateRandomLevel();
                newLevel.level = levels.size() + 1;
                levels.append(newLevel);
                currentLevel = levels.size();
            }
        }
        updateUI();
        
        // 开始下一关
        LevelConfig next = levels[currentLevel - 1];
        _dlgPlay4 = new play4x4(next.rows, next.cols, this, network_client);
        connectGameSignals();
        
        connect(_dlgPlay4, &play4x4::sig_successRestart, this, [this]() {
            // 通关成功，解锁下一关
            if (currentLevel == maxUnlockedLevel) {
                maxUnlockedLevel++;
                currentLevel = maxUnlockedLevel;
                saveProgress();
            } else {
                currentLevel++;
                if (currentLevel > levels.size()) {
                    LevelConfig newLevel = generateRandomLevel();
                    newLevel.level = levels.size() + 1;
                    levels.append(newLevel);
                    currentLevel = levels.size();
                }
            }
            updateUI();
            
            // 开始下一关
            LevelConfig next = levels[currentLevel - 1];
            _dlgPlay4 = new play4x4(next.rows, next.cols, this, network_client);
            connectGameSignals();
            _dlgPlay4->start();
            _dlgPlay4->show();
        });
        
        _dlgPlay4->start();
        _dlgPlay4->show();
    });
    
    _dlgPlay4->start();
    _dlgPlay4->show();
}

// 自定义游戏开始后的处理
void DlgMenu::onStartCustomGame(int rows, int cols, const QString& imagePath,CustomMode *widget)
{
    this->hide();
    
    // 设置自定义图片路径
    diyImagePath = imagePath;

    save=0;
    
    // 如果已有游戏实例，先安全删除
    if (_dlgPlay4) {
        // 使用 QPointer 确保指针安全
        QPointer<play4x4> safeGame = _dlgPlay4;
        _dlgPlay4 = nullptr;
        
        if (safeGame) {
            // 断开所有信号连接
            safeGame->disconnect();
            safeGame->close();
            safeGame->deleteLater();
        }
        
        // 等待对象被删除
        QCoreApplication::processEvents();
    }
    
    // 创建游戏
    _dlgPlay4 = new play4x4(rows, cols, widget, network_client, nullptr);
    
    // 使用QPointer来安全地管理widget生命周期
    QPointer<CustomMode> safeWidget = widget;
    
    connect(_dlgPlay4, &play4x4::sig_back, this, [safeWidget]() {
        if (safeWidget) {
            safeWidget->show();
        }
    });
    
    connect(_dlgPlay4, &play4x4::sig_restart, this, [this, rows, cols, safeWidget]() {
        // 安全删除当前游戏实例
        if (_dlgPlay4) {
            // 使用 QPointer 确保指针安全
            QPointer<play4x4> safeGame = _dlgPlay4;
            _dlgPlay4 = nullptr;
            
            if (safeGame) {
                safeGame->disconnect();
                safeGame->close();
                safeGame->deleteLater();
            }
            QCoreApplication::processEvents();
        }
        
        // 重新开始自定义游戏
        if (safeWidget) {
            _dlgPlay4 = new play4x4(rows, cols, safeWidget, network_client);
            
            connect(_dlgPlay4, &play4x4::sig_back, this, [safeWidget]() {
                if (safeWidget) {
                    safeWidget->show();
                }
            });
            
            connect(_dlgPlay4, &play4x4::sig_restart, this, [this, rows, cols, safeWidget]() {
                // 安全删除当前游戏实例
                if (_dlgPlay4) {
                    // 使用 QPointer 确保指针安全
                    QPointer<play4x4> safeGame = _dlgPlay4;
                    _dlgPlay4 = nullptr;
                    
                    if (safeGame) {
                        safeGame->disconnect();
                        safeGame->close();
                        safeGame->deleteLater();
                    }
                    QCoreApplication::processEvents();
                }
                
                if (safeWidget) {
                    _dlgPlay4 = new play4x4(rows, cols, safeWidget, network_client);
                    connect(_dlgPlay4, &play4x4::sig_back, this, [safeWidget]() {
                        if (safeWidget) {
                            safeWidget->show();
                        }
                    });
                    connect(_dlgPlay4, &play4x4::sig_restart, this, [this, rows, cols, safeWidget]() {
                        if (_dlgPlay4) {
                            // 使用 QPointer 确保指针安全
                            QPointer<play4x4> safeGame = _dlgPlay4;
                            _dlgPlay4 = nullptr;
                            
                            if (safeGame) {
                                safeGame->disconnect();
                                safeGame->close();
                                safeGame->deleteLater();
                            }
                            QCoreApplication::processEvents();
                        }
                        
                        if (safeWidget) {
                            _dlgPlay4 = new play4x4(rows, cols, safeWidget, network_client);
                            _dlgPlay4->start();
                            _dlgPlay4->show();
                        }
                    });
                    _dlgPlay4->start();
                    _dlgPlay4->show();
                }
            });
            
            _dlgPlay4->start();
            _dlgPlay4->show();
        }
    });
    
    _dlgPlay4->start();
    _dlgPlay4->show();
}

// 返回主菜单
void DlgMenu::onBackToMenu()
{
    this->show();
}

// 进度重置后的处理
void DlgMenu::onProgressReset()
{
    // 重新加载进度
    loadProgress();
    updateUI();
}
//退出游戏
void DlgMenu::on_btnExit_clicked()
{
    this->close();
}

// 初始化网络客户端
void DlgMenu::initNetwork()
{
    network_client = new NetworkClient(this);
    ranking_dialog = new RankingDialog(network_client, this);
    
    // 连接排行榜返回信号
    connect(ranking_dialog, &RankingDialog::backToMenu, this, &DlgMenu::onBackFromRanking);
    
    // 尝试连接服务器
    network_client->connectToServer("43.143.238.230", 8080);
}

// 排行榜按钮点击
void DlgMenu::on_btnRanking_clicked()
{
    if (ranking_dialog) {
        ranking_dialog->show();
        ranking_dialog->raise();
        ranking_dialog->activateWindow();
    }
}

// 从排行榜返回
void DlgMenu::onBackFromRanking()
{
    this->show();
}


// void DlgMenu::btnphoto_clicked()
// {
//     save=0;
//     QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), QDir::homePath(), tr("Image Files (*.png *.jpg *.bmp)"));
//     if (!fileName.isEmpty()) {
//         // 检查保存目录是否存在，不存在则创建
//         QDir dir("W://img/");
//         if (!dir.exists()) {
//             dir.mkpath(".");
//         }
//         QString baseName = QFileInfo(fileName).baseName();
//         QString newFileName = dir.filePath( "1.png");
//         QPixmap pixmap(fileName);
//         // 将图片保存到指定路径
//         pixmap.save(newFileName);
//     }
// }



// void DlgMenu::btnphoto_clicked()
// {
//     save = 0;                       // 标记：用户已选图

//     /* 1. 让用户选一张图 */
//     QString srcPath = QFileDialog::getOpenFileName(
//         this,
//         tr("选择图片"),
//         QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
//         tr("Images (*.png *.jpg *.bmp *.jpeg)"));
//     if (srcPath.isEmpty())
//         return;                     // 用户取消

//     /* 2. 计算保存目录：程序运行目录下的 img 子目录
//          这样无论有没有 W 盘都能写 */
//     QDir dstDir(QCoreApplication::applicationDirPath() + "/img");
//     if (!dstDir.exists())
//         dstDir.mkpath(".");         // 自动创建

//     /* 3. 用时间戳拼一个不重复的文件名，防止覆盖
//          例如：20240828153045.png */
//     QString timeStr = QDateTime::currentDateTime()
//                           .toString("yyyyMMddhhmmss");
//     QString dstName = dstDir.filePath(timeStr + ".png");

//     /* 4. 拷贝/转换并保存 */
//     QPixmap pix(srcPath);
//     bool flag = pix.save(dstName, "PNG");
//     if (flag) {
//         m_customImagePath = dstName;   // 记住
//         QMessageBox::information(this, tr("保存成功"), tr("图片已保存到：\n%1").arg(dstName));
//     } else {
//         QMessageBox::critical(this,
//                               tr("保存失败"),
//                               tr("无法写入文件：\n%1").arg(dstName));
//     }
// }

void DlgMenu::on_btnJigsawMode_clicked()
{
    this->hide();
    
    // 创建异形拼图游戏窗口
    _dlgIrregular = new IrregularPuzzle(this, network_client, musicPlayer);
    _dlgIrregular->setWindowFlags(Qt::Dialog);
    _dlgIrregular->setAttribute(Qt::WA_DeleteOnClose);
    
    // 连接信号
    connect(_dlgIrregular, &IrregularPuzzle::sig_back, this, [this]() {
        this->show();
    });
    
    connect(_dlgIrregular, &IrregularPuzzle::sig_restart, this, [this]() {
        // 重新开始异形拼图
        if (_dlgIrregular) {
            _dlgIrregular->deleteLater();
        }
        _dlgIrregular = new IrregularPuzzle(this, network_client, musicPlayer);
        connect(_dlgIrregular, &IrregularPuzzle::sig_back, this, [this]() {
            this->show();
        });
        _dlgIrregular->show();
    });
    
    connect(_dlgIrregular, &IrregularPuzzle::sig_return, this, [this]() {
        this->show();
    });
    
    _dlgIrregular->start();
    _dlgIrregular->show();
}

void DlgMenu::onBackFromIrregular()
{
    this->show();
    if (_dlgIrregular) {
        _dlgIrregular->deleteLater();
        _dlgIrregular = nullptr;
    }
}

// 初始化音乐播放器
void DlgMenu::initMusicPlayer()
{
    // 先将指针设为nullptr
    musicPlayer = nullptr;
    audioOutput = nullptr;
    volumeSlider = nullptr;
    musicStatusLabel = nullptr;
    btnImportMusic = nullptr;
    btnPlayPause = nullptr;
    btnStopMusic = nullptr;
    musicControlWidget = nullptr;
    
    try {
        // 创建音乐播放器
        musicPlayer = new QMediaPlayer(this);
        audioOutput = new QAudioOutput(this);
        
        // 连接音频输出到媒体播放器
        musicPlayer->setAudioOutput(audioOutput);
        
        // 设置默认音量
        audioOutput->setVolume(0.5);
        
        // 连接状态变化信号
        connect(musicPlayer, &QMediaPlayer::playbackStateChanged, 
                this, &DlgMenu::onMusicStateChanged);
        
        // 设置循环播放
        musicPlayer->setLoops(QMediaPlayer::Infinite);
        
        isMusicPlaying = false;
        
    } catch (const std::exception& e) {
        qWarning() << "Failed to initialize music player:" << e.what();
        cleanupMusicPlayer();
    }
}

// 清理音乐播放器资源
void DlgMenu::cleanupMusicPlayer()
{
    if (musicPlayer) {
        musicPlayer->stop();
        delete musicPlayer;
        musicPlayer = nullptr;
    }
    
    if (audioOutput) {
        delete audioOutput;
        audioOutput = nullptr;
    }
    
    if (musicControlWidget) {
        delete musicControlWidget;
        musicControlWidget = nullptr;
    }
    
    // 重置所有指针
    volumeSlider = nullptr;
    musicStatusLabel = nullptr;
    btnImportMusic = nullptr;
    btnPlayPause = nullptr;
    btnStopMusic = nullptr;
    
    isMusicPlaying = false;
}

// 创建音乐控制界面
void DlgMenu::createMusicControls()
{
    // 如果音乐播放器初始化失败，不创建控件
    if (!musicPlayer || !audioOutput) {
        qWarning() << "Music player not initialized, skipping control creation";
        return;
    }
    
    try {
        // 创建音乐控制容器
        musicControlWidget = new QWidget(this);
        musicControlWidget->setObjectName("musicControlWidget");
        
        QHBoxLayout *musicLayout = new QHBoxLayout(musicControlWidget);
        musicLayout->setContentsMargins(5, 5, 5, 5);
        musicLayout->setSpacing(5);
        
        // 创建导入音乐按钮
        btnImportMusic = new QPushButton("🎵 导入音乐", musicControlWidget);
        btnImportMusic->setObjectName("btnImportMusic");
        btnImportMusic->setMinimumWidth(100);
        btnImportMusic->setMaximumWidth(120);
        
        // 创建播放/暂停按钮
        btnPlayPause = new QPushButton("▶️ 播放", musicControlWidget);
        btnPlayPause->setObjectName("btnPlayPause");
        btnPlayPause->setMinimumWidth(80);
        btnPlayPause->setMaximumWidth(100);
        btnPlayPause->setEnabled(false);
        
        // 创建停止按钮
        btnStopMusic = new QPushButton("⏹️ 停止", musicControlWidget);
        btnStopMusic->setObjectName("btnStopMusic");
        btnStopMusic->setMinimumWidth(80);
        btnStopMusic->setMaximumWidth(100);
        btnStopMusic->setEnabled(false);
        
        // 创建音量滑块
        volumeSlider = new QSlider(Qt::Horizontal, musicControlWidget);
        volumeSlider->setObjectName("volumeSlider");
        volumeSlider->setMinimumWidth(80);
        volumeSlider->setMaximumWidth(120);
        volumeSlider->setRange(0, 100);
        volumeSlider->setValue(50);
        
        // 创建音乐状态标签
        musicStatusLabel = new QLabel("未选择音乐", musicControlWidget);
        musicStatusLabel->setObjectName("musicStatusLabel");
        musicStatusLabel->setMinimumWidth(100);
        musicStatusLabel->setMaximumWidth(150);
        
        // 添加控件到布局
        musicLayout->addWidget(btnImportMusic);
        musicLayout->addWidget(btnPlayPause);
        musicLayout->addWidget(btnStopMusic);
        musicLayout->addWidget(volumeSlider);
        musicLayout->addWidget(musicStatusLabel);
        musicLayout->addStretch();
        
        // 添加到主布局 - 确保主布局存在
        if (ui->verticalLayout) {
            ui->verticalLayout->insertWidget(ui->verticalLayout->count() - 1, musicControlWidget);
        }
        
        // 手动连接信号和槽（动态创建的控件需要手动连接）
        connect(btnImportMusic, &QPushButton::clicked, this, &DlgMenu::on_btnImportMusic_clicked);
        connect(btnPlayPause, &QPushButton::clicked, this, &DlgMenu::on_btnPlayPause_clicked);
        connect(btnStopMusic, &QPushButton::clicked, this, &DlgMenu::on_btnStopMusic_clicked);
        
        // 音量滑块需要手动连接，因为不是标准的clicked信号
        connect(volumeSlider, &QSlider::valueChanged, this, &DlgMenu::onVolumeChanged);
        
    } catch (const std::exception& e) {
        // 如果初始化失败，禁用音乐功能
        musicPlayer = nullptr;
        audioOutput = nullptr;
        musicControlWidget = nullptr;
        btnImportMusic = nullptr;
        btnPlayPause = nullptr;
        btnStopMusic = nullptr;
        volumeSlider = nullptr;
        musicStatusLabel = nullptr;
        
        qWarning() << "Failed to initialize music controls:" << e.what();
    }
}

// 导入音乐
void DlgMenu::on_btnImportMusic_clicked()
{
    if (!musicPlayer || !audioOutput) {
        QMessageBox::warning(this, "错误", "音乐播放器未正确初始化");
        return;
    }
    
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("选择音乐文件"),
        QStandardPaths::writableLocation(QStandardPaths::MusicLocation),
        tr("音乐文件 (*.mp3 *.wav *.ogg *.flac *.m4a *.wma)")
    );
    
    if (!fileName.isEmpty()) {
        currentMusicPath = fileName;
        musicPlayer->setSource(QUrl::fromLocalFile(fileName));
        
        // 更新界面状态
        if (musicStatusLabel) {
            QFileInfo fileInfo(fileName);
            QString displayName = fileInfo.fileName();
            if (displayName.length() > 20) {
                displayName = displayName.left(17) + "...";
            }
            musicStatusLabel->setText("已选择: " + displayName);
        }
        
        // 启用播放控制按钮
        if (btnPlayPause) btnPlayPause->setEnabled(true);
        if (btnStopMusic) btnStopMusic->setEnabled(true);
        
        // 保存设置
        saveMusicSettings();
    }
}

// 播放/暂停音乐
void DlgMenu::on_btnPlayPause_clicked()
{
    if (!musicPlayer) {
        QMessageBox::warning(this, "错误", "音乐播放器未正确初始化");
        return;
    }
    
    if (isMusicPlaying) {
        musicPlayer->pause();
        isMusicPlaying = false;
        if (btnPlayPause) btnPlayPause->setText("▶️ 播放");
    } else {
        musicPlayer->play();
        isMusicPlaying = true;
        if (btnPlayPause) btnPlayPause->setText("⏸️ 暂停");
    }
}

// 停止音乐
void DlgMenu::on_btnStopMusic_clicked()
{
    if (!musicPlayer) {
        QMessageBox::warning(this, "错误", "音乐播放器未正确初始化");
        return;
    }
    
    musicPlayer->stop();
    isMusicPlaying = false;
    if (btnPlayPause) btnPlayPause->setText("▶️ 播放");
}

// 音量变化
void DlgMenu::onVolumeChanged(int volume)
{
    if (audioOutput) {
        audioOutput->setVolume(volume / 100.0);
        saveMusicSettings();
    }
}

// 音乐状态变化
void DlgMenu::onMusicStateChanged(QMediaPlayer::PlaybackState state)
{
    switch (state) {
    case QMediaPlayer::PlayingState:
        isMusicPlaying = true;
        if (btnPlayPause) btnPlayPause->setText("⏸️ 暂停");
        break;
    case QMediaPlayer::PausedState:
    case QMediaPlayer::StoppedState:
        isMusicPlaying = false;
        if (btnPlayPause) btnPlayPause->setText("▶️ 播放");
        break;
    }
}

// 保存音乐设置
void DlgMenu::saveMusicSettings()
{
    QSettings settings("PuzzleGame", "MusicSettings");
    settings.setValue("lastMusicPath", currentMusicPath);
    settings.setValue("volume", volumeSlider ? volumeSlider->value() : 50);
    settings.setValue("isPlaying", isMusicPlaying);
}

// 加载音乐设置
void DlgMenu::loadMusicSettings()
{
    if (!musicPlayer || !audioOutput) {
        return;
    }
    
    QSettings settings("PuzzleGame", "MusicSettings");
    currentMusicPath = settings.value("lastMusicPath", "").toString();
    int volume = settings.value("volume", 50).toInt();
    bool wasPlaying = settings.value("isPlaying", false).toBool();
    
    if (volumeSlider) {
        volumeSlider->setValue(volume);
    }
    
    // 如果有之前的音乐文件，自动加载
    if (!currentMusicPath.isEmpty()) {
        QFile file(currentMusicPath);
        if (file.exists()) {
            musicPlayer->setSource(QUrl::fromLocalFile(currentMusicPath));
            
            if (musicStatusLabel) {
                QFileInfo fileInfo(currentMusicPath);
                QString displayName = fileInfo.fileName();
                if (displayName.length() > 20) {
                    displayName = displayName.left(17) + "...";
                }
                musicStatusLabel->setText("已选择: " + displayName);
            }
            
            // 启用播放控制按钮
            if (btnPlayPause) btnPlayPause->setEnabled(true);
            if (btnStopMusic) btnStopMusic->setEnabled(true);
            
            // 如果之前正在播放，自动恢复播放
            if (wasPlaying) {
                musicPlayer->play();
                isMusicPlaying = true;
                if (btnPlayPause) btnPlayPause->setText("⏸️ 暂停");
            }
        }
    }
}
