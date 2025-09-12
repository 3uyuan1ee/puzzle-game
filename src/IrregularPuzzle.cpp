#include "IrregularPuzzle.h"
#include "ui_IrregularPuzzle.h"
#include "DlgMenu.h"
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
#include <QMenu>
#include <QAction>

// IrregularPiece implementation
IrregularPiece::IrregularPiece(int id, PieceType type, const QPointF& centerPos, qreal size, const QPixmap& image)
    : QGraphicsItem()
    , _id(id)
    , _pieceType(type)
    , _image(image)
    , _originalPosition(centerPos)
    , _currentPosition(centerPos)
    , _size(size)
    , _rotation(0)
    , _isPlaced(false)
    , _isHighlighted(false)
{
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    
    setPos(centerPos);
    setZValue(1);
    
    generateConnectionPoints();
}

void IrregularPiece::setOriginalPosition(const QPointF& pos)
{
    _originalPosition = pos;
}

void IrregularPiece::setCurrentPosition(const QPointF& pos)
{
    _currentPosition = pos;
    setPos(pos);
}

void IrregularPiece::setRotation(int angle)
{
    _rotation = angle;
    setTransform(QTransform().rotate(angle));
}

QRectF IrregularPiece::boundingRect() const
{
    return QRectF(-_size/2, -_size/2, _size, _size);
}

void IrregularPiece::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    painter->setRenderHint(QPainter::Antialiasing);
    
    // 创建拼图块形状路径
    QPainterPath path = getShapePath();
    
    // 设置颜色
    QColor pieceColor = getPieceColor();
    if (_isHighlighted) {
        pieceColor = pieceColor.lighter(150);
    }
    
    // 绘制填充
    painter->fillPath(path, pieceColor);
    
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
    painter->drawPath(path);
    
    // 绘制连接点
    painter->setBrush(Qt::red);
    painter->setPen(Qt::NoPen);
    for (const QPointF& point : _connectionPoints) {
        painter->drawEllipse(point, 3, 3);
    }
    
    // 如果已放置，添加轻微的阴影效果
    if (_isPlaced) {
        painter->setOpacity(0.9);
    }
}

QPainterPath IrregularPiece::getShapePath() const
{
    switch (_pieceType) {
        case PieceType::TRIANGLE:
            return createTrianglePath();
        case PieceType::SQUARE:
            return createSquarePath();
        case PieceType::PENTAGON:
            return createPentagonPath();
        case PieceType::HEXAGON:
            return createHexagonPath();
        case PieceType::CIRCLE:
            return createCirclePath();
        case PieceType::STAR:
            return createStarPath();
        case PieceType::TRAPEZOID:
            return createTrapezoidPath();
        case PieceType::RHOMBUS:
            return createRhombusPath();
        default:
            return createSquarePath();
    }
}

bool IrregularPiece::contains(const QPointF &point) const
{
    return getShapePath().contains(point);
}

void IrregularPiece::highlight(bool highlight)
{
    _isHighlighted = highlight;
    update();
}

void IrregularPiece::setOpacity(qreal opacity)
{
    QGraphicsItem::setOpacity(opacity);
}

bool IrregularPiece::canConnectTo(IrregularPiece* other) const
{
    if (!other) return false;
    
    // 简化的连接逻辑：不同类型的拼图块可以连接
    return _pieceType != other->_pieceType;
}

bool IrregularPiece::isInCorrectPosition() const
{
    qreal distance = QLineF(_currentPosition, _originalPosition).length();
    return distance < _size * 0.3 && _rotation % 360 == 0;
}

void IrregularPiece::rotate90Degrees()
{
    _rotation = (_rotation + 90) % 360;
    setTransform(QTransform().rotate(_rotation));
}

void IrregularPiece::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        setZValue(1000);
        setCursor(Qt::ClosedHandCursor);
    }
    QGraphicsItem::mousePressEvent(event);
}

void IrregularPiece::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseMoveEvent(event);
}

void IrregularPiece::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    setCursor(Qt::ArrowCursor);
    setZValue(1);
    QGraphicsItem::mouseReleaseEvent(event);
}

void IrregularPiece::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    // 右键旋转
    rotate90Degrees();
}

QVariant IrregularPiece::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionHasChanged) {
        // 位置变化时的处理
    }
    return QGraphicsItem::itemChange(change, value);
}

QPainterPath IrregularPiece::createTrianglePath() const
{
    QPainterPath path;
    QPolygonF triangle;
    triangle << QPointF(0, -_size/2)
             << QPointF(-_size/2, _size/2)
             << QPointF(_size/2, _size/2);
    path.addPolygon(triangle);
    return path;
}

QPainterPath IrregularPiece::createSquarePath() const
{
    QPainterPath path;
    path.addRect(-_size/2, -_size/2, _size, _size);
    return path;
}

QPainterPath IrregularPiece::createPentagonPath() const
{
    QPainterPath path;
    path.addPolygon(createRegularPolygon(5));
    return path;
}

QPainterPath IrregularPiece::createHexagonPath() const
{
    QPainterPath path;
    path.addPolygon(createRegularPolygon(6));
    return path;
}

QPainterPath IrregularPiece::createCirclePath() const
{
    QPainterPath path;
    path.addEllipse(QPointF(0, 0), _size/2, _size/2);
    return path;
}

QPainterPath IrregularPiece::createStarPath() const
{
    QPainterPath path;
    qreal outerRadius = _size / 2;
    qreal innerRadius = _size / 4;
    
    QPolygonF star;
    for (int i = 0; i < 10; ++i) {
        qreal angle = i * M_PI / 5;
        qreal radius = (i % 2 == 0) ? outerRadius : innerRadius;
        star << QPointF(radius * cos(angle - M_PI/2), radius * sin(angle - M_PI/2));
    }
    path.addPolygon(star);
    return path;
}

QPainterPath IrregularPiece::createTrapezoidPath() const
{
    QPainterPath path;
    QPolygonF trapezoid;
    trapezoid << QPointF(-_size/3, -_size/2)
             << QPointF(_size/3, -_size/2)
             << QPointF(_size/2, _size/2)
             << QPointF(-_size/2, _size/2);
    path.addPolygon(trapezoid);
    return path;
}

QPainterPath IrregularPiece::createRhombusPath() const
{
    QPainterPath path;
    QPolygonF rhombus;
    rhombus << QPointF(0, -_size/2)
            << QPointF(_size/2, 0)
            << QPointF(0, _size/2)
            << QPointF(-_size/2, 0);
    path.addPolygon(rhombus);
    return path;
}

QPolygonF IrregularPiece::createRegularPolygon(int sides) const
{
    QPolygonF polygon;
    qreal radius = _size / 2;
    
    for (int i = 0; i < sides; ++i) {
        qreal angle = i * 2 * M_PI / sides - M_PI/2;
        polygon << QPointF(radius * cos(angle), radius * sin(angle));
    }
    
    return polygon;
}

QColor IrregularPiece::getPieceColor() const
{
    switch (_pieceType) {
        case PieceType::TRIANGLE: return QColor(255, 100, 100);
        case PieceType::SQUARE: return QColor(100, 255, 100);
        case PieceType::PENTAGON: return QColor(100, 100, 255);
        case PieceType::HEXAGON: return QColor(255, 255, 100);
        case PieceType::CIRCLE: return QColor(255, 100, 255);
        case PieceType::STAR: return QColor(100, 255, 255);
        case PieceType::TRAPEZOID: return QColor(255, 200, 100);
        case PieceType::RHOMBUS: return QColor(200, 100, 255);
        default: return QColor(200, 200, 200);
    }
}

void IrregularPiece::generateConnectionPoints()
{
    _connectionPoints.clear();
    
    // 在拼图块的边缘生成连接点
    qreal radius = _size * 0.3;
    for (int i = 0; i < 4; ++i) {
        qreal angle = i * M_PI / 2;
        _connectionPoints << QPointF(radius * cos(angle), radius * sin(angle));
    }
}

// IrregularPuzzle implementation
IrregularPuzzle::IrregularPuzzle(QWidget *parent, NetworkClient *networkClient, QMediaPlayer *musicPlayer)
    : QDialog(parent)
    , ui(new Ui::IrregularPuzzle)
    , _bStart(false)
    , _iStep(0)
    , network_client(networkClient)
    , backgroundMusicPlayer(musicPlayer)
    , _placedPieces(0)
    , _gameWon(false)
    , _pieceSize(60)
    , _maxHistorySize(50)
    , _isUndoing(false)
{
    setAttribute(Qt::WA_DeleteOnClose);
    
    ui->setupUi(this);
    
    // 初始化计时器
    time.setHMS(0, 0, 0, 0);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &IrregularPuzzle::updateTimer);
    
    // 初始化历史记录栈
    _historyStack.clear();
    
    setupScene();
    init();
    
    // 重新添加拼图块到场景（因为init()创建了拼图块）
    for (IrregularPiece* piece : _pieces) {
        _scene->addItem(piece);
    }
    
    // 连接信号
    connect(this, &IrregularPuzzle::sig_restart, this, &IrregularPuzzle::on_btnRestart_clicked);
    connect(this, &IrregularPuzzle::sig_back, this, &IrregularPuzzle::on_btnBack_clicked);
}

IrregularPuzzle::~IrregularPuzzle()
{
    // 清理拼图块
    for (IrregularPiece* piece : _pieces) {
        delete piece;
    }
    _pieces.clear();
    
    delete ui;
}

void IrregularPuzzle::start()
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

void IrregularPuzzle::setNetworkClient(NetworkClient *client)
{
    network_client = client;
}

void IrregularPuzzle::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    
    // 调整场景大小
    if (_graphicsView && _scene) {
        _scene->setSceneRect(_graphicsView->rect());
    }
}

void IrregularPuzzle::on_btnBack_clicked()
{
    emit sig_back();
}

void IrregularPuzzle::on_btnRestart_clicked()
{
    // 重新开始游戏
    _bStart = false;
    timer->stop();
    _placedPieces = 0;
    _gameWon = false;
    
    // 重置拼图块
    for (IrregularPiece* piece : _pieces) {
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

void IrregularPuzzle::on_btnReturn_clicked()
{
    emit sig_return();
}

void IrregularPuzzle::on_btnShowImage_clicked()
{
    // 显示游戏说明
    QMessageBox::information(this, "游戏说明", 
        "异形拼图游戏说明：\n\n"
        "1. 拖动不同形状的拼图块到中央拼图区域\n"
        "2. 右键点击拼图块可以旋转90度\n"
        "3. 将拼图块拖到正确位置时会自动吸附\n"
        "4. 完成所有拼图块即可获胜\n\n"
        "拼图块类型：三角形、正方形、五边形、六边形、圆形、星形、梯形、菱形");
}

void IrregularPuzzle::updateTimer()
{
    time = time.addSecs(1);
    ui->lcdStep_2->display(time.toString("hh:mm:ss"));
}

void IrregularPuzzle::checkWinCondition()
{
    if (_placedPieces == _pieces.size()) {
        _gameWon = true;
        timer->stop();
        
        QMessageBox::information(this, "恭喜", "您已完成异形拼图！");
        
        // 发送成功信号
        emit sig_successRestart();
    }
}

void IrregularPuzzle::onPiecePlaced(IrregularPiece* piece)
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

void IrregularPuzzle::onPieceMoved(IrregularPiece* piece)
{
    if (piece) {
        // 检查拼图块是否在正确位置
        snapToGrid(piece);
    }
}

void IrregularPuzzle::init()
{
    // 生成目标位置
    generateTargetPositions();
    
    // 创建拼图块
    createPuzzlePieces();
    
    // 初始化UI
    updateUI();
}

void IrregularPuzzle::createPuzzlePieces()
{
    // 清空现有拼图块
    for (IrregularPiece* piece : _pieces) {
        delete piece;
    }
    _pieces.clear();
    
    // 获取拼图块数量
    int pieceCount = getPieceCountForDifficulty();
    
    // 确保目标位置数组已正确生成
    if (_targetPositions.size() != pieceCount) {
        qWarning() << "Target positions size mismatch. Expected:" << pieceCount << "Actual:" << _targetPositions.size();
        // 重新生成目标位置
        generateTargetPositions();
        if (_targetPositions.size() != pieceCount) {
            qCritical() << "Failed to generate target positions. Cannot create puzzle pieces.";
            return;
        }
    }
    
    // 创建拼图块
    for (int i = 0; i < pieceCount; ++i) {
        if (i >= _targetPositions.size()) {
            qCritical() << "Index out of bounds when accessing target positions:" << i << "Size:" << _targetPositions.size();
            break;
        }
        
        IrregularPiece::PieceType type = getRandomPieceType();
        QPixmap image; // 简化版本，不使用图片
        
        IrregularPiece* piece = new IrregularPiece(i, type, QPointF(0, 0), _pieceSize, image);
        
        // 设置初始位置为目标位置
        piece->setOriginalPosition(_targetPositions[i]);
        piece->setCurrentPosition(_targetPositions[i]); // 设置当前位置
        
        _pieces.append(piece);
    }
}

void IrregularPuzzle::setupScene()
{
    // 创建场景
    _scene = new QGraphicsScene(this);
    _scene->setBackgroundBrush(Qt::lightGray);
    
    // 设置图形视图
    _graphicsView = ui->graphicsView;
    _graphicsView->setScene(_scene);
    _graphicsView->setRenderHint(QPainter::Antialiasing);
    _graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    
    // 创建中央拼图区域
    createPuzzleArea();
}

void IrregularPuzzle::createPuzzleArea()
{
    // 在视图中央创建拼图区域
    QRectF viewRect = _graphicsView->rect();
    qreal areaSize = qMin(viewRect.width(), viewRect.height()) * 0.6;
    qreal areaX = (viewRect.width() - areaSize) / 2;
    qreal areaY = (viewRect.height() - areaSize) / 2;
    
    QRectF puzzleAreaRect(areaX, areaY, areaSize, areaSize);
    
    // 创建拼图区域边框
    _puzzleArea = new QGraphicsRectItem(puzzleAreaRect);
    _puzzleArea->setPen(QPen(Qt::darkGray, 3, Qt::DashLine));
    _puzzleArea->setBrush(QColor(240, 240, 240, 100));
    _puzzleArea->setZValue(0);
    _scene->addItem(_puzzleArea);
    
    // 添加标题
    QGraphicsTextItem* title = new QGraphicsTextItem("拼图区域");
    title->setPos(areaX, areaY - 30);
    title->setDefaultTextColor(Qt::darkGray);
    title->setFont(QFont("Arial", 12, QFont::Bold));
    _scene->addItem(title);
}

void IrregularPuzzle::shufflePieces()
{
    // 计算打乱后的位置
    QSize viewSize = _graphicsView->size();
    qreal margin = 50;
    
    for (IrregularPiece* piece : _pieces) {
        qreal x = margin + QRandomGenerator::global()->bounded(viewSize.width() - 2 * margin);
        qreal y = margin + QRandomGenerator::global()->bounded(viewSize.height() - 2 * margin);
        
        piece->setCurrentPosition(QPointF(x, y));
        piece->setRotation(QRandomGenerator::global()->bounded(4) * 90);
    }
}

void IrregularPuzzle::updateUI()
{
    ui->labelProgress->setText(QString("进度: %1/%2").arg(_placedPieces).arg(_pieces.size()));
    
    // 更新进度条
    if (_pieces.size() > 0) {
        int progress = (_placedPieces * 100) / _pieces.size();
        ui->progressBar->setValue(progress);
    }
}

void IrregularPuzzle::saveCurrentState()
{
    // 暂时禁用状态保存
}

void IrregularPuzzle::updateUndoButton()
{
    // 更新撤销按钮状态
    QPushButton* undoButton = findChild<QPushButton*>("btnUndo");
    if (undoButton) {
        undoButton->setEnabled(false);
    }
}

void IrregularPuzzle::snapToGrid(IrregularPiece* piece)
{
    QPointF expectedPos = piece->getOriginalPosition();
    QPointF currentPos = piece->pos();
    
    // 计算距离
    qreal distance = QLineF(currentPos, expectedPos).length();
    
    // 如果距离足够近且旋转角度正确，则吸附到正确位置
    if (distance < _pieceSize * 0.5 && piece->getRotation() % 360 == 0) {
        piece->setCurrentPosition(expectedPos);
        piece->setRotation(0);
        
        if (!piece->isPlaced()) {
            onPiecePlaced(piece);
        }
    }
}

void IrregularPuzzle::showHint()
{
    // 显示提示：高亮显示一个未放置的拼图块
    for (IrregularPiece* piece : _pieces) {
        if (!piece->isPlaced()) {
            piece->highlight(true);
            QTimer::singleShot(2000, [piece]() {
                piece->highlight(false);
            });
            break;
        }
    }
}

void IrregularPuzzle::arrangePiecesInGrid()
{
    // 在中央拼图区域按网格排列目标位置
    if (!_puzzleArea) return;
    
    QRectF areaRect = _puzzleArea->rect();
    int pieceCount = getPieceCountForDifficulty();
    int piecesPerRow = qCeil(qSqrt(pieceCount));
    qreal cellWidth = areaRect.width() / piecesPerRow;
    qreal cellHeight = areaRect.height() / piecesPerRow;
    
    for (int i = 0; i < pieceCount; ++i) {
        int row = i / piecesPerRow;
        int col = i % piecesPerRow;
        
        qreal x = areaRect.x() + col * cellWidth + cellWidth / 2;
        qreal y = areaRect.y() + row * cellHeight + cellHeight / 2;
        
        _targetPositions.append(QPointF(x, y));
    }
}

int IrregularPuzzle::getPieceCountForDifficulty()
{
    // 返回适合的拼图块数量
    return 16; // 4x4网格
}

void IrregularPuzzle::generateTargetPositions()
{
    _targetPositions.clear();
    arrangePiecesInGrid();
}

IrregularPiece::PieceType IrregularPuzzle::getRandomPieceType()
{
    static QVector<IrregularPiece::PieceType> allTypes = {
        IrregularPiece::PieceType::TRIANGLE,
        IrregularPiece::PieceType::SQUARE,
        IrregularPiece::PieceType::PENTAGON,
        IrregularPiece::PieceType::HEXAGON,
        IrregularPiece::PieceType::CIRCLE,
        IrregularPiece::PieceType::STAR,
        IrregularPiece::PieceType::TRAPEZOID,
        IrregularPiece::PieceType::RHOMBUS
    };
    
    return allTypes[QRandomGenerator::global()->bounded(allTypes.size())];
}