#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "downloaderdialog_v1.h"
#include "downloaderdialog_v2.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_actionDownloader_Example_1_triggered()
{
    qInfo() << Q_FUNC_INFO;
    auto dialog = new DownloaderDialog_V1(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    if(dialog->exec() == QDialog::Accepted){
        qDebug() << "Accepted";
    }else{
        qDebug() << "Rejected";
    }
}

void MainWindow::on_actionDownloader_Example_2_triggered()
{
    qInfo() << Q_FUNC_INFO;
    auto dialog = new DownloaderDialog_V2(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    if(dialog->exec() == QDialog::Accepted){
        qDebug() << "Accepted";
    }else{
        qDebug() << "Rejected";
    }
}
