#include "socket.h"

Socket::Socket()
{   
    Port = new quint16(2000);
    Server_IP = new QHostAddress(QHostAddress::LocalHost);
    setup_correct = true;
    connection=false;
    wrap_arround= false;

    virtual_port= new quint16(0);
    ext_Server = new QHostAddress();

    setup_send="TEST";
    setup_receive="SUCCESS";

    // 8 byte header +  100 byte data
    pkt_size=default_pkt_size;
    pkt_index=0;

    recv_pkt_cnt=0;
    send_pkt_cnt=0;

    // Setup Data arrays
    array_size=default_arr_size;
    send_data=new data_packet* [array_size];
    recv_data=new data_packet* [array_size];
    default_size=true;

    for (quint16 i = 0; i<array_size; i++)
    {
        send_data[i] = new data_packet(pkt_size);
        recv_data[i] = new data_packet(pkt_size);
    }

    graph_1 = new data_graph();

    nr_iterations=0;
    test_operation=0;

    pkt_lost=0;

    os_busy_error=0;

}

Socket::Socket(quint16 Port_in, QHostAddress IP_in)
{
    if (Port_in != 0) {Port = new quint16(Port_in);}
    else {Port = new quint16(2000);};

    // Check if server address is valid
    if(IP_in.toString()!="")
    {
        Server_IP = new QHostAddress(IP_in);
    }
    else    // Default value
    {
         Server_IP = new QHostAddress(QHostAddress::LocalHost);
    }

    setup_correct = true;
    connection=false;
    wrap_arround= false;

    virtual_port= new quint16(0);
    ext_Server = new QHostAddress();

    setup_send="TEST";
    setup_receive="SUCCESS";

    // 8 byte header +  12 byte data
    pkt_size=default_pkt_size;
    pkt_index=0;

    recv_pkt_cnt=0;
    send_pkt_cnt=0;

    // Setup Data arrays
    array_size=default_arr_size;
    send_data=new data_packet* [array_size];
    recv_data=new data_packet* [array_size];
    default_size=true;

    for (quint16 i = 0; i<array_size; i++)
    {
        send_data[i] = new data_packet(pkt_size);
        recv_data[i] = new data_packet(pkt_size);
    }

    graph_1 = new data_graph();

    nr_iterations=0;
    test_operation=0;

    pkt_lost=0;

    os_busy_error=0;
}

void Socket::write_IP(QHostAddress IP_in)
{
    *Server_IP = IP_in;
}

void Socket::write_Port(quint16 Port_in)
{
    *Port = Port_in;
}

quint16 Socket::get_Port()
{
    return *Port;
}
QHostAddress Socket::get_IP()
{
    return *Server_IP;
}

quint16 Socket::setup_success()
{
    return setup_correct;
}

bool Socket::connection_alive()
{
    return connection;
}

bool Socket::connection_alive(bool set)
{
    connection=set;
    return connection;
}

quint64 Socket::Setup_connection_test()
{
    // Check if a connection can be realised
    QByteArray datagram;
    datagram.append(setup_send);
    return(send_Client(datagram.data(),datagram.size()));
}

bool Socket::verify_Recv_string(QString received)
{
    // return 0 when equal
    // Verify if server answers with one of expected data
    if((!received.compare(setup_receive,Qt::CaseInsensitive) || !received.compare(setup_send)))
    {
        connection = true;
    }
    else
    {
        connection = false;
    }

    return connection;
}

void Socket::setup_socket(QObject* ptr)
{
    // Create a ne UDP Socket and set it to low delay operation mode
    udp_Client=new QUdpSocket();
    udp_Client->setSocketOption(QAbstractSocket::LowDelayOption,03);
}

 // Send data function
 quint64 Socket::send_Client(const char *data, quint64 len)
 {
    quint64 byte_sent=0;

    // If wrap arround is set return data to sender, else send data to Address specified by user
    if (!wrap_arround)
    {
        byte_sent=udp_Client->writeDatagram(data, len, *Server_IP, *Port);
    }
    else
    {
        byte_sent=udp_Client->writeDatagram(data, len, *ext_Server, *Port);
    }

    // Increment the packet send count
    if (byte_sent==len)
    {
        send_pkt_cnt++;
    }
    return byte_sent;
 }

 // Returns the packet send count (packet count is cleared when clear is true)
 quint64 Socket::get_send_pkt_cnt(bool clear)
 {
     if (clear==true)
     {
         int send_cnt = send_pkt_cnt;
         send_pkt_cnt=0;
         return send_cnt;
     }

     return send_pkt_cnt;
 }

 bool Socket::bind_Client(QHostAddress::SpecialAddress Mode)
 {
    // Bind socket
     return(udp_Client->bind(Mode, get_Port()));
 }

 bool Socket::connect_Client(QObject *ptr, const char *method)
 {
     // Connect Signal of network interface to Slot of parent
    return (QObject::connect(udp_Client, SIGNAL(readyRead()), ptr, method, Qt::DirectConnection));
 }

 bool Socket::disconnect_Client(QObject *ptr, const char *method)
 {
     // Disconnects the Signal from the processing function
    return (QObject::disconnect(udp_Client, SIGNAL(readyRead()), ptr, method));
 }

 bool Socket::pending_Client()
 {
     // Check if data is waiting to be read from network queue
    return (udp_Client->hasPendingDatagrams());
 }

 quint64 Socket::datasize_Client()
 {
     // Get the size of the data packet pending in network receive queue
    return (udp_Client->pendingDatagramSize());
 }

 QString Socket::HPF_bottleneck_linkrate(quint64 iterations, quint64* pkt_loss, quint64* os_exe_error, bool use_global_var)
 {
     // Calculate the bottleneck as accurate as possible
     QThread::currentThread()->setPriority(QThread::TimeCriticalPriority);
     // Store time critical variables in local variables, so no cache miss operation will occure during execution
     // Create two local send arrays to improve systeme performance during critical execution.
     QByteArray *l_send_data0 = new QByteArray(' ',pkt_size);
     QByteArray *l_send_data1 = new QByteArray(' ',pkt_size);

     // Create two local receive arrays to improve systeme performance during critical execution.
     QByteArray *l_recv_data0 = new QByteArray(' ',pkt_size);
     QByteArray *l_recv_data1 = new QByteArray(' ',pkt_size);

     // Create other local variables to reduce memory access delays and cache miss penalties
     QHostAddress l_IP = *Server_IP;
     quint64 l_Port = *Port;

     QElapsedTimer l_tmr0;
     QElapsedTimer l_tmr1;

     quint64 l_t0=0;
     quint64 l_t1=0;

     quint64 dispersion;
     QString result_string="";

     os_busy_error=0;

     //Start operation timeout timer
     time_out_timer.start();
     pkt_lost=0;


     // Determine if variables belonging to the class, but which are slower shall be used
     if (use_global_var == true)
     {
         // Setup Array for High Performance measurement
         if (array_size!=2)
         {
            reinitialize_data_array(pkt_size, 2);
         }

         // Loop flattened
         if (pkt_size!=0)
         {
             QString nr=QString::number(0);
             send_data[0]->data.replace(0,nr.length(),nr.toUtf8());
             send_data[0]->index=0;
             send_data[0]->pending=true;
             *l_send_data0 = send_data[0]->data;

             nr=QString::number(1);
             send_data[1]->data.replace(0,nr.length(),nr.toUtf8());
             send_data[1]->index=1;
             send_data[1]->pending=true;
             *l_send_data1 = send_data[1]->data;
         }
     }
     else
     {
         // Set data packet index and content
        QString nr=QString::number(0);
        l_send_data0->replace(0,nr.length(),nr.toUtf8());
        nr=QString::number(1);
        l_send_data1->replace(0,nr.length(),nr.toUtf8());
     }

     // Sending iterations and data aquisition
     for (int index=0; index<iterations; index++)
     {
         time_out_timer.restart();
         // TIME CRITICAL SECTION START /////////////////////////////////////
         // Timer initialised in sequence because data handling takes not much time.
         // Precision not influenced by order of function calls. Limits are represented by network queue
         l_tmr0.start();
         l_tmr1.start();
         // Access udp send function directly to reduce tree height
         udp_Client->writeDatagram(l_send_data0->data(),l_IP,l_Port);
         udp_Client->writeDatagram(l_send_data1->data(),l_IP,l_Port);

         // Poll network new data available flag as long as no time out occurs
         while (!udp_Client->hasPendingDatagrams() && (time_out_timer.elapsed()<default_timeout));
         // Stop first timer
         l_t0=l_tmr0.nsecsElapsed();
         // Read data from input queue to clear flag
         udp_Client->readDatagram(l_recv_data0->data(),l_recv_data0->size());

         // CHECK if there is already another datagram in Buffer,
         // this would mean the system scheduling was to slow at aquiring data
         if (udp_Client->hasPendingDatagrams())
         {
             // increment the operating system scheduler failure counter
             os_busy_error++;
             // Read data out to clear input queue
             udp_Client->readDatagram(l_recv_data1->data(),l_recv_data1->size());
         }
         else
         {
             // Wait for second data packet to arrive
             while (!udp_Client->hasPendingDatagrams() && (time_out_timer.elapsed()<default_timeout));
             // Stop second timer
             l_t1=l_tmr1.nsecsElapsed();
             // Read data from input queue
             udp_Client->readDatagram(l_recv_data1->data(),l_recv_data1->size());
             // TIME CRITICAL SECTION END ////////////////////////////////////////

             // ONLY Process data when no timeout occured
             if (time_out_timer.elapsed()<default_timeout)
             {
                 // DATA processing
                 // Calculate dispersion
                 dispersion=l_t1-l_t0;
                 // Calculate Bottleneck
                 int index=graph_1->index;
                 // Store dispersion as well as pkt size into data container
                 graph_1->insert_element_dispersion(dispersion,1,pkt_size);
                 result_string+=("Measurement Nr: "+QString::number(graph_1->index)+"   Packet size: "+QString::number(pkt_size)+"\n"+
                                 "Link_Rate (MB/s): "+QString::number(graph_1->bit_rate[index],'f',2)+"\n"+
                                 "dispersion (us): "+QString::number(graph_1->y[index])+
                                 " (Approx.(ns): " +QString::number(dispersion)+") \n\n");
             }
             else
             {
                 // When a timeout occurs the packet is assumed to be lost
                pkt_lost++;
             }

         }
     }
     //result_string+=("Average (MB/s): "+QString::number(graph_1->total_avg_rate,'f',3)+"\n");

     // Return packet lost and scheduler error count
     *os_exe_error=os_busy_error;
     *pkt_loss=pkt_lost;
     return result_string;
 }

 quint64 Socket::read_Client(QByteArray* datagram, QHostAddress *sender, quint16 *senderport)
 {   
     // Read data from input queue
     quint64 datapackets=udp_Client->readDatagram(datagram->data(), datagram->size(), ext_Server, virtual_port);
     *sender=*ext_Server;
     *senderport=*virtual_port;

     // Increment the number of received packets
     recv_pkt_cnt++;
     return datapackets;
 }

quint64 Socket::get_recv_pkt_cnt(bool clear)
{
    if (clear==true)
    {
        int recv_cnt = recv_pkt_cnt;
        recv_pkt_cnt=0;
        return recv_cnt;
    }

    return recv_pkt_cnt;
}

 bool Socket::set_wrap_arround(bool wrap)
 {
    wrap_arround=wrap;
    return wrap_arround;
 }

 bool Socket::get_wrap_arround(){return wrap_arround;}

 // Sets strings that are used to test the server response
void Socket::set_test_string(QString test_send, QString test_recv)
 {
    setup_send=test_send;
    setup_receive=test_recv;
 }

// Setup data handling arrays
 void Socket::reinitialize_data_array(quint16 size_pkt, quint16 size_array)
 {   
     for (quint16 i=0; i<array_size; i++)
     {
         delete send_data[i];
         delete recv_data[i];
     }

     // 8 byte UDP header + 20 byte IP header
     pkt_size=size_pkt+8+20;
     pkt_index=0;

     // Setup Data arrays
     array_size=size_array;
     send_data=new data_packet* [array_size];
     recv_data=new data_packet* [array_size];
     default_size=true;

     for (quint16 i = 0; i<array_size; i++)
     {
         send_data[i] = new data_packet(pkt_size);
         recv_data[i] = new data_packet(pkt_size);
     }
 }

quint8 Socket::get_test_operation(){return test_operation;}

void Socket::set_packet_size(quint16 size)
{
    pkt_size=size;
    reinitialize_data_array(pkt_size,array_size);
}

quint16 Socket::get_packet_size()
{
    return pkt_size;
}

quint16 Socket::set_operation_mode(quint16 Mode)
{
    test_operation=Mode;
    return test_operation;
}

void Socket::set_graph_x_axis_range(int iter, int interval)
{
    delete graph_1;
    graph_1=new data_graph(iter, interval);
}

// return data storage of the taken measurements
data_graph Socket::get_graph1()
{
    return *graph_1;
}

void Socket::Make_histogram(data_graph data_process, data_graph *histogram, int resolution)
{
    // Determine floating point precision
    int mult=qPow(10,resolution);
    // Calculate offset
    int min_offset = data_process.min_bit_rate*mult;

    // Initialize Histogram
    for (int index=0; index<histogram->size; index++)
    {
        histogram->y[index]=0;
        histogram->x[index]=((double)(min_offset+index)/qPow(10,resolution));
    }

    for (int index=0; index<data_process.size; index++)
    {
        // Sort values from graph into histogram
        int bitrate=(data_process.bit_rate[index]-data_process.min_bit_rate)*mult;
        histogram->y[bitrate]+=1;
    }

    // Search for max values in histogram
    for (int index=0; index<histogram->size; index++)
    {
        if (histogram->y[index]>histogram->max)
        {
            histogram->max=histogram->y[index];
        }
        if (histogram->y[index]<histogram->min) //|| histogram->min==0)
        {
            histogram->min=histogram->y[index];
        }
    }

    histogram->index=(histogram->size-1);

    return;
}

void Socket::delete_socket()
{
    delete udp_Client;
}
