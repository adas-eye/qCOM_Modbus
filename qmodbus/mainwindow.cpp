#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtGui>
#include <QDebug>
#include <QString>
#include <QSettings>
#include <QComboBox>
#include <QtNetwork>
//#include <Q3PopupMenu>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    startInit();
    readSettings();
}

MainWindow::~MainWindow()
{
    writeSettings();
    delete ui;
}

//��ʼ��
void MainWindow::startInit()
{
    ui->actionOpen->setEnabled(true);
    ui->actionClose->setEnabled(false);
    ui->senddataButton->setEnabled(false);

    ui->tabWidget->setTabEnabled(1, false);
    ui->modeStackedWidget->setCurrentIndex(0);
    ui->tabWidget->setCurrentIndex(0);
    ui->actionComment->setChecked(true);
    ui->pushButton->setEnabled(false);
    ui->timeSendCB->setEnabled(false);

    testMode = 0;

    receiveStr = "";

    //    ui->tabWidget->setCurrentIndex(0);
    //    ui->firstEdit->setStatusTip(tr("�����ʽ���ӣ�01 01 00 00 00 01�����豸��ַ�������롢��ʼ��ַ����ѯ������CRCУ����Ϊ�Զ�����"));
    sendflag = ui->tabWidget->currentIndex();
    if(ui->tabWidget->currentIndex() == 1)
    {
        ui->actionOpen->setEnabled(false);
        ui->actionTcpOpen->setEnabled(true);
    }
    else if(ui->tabWidget->currentIndex() == 0)
    {
        ui->actionOpen->setEnabled(true);
        ui->actionTcpOpen->setEnabled(false);
    }
    //��ʼ����ȡ��ʱ����ʱ���
    timerdly = TIMER_INTERVAL;

    //���ö�ȡ��ʱ��
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(readMyCom()));

    //���ö�ȡ��ʱ��
    sendTimer = new QTimer(this);
    connect(sendTimer, SIGNAL(timeout()), this, SLOT(on_senddataButton_clicked()));

    tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(recv_slot()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(tcpconnected()));
}

void MainWindow::setComboBoxEnabled(bool status)
{
    ui->portNameComboBox->setEnabled(status);
    ui->baudRateComboBox->setEnabled(status);
    ui->dataBitsComboBox->setEnabled(status);
    ui->parityComboBox->setEnabled(status);
    ui->stopBitsComboBox->setEnabled(status);
}

//�򿪴���
void MainWindow::on_actionOpen_triggered()
{
    QString portName = ui->portNameComboBox->currentText();   //��ȡ������
    myCom = new Win_QextSerialPort(portName, QextSerialBase::Polling);
    //����QextSerialBase::QueryModeӦ��ʹ��QextSerialBase::Polling

    //    if(myCom->open(QIODevice::ReadWrite)){
    //        QMessageBox::information(this, tr("�򿪳ɹ�"), tr("�ѳɹ��򿪴��� ") + portName, QMessageBox::Ok);
    //    }else{
    //        QMessageBox::critical(this, tr("��ʧ��"), tr("δ�ܴ򿪴��� ") + portName + tr("\n�ô����豸�����ڻ��ѱ�ռ��"),
    //                             QMessageBox::Ok);
    //        return;
    //    }
    if(!myCom->open(QIODevice::ReadWrite)){
        QMessageBox::critical(this, tr("��ʧ��"), tr("δ�ܴ򿪴��� ") + portName + tr("\n�ô����豸�����ڻ��ѱ�ռ��"), QMessageBox::Ok);
        return;
    }

    //���ò�����
    myCom->setBaudRate((BaudRateType)ui->baudRateComboBox->currentIndex());

    //��������λ
    myCom->setDataBits((DataBitsType)ui->dataBitsComboBox->currentIndex());

    //����У��
    myCom->setParity((ParityType)ui->parityComboBox->currentIndex());

    //����ֹͣλ
    myCom->setStopBits((StopBitsType)ui->stopBitsComboBox->currentIndex());

    //������ȡ��ʱ��
    timer->start(timerdly);

    //��������������
    myCom->setFlowControl(FLOW_OFF);

    //������ʱ
    myCom->setTimeout(TIME_OUT);

    setComboBoxEnabled(false);

    ui->actionOpen->setEnabled(false);
    ui->actionClose->setEnabled(true);
    ui->senddataButton->setEnabled(true);
    ui->actionTcpOpen->setEnabled(false);
    ui->tabWidget->setEnabled(false);
    ui->timeSendCB->setEnabled(true);
}

//��ȡ��������
void MainWindow::readMyCom()
{
    QByteArray temp = myCom->readAll();
    if(temp.isEmpty())
    {
        return;
    }

    QString restr1, restr2, restr3;

    switch(testMode)
    {
    case 0:
    {
        if(ui->hexReceiveCB->isChecked())
        {
            QByteArray crctmp;
            qDebug() << temp.toHex();
            if(crctmp.toInt(0, 16) != 0)
            {
                QMessageBox::critical(this, tr("����"), tr("CRCУ����� "), QMessageBox::Ok);
            }
            //            ui->textBrowser->setTextColor(Qt::lightGray);
            //            ui->textBrowser->append(tr("����: "));
            //            ui->textBrowser->setTextColor(Qt::black);
            restr1.append(temp.toHex());
            while(restr1.length() > 0)
            {
                restr2 = restr1.left(2);
                restr3 = restr3 + restr2 + " ";
                if(restr1.length() == 2)
                    restr1.clear();
                else
                    restr1 = restr1.right(restr1.length() - 2);
            }
            restr3 = restr3.left(restr3.length() - 1);
            ui->receiveDataTB->append(restr3.toUpper());
        }
        else
        {
            restr1 = QString(temp);
            receiveStr = receiveStr + restr1;
            ui->receiveDataTB->setText(receiveStr);
        }
        break;
    }
    case 1:
    {
        QByteArray crctmp;
        qDebug() << temp.toHex();
        ui->receiveBrowser->clear();
        crctmp = gemfieldCRC(temp);
        if(crctmp.toInt(0, 16) != 0)
        {
            QMessageBox::critical(this, tr("����"), tr("CRCУ����� "), QMessageBox::Ok);
        }
        if(write2fileName.isEmpty())
        {
            //            ui->textBrowser->setTextColor(Qt::lightGray);
            //            ui->textBrowser->append(tr("����: "));
            //            ui->textBrowser->setTextColor(Qt::black);
            restr1.append(temp.toHex());
            while(restr1.length() > 0)
            {
                restr2 = restr1.left(2);
                restr3 = restr3 + restr2 + " ";
                if(restr1.length() == 2)
                    restr1.clear();
                else
                    restr1 = restr1.right(restr1.length() - 2);
            }
            restr3 = restr3.left(restr3.length() - 1);
            ui->receiveBrowser->append(restr3.toUpper());
        }
        break;
    }
    }
    //    temp.clear();
    //    crctmp.clear();
}

//��ȡTCP����
void MainWindow::recv_slot()
{

    QByteArray byte;
    byte = tcpSocket->readAll();
    QString restr1, restr2, restr3;
    if(!byte.isEmpty())
    {
        ui->receiveBrowser->clear();
        if(write2fileName.isEmpty())
        {
            restr1.append(byte.toHex());
            while(restr1.length() > 0)
            {
                restr2 = restr1.left(2);
                restr3 = restr3 + restr2 + " ";
                if(restr1.length() == 2)
                    restr1.clear();
                else
                    restr1 = restr1.right(restr1.length() - 2);
            }
            restr3 = restr3.left(restr3.length() - 1);
            ui->receiveBrowser->append(restr3.toUpper());
        }
    }
}

//��������
void MainWindow::on_senddataButton_clicked()
{
    int i = 0;
    int j;
    switch(testMode)
    {
    case 0:
    {
        if(ui->sendDataLE->text().isEmpty())
        {
            return;
        }
        QByteArray cmdtmp;
        QString cmdstr1, cmdstr2;
        cmdstr1 = ui->sendDataLE->text();
        cmdstr1 = cmdstr1.trimmed();
        if(ui->hexSendCB->isChecked())
        {
            while(cmdstr1.length() > 0)
            {
                cmdstr2 = cmdstr1.left(2);
                qDebug() << cmdstr2;
                qDebug() << "**" + cmdstr2.toAscii();
                cmdtmp[i++] = cmdstr2.toUInt(0,16);
                if(cmdstr1.length() == 2)
                    cmdstr1.clear();
                else
                    cmdstr1 = cmdstr1.right(cmdstr1.length() - 3);
            }
            qDebug() << "cmdtmp = "<< cmdtmp.toHex();
            myCom->write(cmdtmp);
            cmdtmp.clear();
        }
        else
        {
            myCom->write(cmdstr1.toLatin1().data());
        }
        break;
    }
    case 1:
    {
        //�����������Ϊ�գ�������ʾ������
        if(ui->firstEdit->toPlainText().isEmpty()){
            QMessageBox::information(this, tr("��ʾ��Ϣ"), tr("û����Ҫ���͵�����"), QMessageBox::Ok);
            return;
        }
        //��������
        QByteArray tmp, tmp2, tmp3;
        QString str1, str2, str3, str4;
        QString sdstr1, sdstr2, sdstr3, tmpstr;

        str1 = ui->firstEdit->toPlainText();
        qDebug() << str1;
        str1 = str1.trimmed();                            //ȥ����������ո�
        if(sendflag == 0)
        {
            while(str1.length() > 0)
            {
                str2 = str1.left(2);
                qDebug() << str2;
                qDebug() << "**" + str2.toAscii();
                tmp3[i++] = str2.toUInt(0,16);
                if(str1.length() == 2)
                    str1.clear();
                else
                    str1 = str1.right(str1.length() - 3);
            }
            qDebug() << "tmp3 = "<< tmp3.toHex();
            //        qDebug() << "str3 = "<< str3;
            tmp = gemfieldCRC(tmp3);
            qDebug() << "tmp = "<< tmp.toHex();
            //        tmp = gemfieldCRC(tmp3);
            //        tmp2 = tmp3 + tmp;
            tmp2 = tmp3 + tmp;
            qDebug() << "tmp2 = "<< tmp2.toHex();
            myCom->write(tmp2);
            tmp.clear();
        }
        else if(sendflag == 1)
        {
            j = (str1.length() + 1)/3;
            if(j < 16)
            {
                tmpstr = "0" + QString::number(j, 16);
            }
            else tmpstr = QString::number(j, 16);
            str1 = "00 00 00 00 00 " + tmpstr + QString('\t') + str1;  //QString('\t')Ϊ���һ���ո�,����QString('\40')
            while(str1.length() > 0)
            {
                str2 = str1.left(2);
                tmp3[i++] = str2.toUInt(0,16);
                if(str1.length() == 2)
                    str1.clear();
                else
                    str1 = str1.right(str1.length() - 3);
            }
            qDebug() << "tmp3 = "<< tmp3.toHex();
            //        qDebug() << "str3 = "<< str3;
            tmp = gemfieldCRC(tmp3);
            qDebug() << "tmp = "<< tmp.toHex();
            //        tmp = gemfieldCRC(tmp3);
            //        tmp2 = tmp3 + tmp;
            tmp2 = tmp3 + tmp;
            qDebug() << "tmp2 = "<< tmp2.toHex();
            tcpSocket->write(tmp2);
            tmp.clear();
        }
        ui->sendBrowser->clear();

        sdstr1.append(tmp2.toHex());
        while(sdstr1.length() > 0)
        {
            sdstr2 = sdstr1.left(2);
            sdstr3 = sdstr3 + sdstr2 + " ";
            if(sdstr1.length() == 2)
                sdstr1.clear();
            else
                sdstr1 = sdstr1.right(sdstr1.length() - 2);
        }
        sdstr3 = sdstr3.left(sdstr3.length() - 1);
        ui->sendBrowser->append(sdstr3.toUpper());
        tmp2.clear();
        break;
    }
    }
}

//�ر�����
void MainWindow::on_actionClose_triggered()
{
    if(sendflag == 0)
    {
        if (sendTimer->isActive())
        {
            ui->timeSendCB->setChecked(false);
        }
        timer->stop();
        myCom->close();
        delete myCom;
    }
    else if(sendflag == 1)
    {
        tcpSocket->disconnectFromHost();
        ui->actionComment->setEnabled(true);
        ui->actionModbus->setEnabled(true);
    }

    ui->timeSendCB->setEnabled(false);

    setComboBoxEnabled(true);

    if(ui->tabWidget->currentIndex() == 1)
    {
        ui->actionOpen->setEnabled(false);
        ui->actionTcpOpen->setEnabled(true);
    }
    else if(ui->tabWidget->currentIndex() == 0)
    {
        ui->actionOpen->setEnabled(true);
        ui->actionTcpOpen->setEnabled(false);
    }
    ui->actionClose->setEnabled(false);
    ui->senddataButton->setEnabled(false);
    ui->tabWidget->setEnabled(true);
}

//�˳�����
void MainWindow::on_actionExit_triggered()
{
    writeSettings();
    app->exit(0);
}

void MainWindow::on_clearUpBtn_clicked()
{
    ui->firstEdit->clear();
    ui->sendBrowser->clear();
    ui->receiveBrowser->clear();
    ui->receiveDataTB->clear();
}

//CRCУ��
QByteArray MainWindow::gemfieldCRC(QByteArray gemfield)
{
    QByteArray temp;

    //����Ϊcrc����2���ֽڵ��ڴ�ռ䣬������λȫ����Ϊ1��
    unsigned short crc = 0xffff;
    unsigned short a,j,k;
    //����ÿһ���ֽڣ�ִ��forѭ�������䡣
    for(a=0;a<gemfield.size();a++)
    {
        //crc�͵�a���ֽ����ֵ�����ֵ����crc��ע��gemfield[a]��ת��Ϊint��ʱ���з��ŵģ�
        //��ˣ���������ֵ����7Fʱ����Ҫ����Ӧ��ת�������㶮�ġ�
        crc =crc ^ ( (int)gemfield[a]>=0 ? gemfield[a] : (gemfield[a]+256) );
        for(j=0;j<8;j++)
        {
            k=crc & 01;//��crc�����λ�ǲ���0���������0������0xA001���
            crc = crc >> 1;
            if (k==0) continue;
            crc =crc ^ 0xA001;
        }
    }
    temp[1] = crc/256;
    temp[0] = crc%256;
    return temp;    //������ת��ΪQByteArray���ء�
}

//���ڶԻ���
void MainWindow::on_actionAbout_triggered()
{
    abdlg.show();
}

//������������
void MainWindow::readSettings()
{
    QSettings settings("Gateslu", "Qmodbus");

    int coms = settings.value("COMS").toInt();
    ui->portNameComboBox->setCurrentIndex(coms);

    int baud = settings.value("BAUDRATE").toInt();
    ui->baudRateComboBox->setCurrentIndex(baud);

    QString ipaddress = settings.value("IP_Adress").toString();
    ui->ipAddressEdit->setText(ipaddress);

    int index = settings.value("tabcurrentindex").toInt();
    ui->tabWidget->setCurrentIndex(index);

    int flag = settings.value("sendflag").toInt();
    sendflag = flag;

    QString editstring = settings.value("firsteditstring").toString();
    ui->firstEdit->setStatusTip(editstring);

    //    int databits = settings.value("dataBits").toInt();
    //    ui->dataBitsComboBox->setCurrentIndex(databits);

    int parity = settings.value("parity").toInt();
    ui->parityComboBox->setCurrentIndex(parity);

    //    int stopbits = settings.value("stopbits").toInt();
    //    ui->stopBitsComboBox->setCurrentIndex(stopbits);

}

//������������
void MainWindow::writeSettings()
{
    QSettings settings("Gateslu", "Qmodbus");

    settings.setValue("COMS", ui->portNameComboBox->currentIndex());
    settings.setValue("BAUDRATE", ui->baudRateComboBox->currentIndex());
    settings.setValue("dataBits", ui->dataBitsComboBox->currentIndex());
    settings.setValue("parity", ui->parityComboBox->currentIndex());
    settings.setValue("stopbits", ui->stopBitsComboBox->currentIndex());
    settings.setValue("IP_Adress", ui->ipAddressEdit->text());
    settings.setValue("tabcurrentindex", ui->tabWidget->currentIndex());
    settings.setValue("sendflag", sendflag);
    settings.setValue("firsteditstring", ui->firstEdit->statusTip());
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    if(index == 1)
    {
        ui->firstEdit->setStatusTip(tr("�����ʽ���ӣ�01 01 00 00 00 01�����豸��ַ�������롢��ʼ��ַ����ѯ����"));
        sendflag = ui->tabWidget->currentIndex();
        ui->actionOpen->setEnabled(false);
        ui->actionTcpOpen->setEnabled(true);
    }
    else if(index == 0)
    {
        ui->firstEdit->setStatusTip(tr("�����ʽ���ӣ�01 01 00 00 00 01�����豸��ַ�������롢��ʼ��ַ����ѯ������CRCУ����Ϊ�Զ�����"));
        sendflag = ui->tabWidget->currentIndex();
        ui->actionOpen->setEnabled(true);
        ui->actionTcpOpen->setEnabled(false);
    }
}

/*TCP/IPѡ��*/
void MainWindow::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("QModbus"),
                                 tr("�޷��ҵ��豸������IP��ַ�Ƿ���ȷ��"));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("QModbus"),
                                 tr("���ӱ��ܾ�����ȷ����ǰ�豸�Ƿ����������У�������IP��ַ�Ƿ���ȷ��"));
        break;
    default:
        QMessageBox::information(this, tr("QModbus"),
                                 tr("The following error occurred: %1.")
                                 .arg(tcpSocket->errorString()));
    }
    ui->actionComment->setEnabled(true);
    ui->actionModbus->setEnabled(true);
    ui->actionClose->setEnabled(false);
    ui->actionTcpOpen->setEnabled(true);
    ui->senddataButton->setEnabled(false);
    ui->tabWidget->setEnabled(true);
}

//����TCP
void MainWindow::on_actionTcpOpen_triggered()
{
    tcpSocket->connectToHost(ui->ipAddressEdit->text(), ui->portEdit->text().toUShort());
    ui->actionComment->setEnabled(false);
    ui->actionModbus->setEnabled(false);
    ui->actionClose->setEnabled(true);
    ui->actionTcpOpen->setEnabled(false);
    ui->tabWidget->setEnabled(false);
}

void MainWindow::tcpconnected()
{
    ui->senddataButton->setEnabled(true);
}

//combox����
void MainWindow::on_funCode_currentIndexChanged(int index)
{
    if (index == 0 || index == 1)
    {
        ui->setPropertySW->setCurrentIndex(0);
        ui->fp3Text->setText(tr("��ʼ��ַ:"));
        ui->fp4Text->setText(tr("��ȡ����:"));
    }
    else if (index == 4 || index == 5)
    {
        ui->setPropertySW->setCurrentIndex(0);
        ui->fp3Text->setText(tr("��ʼ��ַ:"));
        ui->fp4Text->setText(tr("�Ĵ�������:"));
    }
    else if (index == 2)
    {
        ui->setPropertySW->setCurrentIndex(0);
        ui->fp3Text->setText(tr("�����ַ:"));
        ui->fp4Text->setText(tr("���״̬:"));
    }
    else if (index == 6)
    {
        ui->setPropertySW->setCurrentIndex(0);
        ui->fp3Text->setText(tr("�Ĵ�����ַ:"));
        ui->fp4Text->setText(tr("�Ĵ���ֵ:"));
    }
    else if (index == 3)
    {
        ui->setPropertySW->setCurrentIndex(1);
        //        ui->fp3Text->setText(tr("��ʼ��ַ:"));
        //        ui->fp4Text->setText(tr("�������:"));
    }
    else if (index == 7)
    {
        ui->setPropertySW->setCurrentIndex(2);
        //        ui->fp3Text->setText(tr("��ʼ��ַ:"));
        //        ui->fp4Text->setText(tr("�Ĵ�������:"));
    }
    else if (index == 8)
    {
        ui->setPropertySW->setCurrentIndex(3);
    }
}

//���ɱ���
void MainWindow::on_pushButton_clicked()
{
    int index = ui->funCode->currentIndex();
    QString DADDR;                             //�豸��ַ
    QString FCODE;                             //������
    QString SADDR;                             //��ʼ��ַ
    QString C_O_S;                             //������״̬

    DADDR = ui->deviceAddr->text();
    FCODE = ui->funCode->currentText().left(2);

    if ( index == 3)
    {
        QString frame;
        int x,y;
        SADDR = ui->MCAddrHi->text() + " " + ui->MCAddrLo->text();
        C_O_S = ui->MCCountHi->text() + " " + ui->MCCountLo->text();
        x = ui->MCCountLo->text().toInt(0,16);
        if (x%8 == 0)
        {
            y = x/8;
        }else
        {
            y = x/8 + 1;
        }
        //        qDebug() << y;
        for(int cnt = 1; cnt <= y; cnt++)
        {
            frame += " 00";
        }

        ui->firstEdit->setText(DADDR + " " + FCODE + " " + SADDR + " " + C_O_S + frame);
    }
    else if (index == 7)
    {
        QString frame;
        SADDR = ui->MRAddrHi->text() + " " + ui->MRAddrLo->text();
        C_O_S = ui->MRCountHi->text() + " " + ui->MRCountLo->text();
        //        qDebug() << ui->MRCountLo->text().toInt(0,16);
        for(int cnt = 1; cnt <= ui->MRCountLo->text().toInt(0,16); cnt++)
        {
            frame += " 00 00";
        }
        ui->firstEdit->setText(DADDR + " " + FCODE + " " + SADDR + " " + C_O_S + frame);
    }
    else if (index == 8)
    {
        SADDR = ui->fp3Hi->text() + " " + ui->fp3Lo->text();
        C_O_S = ui->fp4Hi->text() + " " + ui->fp4Lo->text();
        ui->firstEdit->setText(DADDR + " " + FCODE + " " + SADDR + " " + C_O_S);
    }
    else
    {
        SADDR = ui->fp3Hi->text() + " " + ui->fp3Lo->text();
        C_O_S = ui->fp4Hi->text() + " " + ui->fp4Lo->text();
        ui->firstEdit->setText(DADDR + " " + FCODE + " " + SADDR + " " + C_O_S);
    }
}

void MainWindow::on_actionComment_triggered()
{
    ui->modeStackedWidget->setCurrentIndex(0);
    ui->tabWidget->setCurrentIndex(0);
    ui->tabWidget->setTabEnabled(1, false);
    ui->actionComment->setChecked(true);
    ui->actionModbus->setChecked(false);
    ui->pushButton->setEnabled(false);
    testMode = 0;
}

void MainWindow::on_actionModbus_triggered()
{
    ui->modeStackedWidget->setCurrentIndex(1);
    ui->tabWidget->setTabEnabled(1, true);
    ui->actionModbus->setChecked(true);
    ui->actionComment->setChecked(false);
    ui->pushButton->setEnabled(true);
    if (sendTimer->isActive())
    {
        ui->timeSendCB->setChecked(false);
    }
    testMode = 1;
}

void MainWindow::on_timeSendCB_stateChanged(int arg1)
{
    switch(arg1)
    {
    case 0:
        sendTimer->stop();
        ui->timeLE->setDisabled(false);
        break;
    case 2:
        sendTimer->start(ui->timeLE->text().toInt());
        ui->timeLE->setDisabled(true);
        break;
    default:
        break;
    }
}
