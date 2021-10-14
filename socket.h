#ifndef SOCKET_H
#define SOCKET_H

#include <QtNetwork/QtNetwork>
#include <QtNetwork/QUdpSocket>
#include <QString>

#include "data_packet.h"

#define default_pkt_size 100
#define default_arr_size 2
#define default_timeout 500
#define HW_Buffer_Limit_Byte 5000

class Socket
{
public:
    Socket();
    Socket(quint16 Port_in, QHostAddress IP_in);
    void write_IP(QHostAddress IP_in);
    void write_Port(quint16 Port_in);

    quint16 get_Port();
    QHostAddress get_IP();

    quint16 setup_success();

    bool connection_alive();
    bool connection_alive(bool set);

    quint64 Setup_connection_test();
    bool verify_Recv_string(QString received);

    // Setup System
    void setup_socket(QObject *ptr);
    void delete_socket();

    quint64 send_Client(const char *data, quint64 len);

    // Qt Client
    bool bind_Client(QHostAddress::SpecialAddress Mode);
    bool connect_Client(QObject *ptr, const char *method);
    bool disconnect_Client(QObject *ptr, const char *method);

    bool pending_Client();
    quint64 datasize_Client();
    quint64 read_Client(QByteArray* datagram, QHostAddress *sender, quint16 *senderport);

    QString HPF_bottleneck_linkrate(quint64 iterations, quint64 *pkt_loss, quint64* os_exe_error, bool use_global_var);

    data_packet store_incoming_packets(QByteArray *datagram);

    bool set_wrap_arround(bool wrap);
    bool get_wrap_arround();

    void set_test_string(QString test_send, QString test_recv);

    void reinitialize_data_array(quint16 size_pkt, quint16 size_array);

    void calculate_bottleneck(quint64 iterations);
    //void calculate_bottleneck(quint16 packet_size);
    quint8 get_test_operation();

    void Setup_pkt_size(quint16 size);
    void set_packet_size(quint16 size);
    quint16 get_packet_size();

    quint16 set_operation_mode(quint16 Mode);
    quint64 process_bottleneck_dispersion(quint16* nr_p_t0, quint16* nr_p_t1, int *t1, int *t2, int *diff, int *diff_us);

    // datagraph
    void set_graph_x_axis_range(int iter, int interval);

    data_graph get_graph1();

    quint64 get_recv_pkt_cnt(bool clear);
    quint64 get_send_pkt_cnt(bool clear);    

    quint64 get_n_pkt_lost();

    data_graph* graph_1;

    void Make_histogram(data_graph data_process, data_graph *histogram, int resolution);

    void thread_var(QUdpSocket *skt, QHostAddress *ip, quint16 *prt, quint64 *recv_cnt, data_packet **data);

private:

    quint16 *Port;
    QHostAddress *Server_IP;
    quint16 setup_correct;

    bool connection;
    bool wrap_arround;

    // System test variable
    quint8 test_operation;

    // Setup of external Server
    QHostAddress *ext_Server;
    quint16 *virtual_port;

    // SETUP DATA
    QString setup_send;
    QString setup_receive;

    QUdpSocket *udp_Client;

    bool default_size;
    quint16 pkt_size;
    quint16 pkt_index;

    // Variables to determine packet loss rate
    // time_out_times should only be used for test operations
    quint64 recv_pkt_cnt;
    quint64 send_pkt_cnt;

    quint64 pkt_lost;
    bool process_pkt;
    QElapsedTimer time_out_timer;

    quint64 nr_iterations;

    quint16 array_size;
    data_packet** send_data;
    data_packet** recv_data;

    quint64 os_busy_error;

    QElapsedTimer timer_test;
};

#endif // SOCKET_H
