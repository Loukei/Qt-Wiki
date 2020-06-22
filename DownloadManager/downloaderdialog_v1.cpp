#include "downloaderdialog_v1.h"
#include "ui_downloaderdialog_v1.h"

#include <QNetworkAccessManager>
#include <QFileDialog>
#include <QMessageBox>

static const QLatin1String ErrorTag("[Error]");
static const QLatin1String InfoTag("[Info]");

DownloaderDialog_V1::DownloaderDialog_V1(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DownloaderDialog_V1)
{
    ui->setupUi(this);
    ui->lineEdit_DownloadURL->setText("https://www.qt.io/");
    ui->lineEdit_DownloadPath->setText("D:/index.html");
    connect(ui->pushButton_OpenSaveFile,&QPushButton::clicked,
            this,&DownloaderDialog_V1::browseSaveFilePath);

    QPushButton *closeBtn = ui->buttonBox->button(QDialogButtonBox::Close);
    connect(closeBtn,&QPushButton::clicked,this,&QDialog::accept);

    QPushButton *saveBtn = ui->buttonBox->button(QDialogButtonBox::Save);
    connect(saveBtn,&QPushButton::clicked,this,&DownloaderDialog_V1::onButtonDownloadClicked);

    /* setup network manager */
    m_manager = new QNetworkAccessManager(this);
#ifndef QT_NO_SSL
    connect(m_manager,&QNetworkAccessManager::sslErrors,
            this,&DownloaderDialog_V1::onManagersslErrors);
#endif
}

DownloaderDialog_V1::~DownloaderDialog_V1()
{
    delete ui;
}

void DownloaderDialog_V1::doDownload(const QUrl &url, const QString &filepath)
{
    if(!checkAndOpenFile(filepath)){
        return;
    }
    const QNetworkRequest request(url);
    QNetworkReply* reply = m_manager->get(request);
    /* write file when readyRead */
    connect(reply,&QNetworkReply::readyRead,
            this,&DownloaderDialog_V1::onNetworkReplyReadyRead);
    /* Print message when error occurred */
    connect(reply,QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this,&DownloaderDialog_V1::onNetworkReplyError);
    /* Process and close file when finished */
    connect(reply,&QNetworkReply::finished,
            this,&DownloaderDialog_V1::onNetworkReplyFinished);
    /* Report download progress */
    connect(reply,&QNetworkReply::downloadProgress,
            this,&DownloaderDialog_V1::onNetworkReplyProgress);
    /* close download button */
    setUIEnabled(false);
}

void DownloaderDialog_V1::browseSaveFilePath()
{
    const QString path = ui->lineEdit_DownloadPath->text();
    QString fileName = QFileDialog::getSaveFileName(this,tr("Save File"),path);
    ui->lineEdit_DownloadPath->setText(fileName);
}

void DownloaderDialog_V1::setStatusText(const QString &message)
{
    ui->label_Satus->setText(message);
}

bool DownloaderDialog_V1::checkAndOpenFile(const QString &filepath)
{
    m_outputFile.setFileName(filepath);
    if(!m_outputFile.open(QIODevice::WriteOnly)){
        setStatusText(ErrorTag + tr("Open %1:%2.").arg(filepath,m_outputFile.errorString()));
        return false;
    }
    setStatusText(InfoTag + tr("Open file %1.").arg(filepath));
    return true;
}

void DownloaderDialog_V1::onButtonDownloadClicked()
{
    const QUrl newUrl = checkFormUserInput(ui->lineEdit_DownloadURL->text());
    if(newUrl.isValid()){
        doDownload(newUrl,ui->lineEdit_DownloadPath->text());
    }else {
        setStatusText(ErrorTag + tr("Invalid URL input %1").arg(newUrl.toDisplayString()));
    }
}

QUrl DownloaderDialog_V1::checkFormUserInput(const QString &input) const
{
    return QUrl::fromUserInput(input.trimmed());
}

void DownloaderDialog_V1::setUIEnabled(bool enable)
{
    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(enable);
}

void DownloaderDialog_V1::setFileCommitResult(bool success, const QString &filename, const QString &errorString)
{
    if(success)
        setStatusText(InfoTag + tr("%1 download success.").arg(filename));
    else
        setStatusText(ErrorTag + tr("%1 commit error:%2.").arg(filename,errorString));
}

bool DownloaderDialog_V1::hasNetworkReplyRedirected(const QNetworkReply *reply) const
{
    // method 1. use status code
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    return statusCode == 301 || statusCode == 302 || statusCode == 303
           || statusCode == 305 || statusCode == 307 || statusCode == 308;

    // method 2. use http redirect header
//    const QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
//    return  redirectionTarget.isValid();
}

bool DownloaderDialog_V1::handleNetworkReplyRedirected(const QNetworkReply *reply)
{
    // if reply has redirect URL, ask user to set URL to redirect URL
    const QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if(!redirectionTarget.isValid()){
        return false;
    }

    const QString redirectURL = redirectionTarget.toString();
    const QString question = tr("Resource has redirected to %1.\n"
                                "Change download URL?").arg(redirectURL);
    int ret = QMessageBox::question(this,tr("Resource has moved."),question);
    if(ret == QMessageBox::Yes){
        ui->lineEdit_DownloadURL->setText(redirectURL);
    }
    return true;
}

void DownloaderDialog_V1::onNetworkReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    /* if error Occurred, simply delete reply (handle by error slot) */
    if(reply->error()){
        reply->deleteLater();
        // open download button
        setUIEnabled(true);
        return;
    }
    /* if redirect URL exist,asling user to change download URL.
     * then dump the download result*/
    if(handleNetworkReplyRedirected(reply)){
        m_outputFile.cancelWriting();
        reply->deleteLater();
        setStatusText(ErrorTag + tr("Resource has redirected."));
        setUIEnabled(true);
        return;
    }
    /* download has successful,but we need to check file commit result */
    const QString filename = m_outputFile.fileName();
    const bool commitSuccess = m_outputFile.commit();
    setFileCommitResult(commitSuccess,filename,m_outputFile.errorString());
    reply->deleteLater();
    setUIEnabled(true);
}

void DownloaderDialog_V1::onNetworkReplyReadyRead()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    m_outputFile.write(reply->readAll());
}

void DownloaderDialog_V1::onNetworkReplyError(QNetworkReply::NetworkError code)
{
    /* if error Occurred, delete saved file */
    m_outputFile.cancelWriting();
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    setStatusText(ErrorTag + tr("Network error code %1:%2")
                  .arg(code).arg(reply->errorString()));
}

void DownloaderDialog_V1::onNetworkReplyProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    const QString urlStr = reply->url().toDisplayString();
    setStatusText(InfoTag + tr("Downloading %1\nReceive: %2/%3")
                  .arg(urlStr).arg(bytesReceived).arg(bytesTotal));
}

#ifndef QT_NO_SSL
void DownloaderDialog_V1::onManagersslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    QString errorStrings;
    for(const QSslError &error : errors){
        if(!errorStrings.isEmpty())
            errorStrings.append("\n");
        errorStrings += error.errorString();
    }
    setStatusText(ErrorTag + errorStrings);
    reply->ignoreSslErrors();
}
#endif
