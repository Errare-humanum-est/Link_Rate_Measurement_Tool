#include "time_struct.h"

Time_struct::Time_struct()
{
    del_min = 0;
    del_max = 0;

    t_average[0] = 0;
    t_average[1] = 0;

    t_data_sum[0] = 0;
    t_data_sum[1] = 0;

    t_n_values[0] = 0;
    t_n_values[1] = 0;

    t[0]=0;
    t[1]=0;

}

void Time_struct::Set_Zero()
{
    del_min = 0;
    del_max = 0;

    t_average[0] = 0;
    t_average[1] = 0;

    t_data_sum[0] = 0;
    t_data_sum[1] = 0;

    t_n_values[0] = 0;
    t_n_values[1] = 0;

    t[0]=0;
    t[1]=0;

}
