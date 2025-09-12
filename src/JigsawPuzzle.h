#ifndef JIGSAWPUZZLE_H
#define JIGSAWPUZZLE_H

#include <QDialog>
#include <QTimer>
#include <QTime>
#include <QPixmap>
#include <QLabel>
#include <QVector>
#include <QPolygon>
#include <QPainterPath>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QTransform>
#include "NetworkClient.h"

namespace Ui {
class JigsawPuzzle;
}

// 异形拼图块类
class JigsawPiece : public QGraphicsItem
{
public:
    enum class ShapeType {
        STRAIGHT,
        TAB,
        BLANK
    };

    JigsawPiece(int id, int row, int col, int totalRows, int totalCols, const QPixmap& image, const QRect& sourceRect);
    
    void setOriginalPosition(const QPointF& pos);
    void setCurrentPosition(const QPointF& pos);
    void setRotation(int angle);
    int getRotation() const { return _rotation; }
    bool isPlaced() const { return _isPlaced; }
    void setPlaced(bool placed) { _isPlaced = placed; }
    
    int getId() const { return _id; }
    int getRow() const { return _row; }
    int getCol() const { return _col; }
    
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    
    QPainterPath getShapePath() const;
    bool matchesNeighbor(JigsawPiece* neighbor, const QString& direction) const;
    
    void highlight(bool highlight);
    void setOpacity(qreal opacity);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    int _id;
    int _row;
    int _col;
    int _totalRows;
    int _totalCols;
    QPixmap _image;
    QRect _sourceRect;
    QPointF _originalPosition;
    QPointF _currentPosition;
    int _rotation;
    bool _isPlaced;
    bool _isHighlighted;
    
    // 拼图块的形状定义
    ShapeType _topShape;
    ShapeType _bottomShape;
    ShapeType _leftShape;
    ShapeType _rightShape;
    
    void generateShape();
    JigsawPiece* getPieceAt(int row, int col);
    void drawTargetArea();
    QPainterPath createPiecePath() const;
    qreal getTabSize() const;
    void drawTab(QPainterPath& path, const QPointF& center, ShapeType type, qreal size) const;
};

// 异形拼图游戏主类
class JigsawPuzzle : public QDialog
{
    Q_OBJECT

public:
    explicit JigsawPuzzle(int rows = 6, int cols = 6, QWidget *parent = nullptr, NetworkClient *networkClient = nullptr);
    ~JigsawPuzzle();

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
    void onPiecePlaced(JigsawPiece* piece);
    void onPieceMoved(JigsawPiece* piece);

private:
    Ui::JigsawPuzzle *ui;
    bool _bStart;
    int _iStep;
    QTimer* timer;
    QTime time;
    QString _strPos;
    NetworkClient *network_client;

    int _rows;
    int _cols;
    int _totalPieces;
    
    QGraphicsScene* _scene;
    QGraphicsView* _graphicsView;
    
    QVector<JigsawPiece*> _pieces;
    QVector<QVector<int>> _puzzleMap;
    QPixmap _originalImage;
    QRect _targetArea;
    
    // 游戏状态
    int _placedPieces;
    bool _gameWon;
    
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
    void shufflePieces();
    void updateUI();
    void saveCurrentState();
    void updateUndoButton();
    bool checkPieceConnection(JigsawPiece* piece);
    void snapToGrid(JigsawPiece* piece);
    void showHint();
    void generatePieceShapes();
    int calculateDifficulty();
    JigsawPiece* getPieceAt(int row, int col);
    void drawTargetArea();
};

#endif // JIGSAWPUZZLE_H