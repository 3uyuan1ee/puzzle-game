#ifndef LEVELSELECT_H
#define LEVELSELECT_H

#include <QDialog>
#include <QVector>
#include "play4x4.h"
#include "LevelConfig.h"

namespace Ui {
class LevelSelect;
}

class LevelSelect : public QDialog
{
    Q_OBJECT

public:
    explicit LevelSelect(int maxUnlockedLevel, QWidget *parent = nullptr);
    ~LevelSelect();

    void setLevels(const QVector<LevelConfig>& levels);
    void updateLevelButtons();
    void loadProgress();
    void saveProgress();

signals:
    void levelSelected(int level, int rows, int cols,LevelSelect *widget);
    void backToMenu();
    void progressReset();

private slots:
    void onLevelButtonClicked();
    void on_btnBack_clicked();
    void on_btnResetProgress_clicked();

private:
    Ui::LevelSelect *ui;
    QVector<LevelConfig> m_levels;
    int m_maxUnlockedLevel;
    QVector<QPushButton*> m_levelButtons;

    void initLevels();
    void createLevelButtons();
    void updateButtonStyle(QPushButton* button, const LevelConfig& level);
};

#endif // LEVELSELECT_H
