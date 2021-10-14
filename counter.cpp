#include "counter.h"

Counter::Counter()
{
    clock_type=QElapsedTimer::clockType();

    timer=new QElapsedTimer();

    n_val=100;

    val = new quint64[n_val];


}

qint64 Counter::avg_val_cnt(quint64 n_values)
{
    if(n_val!=n_values)
    {
        delete val;
        n_val=n_values;
        val = new quint64[n_val];
    }
    return n_val;
}

void Counter::counter_start()
{
    if (!timer->isValid())
    {
        timer->restart();
    }
    else
    {
        timer->start();
    }
}

Time_struct Counter::counter_stop()
{
    data.t[1]=data.t[0];
    data.t[0]=timer->nsecsElapsed();
    timer->invalidate();

    // Check for min/max delay
    if (data.del_min!=0 && data.t[0]<data.del_min)
        data.del_min=data.t[0];
    if (data.del_max<data.t[0])
        data.del_max=data.t[0];

    // Calculate Average
    if (data.t_n_values[0]<n_val)
    {
        data.t_n_values[0]++;
        data.t_data_sum[0]=data.t_data_sum[0]+data.t[0];
    }
    else
    {
        // Save values of previos period for one further period
        data.t_n_values[1]= data.t_n_values[0];
        data.t_data_sum[1]=data.t_data_sum[0];
        data.t_average[1]=data.t_average[0];

        data.t_n_values[0]=1;
        data.t_data_sum[0]=data.t[0];
    }

    data.t_average[0]=(double)data.t_data_sum[0]/(double)data.t_n_values[0];

    val[data.t_n_values[0]]=data.t[0];

    return data;
}

