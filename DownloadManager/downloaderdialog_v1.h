#ifndef DOWNLOADERDIALOG_V1_H
#define DOWNLOADERDIALOG_V1_H

#include <QDialog>

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSaveFile>
QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
QT_END_NAMESPACE

namespace Ui {
class DownloaderDialog_V1;
}

/*!
 * \brief The DownloaderDialog_V1 class let user input download url and select save file path
 */
class DownloaderDialog_V1 : public QDialog
{
    Q_OBJECT

public:
    explicit DownloaderDialog_V1(QWidget *parent = nullptr);
    ~DownloaderDialog_V1();

    void doDownload(const QUrl &url,const QString &filepath);

public slots:
    void browseSaveFilePath();

private:
    Ui::DownloaderDialog_V1 *ui;
    /// network manager to send request
    QNetworkAccessManager* m_manager;
    /// a file ready to be saved by filepath
    QSaveFile m_outputFile;

private:
    /// set ui->label_status to message
    void setStatusText(const QString &message);
    /// check if filepath is writeable,
    /// then open file if its true.
    bool checkAndOpenFile(const QString &filepath);
    /// check user input URL is vailed
    QUrl checkFormUserInput(const QString &input) const;
    /// detect if reply has redirect header (Not used, but useful function)
    bool hasNetworkReplyRedirected(const QNetworkReply *reply) const;

private slots:
    /// when Save button click,check user input then doDownload()
    void onButtonDownloadClicked();
    /// enable save button after network process finished
    void setUIEnabled(bool enable);
    /// print file commit result
    void setFileCommitResult(bool success,const QString &filename,const QString &errorString);
    /// if reply has redirect URL, ask user to set URL to redirect URL
    bool handleNetworkReplyRedirected(const QNetworkReply *reply);

    void onNetworkReplyFinished();
    void onNetworkReplyReadyRead();
    void onNetworkReplyError(QNetworkReply::NetworkError code);
    void onNetworkReplyProgress(qint64 bytesReceived, qint64 bytesTotal);

#ifndef QT_NO_SSL
    void onManagersslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
#endif
};

#endif // DOWNLOADERDIALOG_V1_H
