#ifndef DOWNLOADERDIALOG_V2_H
#define DOWNLOADERDIALOG_V2_H

#include <QDialog>

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSaveFile>
#include <QPointer>

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
QT_END_NAMESPACE

namespace Ui {
class DownloaderDialog_V2;
}

class DownloadManager : public QObject
{
    Q_OBJECT
public:
    DownloadManager(QObject *parent = nullptr);
    ~DownloadManager();
    /// check some conditions,if all true, send download request
    /// 1. No any unfinished download?
    /// 2. filepath is correct and open file success?
    bool startDownload(const QUrl &url,const QString &filepath);

    inline bool hasRunningProcess() const;

public slots:
    /// cancle download, dump file and abort reply
    void abort();
    /// abort reply,BUT KEEP THE FILE until user call resume()
    /// startDownload() will block
    void pause();
    /// resume the current download
    void resume();

signals:
    void finished();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void debugInfo(QString info);

private:
    QNetworkAccessManager *m_manager;
    /// Use setFileName to point on different file
    QSaveFile m_outputFile;
    /// m_currentReply is create by startDownload(),resume().
    /// Detele when abort(), pause(), onNetworkReplyFinished().
    /// there were only one download request in one time
    QPointer<QNetworkReply> m_currentReply;
    /// store the URL for resume(),create by pause(),delete on resume()
    QString m_resumeURL;

private:
    /// Build request by assign range
    QNetworkRequest buildRangedRequest(const QUrl &url,qint64 min,qint64 max = 0) const;
    /// get request and set to m_currentReply, connect slots
    QNetworkReply* download(const QNetworkRequest& request);
    /// check if filepath is writeable,then open file if its true.
    bool checkAndOpenFile(const QString &filepath);
    /// set debug message to UI
    inline void setDebugInfo(const QString &msg);

private slots:
    /// if reply has redirect URL,
    bool handleNetworkReplyRedirected(const QNetworkReply *reply);

    void onNetworkReplyFinished();
    void onNetworkReplyReadyRead();
    void onNetworkReplyError(QNetworkReply::NetworkError code);

#ifndef QT_NO_SSL
    void onSSLErrors(QNetworkReply *reply, const QList<QSslError> &errors);
#endif
};

class DownloaderDialog_V2 : public QDialog
{
    Q_OBJECT

public:
    explicit DownloaderDialog_V2(QWidget *parent = nullptr);
    ~DownloaderDialog_V2();

    void doDownload(const QUrl &url,const QString &filepath);

private:
    Ui::DownloaderDialog_V2 *ui;
    DownloadManager *m_manager;

private:
    void disableDownloadWidgets();
    void enableDownloadWidgets();
    inline void appendConsoleMessage(const QString &message);
    inline QUrl checkInputUrl(const QString &input) const;

private slots:
    void onButtonOpenSaveFileClicked();
    void onButtonDownloadClicked();

    void onNetworkReplyProgress(qint64 bytesReceived, qint64 bytesTotal);
};

#endif // DOWNLOADERDIALOG_V2_H
