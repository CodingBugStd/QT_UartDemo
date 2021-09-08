#include "serial.h"

#include <QDebug>

namespace selfSpace
{

QList<QSerialPortInfo>  Serial::List;

Serial::Serial()
{
    GetPortInfo();
    RecieveSbuff.clear();
    status = statusDisconnect;
    RecievByte_Count = 0;
    SendByte_Count = 0;
}

QList<QSerialPortInfo>& Serial::GetPortInfo()
{
    List = QSerialPortInfo::availablePorts();
    return List;
}

bool Serial::Connect(QString portName, Serial_Set &set)
{
    if(status == statusConnect)
        return false;
    GetPortInfo();
    foreach (const QSerialPortInfo&info, List)
    {
        if(info.portName() == portName)
        {
            port = new QSerialPort;
            port->setPort(info);
            port->setBaudRate(set.BoundRate);
            port->setStopBits(set.stopBit);
            port->setDataBits(set.dataBit);
            if(port->open(QIODevice::ReadWrite))
            {
                status = statusConnect;
                emit Signal_StatusChange(statusConnect);
                connect(port,QSerialPort::readyRead,this,Slot_SerialRecieve);
                return true;
            }else
            {
                delete port;
                port = NULL;
                status = statusDisconnect;
                emit Signal_StatusChange(statusConnectErr);
                return false;
            }
        }
    }
    emit Signal_StatusChange(statusConnectErr);
    return false;
}

void Serial::Slot_SerialRecieve()
{
    RecieveSbuff += port->readAll();    //在接收缓存末尾追加
    RecievByte_Count = RecieveSbuff.length();
    emit Signal_RecieveData(RecieveSbuff);
}

void Serial::Serial_Send(QString &str, bool Hex, bool Utf8)
{
    if(status == statusConnect)
    {
        QByteArray dat;
        if(Utf8)
            dat = str.toUtf8();
        else
            dat = str.toLocal8Bit();
        if(Hex)
            dat = dat.toHex();
        Serial_Send(dat);
        SendByte_Count += dat.length();
    }
}

void Serial::Serial_Send(QByteArray &arr)
{
    if(status == statusConnect)
    {
        port->write(arr);
        SendByte_Count += arr.length();
    }
}

void Serial::Serial_Send(char *dat)
{
    if(status == statusConnect)
    {
        while(*dat!='\0')
        {
            port->putChar(*dat);
            dat++;
            SendByte_Count++;
        }
    }
}

QByteArray& Serial::Read_RecieveSbuffer()
{
    return RecieveSbuff;
}

void Serial::Disconnect()
{
    if(status == statusConnect)
    {
        disconnect(port,QSerialPort::readyRead,this,Slot_SerialRecieve);
        port->close();
        delete port;
        port = NULL;
        status = statusDisconnect;
        emit Signal_StatusChange(status);
    }
}

void Serial::Serial_SbufferClear()
{
    RecieveSbuff.clear();
    SendByte_Count = 0;
    RecievByte_Count = 0;
}

Serial::~Serial()
{
    Disconnect();
}

}

