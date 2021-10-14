#include "data_packet.h"

data_graph::data_graph()
{
    x = new double;
    y = new double;
    avg= new double;

    //roundtrip=0;
    //avg_roundtrip=new double;

    size=1;
    index=0;

    bit_rate=new double;
    total_avg_rate=0;
    avg_rate=new double;
    sum_rate=0;
    max_bit_rate=0;
    min_bit_rate=0;

    timebase=1;

    sum=0;
    min=0;
    max=0;
}

data_graph::data_graph(int b_size, int set_timebase)
{
    x = new double[b_size];
    y = new double[b_size];

    avg=new double[b_size];

    //roundtrip=0;
    //avg_roundtrip=new double [b_size];

    size=b_size;
    index=0;

    bit_rate=new double[b_size];
    total_avg_rate=0;
    avg_rate=new double[b_size];
    sum_rate=0;

    max_bit_rate=0;
    min_bit_rate=0;

    timebase=set_timebase;

    sum=0;
    min=0;
    max=0;
}

void data_graph::insert_element_dispersion(quint64 dispersion, int set_timebase, int pkt_size)
//void data_graph::insert_element(double value, double round_t, int pkt_size)
{
    int l_pkt_size=pkt_size+20+8;   // 20 Byte IP header + 8 Byte UDP header
    // Set points
    x[index]=index;
    // Display dispersion in us
    y[index]=(double)dispersion/qPow(10,3);

    // Set timebase
    timebase=set_timebase;

    // determine min
    if (y[index]<min || min==0)
    {
        min=y[index];
    }

    // determine max
    if (y[index]>max)
    {
        max=y[index];
    }

    // determine sum and average dispersion
    if (index==0)
    {
        sum=y[index];
        avg[index]=y[index];

    }
    else
    {
        sum=sum+y[index];
        avg[index]=sum/(index+1);
    }

    // Calculate bit rate in MBit/s
    bit_rate[index]=((double)l_pkt_size*8)/((double) dispersion*qPow(10,-3));

    // determine average temporary bitrate
    if (index==0)
    {
        sum_rate=bit_rate[index];
        avg_rate[index]=bit_rate[index];

    }
    else
    {
        sum_rate+=bit_rate[index];
        avg_rate[index]=sum_rate/(index+1);
    }

    // calculate total average bitrate
    total_avg_rate=sum_rate/(index+1);

    // determine mininmum bitrate
    if (bit_rate[index]<min_bit_rate || min_bit_rate==0)
    {
        min_bit_rate=bit_rate[index];
    }

    // determine maximum bitrate
    if (bit_rate[index]>max_bit_rate)
    {
        max_bit_rate=bit_rate[index];
    }

    // increment internal index to make sure new data is written into the next array element
    if (index<size)
    {
        index++;
    }
    else
    {
        index=0;
    }

}

data_packet::data_packet()
{
    data.clear();
    index = 0;
    tstart = 0;
    tstop = 0;

    pending=true;
}

data_packet::data_packet(int size)
{
    data.resize(size);
    data.clear();
    index = 0;
    tstart = 0;
    tstop = 0;

    pending=true;
}

slim_data_packet::slim_data_packet()
{
    data.clear();
    index=0;
}
