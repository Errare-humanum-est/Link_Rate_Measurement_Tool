#ifndef COUNTER_H
#define COUNTER_H

#include <QElapsedTimer>
#include "time_struct.h"

// Operation mode TickCounter is used to make sure the High PerformanceCounter
// available under MS Windows only is used if possible.

class Counter
{
public:
    Counter();

    qint64 avg_val_cnt(quint64 n_values);

    void counter_start();

    Time_struct counter_stop();

    int getAverage();

private:
    QElapsedTimer* timer;
    quint8 clock_type;
    Time_struct data;

    quint64 *val;
    quint64 n_val;



};

#endif // COUNTER_H
