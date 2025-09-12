#ifndef IRREGULARPUZZLE_H
#define IRREGULARPUZZLE_H

#include <QDialog>
#include <QTimer>
#include <QTime>
#include <QPixmap>
#include <QVector>
#include <QPolygon>
#include <QPainterPath>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QTransform>
#include <QPointF>
#include <QMediaPlayer>
#include "NetworkClient.h"

namespace Ui {
class IrregularPuzzle;
}

// 不规则拼图块类
class IrregularPiece : public QGraphicsItem
{
public:
    enum class PieceType {
        TRIANGLE,
        SQUARE,
        PENTAGON,
        HEXAGON,
        CIRCLE,
        STAR,
        TRAPEZOID,
        RHOMBUS
    };

    IrregularPiece(int id, PieceType type, const QPointF& centerPos, qreal size, const QPixmap& image);
    
    void setOriginalPosition(const QPointF& pos);
    void setCurrentPosition(const QPointF& pos);
    void setRotation(int angle);
    int getRotation() const { return _rotation; }
    bool isPlaced() const { return _isPlaced; }
    void setPlaced(bool placed) { _isPlaced = placed; }
    
    int getId() const { return _id; }
    PieceType getPieceType() const { return _pieceType; }
    QPointF getOriginalPosition() const { return _originalPosition; }
    
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    
    QPainterPath getShapePath() const;
    bool contains(const QPointF &point) const override;
    void highlight(bool highlight);
    void setOpacity(qreal opacity);
    
    // 形状匹配检查
    bool canConnectTo(IrregularPiece* other) const;
    bool isInCorrectPosition() const;
    
    // 右键旋转
    void rotate90Degrees();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    int _id;
    PieceType _pieceType;
    QPixmap _image;
    QPointF _originalPosition;
    QPointF _currentPosition;
    qreal _size;
    int _rotation;
    bool _isPlaced;
    bool _isHighlighted;
    
    // 创建不同形状的路径
    QPainterPath createTrianglePath() const;
    QPainterPath createSquarePath() const;
    QPainterPath createPentagonPath() const;
    QPainterPath createHexagonPath() const;
    QPainterPath createCirclePath() const;
    QPainterPath createStarPath() const;
    QPainterPath createTrapezoidPath() const;
    QPainterPath createRhombusPath() const;
    
    QPolygonF createRegularPolygon(int sides) const;
    QColor getPieceColor() const;
    
    // 形状特征
    QVector<QPointF> _connectionPoints;
    void generateConnectionPoints();
};

// 不规则拼图游戏主类
class IrregularPuzzle : public QDialog
{
    Q_OBJECT

public:
    explicit IrregularPuzzle(QWidget *parent = nullptr, NetworkClient *networkClient = nullptr, QMediaPlayer *musicPlayer = nullptr);
    ~IrregularPuzzle();

    void start();
    void setNetworkClient(NetworkClient *client);

signals:
    void sig_restart();
    void sig_back();
    void sig_return();
    void sig_successRestart();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void on_btnBack_clicked();
    void on_btnRestart_clicked();
    void on_btnReturn_clicked();
    void on_btnShowImage_clicked();
    void updateTimer();
    void checkWinCondition();
    void onPiecePlaced(IrregularPiece* piece);
    void onPieceMoved(IrregularPiece* piece);

private:
    Ui::IrregularPuzzle *ui;
    bool _bStart;
    int _iStep;
    QTimer* timer;
    QTime time;
    QString _strPos;
    NetworkClient *network_client;
    QMediaPlayer *backgroundMusicPlayer;

    QGraphicsScene* _scene;
    QGraphicsView* _graphicsView;
    QGraphicsRectItem* _puzzleArea;  // 中央拼图区域
    
    QVector<IrregularPiece*> _pieces;
    QVector<QPointF> _targetPositions;
    QPixmap _originalImage;
    
    // 游戏状态
    int _placedPieces;
    bool _gameWon;
    qreal _pieceSize;
    
    // 撤销功能相关
    struct GameState {
        QVector<QPointF> piecePositions;
        QVector<int> pieceRotations;
        QVector<bool> piecePlacedStates;
        int stepCount;
        QTime gameTime;
    };
    QVector<GameState> _historyStack;
    int _maxHistorySize;
    bool _isUndoing;
    
    void init();
    void createPuzzlePieces();
    void setupScene();
    void createPuzzleArea();
    void shufflePieces();
    void updateUI();
    void saveCurrentState();
    void updateUndoButton();
    void snapToGrid(IrregularPiece* piece);
    void showHint();
    void arrangePiecesInGrid();
    
    // 难度设置
    int getPieceCountForDifficulty();
    void generateTargetPositions();
    IrregularPiece::PieceType getRandomPieceType();
};

#endif // IRREGULARPUZZLE_H