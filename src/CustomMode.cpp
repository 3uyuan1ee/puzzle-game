#include "CustomMode.h"
#include "ui_CustomMode.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>

CustomMode::CustomMode(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CustomMode)
{
    ui->setupUi(this);
    
    // 设置窗口标题和属性
    setWindowTitle("自定义模式");
    setFixedSize(600, 500);
    
    // 初始化界面
    ui->spinRows->setMinimum(2);
    ui->spinRows->setMaximum(10);
    ui->spinRows->setValue(4);
    
    ui->spinCols->setMinimum(2);
    ui->spinCols->setMaximum(10);
    ui->spinCols->setValue(4);
    
    // 设置预览标签
    ui->labelPreview->setAlignment(Qt::AlignCenter);
    ui->labelPreview->setMinimumSize(200, 200);
    ui->labelPreview->setStyleSheet("border: 2px dashed #cccccc; background-color: #f9f9f9;");
    ui->labelPreview->setText("请上传图片");
    
    // 更新预设按钮
    updatePresetButtons();
    
    // 连接信号
    connect(ui->btnUploadImage, &QPushButton::clicked, this, &CustomMode::on_btnUploadImage_clicked);
    connect(ui->btnStartGame, &QPushButton::clicked, this, &CustomMode::on_btnStartGame_clicked);
    connect(ui->btnBack, &QPushButton::clicked, this, &CustomMode::on_btnBack_clicked);
    connect(ui->btnPresetSize, &QPushButton::clicked, this, &CustomMode::on_btnPresetSize_clicked);
    connect(ui->spinRows, QOverload<int>::of(&QSpinBox::valueChanged), this, &CustomMode::on_spinRows_valueChanged);
    connect(ui->spinCols, QOverload<int>::of(&QSpinBox::valueChanged), this, &CustomMode::on_spinCols_valueChanged);
    
    // 初始验证输入
    validateInputs();
}

CustomMode::~CustomMode()
{
    delete ui;
}

void CustomMode::on_btnUploadImage_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this, 
        tr("选择图片"), 
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
        tr("图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)")
    );
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // 检查图片是否有效
    QPixmap testPixmap(fileName);
    if (testPixmap.isNull()) {
        QMessageBox::warning(this, tr("错误"), tr("无法加载图片文件，请选择有效的图片格式。"));
        return;
    }
    
    // 检查图片尺寸
    if (testPixmap.width() < 200 || testPixmap.height() < 200) {
        QMessageBox::warning(this, tr("错误"), tr("图片尺寸太小，请选择至少200x200像素的图片。"));
        return;
    }
    
    // 保存图片到缓存目录
    QDir cacheDir(QCoreApplication::applicationDirPath() + "/cache");
    if (!cacheDir.exists()) {
        cacheDir.mkpath(".");
    }
    
    QString cachePath = cacheDir.filePath("custom_image.png");
    if (!testPixmap.save(cachePath, "PNG")) {
        QMessageBox::warning(this, tr("错误"), tr("无法保存图片到缓存目录。"));
        return;
    }
    
    m_imagePath = cachePath;
    m_previewPixmap = testPixmap;
    updatePreview();
    validateInputs();
    
    QMessageBox::information(this, tr("成功"), tr("图片上传成功！"));
}

void CustomMode::on_btnStartGame_clicked()
{
    if (m_imagePath.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("请先上传图片。"));
        return;
    }
    
    int rows = ui->spinRows->value();
    int cols = ui->spinCols->value();
    
    // 检查拼图块数量是否合理
    int totalPieces = rows * cols;
    if (totalPieces > 64) {
        QMessageBox::warning(this, tr("错误"), tr("拼图块数量过多，请减少行数或列数（最大64块）。"));
        return;
    }
    
    if (totalPieces < 4) {
        QMessageBox::warning(this, tr("错误"), tr("拼图块数量过少，请增加行数或列数（最少4块）。"));
        return;
    }
    
    emit startCustomGame(rows, cols, m_imagePath,this);
    close();
}

void CustomMode::on_btnBack_clicked()
{
    emit backToMenu();
    close();
}

void CustomMode::on_btnPresetSize_clicked()
{
    // 预设尺寸循环切换
    static int presetIndex = 0;
    QVector<QPair<int, int>> presets = {
        {3, 3}, {4, 4}, {5, 5}, {6, 6}, {4, 5}, {5, 6}, {6, 8}
    };
    
    presetIndex = (presetIndex + 1) % presets.size();
    ui->spinRows->setValue(presets[presetIndex].first);
    ui->spinCols->setValue(presets[presetIndex].second);
    
    // 更新按钮文本
    QString btnText = QString("预设: %1x%2").arg(presets[presetIndex].first).arg(presets[presetIndex].second);
    ui->btnPresetSize->setText(btnText);
}

void CustomMode::on_spinRows_valueChanged(int arg1)
{
    Q_UNUSED(arg1);
    validateInputs();
}

void CustomMode::on_spinCols_valueChanged(int arg1)
{
    Q_UNUSED(arg1);
    validateInputs();
}

void CustomMode::updatePreview()
{
    if (m_previewPixmap.isNull()) {
        ui->labelPreview->setText("请上传图片");
        ui->labelPreview->clear();
    } else {
        // 缩放图片以适应预览区域
        QPixmap scaled = m_previewPixmap.scaled(
            ui->labelPreview->size(), 
            Qt::KeepAspectRatio, 
            Qt::SmoothTransformation
        );
        ui->labelPreview->setPixmap(scaled);
        
        // 更新信息标签
        int pieces = ui->spinRows->value() * ui->spinCols->value();
        ui->labelInfo->setText(QString("图片尺寸: %1x%2\n拼图规格: %3x%4 (%5块)")
                              .arg(m_previewPixmap.width())
                              .arg(m_previewPixmap.height())
                              .arg(ui->spinRows->value())
                              .arg(ui->spinCols->value())
                              .arg(pieces));
    }
}

void CustomMode::validateInputs()
{
    bool valid = true;
    
    // 检查是否上传了图片
    if (m_imagePath.isEmpty()) {
        valid = false;
    }
    
    // 检查拼图块数量
    int rows = ui->spinRows->value();
    int cols = ui->spinCols->value();
    int pieces = rows * cols;
    
    if (pieces < 4 || pieces > 64) {
        valid = false;
    }
    
    // 启用/禁用开始按钮
    ui->btnStartGame->setEnabled(valid);
    
    // 更新按钮样式
    if (valid) {
        ui->btnStartGame->setStyleSheet(
            "QPushButton {"
            "background-color: #90EE90;"
            "color: #006400;"
            "border: 2px solid #32CD32;"
            "border-radius: 8px;"
            "font: bold 14px;"
            "padding: 8px;"
            "}"
            "QPushButton:hover {"
            "background-color: #7FDD7F;"
            "border: 2px solid #228B22;"
            "}"
        );
    } else {
        ui->btnStartGame->setStyleSheet(
            "QPushButton {"
            "background-color: #cccccc;"
            "color: #666666;"
            "border: 2px solid #999999;"
            "border-radius: 8px;"
            "font: bold 14px;"
            "padding: 8px;"
            "}"
        );
    }
    
    // 更新预览和信息
    updatePreview();
}

void CustomMode::updatePresetButtons()
{
    // 设置初始预设按钮文本
    ui->btnPresetSize->setText("预设: 3x3");
}
