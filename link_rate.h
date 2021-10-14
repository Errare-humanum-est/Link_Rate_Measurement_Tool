#ifndef LINK_RATE_H
#define LINK_RATE_H

#include <QMainWindow>
#include <QtNetwork/QtNetwork>
#include <QtNetwork/QUdpSocket>
#include <QString>
#include <QIODevice>
#include <QFileDialog>
#include "data_packet.h"
#include "socket.h"
#include "debug_window.h"
#include "counter.h"
#include "qcustomplot.h"

#define output_max_char 50
#define STR_END "END"

namespace Ui {
class Link_Rate;
}

class Link_Rate : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit Link_Rate(QWidget *parent = 0);
    ~Link_Rate();
    
private slots:

    void mousePress_g1();
    void mouseWheel_g1();

    void mousePress_g2();
    void mouseWheel_g2();


    void on_Check_Server_IP_clicked();

    void on_Test_data_clicked();

    void Test_data_process(quint64 iterations, int output_en, bool use_global_var, quint64 *packets_lost);

    void readPendingDatagrams();

    void on_Server_IP_textEdited(const QString &arg1);

    void on_Send_clicked();

    void on_checkBox_clicked();

    void on_wrap_arround_clicked();

    void on_udp_pkt_size_valueChanged(int arg1);

    void on_bottleneck_iteration_valueChanged(int arg1);

    void setup_customPlot();

    QColor get_color(quint16 color);

    void wrap_arround(QHostAddress Client_IP, quint64 Client_Port);

    void on_pushButton_clicked();

    void handle_tmr_interrupt();

    void on_tmr_iterations_valueChanged(int arg1);

    void convert_ms_to_sec(int in_ms, int *out_min, int *out_sec, int *out_msec);

    void on_tmr_interval_valueChanged(int arg1);

    void set_graph1(quint8 fast_draw);
    void set_graph2(quint8 fast_draw);

    void draw_graph1(quint16 Mode);
    void draw_graph2(quint16 Mode);

    void Recv_Timeout_setup();
    void Recv_Timeout_detected();

    void Recv_watchdog();

    void on_pushButton_2_clicked();

    void measure_bottleneck();

    void update_graphs();

    void on_Draw_bitrate_clicked();

    void on_draw_Hist_clicked();

    void on_Draw_diff_clicked();

    void on_Line_Style_valueChanged(int arg1);

    void on_Scatter_Style_valueChanged(int arg1);

    void on_color_sel_valueChanged(int arg1);

    void on_width_sel_valueChanged(double arg1);

    void on_Line_Style_2_valueChanged(int arg1);

    void on_Scatter_Style_2_valueChanged(int arg1);

    void on_color_sel_2_valueChanged(int arg1);

    void on_width_sel_2_valueChanged(double arg1);

    void on_Line_Style_3_valueChanged(int arg1);

    void on_Scatter_Style_3_valueChanged(int arg1);

    void on_color_sel_3_valueChanged(int arg1);

    void on_width_sel_3_valueChanged(double arg1);

    void on_Line_Style_4_valueChanged(int arg1);

    void on_Scatter_Style_4_valueChanged(int arg1);

    void on_color_sel_4_valueChanged(int arg1);

    void on_width_sel_4_valueChanged(double arg1);

    void on_Line_Style_5_valueChanged(int arg1);

    void on_Scatter_Style_5_valueChanged(int arg1);

    void on_color_sel_5_valueChanged(int arg1);

    void on_width_sel_5_valueChanged(double arg1);

    void on_Line_Style_6_valueChanged(int arg1);

    void on_Scatter_Style_6_valueChanged(int arg1);

    void on_color_sel_6_valueChanged(int arg1);

    void on_width_sel_6_valueChanged(double arg1);

    void on_Line_Style_7_valueChanged(int arg1);

    void on_Scatter_Style_7_valueChanged(int arg1);

    void on_color_sel_7_valueChanged(int arg1);

    void on_width_sel_7_valueChanged(double arg1);

    void on_Line_Style_8_valueChanged(int arg1);

    void on_Scatter_Style_8_valueChanged(int arg1);

    void on_color_sel_8_valueChanged(int arg1);

    void on_width_sel_8_valueChanged(double arg1);

    void on_Draw_bitrate_2_clicked();

    void on_draw_Hist_2_clicked();

    void on_Draw_diff_2_clicked();

    void on_g1_histo_res_valueChanged(int arg1);

    void on_g2_histo_res_valueChanged(int arg1);

    void on_Text_Receive_clicked();

    void on_Text_save_clicked();

    void on_MP3_Receive_clicked();

    void on_Music_Command_editingFinished();

    void on_Music_save_clicked();

    void on_Video_Receive_clicked();

    void on_Video_save_clicked();

    void on_Video_save_2_clicked();

private:
    Ui::Link_Rate *ui;
    bool socket_setup;

    QUdpSocket udpServer;
    QUdpSocket udpClient;

    Debug_Window Debug;
    Debug_Window Output;
    Debug_Window Out_Text;

    Counter delay;

    data_graph l_graph_1;

    QTimer *tmr_interval;
    quint64 tmr_iterations;
    quint64 tmr_iteration_cnt;

    quint64 M1_send_iter;

    bool recv_out_of_order;

    quint64 pkt_lost;

    Socket *Setup;

    // RAW Data storage
    QMap <quint64,slim_data_packet> recv_data;
    // Retransmitted Data storage
    QMap <quint64,slim_data_packet> retr_data;

    quint64 max_index_map;
    quint64 missing_packets;
};




#endif // LINK_RATE_H


