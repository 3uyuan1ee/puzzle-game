#ifndef DLGMENU_H
#define DLGMENU_H
#include <QDialog>
#include <QImage>
#include <QPixmap>
#include <QFileDialog>
#include <QMessageBox>
#include <QScreen>
#include <QGuiApplication>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QSlider>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSettings>
#include "play4x4.h"
#include "IrregularPuzzle.h"
#include "LevelSelect.h"
#include "CustomMode.h"
#include "LevelConfig.h"
#include "NetworkClient.h"
#include "RankingDialog.h"

extern QString _strPos;

namespace Ui {
class dlgMenu;
}

class DlgMenu : public QDialog
{
    Q_OBJECT

public:
    explicit DlgMenu(QWidget *parent = nullptr);
    ~DlgMenu();
    static QString diyImagePath;   // 记录用户选图后保存的完整路径

private:
    Ui::dlgMenu *ui;
    play4x4 *_dlgPlay4;
    IrregularPuzzle *_dlgIrregular;
    LevelSelect *m_levelSelect;
    CustomMode *m_customMode;
    NetworkClient *network_client;
    RankingDialog *ranking_dialog;

    // 闯关模式相关
    int currentLevel;
    int maxUnlockedLevel;
    QVector<LevelConfig> levels;
    
    // 音乐播放器相关
    QMediaPlayer *musicPlayer;
    QAudioOutput *audioOutput;
    QSlider *volumeSlider;
    QLabel *musicStatusLabel;
    QPushButton *btnImportMusic;
    QPushButton *btnPlayPause;
    QPushButton *btnStopMusic;
    QWidget *musicControlWidget;
    QString currentMusicPath;
    bool isMusicPlaying;
    
    void init();
    void initLevels();
    LevelConfig generateRandomLevel();
    void saveProgress();
    void loadProgress();
    void updateUI();
    void connectGameSignals();
    void initNetwork();
    
    // 音乐播放器相关方法
    void initMusicPlayer();
    void createMusicControls();
    void cleanupMusicPlayer();
    void importMusic();
    void playPauseMusic();
    void stopMusic();
    void onVolumeChanged(int volume);
    void onMusicStateChanged(QMediaPlayer::PlaybackState state);
    void saveMusicSettings();
    void loadMusicSettings();

private slots:
    void on_btStart_clicked();
    void on_btnExit_clicked();
    void on_btnLevelSelect_clicked();
    void on_btnCustomMode_clicked();
    void on_btnJigsawMode_clicked();
    void on_btnRanking_clicked();
    void onLevelSelected(int level, int rows, int cols,LevelSelect* widget);
    void onStartCustomGame(int rows, int cols, const QString& imagePath,CustomMode *widget);
    void onBackFromIrregular();
    void onBackToMenu();
    void onProgressReset();
    void onBackFromRanking();
    
    // 音乐控制槽函数
    void on_btnImportMusic_clicked();
    void on_btnPlayPause_clicked();
    void on_btnStopMusic_clicked();
};

#endif // DLGMENU_H
