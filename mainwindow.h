#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include "waveform.h"
#include "serial.h"

namespace Ui {
class MainWindow;
}

namespace selfSpace {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
public slots:
    void Slot_SbufferClear();
    void Slot_SerialSend();
    //void Slot_SerialRecieve(QByteArray&dat);
    void Slot_SerialConnected(SerialStatus status);     //连接操作后的槽函数
    void Slot_SerialConnect();                          //连接的槽函数
    void Slot_SendTextRefresh();                   //发送文本框刷新
    void Slot_RecieveTextRefresh();                //接收文本框刷新
    void Slot_roughTimer();
    void Slot_accurateTimer();
private:
    void Refresh_SerialInfo();          //刷新串口列表
    void Refresh_ByteCount();
    QByteArray  SendSbuffer;
    QLabel  *Label_status;
    Ui::MainWindow *ui;
    Serial  *serial;
    QTimer  *roughTimer;
    QTimer  *accurateTimer;

    waveform    *k;
};

}


#endif // MAINWINDOW_H
