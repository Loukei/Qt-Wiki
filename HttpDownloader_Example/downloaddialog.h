#ifndef DOWNLOADDIALOG_H
#define DOWNLOADDIALOG_H

/*!
 * \class DownloadDialog
 * \brief A simple example to download network resource
 *
 * ## Reference
 * - [QtDoc HTTP Example](https://doc.qt.io/qt-5/qtnetwork-http-example.html)
 * - [QprogressDialog appears at creation with Qt5.5rc](https://bugreports.qt.io/browse/QTBUG-47042)
*/

#include <QDialog>
#include <QPointer>
#include <QProgressDialog>
#include <QNetworkReply>

namespace Ui {
class DownloadDialog;
}

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
class QFile;
QT_END_NAMESPACE

class NetworkProgressDialog : public QProgressDialog
{
    Q_OBJECT
public:
    explicit NetworkProgressDialog(const QUrl &url,QWidget *parent = nullptr);
public slots:
    void onReplyProgress(qint64 bytesReceived, qint64 bytesTotal);
};

class DownloadDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DownloadDialog(QWidget *parent = nullptr);
    ~DownloadDialog();

    void requestDownload(const QUrl &url);

private slots:
    /// close window
    void on_pushButton_Quit_clicked();
    /// check user input url,
    /// if it's legal, open FileDialog to choose dir
    /// then start request url
    void on_pushButton_Download_clicked();
    /// Disable download button if url is empty
    void on_lineEdit_Url_textChanged(const QString &arg1);

    /// write file when network ready to read
    void onhttp_ReadyRead();
    /// process reply when finished/abort/error
    void onhttp_Finished();
    /// print error message
    void onhttp_Error(QNetworkReply::NetworkError);
    /// abort download when user click cancel in progressDialog
    void onProgressDialog_canceled();

private:
    /// get filename form Url
    QString getFileName(const QUrl &url) const;
    /// try to create file and open,return 0 if not success
    QFile *createOpenFile(const QString &filepath);

private:
    Ui::DownloadDialog *ui;
    QNetworkAccessManager *m_manager;
    /// current reply create by m_manager
    QPointer<QNetworkReply> m_reply;
    /// the file ready to write,m_file wiil be delete and set to 0 if file open failed
    QPointer<QFile> m_file;
    /// a value to determine if user click abort
    bool m_AbortRequest = false;
};

#endif // DOWNLOADDIALOG_H
