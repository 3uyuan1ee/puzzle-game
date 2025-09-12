
#ifndef PLAY4X4_H
#define PLAY4X4_H

#include <QDialog>
#include <QTimer>
#include <QTime>
#include <QPixmap>
#include <QLabel>
#include <QVector>
#include <QMediaPlayer>
#include <QAudioOutput>
#include "NetworkClient.h"

namespace Ui {
class play4x4;
}

class play4x4 : public QDialog
{
    Q_OBJECT

public:
    explicit play4x4(int rows = 4, int cols = 4, QWidget *parent = nullptr, NetworkClient *networkClient = nullptr, QMediaPlayer *musicPlayer = nullptr);
    ~play4x4();

    void start();
    void setNetworkClient(NetworkClient *client);
    void setMusicPlayer(QMediaPlayer *player);

signals:
    void sig_restart();
    void sig_back();
    void sig_return();
    void sig_successRestart();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *e) override;

private slots:
    void on_btnBack_clicked();
    void on_btnRestart_clicked();
    void on_btnreturn_clicked();
    void on_pushButton_3_clicked();
    void update();
    void SuccessRestart();

    void on_pushButton_2_clicked();

private:
    Ui::play4x4 *ui;
    bool _bStart;
    int _iStep;
    QLabel* _draggedLabel;
    int _draggedIndex;
    QPoint _dragStartPosition;
    QTimer* timer;
    QTime time;
    QString _strPos;
    NetworkClient *network_client;

    int _rows;
    int _cols;
    int _totalPieces;
    QVector<QVector<int>> _iarrMap;
    QVector<QVector<int>> _iarrRot;
    QVector<QLabel*> _labels;
    QVector<QPixmap> _pieceImages;
    
    // Sound effects
    QMediaPlayer* _moveSound;
    QMediaPlayer* _rotateSound;
    QMediaPlayer* _correctSound;
    
    // Sound effect logic
    int _lastCorrectPieceCount;
    
    // Background music player
    QMediaPlayer *_backgroundMusicPlayer;

    // 撤销功能相关
    struct GameState {
        QVector<QVector<int>> mapState;
        QVector<QVector<int>> rotState;
        int stepCount;
        QTime gameTime;
    };
    QVector<GameState> _historyStack;  // 历史状态栈
    int _maxHistorySize;               // 最大历史记录数
    bool _isUndoing;                   // 撤销操作标志
    
    void saveCurrentState();           // 保存当前状态
    void updateUndoButton();           // 更新撤销按钮状态
    void initSoundEffects();           // 初始化音效

    void init();
    void createLabels();
    void upset();
    void showpicture();
    bool bSuccessful();
    QLabel* getLabelAtGridPos(const QPoint& globalPos);
    QPoint getGridPositionFromGlobalPos(const QPoint& globalPos);
    void submitGameResult();
int b[100];
int ss;
};


extern int save;
extern int xx;
extern int state;

#endif // PLAY4X4_H
