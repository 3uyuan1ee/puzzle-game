#include "play4x4.h"
#include "ui_play4x4.h"
#include "DlgMenu.h"
#include "ui/help.h"
#include <QApplication>
#include <QDialog>
#include <QFileDialog>
#include <QImage>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QRandomGenerator>
#include <QStringList>
#include <QTime>
#include <QTimer>
#include <QMimeData>
#include <QDrag>
#include <QTransform>
#include <QGridLayout>
#include <cstdlib>
#include <ctime>

int xx = 0;

play4x4::play4x4(int rows, int cols, QWidget *parent, NetworkClient *networkClient, QMediaPlayer *musicPlayer)
    : QDialog(parent)
    , ui(new Ui::play4x4)
    , _bStart(false)
    , _iStep(0)
    , _draggedLabel(nullptr)
    , _draggedIndex(-1)
    , network_client(networkClient)
    , _backgroundMusicPlayer(musicPlayer)
    , _rows(rows)
    , _cols(cols)
    , _totalPieces(rows * cols)
    , _maxHistorySize(50)  // 最多保存50步历史记录
    , _lastCorrectPieceCount(0)  // 初始化正确拼图块计数器
    , _isUndoing(false)     // 初始化撤销标志
{
    setAttribute(Qt::WA_DeleteOnClose);

    time.setHMS(0, 0, 0, 0);
    timer = new QTimer(this);
    
    // 初始化历史记录栈
    _historyStack.clear();

    QStringList imagePaths = {
        ":photo/img/1.jpg",
        ":photo/img/2.jpg",
        ":photo/img/3.jpg",
        ":photo/img/4.jpg",
        ":photo/img/5.jpg",
        ":photo/img/6.jpeg",
        ":photo/img/7.jpg",
        ":photo/img/8.jpg",
    };

    if (save == 1) {
        int randomIndex = QRandomGenerator::global()->bounded(imagePaths.size());
        xx = randomIndex;
        _strPos = imagePaths[randomIndex];
    } else {
        _strPos = DlgMenu::diyImagePath;
    }

    ui->setupUi(this);

    // 动态创建标签控件
    createLabels();

    init();
    initSoundEffects();

    connect(this, &play4x4::sig_restart, this, &play4x4::on_btnRestart_clicked);
    connect(this, &play4x4::sig_back, this, &play4x4::on_btnBack_clicked);
    // connect(this, &play4x4::sig_return, this, &play4x4::on_btnreturn_clicked);
    connect(this, &play4x4::sig_successRestart, this, &play4x4::SuccessRestart);
    
    // 计时器信号连接只在这里执行一次
    connect(timer, &QTimer::timeout, this, &play4x4::update);
    
}

play4x4::~play4x4()
{
    // 清理动态创建的标签
    for (QLabel* label : _labels) {
        delete label;
    }
    _labels.clear();

    // 清理音效
    delete _moveSound;
    delete _rotateSound;
    delete _correctSound;

    delete ui;
}

void play4x4::initSoundEffects()
{
    // 初始化所有音效指针为nullptr
    _moveSound = nullptr;
    _rotateSound = nullptr;
    _correctSound = nullptr;
    
    try {
        // 初始化移动音效
        _moveSound = new QMediaPlayer(this);
        _moveSound->setAudioOutput(new QAudioOutput(this));
        _moveSound->setSource(QUrl::fromLocalFile(":/sounds/sounds/move.mp3"));
        _moveSound->audioOutput()->setVolume(0.8);
        
        // 初始化旋转音效
        _rotateSound = new QMediaPlayer(this);
        _rotateSound->setAudioOutput(new QAudioOutput(this));
        _rotateSound->setSource(QUrl::fromLocalFile(":/sounds/sounds/rotate.mp3"));
        _rotateSound->audioOutput()->setVolume(0.8);
        
        // 初始化正确放置音效
        _correctSound = new QMediaPlayer(this);
        _correctSound->setAudioOutput(new QAudioOutput(this));
        _correctSound->setSource(QUrl::fromLocalFile(":/sounds/sounds/correct.mp3"));
        _correctSound->audioOutput()->setVolume(0.8);
        
    } catch (const std::exception& e) {
        qWarning() << "Failed to initialize sound effects:" << e.what();
        // 清理已创建的音效
        if (_moveSound) delete _moveSound;
        if (_rotateSound) delete _rotateSound;
        if (_correctSound) delete _correctSound;
        _moveSound = nullptr;
        _rotateSound = nullptr;
        _correctSound = nullptr;
    }
}

void play4x4::createLabels()
{
    // 清除现有的标签
    for (QLabel* label : _labels) {
        delete label;
    }
    _labels.clear();

    // 获取网格布局
    QGridLayout *gridLayout = ui->gridLayout;

    // 清除布局中的所有项目
    QLayoutItem *child;
    while ((child = gridLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }

    // 设置布局间距和边距为最小
    gridLayout->setSpacing(0);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    
    // 设置布局为自动拉伸，确保填满整个容器
    gridLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    // 创建新的标签
    for (int i = 0; i < _totalPieces; ++i) {
        QLabel *label = new QLabel(ui->gridLayoutWidget); // 注意：父对象是gridLayoutWidget
        label->setObjectName(QString("label_%1").arg(i + 1));
        label->setAlignment(Qt::AlignCenter);
        
        // 设置尺寸策略为扩展，确保填满可用空间
        label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        label->setMinimumSize(30, 30); // 最小尺寸30x30
        label->setFrameShape(QFrame::Box);
        label->setText(QString::number(i + 1));

        int row = i / _cols;
        int col = i % _cols;
        gridLayout->addWidget(label, row, col);

        _labels.append(label);
    }
}

QLabel* play4x4::getLabelAtGridPos(const QPoint& globalPos)
{
    // 将全局坐标转换为gridLayoutWidget内的坐标
    QPoint localPos = ui->gridLayoutWidget->mapFromGlobal(globalPos);

    // 检查坐标是否在gridLayoutWidget内
    if (!ui->gridLayoutWidget->rect().contains(localPos)) {
        return nullptr;
    }

    // 遍历所有标签，找到包含该坐标的标签
    for (QLabel* label : _labels) {
        // 将标签的几何位置转换为gridLayoutWidget内的坐标
        QRect labelGeometry = label->geometry();
        if (labelGeometry.contains(localPos)) {
            return label;
        }
    }

    return nullptr;
}

QPoint play4x4::getGridPositionFromGlobalPos(const QPoint& globalPos)
{
    // 将全局坐标转换为gridLayoutWidget内的坐标
    QPoint localPos = ui->gridLayoutWidget->mapFromGlobal(globalPos);

    // 检查坐标是否在gridLayoutWidget内
    if (!ui->gridLayoutWidget->rect().contains(localPos)) {
        return QPoint(-1, -1);
    }

    // 计算考虑间距的实际网格大小
    QGridLayout *gridLayout = ui->gridLayout;
    int spacing = gridLayout->spacing();
    int availableWidth = ui->gridLayoutWidget->width() - (spacing * (_cols - 1));
    int availableHeight = ui->gridLayoutWidget->height() - (spacing * (_rows - 1));
    
    int cellWidth = availableWidth / _cols;
    int cellHeight = availableHeight / _rows;

    // 计算网格位置，考虑间距
    int col = (localPos.x() + spacing/2) / (cellWidth + spacing);
    int row = (localPos.y() + spacing/2) / (cellHeight + spacing);

    // 确保在有效范围内
    col = qBound(0, col, _cols - 1);
    row = qBound(0, row, _rows - 1);

    return QPoint(col, row);
}

void play4x4::start()
{
    time.setHMS(0, 0, 0, 0);
    ui->lcdStep_2->display(time.toString("hh:mm:ss"));

    _bStart = true;
    _iStep = 0;
    ui->lcdStep->display("0");
    
    // 清空历史记录
    _historyStack.clear();
    
    upset();
    
    showpicture();
    
    // 重置正确拼图块计数器
    _lastCorrectPieceCount = 0;
}

void play4x4::setNetworkClient(NetworkClient *client)
{
    network_client = client;
}

void play4x4::setMusicPlayer(QMediaPlayer *player)
{
    _backgroundMusicPlayer = player;
}

void play4x4::update()
{
    time = time.addSecs(1);
    ui->lcdStep_2->display(time.toString("hh:mm:ss"));
}

void play4x4::init()
{
    // 初始化拼图数组
    _iarrMap.resize(_rows);
    _iarrRot.resize(_rows);
    for (int i = 0; i < _rows; i++) {
        _iarrMap[i].resize(_cols);
        _iarrRot[i].resize(_cols);
    }

    // 初始化拼图
    int num = 1;
    for (int i = 0; i < _rows; i++) {
        for (int j = 0; j < _cols; j++) {
            _iarrMap[i][j] = num;
            num++;
        }
    }

    // 初始化旋转角度
    for (int i = 0; i < _rows; ++i) {
        for (int j = 0; j < _cols; ++j) {
            _iarrRot[i][j] = 0;
        }
    }

    if (_strPos.isEmpty()) {
        qDebug() << "图片路径为空";
        return;
    }

    QPixmap sourcePixmap(_strPos);
    if (sourcePixmap.isNull()) {
        qDebug() << "无法加载图片:" << _strPos;
        QMessageBox::warning(this, tr("错误"), tr("无法加载自定义图片"));
        return;
    }

    qDebug() << "图片加载成功，尺寸:" << sourcePixmap.width() << "x" << sourcePixmap.height();

    // 为了更好的视觉效果，将图片缩放到合适的大小
    int targetWidth = 600; // 目标宽度
    int targetHeight = 600; // 目标高度
    QPixmap scaledPixmap = sourcePixmap.scaled(targetWidth, targetHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 计算每块的尺寸
    int blockWidth = scaledPixmap.width() / _cols;
    int blockHeight = scaledPixmap.height() / _rows;

    qDebug() << "缩放后图片尺寸:" << scaledPixmap.width() << "x" << scaledPixmap.height();
    qDebug() << "每块尺寸:" << blockWidth << "x" << blockHeight;

    // 清空现有的图片
    _pieceImages.clear();

    // 分割图片
    for (int row = 0; row < _rows; ++row) {
        for (int col = 0; col < _cols; ++col) {
            QPixmap piece = scaledPixmap.copy(col * blockWidth, row * blockHeight, blockWidth, blockHeight);
            _pieceImages.append(piece);
        }
    }

    // 设置标签属性并启用拖放
    for (QLabel* label : _labels) {
        label->setScaledContents(true);
        label->setAcceptDrops(true);
        label->installEventFilter(this);
        label->setStyleSheet("border: 1px solid #666666; margin: 0; padding: 0;");
    }

    showpicture();
    setAcceptDrops(true);
}

bool play4x4::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent*>(e);

        if (me->button() == Qt::LeftButton) {
            // 处理左键拖拽
            for (int idx = 0; idx < _labels.size(); ++idx) {
                if (obj == _labels[idx]) {
                    _draggedLabel = static_cast<QLabel*>(obj);
                    _draggedIndex = idx;
                    // 使用相对于标签的坐标
                    _dragStartPosition = me->pos();
                    return true;
                }
            }
        }
        else if (me->button() == Qt::RightButton) {
            // 右键旋转
            for (int idx = 0; idx < _labels.size(); ++idx) {
                if (obj == _labels[idx]) {
                    // 保存操作前的状态
                    saveCurrentState();
                    
                    int r = idx / _cols;
                    int c = idx % _cols;

                    _iarrRot[r][c] = (_iarrRot[r][c] + 1) % 4;
                    showpicture();
                    ++_iStep;
                    ui->lcdStep->display(_iStep);
                    
                    // 播放旋转音效
                    if (_rotateSound) {
                        _rotateSound->setPosition(0);
                        _rotateSound->play();
                    }
                    
                    return true;
                }
            }
        }
    }
    else if (e->type() == QEvent::MouseMove) {
        QMouseEvent *me = static_cast<QMouseEvent*>(e);
        if (me->buttons() & Qt::LeftButton && _draggedLabel) {
            int distance = (me->pos() - _dragStartPosition).manhattanLength();
            if (distance >= QApplication::startDragDistance()) {
                QDrag *drag = new QDrag(this);
                QMimeData *mimeData = new QMimeData;

                int draggedRow = _draggedIndex / _cols;
                int draggedCol = _draggedIndex % _cols;
                QString data = QString("%1,%2").arg(draggedRow).arg(draggedCol);
                mimeData->setText(data);
                drag->setMimeData(mimeData);

                QPixmap pixmap = _draggedLabel->pixmap(Qt::ReturnByValue);
                if (pixmap.isNull()) {
                    // 如果没有图片，创建一个默认的
                    pixmap = QPixmap(60, 60);
                    pixmap.fill(Qt::lightGray);
                }
                drag->setPixmap(pixmap);

                // 修正hotspot，使用鼠标在标签内的相对位置
                QPoint hotspot = me->pos();
                drag->setHotSpot(hotspot);

                drag->exec(Qt::MoveAction);
                _draggedLabel = nullptr;
                return true;
            }
        }
    }

    return QDialog::eventFilter(obj, e);
}

void play4x4::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasText()) {
        event->acceptProposedAction();
    }
}

void play4x4::dragMoveEvent(QDragMoveEvent *event)
{
    QPoint globalPos = event->position().toPoint();

    // 重置所有标签样式
    for (QLabel* label : _labels) {
        label->setStyleSheet("border: 1px solid gray;");
    }

    // 高亮当前拖拽位置
    QLabel* targetLabel = getLabelAtGridPos(globalPos);
    if (targetLabel) {
        targetLabel->setStyleSheet("border: 3px solid red; background-color: #FFFACD;");
    }

    event->acceptProposedAction();
}

void play4x4::dropEvent(QDropEvent *event)
{
    if (ss == 0) {
        timer->start(1000);
        ss++;
    }

    // 获取拖拽源位置
    QStringList sourceData = event->mimeData()->text().split(",");
    int sourceRow = sourceData[0].toInt();
    int sourceCol = sourceData[1].toInt();

    // 获取释放目标位置的网格坐标
    QPoint gridPos = getGridPositionFromGlobalPos(event->position().toPoint());
    
    // 检查是否在有效网格范围内
    if (gridPos.x() == -1 || gridPos.y() == -1) {
        event->ignore();
        return;
    }
    
    int targetRow = gridPos.y();
    int targetCol = gridPos.x();

    // 重置所有标签样式
    for (QLabel* label : _labels) {
        label->setStyleSheet("border: 1px solid gray;");
    }

    // 获取目标标签
    int targetIndex = targetRow * _cols + targetCol;
    if (targetIndex >= 0 && targetIndex < _labels.size()) {
        QLabel* targetLabel = _labels[targetIndex];

        // 保存操作前的状态
        saveCurrentState();

        // 交换两块拼图
        qSwap(_iarrMap[sourceRow][sourceCol], _iarrMap[targetRow][targetCol]);
        qSwap(_iarrRot[sourceRow][sourceCol], _iarrRot[targetRow][targetCol]);

        _iStep++;
        showpicture();
        ui->lcdStep->display(QString::number(_iStep));
        
        // 播放移动音效
        if (_moveSound) {
            _moveSound->setPosition(0);
            _moveSound->play();
        }

        // 检查是否有拼图块被放置到正确位置
        int correctPieceCount = 0;
        int num = 1;
        for (int j = 0; j < _rows; ++j) {
            for (int i = 0; i < _cols; ++i) {
                if (_iarrMap[j][i] == num && _iarrRot[j][i] == 0) {
                    correctPieceCount++;
                }
                num++;
            }
        }
        
        // 如果这次移动后拼图块数量增加了，说明有拼图块被正确放置
        if (correctPieceCount > _lastCorrectPieceCount) {
            // 播放正确放置音效
            if (_correctSound) {
                _correctSound->setPosition(0);
                _correctSound->play();
            }
        }
        _lastCorrectPieceCount = correctPieceCount;

        // 短暂高亮交换的区域
        targetLabel->setStyleSheet("border: 3px solid green; background-color: #90EE90;");
        QTimer::singleShot(300, this, [this]() {
            for (QLabel* label : _labels) {
                label->setStyleSheet("border: 1px solid gray;");
            }
        });

        if (bSuccessful()) {
            // 重置计数器，为下一局游戏做准备
            _lastCorrectPieceCount = 0;
            QApplication::setQuitOnLastWindowClosed(false);
            
            // 提交游戏数据到服务器
            submitGameResult();
            
            // 判断游戏模式：自定义模式还是闯关模式
            if (save == 0) {
                // 自定义模式：只有两个选项
                QMessageBox::StandardButton result = QMessageBox::question(
                    this, "游戏胜利！", "恭喜你完成了拼图！\n是否重新开始？",
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

                if (result == QMessageBox::Yes) {
                    emit sig_restart();  // 重新开始当前游戏
                } else {
                    emit sig_back();     // 返回
                }
            } else {
                // 闯关模式：三个选项
                QMessageBox msgBox(this);
                msgBox.setWindowTitle("游戏胜利！");
                msgBox.setText("恭喜你完成了拼图！");
                msgBox.setInformativeText("请选择下一步操作：");
                
                QAbstractButton *restartButton = msgBox.addButton("重新开始当前关卡", QMessageBox::ActionRole);
                QAbstractButton *nextButton = msgBox.addButton("开始下一关", QMessageBox::ActionRole);
                QAbstractButton *quitButton = msgBox.addButton("不玩了", QMessageBox::RejectRole);
                
                msgBox.exec();
                
                if (msgBox.clickedButton() == restartButton) {
                    emit sig_restart();  // 重新开始当前关卡
                } else if (msgBox.clickedButton() == nextButton) {
                    emit sig_successRestart();  // 开始下一关
                } else {
                    emit sig_back();     // 返回
                }
            }
        }

        event->acceptProposedAction();
        return;
    }

    event->ignore();
}

void play4x4::upset()
{
    // 随机交换多次来打乱
    for (int i = 0; i < _totalPieces * 10; i++) {
        int row1 = QRandomGenerator::global()->bounded(_rows);
        int col1 = QRandomGenerator::global()->bounded(_cols);
        int row2 = QRandomGenerator::global()->bounded(_rows);
        int col2 = QRandomGenerator::global()->bounded(_cols);

        qSwap(_iarrMap[row1][col1], _iarrMap[row2][col2]);
        qSwap(_iarrRot[row1][col1], _iarrRot[row2][col2]);
    }

    // 随机旋转角度
    for (int i = 0; i < _rows; ++i) {
        for (int j = 0; j < _cols; ++j) {
            _iarrRot[i][j] = QRandomGenerator::global()->bounded(4);
        }
    }
}

void play4x4::showpicture()
{
    for (int idx = 0; idx < _totalPieces; ++idx) {
        if (idx >= _labels.size()) break;

        QLabel *p = _labels[idx];
        if (!p) continue;

        int row = idx / _cols;
        int col = idx % _cols;

        int pieceIndex = _iarrMap[row][col] - 1; // 调整为0-based索引
        if (pieceIndex < 0 || pieceIndex >= _pieceImages.size()) continue;

        int angle = _iarrRot[row][col] * 90;

        QTransform trans;
        trans.rotate(angle);
        QPixmap rotated = _pieceImages[pieceIndex].transformed(trans, Qt::SmoothTransformation);
        p->setPixmap(rotated);
    }
}

void play4x4::saveCurrentState()
{
    // 如果正在撤销操作，不保存状态
    if (_isUndoing) {
        return;
    }
    
    // 创建当前状态的副本
    GameState currentState;
    currentState.mapState = _iarrMap;
    currentState.rotState = _iarrRot;
    currentState.stepCount = _iStep;
    currentState.gameTime = time;
    
    // 添加到历史栈
    _historyStack.append(currentState);
    
    // 限制历史记录数量
    if (_historyStack.size() > _maxHistorySize) {
        _historyStack.removeFirst();
    }
    
    // 更新撤销按钮状态
    updateUndoButton();
}

void play4x4::updateUndoButton()
{
    // 更新撤销按钮状态
    int availableUndos = _historyStack.size();
    ui->btnreturn->setEnabled(availableUndos > 0);
    
    // 更新按钮文本显示可撤销步数
    if (availableUndos > 0) {
        ui->btnreturn->setText(QString("后退 (%1)").arg(availableUndos));
    } else {
        ui->btnreturn->setText("后退");
    }
}

bool play4x4::bSuccessful()
{
    int num = 1;
    for (int j = 0; j < _rows; ++j) {
        for (int i = 0; i < _cols; ++i) {
            if (_iarrMap[j][i] != num || _iarrRot[j][i] != 0) {
                return false;
            }
            num++;
        }
    }
    return true;
}

void play4x4::on_btnBack_clicked()
{
    timer->stop();


    this->parentWidget()->show();

    this->hide();

    state = 4;

    save=1;
}

void play4x4::on_btnRestart_clicked()
{
    timer->stop();
    time.setHMS(0, 0, 0, 0);
    ui->lcdStep_2->display(time.toString("hh:mm:ss"));
    
    // 重置正确拼图块计数器
    _lastCorrectPieceCount = 0;
    
    start();
    ss = 0;
}

void play4x4::on_btnreturn_clicked()
{
    if (_historyStack.size() > 0) {  // 至少有一个状态才能撤销
        // 设置撤销标志，防止撤销操作被保存到历史记录
        _isUndoing = true;
        
        // 弹出最后一个状态（即上一个操作前的状态）
        GameState previousState = _historyStack.takeLast();
        
        // 恢复游戏状态
        _iarrMap = previousState.mapState;
        _iarrRot = previousState.rotState;
        _iStep = previousState.stepCount;
        time = previousState.gameTime;
        
        // 更新显示
        showpicture();
        ui->lcdStep->display(QString::number(_iStep));
        ui->lcdStep_2->display(time.toString("hh:mm:ss"));
        
        // 重置撤销标志
        _isUndoing = false;
        
        // 更新撤销按钮状态
        updateUndoButton();
        
        // 更新正确拼图块计数器
        int currentCorrectCount = 0;
        int num = 1;
        for (int j = 0; j < _rows; ++j) {
            for (int i = 0; i < _cols; ++i) {
                if (_iarrMap[j][i] == num && _iarrRot[j][i] == 0) {
                    currentCorrectCount++;
                }
                num++;
            }
        }
        _lastCorrectPieceCount = currentCorrectCount;
        
        // 检查是否完成拼图
        if (bSuccessful()) {
            QApplication::setQuitOnLastWindowClosed(false);
            
            // 提交游戏数据到服务器
            submitGameResult();
            
            // 判断游戏模式：自定义模式还是闯关模式
            if (save == 0) {
                // 自定义模式：只有两个选项
                QMessageBox::StandardButton result = QMessageBox::question(
                    this, "游戏胜利！", "恭喜你完成了拼图！\n是否重新开始？",
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

                if (result == QMessageBox::Yes) {
                    emit sig_restart();  // 重新开始当前游戏
                } else {
                    emit sig_back();     // 返回
                }
            } else {
                // 闯关模式：三个选项
                QMessageBox msgBox(this);
                msgBox.setWindowTitle("游戏胜利！");
                msgBox.setText("恭喜你完成了拼图！");
                msgBox.setInformativeText("请选择下一步操作：");
                
                QAbstractButton *restartButton = msgBox.addButton("重新开始当前关卡", QMessageBox::ActionRole);
                QAbstractButton *nextButton = msgBox.addButton("开始下一关", QMessageBox::ActionRole);
                QAbstractButton *quitButton = msgBox.addButton("不玩了", QMessageBox::RejectRole);
                
                msgBox.exec();
                
                if (msgBox.clickedButton() == restartButton) {
                    emit sig_restart();  // 重新开始当前关卡
                } else if (msgBox.clickedButton() == nextButton) {
                    emit sig_successRestart();  // 开始下一关
                } else {
                    emit sig_back();     // 返回
                }
            }
        }
    }
}

void play4x4::on_pushButton_3_clicked()
{
    help *configWindow = new help;
    configWindow->show();
}

void play4x4::SuccessRestart()
{
    this->close();
}

void play4x4::on_pushButton_2_clicked()
{
    // 停止计时器
    timer->stop();
    
    // 重置拼图到正确状态
    int num = 1;
    for (int i = 0; i < _rows; i++) {
        for (int j = 0; j < _cols; j++) {
            _iarrMap[i][j] = num;
            _iarrRot[i][j] = 0;  // 重置旋转角度
            num++;
        }
    }
    
    // 重置步数和时间
    _iStep = 0;
    ui->lcdStep->display("0");
    time.setHMS(0, 0, 0, 0);
    ui->lcdStep_2->display(time.toString("hh:mm:ss"));
    
    // 清空历史记录
    _historyStack.clear();
    updateUndoButton();
    
    // 重置正确拼图块计数器
    _lastCorrectPieceCount = _totalPieces; // 所有拼图块都正确
    
    // 更新显示
    showpicture();
    
    // 显示还原完成提示
    QMessageBox::information(this, "还原完成", "拼图已自动还原到正确位置！");
}

void play4x4::submitGameResult()
{
    if (!network_client || !network_client->isLoggedIn()) {
        return; // 未登录或网络客户端不可用
    }
    
    // 计算游戏时间（秒）
    int time_seconds = QTime(0, 0, 0).secsTo(time);
    
    // 判断游戏模式并提交数据
    if (save == 1) {
        // 闯关模式 - 提交关卡进度
        // 这里假设当前关卡数可以通过某种方式获取
        // 暂时使用网格大小来估算关卡
        int estimated_level = 0;
        if (_rows == 2 && _cols == 2) estimated_level = 1;
        else if (_rows == 3 && _cols == 3) estimated_level = 2;
        else if (_rows == 3 && _cols == 4) estimated_level = 3;
        else if (_rows == 4 && _cols == 4) estimated_level = 4;
        else if (_rows == 4 && _cols == 5) estimated_level = 5;
        else estimated_level = 6; // 其他情况都算第6关
        
        network_client->submitGameResult("level", 0, estimated_level, 0, 0, false);
    } else {
        // 自定义模式 - 提交时间和步数
        bool usedUndo = _historyStack.size() > 0; // 是否使用了撤销功能
        
        // 提交时间排行榜数据
        network_client->submitGameResult("time", _rows, 0, time_seconds, 0, usedUndo);
        
        // 提交步数排行榜数据
        network_client->submitGameResult("step", _rows, 0, 0, _iStep, usedUndo);
    }
}

