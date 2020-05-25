#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QLabel>
#include <QPainter>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QLabel *label = new QLabel(this);
    m_icon = paintText(tr("Hello"));
    label->setPixmap(m_icon);
    this->setCentralWidget(label);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QPixmap MainWindow::paintText(const QString &text)
{
    QPixmap icon(128,128);  // Construct with 128 128 image
    icon.fill(Qt::black);   // Clear buffer with white color,or image will have noise
    QPainter painter(&icon); // set painter paint to QPaintDevice
    /*Draw Rect frame first*/
    const int frameWidth = 1;
    QPen framePen(Qt::green,frameWidth,Qt::SolidLine);
    painter.setPen(framePen);
    const int margin = 2;
    const QRect frameRect = icon.rect().marginsRemoved(QMargins(margin,margin,margin,margin));
    painter.drawRect(frameRect);
    /*Draw Text*/
    QPen textPen(Qt::yellow,1,Qt::SolidLine);
    painter.setPen(textPen);
    painter.drawText(frameRect,Qt::AlignCenter,text);
    return icon;
}
