# Http download Dialog example

###### tags: `QNetworkReply` `QProgressDialog` `QNetworkAccessManager` `Http`

## Introduction

<Img src="https://raw.githubusercontent.com/Loukei/Qt-Wiki/master/HttpDownloader_Example/Demo.png" />

使用Qt網路模組下載Url檔案的範例，配合`QProgressDialog`類別可以顯示網路封包的上傳/下載進度，也可以中斷下載工作。

## Note

- 從5.4版本開始，`QProgressDialog`類別將會在宣告並初始化時立即顯示，因此請在下載事件開始才宣告`QProgressDialog`物件，並在下載結束時銷毀。(參考2)

- 雖然QObject本身支援資源回收機制，但是對於function內部創造後就不使用的物件最好還是盡快回收，`QScopedPointer`可以在function結束時自動回收對象。

``` c++
QFile *DownloadDialog::createOpenFile(const QString &filepath)
{
    QScopedPointer<QFile> newfile(new QFile(filepath,this));
    if(newfile->open(QIODevice::WriteOnly)){
        return newfile.take();
    }else {
        /* newfile will auto delete after function leave */
        return nullptr;
    }
}
```

- `QNetworkReply`想要寫入檔案的時候，利用`QNetworkReply::readyRead`的signal來寫入檔案，比起在`QNetworkReply::finished`下載完成才寫入更節省RAM

- 對於使用者輸入，永遠不要照單全收

``` c++
const QUrl url = QUrl::fromUserInput(ui->lineEdit_Url->text().trimmed());
if(!url.isValid()){
    ui->textBrowser->append(tr("[Error]Input invalid URL:%1.").arg(url.toString()));
    return;
}
```

- 範例中使用`QFile`與`QNetworkReply`指針在全域宣告，在我看來是相當不得已的做法，因為將QObject宣告在class member一般會認為是跟隨class本身產生與消失的資源，但是在這裡`QFile`所指向的內容會隨著下載任務的產生才有實質內容，並跟隨下載完成而消失。
如果可以最好是不宣告在class member。

- 接下來會測試將`QFile` parent設置為`QNetworkReply` 是否可行。

- 此範例並沒有處理需要認證或轉網址的問題

## Environment

- c++ 11
- Qt 5.13
- win7 x86

## Reference

[QtDoc HTTP Example](https://doc.qt.io/qt-5/qtnetwork-http-example.html)
[QprogressDialog appears at creation with Qt5.5rc](https://bugreports.qt.io/browse/QTBUG-47042)
