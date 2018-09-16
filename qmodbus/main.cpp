#include <QtGui/QApplication>
#include "mainwindow.h"
#include <QTextCodec>
#include <QDesktopWidget>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTextCodec *codec = QTextCodec::codecForName("GB2312");
    QTextCodec::setCodecForTr(codec);
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForLocale(codec);

//    //窗体居中显示
//    QDesktopWidget *desktop=QApplication::desktop();
//    int width=desktop->width();
//    int height=desktop->height();

    MainWindow w;
//    w.setWindowFlags(Qt::windowMinimizeButtonHint);
//    w.move((width-500-w.width())/2,(height-400-w.height())/2);
//    w.resize(1000, 650);
//    w.resize(w.width(),w.height());
    w.show();

    //应用样式
    QApplication::setStyle(QStyleFactory::create("Cleanlooks"));       //Plastique/Cleanlooks/windows/CDE

    return a.exec();
}
