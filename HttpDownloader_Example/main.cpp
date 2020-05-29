#include "downloaddialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DownloadDialog w;
    w.show();

    return a.exec();
}
