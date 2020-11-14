
## QDebug

透過`#include <QDebug>`來使用，可以輸出試錯訊息，同時對所有Qt內建類別提供輸出，這是一般`std::cout`所沒有辦法提供的。

### QDebug 輸出自訂資料

有時候我們要列印自訂的資料結構，可以透過`operator<<`來輸出。[參考][ref 1]

``` c++
struct Student{
    QString name;
    int age;
    QString address;
};
QDebug operator<<(QDebug qdb,const Student& s){
    QDebugStateSaver saver(qdb);
    qdb << "Student:("
        << "name:" << s.name << ","
        << "age:" << s.age << ","
        << "address:" << s.address
        << ")";
    return qdb;
}
```

``` c++
QCoreApplication a(argc, argv);
Student john;
// set john
qDebug() << john;
```

輸出
`Student:( name: "John" , age: 20 , address: "New York" )`

### QDebug 輸出額外資訊

qDebug可以透過`qSetMessagePattern()` 修改，使其附帶輸出檔名、第幾行、時間等資訊，使其不只單純是像`std::cout`一樣。[參考][ref 2]

``` c++
qSetMessagePattern("[%{type}] %{appname} (%{file}:%{line}) - %{message}");
qDebug() << "hallo";
```

輸出
`[debug] QtDebug_Demo (..\QtDebug_Demo\main.cpp:31) - hallo`

### Q_FUNC_INFO

輸出所在位置的函數名[Q_FUNC_INFO][ref 3]

``` c++
// in main()
qDebug() << Q_FUNC_INFO;
```

輸出
`int main(int, char**)`

### 將qDebug訊息導向Widget(如texteditor)


## 斷言

[斷言][ref 4]是一種放在程式片段中的一階邏輯，用以檢測程式執行到此時是否有正確的值，如果出現錯誤就會強制中斷並輸出錯誤訊息，Qt提供三種斷言:

- Q_ASSERT(cond)
- Q_ASSERT_X(cond, where, what)
- Q_CHECK_PTR(ptr)

斷言並非Qt的單元測試框架

## QLoggingCategory QMessageLogger 

## Reference

[ref 1]:    https://doc.qt.io/qt-5/debug.html#providing-support-for-the-qdebug-stream-operator
[ref 2]:    https://doc.qt.io/qt-5/qtglobal.html#qSetMessagePattern
[ref 3]:    https://doc.qt.io/qt-5/qtglobal.html#Q_FUNC_INFO
[ref 4]:    https://doc.qt.io/qt-5/debug.html#debugging-macros

[Debugging Techniques](https://doc.qt.io/qt-5/debug.html)
[Testing and Debugging](https://doc.qt.io/qt-5/testing-and-debugging.html)
[redirect qDebug to QTextEdit](https://stackoverflow.com/questions/22485208/redirect-qdebug-to-qtextedit)
[How to redirect qDebug, qWarning, qCritical etc output?](https://stackoverflow.com/questions/4954140/how-to-redirect-qdebug-qwarning-qcritical-etc-output)