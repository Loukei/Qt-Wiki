#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionDownloader_Example_1_triggered();

    void on_actionDownloader_Example_2_triggered();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
