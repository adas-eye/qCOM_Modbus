#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore>
#include "aboutdialog.h"
#include "win_qextserialport.h"
#include <QMessageBox>
#include <QFile>
#include <QInputDialog>
#include <QFileDialog>
#include <QTextStream>
#include <QTcpSocket>
#include <QRegExpValidator>
#include <QTextEdit>


//延时，TIME_OUT是串口读写的延时,3ms
#define TIME_OUT 3

//读取定时器计时间隔,3ms，读取定时器是我们读取串口缓存的延时
#define TIMER_INTERVAL 3

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void startInit();
    void setComboBoxEnabled(bool status);
    QByteArray gemfieldCRC(QByteArray gemfield);


private slots:
    void on_actionOpen_triggered();
    void on_actionClose_triggered();
    void on_senddataButton_clicked();
    void on_actionExit_triggered();
    void on_clearUpBtn_clicked();
    void on_actionAbout_triggered();
    void on_tabWidget_currentChanged(int index);

    void readMyCom();

    void recv_slot();
    void displayError(QAbstractSocket::SocketError socketError);
    void on_actionTcpOpen_triggered();
    void tcpconnected();

    void on_funCode_currentIndexChanged(int index);

    void on_pushButton_clicked();

    void on_actionComment_triggered();

    void on_actionModbus_triggered();

    void on_timeSendCB_stateChanged(int arg1);

private:
    Ui::MainWindow *ui;
    QTcpSocket *tcpSocket;
    int sendflag;              //发送方式标志位，0为串口，1为TCP
    Win_QextSerialPort *myCom;
    QApplication *app;
    AboutDialog abdlg;
    unsigned int timerdly;
    QString write2fileName;    //写读取的串口数据到该文件
    QTimer *timer;
    QTimer *sendTimer;
    int testMode;

    QString receiveStr;

    void readSettings();
    void writeSettings();
};

#endif // MAINWINDOW_H
