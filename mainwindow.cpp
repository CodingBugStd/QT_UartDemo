#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDebug>

namespace selfSpace {

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    serial = new Serial;
    roughTimer = new QTimer(this);
    accurateTimer = new QTimer(this);
    Label_status = new QLabel("未连接",this);
    SendSbuffer.clear();

    ui->setupUi(this);

    //将状态标签加入状态栏
    ui->statusBar->addWidget(Label_status);
    //限定波特率输入框只能为数字
    ui->lineEdit_BoundRate->setValidator(new QRegExpValidator(QRegExp("[0-9]+$")));

    //连接串口连接
    connect(ui->pushButton_connect,QPushButton::clicked,this,Slot_SerialConnect);
    connect(ui->pushButton,QPushButton::clicked,this,Slot_SerialConnect);
    //连接串口断开
    connect(ui->pushButton_disconnect,QPushButton::clicked,serial,Serial::Disconnect);
    //连接串口连接情况信号
    connect(serial,Serial::Signal_StatusChange,this,Slot_SerialConnected);
    //连接clear按钮     同时清除串口接收缓存
    connect(ui->pushButton_Clear,QPushButton::clicked,this,Slot_SbufferClear);
    //连接串口接收信号   槽函数不接收变量,由槽函数再次通过Serial的接口获得缓存区的引用
    connect(serial,Serial::Signal_RecieveData,this,Slot_RecieveTextRefresh);
    //串口发送
    connect(ui->pushButton_Send,QPushButton::clicked,this,Slot_SerialSend);
    //字符 与 Hex 转化
    connect(ui->checkBox_RecieveHex,QPushButton::clicked,this,Slot_RecieveTextRefresh);
    connect(ui->checkBox_SendHex,QPushButton::clicked,this,Slot_SendTextRefresh);
    //定时器超时
    connect(roughTimer,QTimer::timeout,this,Slot_roughTimer);
    connect(accurateTimer,QTimer::timeout,this,Slot_accurateTimer);

    Refresh_SerialInfo();
    roughTimer->start(500);
    accurateTimer->start(1);

    k = new waveform;
}

void MainWindow::Slot_accurateTimer()
{
    //Refresh_SerialInfo();
    Refresh_ByteCount();
}

void MainWindow::Slot_SendTextRefresh()
{
    QString InputText = ui->textEdit_Send->toPlainText();   //获取输入框字符串
    QString InputShow;      //重新输入输入框的字符串
    if(ui->checkBox_SendHex->checkState())
        //将输入框中的字符串转换成由其对应的utf-8编码所对应的16进制字符串
        InputShow = QString(InputText.toUtf8().toHex());
    else
        //将输入框中的Hex(本质上是由0~9和a~f组成的字符串)转化为对应编码(utf-8)的字符串
        InputShow = QString(QByteArray::fromHex(InputText.toUtf8()));
    ui->textEdit_Send->setPlainText(InputShow);
}

void MainWindow::Slot_roughTimer()
{
    Refresh_SerialInfo();
    //qDebug()<<'a';
}

void MainWindow::Refresh_ByteCount()
{
    ui->label_RXCount->setText(QString::number(serial->RecievByte_Count) );
    ui->label_TXCount->setText(QString::number(serial->SendByte_Count) );
}

void MainWindow::Slot_RecieveTextRefresh()
{
    QByteArray RecieveSbuffer = serial->Read_RecieveSbuffer();  //获取接收缓存
    QString RecieveText;                            //接收框将要显示的字符串
    RecieveText.clear();
    if(ui->checkBox_RecieveHex->checkState())
    {
        QString tempStr = QString(RecieveSbuffer.toHex().toUpper());
        //字节间用' '隔开
        for(int temp=0;temp<tempStr.length();temp++)
        {
            //注意!两个16进制数为1Byte
            RecieveText.append(tempStr.at(temp));
            RecieveText.append(tempStr.at(++temp));
            RecieveText.append(' ');
        }
    }
    else
        RecieveText = QString(RecieveSbuffer);
    ui->textEdit_Recieve->setPlainText(RecieveText);
    //ui->textEdit_Recieve->
}

void MainWindow::Slot_SerialConnect()
{
    QString lock;
    QString portName;
    Serial_Set  set;
    lock = ui->comboBox_port->currentText();
    Refresh_SerialInfo();
    if(lock!=ui->comboBox_port->currentText())
    {
        QMessageBox::critical(this,"错误","串口不存在",QMessageBox::Ok,QMessageBox::Ok);
        return;
    }
    set.BoundRate = ui->lineEdit_BoundRate->displayText().toInt();
    set.dataBit = QSerialPort::Data8;
    set.stopBit = QSerialPort::OneStop;
    foreach (const QSerialPortInfo&info, serial->GetPortInfo()) {
        if(ui->comboBox_port->currentText() == (info.portName() + " " + info.description()))
        {
            portName = info.portName();
        }
    }
    serial->Connect(portName,set);
}

void MainWindow::Slot_SbufferClear()
{
    ui->textEdit_Recieve->clear();
    serial->Serial_SbufferClear();
}

void MainWindow::Refresh_SerialInfo()
{
    QString Lock = ui->comboBox_port->currentText();    //保存本次选项
    ui->comboBox_port->clear();
    //调用了Serial类中的GetPortInfo方法获取接口列表
    foreach (const QSerialPortInfo&info, serial->GetPortInfo()) {
        ui->comboBox_port->addItem(info.portName() + " " + info.description());
    }
    ui->comboBox_port->setCurrentText(Lock);    //在新列表中找到保存的选项
}

void MainWindow::Slot_SerialSend()
{
    QString InputText = ui->textEdit_Send->toPlainText();               //获取输入框字符串
    if(ui->checkBox_SendHex->checkState())
        SendSbuffer = QByteArray::fromHex(InputText.toUtf8());  //字符串 转 hex
    else
        SendSbuffer = InputText.toUtf8();
    if(ui->checkBox_rn->checkState())
        SendSbuffer += "\r\n";
    serial->Serial_Send(SendSbuffer.data());        //发送SendSbuffer
    SendSbuffer.clear();                            //清空发送缓存
}

void MainWindow::Slot_SerialConnected(SerialStatus status)
{
    bool enable;
    QString temp;
    switch (status) {
    case statusConnect:
        enable = false;
        disconnect(ui->pushButton,QPushButton::clicked,ui->pushButton_connect,QPushButton::clicked);
        connect(ui->pushButton,QPushButton::clicked,ui->pushButton_disconnect,QPushButton::clicked);
        ui->pushButton->setIcon(QIcon(":/resouce/disconnect.jpg"));
        temp = "已连接";
        break;
    case statusDisconnect:
        enable = true;
        connect(ui->pushButton,QPushButton::clicked,ui->pushButton_connect,QPushButton::clicked);
        disconnect(ui->pushButton,QPushButton::clicked,ui->pushButton_disconnect,QPushButton::clicked);
        ui->pushButton->setIcon(QIcon(":/resouce/connect.jpg"));
        temp = "未连接";
        break;
    case statusConnectErr:
        QMessageBox::critical(this,"错误","连接失败",QMessageBox::Ok,QMessageBox::Ok);
        enable = true;
        break;
    default:
        break;
    }
    Label_status->setText(temp);
    ui->lineEdit_BoundRate->setEnabled(enable);
    ui->comboBox_databit->setEnabled(enable);
    ui->comboBox_port->setEnabled(enable);
    ui->comboBox_stopbit->setEnabled(enable);
    ui->pushButton_connect->setEnabled(enable);
    if(enable)
        ui->pushButton_disconnect->setEnabled(false);
    else
        ui->pushButton_disconnect->setEnabled(true);
}

MainWindow::~MainWindow()
{
    delete serial;
    delete ui;
    delete Label_status;
}

}

