#include "StepRankingDialog.h"
#include "ui_StepRankingDialog.h"
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QPushButton>

StepRankingDialog::StepRankingDialog(NetworkClient *networkClient, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::StepRankingDialog)
    , network_client(networkClient)
{
    ui->setupUi(this);
    
    // 设置窗口属性
    setModal(true);
    setWindowTitle("步数排行榜");
    
    // 连接信号
    connect(network_client, &NetworkClient::stepRankingsFinished, this, &StepRankingDialog::onStepRankingsFinished);
    
    // 初始化表格
    clearTable();
    showStatus("选择规格和撤销功能后点击查询");
}

StepRankingDialog::~StepRankingDialog()
{
    delete ui;
}

void StepRankingDialog::on_btnRefresh_clicked()
{
    loadRankings();
}

void StepRankingDialog::on_btnBack_clicked()
{
    accept();
}

void StepRankingDialog::onStepRankingsFinished(const NetworkResponse &response, const QList<StepRankingInfo> &rankings)
{
    if (response.success) {
        updateTable(rankings);
        showStatus(QString("查询成功，共 %1 条记录").arg(rankings.size()));
    } else {
        showStatus(response.message, true);
    }
}

void StepRankingDialog::loadRankings()
{
    if (!network_client->isConnected()) {
        showStatus("网络连接失败", true);
        return;
    }
    
    // 获取选择的规格
    QString gridSizeText = ui->comboBoxGridSize->currentText();
    int gridSize = gridSizeText.left(1).toInt(); // 提取数字
    
    // 获取撤销功能选择
    bool usedUndo = false;
    QString undoText = ui->comboBoxUndo->currentText();
    if (undoText == "未使用撤销") {
        usedUndo = false;
    } else if (undoText == "使用撤销") {
        usedUndo = true;
    } else {
        // "不限" - 简化处理，只查询未使用撤销的情况
        usedUndo = false;
    }
    
    ui->btnRefresh->setEnabled(false);
    showStatus("正在查询排行榜数据...");
    
    network_client->getStepRankings(gridSize, usedUndo, 50);
}

void StepRankingDialog::updateTable(const QList<StepRankingInfo> &rankings)
{
    clearTable();
    
    for (int i = 0; i < rankings.size(); ++i) {
        const StepRankingInfo &info = rankings[i];
        
        // 排名
        QTableWidgetItem *rankItem = new QTableWidgetItem(QString::number(i + 1));
        rankItem->setTextAlignment(Qt::AlignCenter);
        
        // 设置排名样式
        if (i == 0) {
            rankItem->setForeground(QColor(255, 215, 0)); // 金色
            rankItem->setFont(QFont("Arial", 10, QFont::Bold));
        } else if (i == 1) {
            rankItem->setForeground(QColor(192, 192, 192)); // 银色
            rankItem->setFont(QFont("Arial", 10, QFont::Bold));
        } else if (i == 2) {
            rankItem->setForeground(QColor(205, 127, 50)); // 铜色
            rankItem->setFont(QFont("Arial", 10, QFont::Bold));
        }
        
        // 用户名
        QTableWidgetItem *usernameItem = new QTableWidgetItem(info.username);
        
        // 昵称
        QTableWidgetItem *nicknameItem = new QTableWidgetItem(info.nickname);
        
        // 规格
        QTableWidgetItem *gridItem = new QTableWidgetItem(QString("%1x%1").arg(info.grid_size));
        gridItem->setTextAlignment(Qt::AlignCenter);
        
        // 步数
        QTableWidgetItem *stepItem = new QTableWidgetItem(QString::number(info.step_count));
        stepItem->setTextAlignment(Qt::AlignCenter);
        stepItem->setFont(QFont("Arial", 10, QFont::Bold));
        
        // 撤销
        QTableWidgetItem *undoItem = new QTableWidgetItem(info.used_undo ? "是" : "否");
        undoItem->setTextAlignment(Qt::AlignCenter);
        if (info.used_undo) {
            undoItem->setForeground(QColor(255, 152, 0)); // 橙色
        } else {
            undoItem->setForeground(QColor(76, 175, 80)); // 绿色
        }
        
        // 完成时间
        QTableWidgetItem *createTimeItem = new QTableWidgetItem(info.create_time);
        
        // 添加到表格
        int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);
        ui->tableWidget->setItem(row, 0, rankItem);
        ui->tableWidget->setItem(row, 1, usernameItem);
        ui->tableWidget->setItem(row, 2, nicknameItem);
        ui->tableWidget->setItem(row, 3, gridItem);
        ui->tableWidget->setItem(row, 4, stepItem);
        ui->tableWidget->setItem(row, 5, undoItem);
        ui->tableWidget->setItem(row, 6, createTimeItem);
    }
    
    // 调整列宽
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
}

void StepRankingDialog::clearTable()
{
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(7);
    
    // 设置列标题
    QStringList headers;
    headers << "排名" << "用户名" << "昵称" << "规格" << "步数" << "撤销" << "完成时间";
    ui->tableWidget->setHorizontalHeaderLabels(headers);
}

void StepRankingDialog::showStatus(const QString &message, bool isError)
{
    ui->labelStatus->setText(message);
    if (isError) {
        ui->labelStatus->setStyleSheet("color: #f44336; font-weight: bold;");
    } else {
        ui->labelStatus->setStyleSheet("color: #FF9800; font-weight: normal;");
    }
    ui->btnRefresh->setEnabled(true);
}