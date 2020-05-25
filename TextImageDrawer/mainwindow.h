#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    QPixmap paintText(const QString &text);
private:
    Ui::MainWindow *ui;
    QPixmap m_icon;
};

#endif // MAINWINDOW_H
