#include "downloaddialog.h"
#include "ui_downloaddialog.h"

#include <QScopedPointer> // for create stack qfile
#include <QFileDialog> // open dialog to select save path

#include <QFile>
#include <QUrl>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include <QtDebug>

DownloadDialog::DownloadDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DownloadDialog),
    m_reply(nullptr),
    m_file(nullptr)
{
    ui->setupUi(this);
    ui->textBrowser->setText(tr("Enter URL to download file.\n"));
    m_manager = new QNetworkAccessManager(this);
}

DownloadDialog::~DownloadDialog()
{
    delete ui;
}

void DownloadDialog::requestDownload(const QUrl &url)
{
    m_reply = m_manager->get(QNetworkRequest(url));
    connect(m_reply,&QNetworkReply::readyRead,
            this,&DownloadDialog::onhttp_ReadyRead);
    connect(m_reply,&QNetworkReply::finished,
            this,&DownloadDialog::onhttp_Finished);
    connect(m_reply,QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this,&DownloadDialog::onhttp_Error);

    NetworkProgressDialog *progressDialog = new NetworkProgressDialog(url,this);
    progressDialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(progressDialog,&QProgressDialog::canceled,
            this,&DownloadDialog::onProgressDialog_canceled);
    connect(m_reply,&QNetworkReply::downloadProgress,
            progressDialog,&NetworkProgressDialog::onReplyProgress);
    connect(m_reply,&QNetworkReply::finished,
            progressDialog,&NetworkProgressDialog::hide);

    ui->textBrowser->append(tr("Downloading %1.")
                            .arg(url.toDisplayString()));
}

void DownloadDialog::on_pushButton_Quit_clicked()
{
    this->close();
}

void DownloadDialog::on_pushButton_Download_clicked()
{
    qDebug(Q_FUNC_INFO);
    /* remove whitespace and turn to Url */
    const QUrl url = QUrl::fromUserInput(ui->lineEdit_Url->text().trimmed());
    if(!url.isValid()){
        ui->textBrowser->append(tr("[Error]Input invalid URL:%1.").arg(url.toString()));
        return;
    }
    /* Open File dialog to determine filepath*/
    QString filename = getFileName(url);
    QString filepath = QFileDialog::getSaveFileName(this, tr("Save File"),filename);
    if(filepath.isEmpty()){
        ui->textBrowser->append(tr("Download cancled."));
        return;
    }
    /* Check if filepath writeable */
    m_file = createOpenFile(filepath);
    if(!m_file)
        return;
    /* When file create success */
    ui->pushButton_Download->setEnabled(false); /* lock UI until download finish or cancel */
    /* Request url */
    requestDownload(url);
}

void DownloadDialog::on_lineEdit_Url_textChanged(const QString &arg1)
{
    ui->pushButton_Download->setEnabled(!arg1.isEmpty());
}

void DownloadDialog::onhttp_ReadyRead()
{
    // this slot gets called every time the QNetworkReply has new data.
    // We read all of its new data and write it into the file.
    // That way we use less RAM than when reading it at the finished()
    // signal of the QNetworkReply
    if(m_file)
        m_file->write(m_reply->readAll());
}

void DownloadDialog::onhttp_Finished()
{
    qDebug() << Q_FUNC_INFO;
    /* keep file info , close file */
    QFileInfo fi;
    if(m_file){
        fi.setFile(m_file->fileName());
        m_file->close();
        m_file->deleteLater();
        m_file.clear(); //clear to 0
    }
    /* reply error => remove file */
    if(m_reply->error() != QNetworkReply::NoError){
        m_reply->deleteLater();
        m_reply.clear();
        QFile::remove(fi.absoluteFilePath());
        ui->pushButton_Download->setEnabled(true);
        return;
    }
    /* User abort => remove file and reset m_AbortRequest */
    if(m_AbortRequest){
        m_AbortRequest = false;
        m_reply->deleteLater();
        m_reply.clear();
        QFile::remove(fi.absoluteFilePath());
        ui->pushButton_Download->setEnabled(true);
        return;
    }
    /* download finish => print FileInfo
    *  this app doesn't handle redirect Url
    */
    qDebug() << "Rawheader: " << m_reply->rawHeaderPairs();
    m_reply->deleteLater();
    m_reply.clear();
    ui->textBrowser->append(tr("Download %1 byte to %2")
                            .arg(fi.size()).arg(fi.filePath()));
    ui->pushButton_Download->setEnabled(true);
}

void DownloadDialog::onhttp_Error(QNetworkReply::NetworkError)
{
    /* print error info */
    ui->textBrowser->append(tr("[Error]Network download error: %1.")
                            .arg(m_reply->errorString()));
    qDebug() << Q_FUNC_INFO << "m_reply:" << m_reply;
}

void DownloadDialog::onProgressDialog_canceled()
{
    m_AbortRequest = true;
    m_reply->abort(); /* QNetworkReply::finished will emit */
    ui->textBrowser->append(tr("Download cancel."));
}

QString DownloadDialog::getFileName(const QUrl &url) const
{
    QString filename = url.fileName();
    if(filename.isEmpty())
        return QString("index.html");
    else
        return filename;
}

QFile *DownloadDialog::createOpenFile(const QString &filepath)
{
    QScopedPointer<QFile> newfile(new QFile(filepath,this));
    if(newfile->open(QIODevice::WriteOnly)){
        return newfile.take();
    }else {
        /* newfile will auto delete after function leave */
        ui->textBrowser->append(tr("[Error]Fail to create file %1 : %2.")
                                .arg(filepath).arg(newfile->errorString()));
        return nullptr;
    }
}

NetworkProgressDialog::NetworkProgressDialog(const QUrl &url, QWidget *parent)
    : QProgressDialog (parent)
{
    setWindowTitle(tr("Download Progress"));
    setLabelText(tr("Downloading %1.").arg(url.toDisplayString()));
    setMinimumDuration(0);
}

void NetworkProgressDialog::onReplyProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    setMaximum(qint32(bytesTotal));
    setValue(qint32(bytesReceived));
}
