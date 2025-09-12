#include "LevelRankingDialog.h"
#include "ui_LevelRankingDialog.h"
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QPushButton>

LevelRankingDialog::LevelRankingDialog(NetworkClient *networkClient, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LevelRankingDialog)
    , network_client(networkClient)
{
    ui->setupUi(this);
    
    // 设置窗口属性
    setModal(true);
    setWindowTitle("闯关排行榜");
    
    // 连接信号
    connect(network_client, &NetworkClient::levelRankingsFinished, this, &LevelRankingDialog::onLevelRankingsFinished);
    
    // 初始化表格
    clearTable();
    showStatus("点击刷新获取排行榜数据");
}

LevelRankingDialog::~LevelRankingDialog()
{
    delete ui;
}

void LevelRankingDialog::on_btnRefresh_clicked()
{
    loadRankings();
}

void LevelRankingDialog::on_btnBack_clicked()
{
    accept();
}

void LevelRankingDialog::onLevelRankingsFinished(const NetworkResponse &response, const QList<LevelRankingInfo> &rankings)
{
    if (response.success) {
        updateTable(rankings);
        showStatus(QString("加载成功，共 %1 条记录").arg(rankings.size()));
    } else {
        showStatus(response.message, true);
    }
}

void LevelRankingDialog::loadRankings()
{
    if (!network_client->isConnected()) {
        showStatus("网络连接失败", true);
        return;
    }
    
    ui->btnRefresh->setEnabled(false);
    showStatus("正在加载排行榜数据...");
    
    network_client->getLevelRankings(50); // 获取前50名
}

void LevelRankingDialog::updateTable(const QList<LevelRankingInfo> &rankings)
{
    clearTable();
    
    for (int i = 0; i < rankings.size(); ++i) {
        const LevelRankingInfo &info = rankings[i];
        
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
        
        // 最高关卡
        QTableWidgetItem *levelItem = new QTableWidgetItem(QString::number(info.max_level));
        levelItem->setTextAlignment(Qt::AlignCenter);
        levelItem->setFont(QFont("Arial", 10, QFont::Bold));
        
        // 更新时间
        QTableWidgetItem *timeItem = new QTableWidgetItem(info.update_time);
        
        // 添加到表格
        int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);
        ui->tableWidget->setItem(row, 0, rankItem);
        ui->tableWidget->setItem(row, 1, usernameItem);
        ui->tableWidget->setItem(row, 2, nicknameItem);
        ui->tableWidget->setItem(row, 3, levelItem);
        ui->tableWidget->setItem(row, 4, timeItem);
    }
    
    // 调整列宽
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
}

void LevelRankingDialog::clearTable()
{
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(5);
    
    // 设置列标题
    QStringList headers;
    headers << "排名" << "用户名" << "昵称" << "最高关卡" << "更新时间";
    ui->tableWidget->setHorizontalHeaderLabels(headers);
}

void LevelRankingDialog::showStatus(const QString &message, bool isError)
{
    ui->labelStatus->setText(message);
    if (isError) {
        ui->labelStatus->setStyleSheet("color: #f44336; font-weight: bold;");
    } else {
        ui->labelStatus->setStyleSheet("color: #4CAF50; font-weight: normal;");
    }
    ui->btnRefresh->setEnabled(true);
}