#include "downloaderdialog_v2.h"
#include "ui_downloaderdialog_v2.h"

#include <QNetworkAccessManager>
#include <QFileDialog>
#include <QTimer>

const QLatin1String ErrorTag("[Error]");
const QLatin1String InfoTag("[Info]");

DownloaderDialog_V2::DownloaderDialog_V2(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DownloaderDialog_V2)
{
    ui->setupUi(this);
    ui->lineEdit_DownloadURL->setText("https://www.qt.io/");
    ui->lineEdit_DownloadPath->setText("D:/index.html");
    enableDownloadWidgets();
    /* click Browse button to select filepath ready to save */
    connect(ui->pushButton_OpenSaveFile,&QPushButton::clicked,
            this,&DownloaderDialog_V2::onButtonOpenSaveFileClicked);
    /* click download button to start download */
    connect(ui->pushButton_Download,&QPushButton::clicked,
            this,&DownloaderDialog_V2::onButtonDownloadClicked);

    m_manager = new DownloadManager(this);
    /* send download message to console widget */
    connect(m_manager,&DownloadManager::debugInfo,
            this,&DownloaderDialog_V2::appendConsoleMessage);
    /* connect finish to open download button */
    connect(m_manager,&DownloadManager::finished,
            this,&DownloaderDialog_V2::enableDownloadWidgets);
    /* connect downloadProgress to progressbar */
    connect(m_manager,&DownloadManager::downloadProgress,
            this,&DownloaderDialog_V2::onNetworkReplyProgress);

    /* Click cancle button to abort download */
    connect(ui->pushButton_Cancle,&QPushButton::clicked,
            m_manager,&DownloadManager::abort);
    /* Click pause button to pause dowload */
    connect(ui->pushButton_Pause,&QPushButton::clicked,
            m_manager,&DownloadManager::pause);
    /* Click resume button to resume download */
    connect(ui->pushButton_Resume,&QPushButton::clicked,
            m_manager,&DownloadManager::resume);
}

DownloaderDialog_V2::~DownloaderDialog_V2()
{
    delete ui;
}

void DownloaderDialog_V2::doDownload(const QUrl &url, const QString &filepath)
{
    if(!m_manager->startDownload(url,filepath)){
        return;
    }
    // close download button when download has been progress
    disableDownloadWidgets();
}

void DownloaderDialog_V2::disableDownloadWidgets()
{
    ui->groupBox_Status->show();
    ui->pushButton_Download->setDisabled(true);
}

void DownloaderDialog_V2::enableDownloadWidgets()
{
    /* Reset progressBar */
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(100);
    /* Hide Download status until next download start */
    ui->groupBox_Status->hide();
    ui->pushButton_Download->setEnabled(true);
}

void DownloaderDialog_V2::appendConsoleMessage(const QString &message)
{
    ui->textBrowser_Console->append(message);
}

QUrl DownloaderDialog_V2::checkInputUrl(const QString &input) const
{
    return QUrl::fromUserInput(input.trimmed());
}

void DownloaderDialog_V2::onButtonOpenSaveFileClicked()
{
    const QString path = ui->lineEdit_DownloadPath->text();
    const QString fileName = QFileDialog::getSaveFileName(this,tr("Save File"),path);
    if(!fileName.isEmpty() && fileName != path){
        ui->lineEdit_DownloadPath->setText(fileName);
    }
}

void DownloaderDialog_V2::onButtonDownloadClicked()
{
    const QUrl newUrl = checkInputUrl(ui->lineEdit_DownloadURL->text());
    if(newUrl.isValid())
        doDownload(newUrl,ui->lineEdit_DownloadPath->text());
    else
        appendConsoleMessage(ErrorTag + tr("Invalid URL input %1").arg(newUrl.toDisplayString()));
}

void DownloaderDialog_V2::onNetworkReplyProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if(bytesTotal != -1){
        ui->progressBar->setMaximum(bytesTotal);
    }
    ui->progressBar->setValue(bytesReceived);
}

DownloadManager::DownloadManager(QObject *parent)
    :QObject(parent),m_currentReply(nullptr)
{
    m_manager = new QNetworkAccessManager(this);
#ifndef QT_NO_SSL
    connect(m_manager,&QNetworkAccessManager::sslErrors,
            this,&DownloadManager::onSSLErrors);
#endif
}

DownloadManager::~DownloadManager()
{
    // delete download request and make opened file close
    if(hasRunningProcess()){
        m_currentReply->abort();
        m_currentReply->deleteLater();
    }
    if(m_outputFile.isOpen()){
        m_outputFile.cancelWriting();
        m_outputFile.commit();
    }
}

bool DownloadManager::startDownload(const QUrl &url, const QString &filepath)
{
    // block if there was a dowload already
    if(hasRunningProcess() || !m_resumeURL.isNull()){
        setDebugInfo(ErrorTag + tr("Still download %1").arg(m_currentReply->url().toString()));
        return false;
    }
    // check the filepath and open
    if(!checkAndOpenFile(filepath)){
        return false;
    }
    const QNetworkRequest request(url);
    m_currentReply = download(request);
    return true;
}

void DownloadManager::abort()
{
    qDebug() << Q_FUNC_INFO;
    /* call rerply abort will cause error with QNetworkReply::OperationCanceledError */
    if(m_currentReply){
        /* reply disconnect and abort */
        m_currentReply->disconnect();
        m_currentReply->abort();
        m_currentReply->deleteLater();
        qDebug() << "remove m_currentReply";
    }

    /* discared file and close */
    m_outputFile.cancelWriting();
    m_outputFile.commit();

    /* if resumeURL exist, clear */
    if(!m_resumeURL.isNull()){
        m_resumeURL.clear();
        qDebug() << "remove m_resumeURL";
    }

    /* emit finished signal, UI will open for next download */
    setDebugInfo(InfoTag + tr("Download abort."));
    emit finished();
}

void DownloadManager::pause()
{
    qDebug() << Q_FUNC_INFO;
    if(!hasRunningProcess() || !m_resumeURL.isNull()){
        qDebug() << "No RunningProcess || m_resumeURL";
        return;
    }
    // disconnect all signal, so onNetworkReplyError wont trigeerd
    // keep other slots code clean
    m_currentReply->disconnect();
    m_currentReply->abort();

    if(m_currentReply->isReadable()){
        qDebug() << "Write file " << m_outputFile.write(m_currentReply->readAll());
        qDebug() << "file size at the time:" << m_outputFile.size();
    }

    // startDownload() blocked until call resume() or abort()
    // also m_outputFile wont change
    m_resumeURL = m_currentReply->url().toString(QUrl::None);
    m_currentReply->deleteLater();
}

void DownloadManager::resume()
{
    qDebug() << Q_FUNC_INFO;
    if(hasRunningProcess() || m_resumeURL.isNull()){
        qDebug() << "hasRunningProcess || No m_resumeURL";
        return;
    }

    if(!m_outputFile.isWritable()){
        m_outputFile.open(QIODevice::WriteOnly | QIODevice::Append);
    }

    // get last download URL and clear m_currentReply data
    QUrl resumeURL(m_resumeURL);
    m_resumeURL.clear();

    // build the request start form pause()
    const QNetworkRequest resumeRequest = buildRangedRequest(resumeURL,m_outputFile.size(),0);
    m_currentReply = download(resumeRequest);
}

QNetworkReply *DownloadManager::download(const QNetworkRequest &request)
{
    QNetworkReply *reply = m_manager->get(request);
    /* write file when readyRead */
    connect(reply,&QNetworkReply::readyRead,
            this,&DownloadManager::onNetworkReplyReadyRead);
    /* Print message when error occurred */
    connect(reply,QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this,&DownloadManager::onNetworkReplyError);
    /* Process and close file when finished */
    connect(reply,&QNetworkReply::finished,
            this,&DownloadManager::onNetworkReplyFinished);
    /* Report download progress wiill handle by UI */
    connect(reply,&QNetworkReply::downloadProgress,
            this,&DownloadManager::downloadProgress);
    return reply;
}

bool DownloadManager::checkAndOpenFile(const QString &filepath)
{
    m_outputFile.setFileName(filepath);
    if(!m_outputFile.open(QIODevice::WriteOnly)){
        setDebugInfo(ErrorTag + tr("Open %1:%2").arg(filepath,m_outputFile.errorString()));
        return false;
    }
    setDebugInfo(InfoTag + tr("Open %1.").arg(filepath));
    return true;
}

void DownloadManager::setDebugInfo(const QString &msg)
{
    emit debugInfo(msg);
}

bool DownloadManager::hasRunningProcess() const
{
    return bool(m_currentReply) && m_currentReply->isRunning();
}

QNetworkRequest DownloadManager::buildRangedRequest(const QUrl &url, qint64 min, qint64 max) const
{
    QByteArray value;
    if(max == 0){ //Unknow file size
        value = "bytes=" + QByteArray::number(min) + "-";
    }else{
        value = "bytes=" + QByteArray::number(min) + "-" + QByteArray::number(max);
    }

    QNetworkRequest request(url);
    request.setRawHeader("Range",value);
    return request;
}

bool DownloadManager::handleNetworkReplyRedirected(const QNetworkReply *reply)
{
    // if reply has redirect URL, send redirect URL message
    const QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if(!redirectionTarget.isValid()){
        return false;
    }
    const QString redirectURL = redirectionTarget.toString();
    setDebugInfo(ErrorTag + tr("Resource has redirected to %1.").arg(redirectURL));
    return true;
}

void DownloadManager::onNetworkReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    /* if error Occurred, simply delete reply (handle by error slot) */
    if(reply->error()){
        reply->deleteLater();
        emit finished();
        return;
    }

    /* if redirect URL exist.
     * then dump the download result*/
    if(handleNetworkReplyRedirected(reply)){
        m_outputFile.cancelWriting();
        m_outputFile.commit();
        reply->deleteLater();
        emit finished();
        return;
    }

    /* download has successful,but we need to check file commit result */
    const QString filename = m_outputFile.fileName();
    const int filesize = m_outputFile.size();
    m_outputFile.commit();
    reply->deleteLater();
    setDebugInfo(InfoTag + tr("Download %1: %2 bytes.").arg(filename).arg(filesize));
    emit finished();
}

void DownloadManager::onNetworkReplyReadyRead()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    m_outputFile.write(reply->readAll());
}

void DownloadManager::onNetworkReplyError(QNetworkReply::NetworkError code)
{
    /* if error Occurred, delete saved file */
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    m_outputFile.cancelWriting();
    m_outputFile.commit();
    setDebugInfo(ErrorTag + tr("Network error code %1:%2").arg(code).arg(reply->errorString()));
}

void DownloadManager::onSSLErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    QString errorStrings;
    for(const QSslError &error : errors){
        if(!errorStrings.isEmpty())
            errorStrings.append("\n");
        errorStrings += error.errorString();
    }
    setDebugInfo(ErrorTag + errorStrings);
    reply->ignoreSslErrors();
}
