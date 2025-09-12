#include "JigsawPuzzle.h"
#include "ui_JigsawPuzzle.h"
#include "DlgMenu.h"
#include "ui/help.h"
#include <QApplication>
#include <QDialog>
#include <QFileDialog>
#include <QImage>
#include <QMessageBox>
#include <QMouseEvent>
#include <QRandomGenerator>
#include <QStringList>
#include <QTime>
#include <QTimer>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QDebug>
#include <cmath>

// JigsawPiece implementation
JigsawPiece::JigsawPiece(int id, int row, int col, int totalRows, int totalCols, const QPixmap& image, const QRect& sourceRect)
    : QGraphicsItem()
    , _id(id)
    , _row(row)
    , _col(col)
    , _totalRows(totalRows)
    , _totalCols(totalCols)
    , _image(image)
    , _sourceRect(sourceRect)
    , _rotation(0)
    , _isPlaced(false)
    , _isHighlighted(false)
    , _topShape(ShapeType::STRAIGHT)
    , _bottomShape(ShapeType::STRAIGHT)
    , _leftShape(ShapeType::STRAIGHT)
    , _rightShape(ShapeType::STRAIGHT)
{
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    
    generateShape();
    setPos(QPointF(0, 0));
    setZValue(1);
}

void JigsawPiece::setOriginalPosition(const QPointF& pos)
{
    _originalPosition = pos;
}

void JigsawPiece::setCurrentPosition(const QPointF& pos)
{
    _currentPosition = pos;
    setPos(pos);
}

void JigsawPiece::setRotation(int angle)
{
    _rotation = angle;
    setTransform(QTransform().rotate(angle));
}

QRectF JigsawPiece::boundingRect() const
{
    return QRectF(_sourceRect);
}

void JigsawPiece::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    painter->setRenderHint(QPainter::Antialiasing);
    
    // 绘制图片
    painter->drawPixmap(_sourceRect, _image, _sourceRect);
    
    // 绘制边框
    painter->setClipping(false);
    QPen pen;
    if (_isHighlighted) {
        pen.setColor(Qt::yellow);
        pen.setWidth(3);
    } else if (isSelected()) {
        pen.setColor(Qt::blue);
        pen.setWidth(2);
    } else {
        pen.setColor(Qt::black);
        pen.setWidth(1);
    }
    painter->setPen(pen);
    painter->drawRect(_sourceRect);
    
    // 如果已放置，添加轻微的阴影效果
    if (_isPlaced) {
        painter->setOpacity(0.9);
    }
}

QPainterPath JigsawPiece::getShapePath() const
{
    QPainterPath path;
    path.addRect(_sourceRect);
    return path;
}

bool JigsawPiece::matchesNeighbor(JigsawPiece* neighbor, const QString& direction) const
{
    if (!neighbor) return false;
    
    // 简化的匹配逻辑
    return true;
}

void JigsawPiece::highlight(bool highlight)
{
    _isHighlighted = highlight;
    update();
}

void JigsawPiece::setOpacity(qreal opacity)
{
    QGraphicsItem::setOpacity(opacity);
}

void JigsawPiece::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        setZValue(1000); // 提升到最顶层
        setCursor(Qt::ClosedHandCursor);
    }
    QGraphicsItem::mousePressEvent(event);
}

void JigsawPiece::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseMoveEvent(event);
}

void JigsawPiece::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    setCursor(Qt::ArrowCursor);
    setZValue(1); // 恢复正常层级
    QGraphicsItem::mouseReleaseEvent(event);
}

void JigsawPiece::generateShape()
{
    // 简化的形状生成
    _topShape = ShapeType::STRAIGHT;
    _bottomShape = ShapeType::STRAIGHT;
    _leftShape = ShapeType::STRAIGHT;
    _rightShape = ShapeType::STRAIGHT;
}

// JigsawPuzzle implementation
JigsawPuzzle::JigsawPuzzle(int rows, int cols, QWidget *parent, NetworkClient *networkClient)
    : QDialog(parent)
    , ui(new Ui::JigsawPuzzle)
    , _bStart(false)
    , _iStep(0)
    , network_client(networkClient)
    , _rows(rows)
    , _cols(cols)
    , _totalPieces(rows * cols)
    , _placedPieces(0)
    , _gameWon(false)
    , _maxHistorySize(50)
    , _isUndoing(false)
{
    setAttribute(Qt::WA_DeleteOnClose);
    
    ui->setupUi(this);
    
    // 初始化计时器
    time.setHMS(0, 0, 0, 0);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &JigsawPuzzle::updateTimer);
    
    // 初始化历史记录栈
    _historyStack.clear();
    
    // 设置图片路径
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
    
    // 使用默认图片
    int randomIndex = QRandomGenerator::global()->bounded(imagePaths.size());
    _strPos = imagePaths[randomIndex];
    
    init();
    setupScene();
    
    // 连接信号
    connect(this, &JigsawPuzzle::sig_restart, this, &JigsawPuzzle::on_btnRestart_clicked);
    connect(this, &JigsawPuzzle::sig_back, this, &JigsawPuzzle::on_btnBack_clicked);
}

JigsawPuzzle::~JigsawPuzzle()
{
    // 清理拼图块
    for (JigsawPiece* piece : _pieces) {
        delete piece;
    }
    _pieces.clear();
    
    delete ui;
}

void JigsawPuzzle::start()
{
    time.setHMS(0, 0, 0, 0);
    ui->lcdStep_2->display(time.toString("hh:mm:ss"));
    
    _bStart = true;
    _iStep = 0;
    ui->lcdStep->display(_iStep);
    
    // 开始计时
    timer->start(1000);
    
    // 打乱拼图块
    shufflePieces();
}

void JigsawPuzzle::setNetworkClient(NetworkClient *client)
{
    network_client = client;
}

void JigsawPuzzle::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    
    // 调整场景大小
    if (_graphicsView && _scene) {
        _scene->setSceneRect(_graphicsView->rect());
    }
}

void JigsawPuzzle::on_btnBack_clicked()
{
    emit sig_back();
}

void JigsawPuzzle::on_btnRestart_clicked()
{
    // 重新开始游戏
    _bStart = false;
    timer->stop();
    _placedPieces = 0;
    _gameWon = false;
    
    // 重置拼图块
    for (JigsawPiece* piece : _pieces) {
        piece->setRotation(0);
        piece->setPlaced(false);
        piece->highlight(false);
    }
    
    // 清空历史记录
    _historyStack.clear();
    updateUndoButton();
    
    // 重新开始
    start();
}

void JigsawPuzzle::on_btnReturn_clicked()
{
    emit sig_return();
}

void JigsawPuzzle::on_btnShowImage_clicked()
{
    // 显示原图参考
    QMessageBox::information(this, "参考图", "参考图功能暂未实现");
}

void JigsawPuzzle::updateTimer()
{
    time = time.addSecs(1);
    ui->lcdStep_2->display(time.toString("hh:mm:ss"));
}

void JigsawPuzzle::checkWinCondition()
{
    if (_placedPieces == _totalPieces) {
        _gameWon = true;
        timer->stop();
        
        QMessageBox::information(this, "恭喜", "您已完成拼图！");
        
        // 发送成功信号
        emit sig_successRestart();
    }
}

void JigsawPuzzle::onPiecePlaced(JigsawPiece* piece)
{
    if (piece && !piece->isPlaced()) {
        piece->setPlaced(true);
        _placedPieces++;
        _iStep++;
        ui->lcdStep->display(_iStep);
        
        // 检查是否完成
        checkWinCondition();
    }
}

void JigsawPuzzle::onPieceMoved(JigsawPiece* piece)
{
    if (piece) {
        // 检查拼图块是否在正确位置
        snapToGrid(piece);
    }
}

void JigsawPuzzle::init()
{
    // 加载原始图片
    _originalImage.load(_strPos);
    if (_originalImage.isNull()) {
        QMessageBox::critical(this, "错误", "无法加载图片");
        return;
    }
    
    // 调整图片大小
    QSize viewSize = ui->graphicsView->size();
    _originalImage = _originalImage.scaled(viewSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    
    // 创建拼图块
    createPuzzlePieces();
    
    // 初始化UI
    updateUI();
}

void JigsawPuzzle::createPuzzlePieces()
{
    // 清空现有拼图块
    for (JigsawPiece* piece : _pieces) {
        delete piece;
    }
    _pieces.clear();
    
    // 计算每个拼图块的尺寸
    qreal pieceWidth = (qreal)_originalImage.width() / _cols;
    qreal pieceHeight = (qreal)_originalImage.height() / _rows;
    
    // 创建拼图块
    for (int row = 0; row < _rows; ++row) {
        for (int col = 0; col < _cols; ++col) {
            int id = row * _cols + col;
            QRect sourceRect(col * pieceWidth, row * pieceHeight, pieceWidth, pieceHeight);
            
            JigsawPiece* piece = new JigsawPiece(id, row, col, _rows, _cols, _originalImage, sourceRect);
            
            // 设置原始位置
            QPointF originalPos(col * pieceWidth, row * pieceHeight);
            piece->setOriginalPosition(originalPos);
            
            _pieces.append(piece);
        }
    }
}

void JigsawPuzzle::setupScene()
{
    // 创建场景
    _scene = new QGraphicsScene(this);
    _scene->setBackgroundBrush(Qt::lightGray);
    
    // 设置图形视图
    _graphicsView = ui->graphicsView;
    _graphicsView->setScene(_scene);
    _graphicsView->setRenderHint(QPainter::Antialiasing);
    _graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    
    // 添加拼图块到场景
    for (JigsawPiece* piece : _pieces) {
        _scene->addItem(piece);
    }
}

void JigsawPuzzle::shufflePieces()
{
    // 计算打乱后的位置
    QSize viewSize = _graphicsView->size();
    qreal margin = 50;
    
    for (JigsawPiece* piece : _pieces) {
        qreal x = margin + QRandomGenerator::global()->bounded(viewSize.width() - 2 * margin);
        qreal y = margin + QRandomGenerator::global()->bounded(viewSize.height() - 2 * margin);
        
        piece->setCurrentPosition(QPointF(x, y));
        piece->setRotation(QRandomGenerator::global()->bounded(4) * 90);
    }
}

void JigsawPuzzle::updateUI()
{
    ui->labelProgress->setText(QString("进度: %1/%2").arg(_placedPieces).arg(_totalPieces));
    
    // 更新进度条
    if (_totalPieces > 0) {
        int progress = (_placedPieces * 100) / _totalPieces;
        ui->progressBar->setValue(progress);
    }
}

void JigsawPuzzle::saveCurrentState()
{
    // 暂时禁用状态保存
}

void JigsawPuzzle::updateUndoButton()
{
    // 更新撤销按钮状态
    QPushButton* undoButton = findChild<QPushButton*>("btnUndo");
    if (undoButton) {
        undoButton->setEnabled(false);
    }
}

bool JigsawPuzzle::checkPieceConnection(JigsawPiece* piece)
{
    return true;
}

void JigsawPuzzle::snapToGrid(JigsawPiece* piece)
{
    // 计算拼图块应该放置的位置
    qreal pieceWidth = (qreal)_originalImage.width() / _cols;
    qreal pieceHeight = (qreal)_originalImage.height() / _rows;
    
    QPointF expectedPos(piece->getCol() * pieceWidth, piece->getRow() * pieceHeight);
    QPointF currentPos = piece->pos();
    
    // 计算距离
    qreal distance = QLineF(currentPos, expectedPos).length();
    
    // 如果距离足够近且旋转角度正确，则吸附到正确位置
    if (distance < 30 && piece->getRotation() % 360 == 0) {
        piece->setCurrentPosition(expectedPos);
        piece->setRotation(0);
        
        if (!piece->isPlaced()) {
            onPiecePlaced(piece);
        }
    }
}

void JigsawPuzzle::showHint()
{
    // 显示提示：高亮显示一个未放置的拼图块
    for (JigsawPiece* piece : _pieces) {
        if (!piece->isPlaced()) {
            piece->highlight(true);
            QTimer::singleShot(2000, [piece]() {
                piece->highlight(false);
            });
            break;
        }
    }
}

void JigsawPuzzle::generatePieceShapes()
{
    // 生成拼图块的形状，确保相邻块能够匹配
    // 简化版本，不调用私有方法
}

int JigsawPuzzle::calculateDifficulty()
{
    // 根据拼图块数量计算难度
    return _totalPieces;
}

JigsawPiece* JigsawPuzzle::getPieceAt(int row, int col)
{
    if (row < 0 || row >= _rows || col < 0 || col >= _cols) {
        return nullptr;
    }
    
    int id = row * _cols + col;
    if (id >= 0 && id < _pieces.size()) {
        return _pieces[id];
    }
    
    return nullptr;
}

void JigsawPuzzle::drawTargetArea()
{
    // 绘制目标区域轮廓
    QGraphicsRectItem* targetRect = new QGraphicsRectItem(0, 0, _originalImage.width(), _originalImage.height());
    targetRect->setPen(QPen(Qt::gray, 2, Qt::DashLine));
    targetRect->setBrush(Qt::NoBrush);
    targetRect->setZValue(0);
    _scene->addItem(targetRect);
}