#ifndef TIME_STRUCT_H
#define TIME_STRUCT_H

#include <QString>

class Time_struct
{
public:
    Time_struct();
    void Set_Zero();

    quint64 del_min;
    quint64 del_max;

    double t_average [2];
    quint64 t_n_values [2];
    quint64 t_data_sum [2];
    quint64 t[2];

};

#endif // TIME_STRUCT_H
