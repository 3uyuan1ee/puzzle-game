#ifndef CUSTOMMODE_H
#define CUSTOMMODE_H

#include <QDialog>
#include <QPixmap>
#include "play4x4.h"

namespace Ui {
class CustomMode;
}

class CustomMode : public QDialog
{
    Q_OBJECT

public:
    explicit CustomMode(QWidget *parent = nullptr);
    ~CustomMode();

signals:
    void startCustomGame(int rows, int cols, const QString& imagePath,CustomMode *widget);
    void backToMenu();

private slots:
    void on_btnUploadImage_clicked();
    void on_btnStartGame_clicked();
    void on_btnBack_clicked();
    void on_btnPresetSize_clicked();
    void on_spinRows_valueChanged(int arg1);
    void on_spinCols_valueChanged(int arg1);

private:
    Ui::CustomMode *ui;
    QString m_imagePath;
    QPixmap m_previewPixmap;

    void updatePreview();
    void validateInputs();
    void updatePresetButtons();
};

#endif // CUSTOMMODE_H
