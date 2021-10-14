#ifndef DATA_PACKET_H
#define DATA_PACKET_H

#include <QtNetwork/QtNetwork>
#include <QtNetwork/QUdpSocket>
#include <QString>
#include <QElapsedTimer>

class data_graph
{
public:
    data_graph();
    data_graph(int b_size, int set_timebase);
    void insert_element_dispersion(quint64 dispersion, int set_timebase, int pkt_size);

    double *x;
    double *y;

    double *avg;

    //double roundtrip;
    //double *avg_roundtrip;

    double *bit_rate;
    double max_bit_rate;
    double min_bit_rate;
    double total_avg_rate;
    double *avg_rate;
    double sum_rate;

    int size;
    int index;

    int timebase;

    double sum;
    double max;
    double min;
};

class data_packet
{
public:
    data_packet();
    data_packet(int size);

    QByteArray data;
    quint64 index;

    QElapsedTimer timer;
    quint64 tstart;
    quint64 tstop;

    bool pending;
};

class slim_data_packet
{
public:
    slim_data_packet();

    QByteArray data;
    quint64 index;
};


#endif // DATA_PACKET_H
