#ifndef WAVEFORM_H
#define WAVEFORM_H

#include <QWidget>
#include <QChart>

namespace selfSpace {

class waveform;

class waveform : public QObject
{
    Q_OBJECT
public:
    waveform(QWidget*parent = 0);
    ~waveform();
private:
    QtCharts::QChart    *QChart;
    //QtCharts::chart
};

}


#endif // WAVEFORM_H
