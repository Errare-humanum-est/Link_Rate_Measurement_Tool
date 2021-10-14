#include "link_rate.h"
#include "ui_link_rate.h"

Link_Rate::Link_Rate(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Link_Rate)
{
    ui->setupUi(this);
    QThread::currentThread()->setPriority(QThread::TimeCriticalPriority);
    int PID=QThread::currentThread()->priority();
    Debug.AddText("PID: "+QString::number(PID)+": (Timecritial=6, Highpriority=5) \n");
    socket_setup=false;
    Out_Text.ClearText();
    //Setup->Setup_pkt_size(ui->udp_pkt_size->value());

    pkt_lost = 0;

    setup_customPlot();
    M1_send_iter=0;

    ui->recv_output->setReadOnly(true);
    ui->Console->setReadOnly(true);

    ui->Music_save->setDisabled(true);

    ui->Video_save->setDisabled(true);
    ui->Video_save_2->setDisabled(true);

    retr_data.clear();
    recv_data.clear();

    connect(ui->customPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress_g1()));
    connect(ui->customPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel_g1()));
    connect(ui->customPlot2, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress_g2()));
    connect(ui->customPlot2, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel_g2()));
}


Link_Rate::~Link_Rate()
{
    delete ui;
}

void Link_Rate::wrap_arround(QHostAddress Client_IP, quint64 Client_Port)
{

}

// GRAPH ZOOM FUNCTIONS
void Link_Rate::mousePress_g1()
{
  if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->customPlot->axisRect()->setRangeDrag(ui->customPlot->xAxis->orientation());
  else if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->customPlot->axisRect()->setRangeDrag(ui->customPlot->yAxis->orientation());
  else
    ui->customPlot->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
}

void Link_Rate::mouseWheel_g1()
{
  if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->customPlot->axisRect()->setRangeZoom(ui->customPlot->xAxis->orientation());
  else if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->customPlot->axisRect()->setRangeZoom(ui->customPlot->yAxis->orientation());
  else
    ui->customPlot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}

void Link_Rate::mousePress_g2()
{
  if (ui->customPlot2->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->customPlot2->axisRect()->setRangeDrag(ui->customPlot2->xAxis->orientation());
  else if (ui->customPlot2->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->customPlot2->axisRect()->setRangeDrag(ui->customPlot2->yAxis->orientation());
  else
    ui->customPlot2->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
}

void Link_Rate::mouseWheel_g2()
{
  if (ui->customPlot2->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->customPlot2->axisRect()->setRangeZoom(ui->customPlot2->xAxis->orientation());
  else if (ui->customPlot2->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->customPlot2->axisRect()->setRangeZoom(ui->customPlot2->yAxis->orientation());
  else
    ui->customPlot2->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}

QColor Link_Rate::get_color(quint16 color)
{
    switch (color)
    {
        case 1: return QColor(255, 0, 255); break;
        case 2: return QColor(0,   0, 255); break;
        case 3: return QColor(0, 255, 255); break;
        case 4: return QColor(0, 255,   0); break;
        case 5: return QColor(255, 255, 0); break;
        default: return QColor(255, 0, 0); break;
    }

}

// SETUP GRAPHS
void Link_Rate::setup_customPlot()
{
    QFont legendFont = font();
    legendFont.setPointSize(10);

    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                    QCP::iSelectLegend | QCP::iSelectPlottables);
    ui->customPlot->xAxis->setRange(-8, 8);
    ui->customPlot->yAxis->setRange(-5, 5);
    ui->customPlot->axisRect()->setupFullAxesBox();

    ui->customPlot->plotLayout()->insertRow(0);
    ui->customPlot->plotLayout()->addElement(0, 0, new QCPPlotTitle(ui->customPlot, "Graph 1"));

    ui->customPlot->xAxis->setLabel("Packet number");
    ui->customPlot->yAxis->setLabel("Time (us)");
    ui->customPlot->legend->setVisible(true);
    ui->customPlot->legend->setFont(legendFont);
    ui->customPlot->legend->setSelectedFont(legendFont);
    ui->customPlot->legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items


    ui->customPlot2->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                    QCP::iSelectLegend | QCP::iSelectPlottables);
    ui->customPlot2->xAxis->setRange(0, 8);
    ui->customPlot2->yAxis->setRange(0, 5);
    ui->customPlot2->axisRect()->setupFullAxesBox();

    ui->customPlot2->plotLayout()->insertRow(0);
    ui->customPlot2->plotLayout()->addElement(0, 0, new QCPPlotTitle(ui->customPlot2, "Graph 2"));

    ui->customPlot2->xAxis->setLabel("Bitrate (Mbit/s)");
    ui->customPlot2->yAxis->setLabel("Distribution");
    ui->customPlot2->legend->setVisible(true);
    ui->customPlot2->legend->setFont(legendFont);
    ui->customPlot2->legend->setSelectedFont(legendFont);
    ui->customPlot2->legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items

}

// Draw first graph
void Link_Rate::draw_graph1(quint16 Mode)
{
    data_graph l_graph=Setup->get_graph1();

    // get resolution of histogram and range of values
    int resolution=ui->g1_histo_res->value();
    int mult=qPow(10,resolution);
    int x_resolution=((l_graph.max_bit_rate-l_graph.min_bit_rate)*mult)+1;
    // Create new Histogram local histogram
    data_graph *histogram=new data_graph(x_resolution,1);

    int n=0;
    int disp_average=0;

    // Determine size of the vectors
    switch (Mode)
    {
    case 2: n = histogram->size; break;
    default: n = l_graph.index; break;
    }

    // Clear all graphs to have a optical processing effect
    // Can be reduced to improve system performance
    if (n>0)ui->customPlot->clearGraphs();

    QVector<double> x(n), y(n), avg(n);
    QVector<double> min(n), max(n);

    QPen graphPen1;
    QPen graphPen2;

    // Setup Graphs according to system test configuration
    // values to be displayed are derived from the l_graph data_packet variable
    switch (Mode)
    {
        // MODE=2 : BIT_RATE_HISTOGRAM
    case 2:
        // Calculate Histogram
        Setup->Make_histogram(l_graph, histogram, resolution);
        // No average provided
        disp_average=0;

        for (int i=0; i<n; i++)
        {
            x[i]=histogram->x[i]*histogram->timebase;

            // dispersion ms
            y[i] = histogram->y[i];
        }
        // Put static values into vector
        min.insert(0,histogram->size,histogram->min);
        max.insert(0,histogram->size,histogram->max);

        ui->customPlot->yAxis->setLabel("Distribution");
        ui->customPlot->xAxis->setLabel("Bitrate (Mbit/s)");

        // Calculate graph plot ranges
        ui->customPlot->xAxis->setRange(0-((double)histogram->x[histogram->size-1]*0.1),
                                         ((double)histogram->x[histogram->size-1]*1.1));
        ui->customPlot->yAxis->setRange(0, histogram->max+(histogram->max/10));
        break;

        // MODE=1 : BIT_RATE
    case 1:
        // Display average true
        disp_average=1;

        for (int i=0; i<n; i++)
        {
            x[i]=l_graph.x[i]*l_graph.timebase;

            // dispersion ms
            y[i] = l_graph.bit_rate[i];
            avg[i]=l_graph.avg_rate[i];
        }
        // Put static values into vector
        min.insert(0,l_graph.size,l_graph.min_bit_rate);
        max.insert(0,l_graph.size,l_graph.max_bit_rate);

        ui->customPlot->yAxis->setLabel("Link rate (MBit/s)");

        // Calculate graph plot ranges
        ui->customPlot->xAxis->setRange(0-((double)l_graph.index*l_graph.timebase/20), l_graph.index*l_graph.timebase+((double)l_graph.index*l_graph.timebase/20));
        ui->customPlot->yAxis->setRange(l_graph.min_bit_rate-(l_graph.min_bit_rate/10), l_graph.max_bit_rate+(l_graph.max_bit_rate/10));

        // Name x axis depending on test mode
        switch (Setup->get_test_operation())
        {
        case 1: ui->customPlot->xAxis->setLabel("Packet number"); break;
        case 2: ui->customPlot->xAxis->setLabel("Time (ms)"); break;
        }

        break;

        // MODE=0 : dispersion
    default:
        // Display average
        disp_average=1;
        for (int i=0; i<n; i++)
        {
            x[i]=l_graph.x[i]*l_graph.timebase;

            // dispersion ms
            y[i] = l_graph.y[i];
            avg[i]=l_graph.avg[i];
        }
        // Put static values into vector
        min.insert(0,l_graph.size,l_graph.min);
        max.insert(0,l_graph.size,l_graph.max);

        ui->customPlot->yAxis->setLabel("Dispersion (us)");

        // Name x axis depending on test mode
        ui->customPlot->xAxis->setRange(0-((double)l_graph.index*l_graph.timebase/20), l_graph.index*l_graph.timebase+((double)l_graph.index*l_graph.timebase/20));
        ui->customPlot->yAxis->setRange(l_graph.min-(l_graph.min/10), l_graph.max+(l_graph.max/10));

        switch (Setup->get_test_operation())
        {
        case 1: ui->customPlot->xAxis->setLabel("Packet number"); break;
        case 2: ui->customPlot->xAxis->setLabel("Time (ms)"); break;
        }

        break;
    }


    // Plot dispersion
    ui->customPlot->addGraph();

    // Plot the curves (min, max, f(x) and avg(except for histogram))
    switch (Mode)
    {
    case 2: ui->customPlot->graph()->setName("Bitrate Distribution"); break;
    case 1: ui->customPlot->graph()->setName("Bitrate"); break;
    default: ui->customPlot->graph()->setName("Dispersion"); break;
    }
    ui->customPlot->graph()->setData(x, y);
    ui->customPlot->graph()->setLineStyle((QCPGraph::LineStyle)(ui->Line_Style->value()));
    ui->customPlot->graph()->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)(ui->Scatter_Style->value())));
    graphPen1.setColor(get_color(ui->color_sel->value()));
    graphPen1.setWidthF(ui->width_sel->value());
    ui->customPlot->graph()->setPen(graphPen1);

    if (disp_average==1)
    {
        ui->customPlot->addGraph();
        ui->customPlot->graph()->setName("Average");
        ui->customPlot->graph()->setData(x, avg);
        ui->customPlot->graph()->setLineStyle((QCPGraph::LineStyle)(ui->Line_Style_2->value()));
        ui->customPlot->graph()->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)(ui->Scatter_Style_2->value())));
        graphPen2.setColor(get_color(ui->color_sel_2->value()));
        graphPen2.setWidthF(ui->width_sel_2->value());
        ui->customPlot->graph()->setPen(graphPen2);
    }

    // Plot average
    ui->customPlot->addGraph();
    ui->customPlot->graph()->setName("Minimum");
    ui->customPlot->graph()->setData(x, min);
    ui->customPlot->graph()->setLineStyle((QCPGraph::LineStyle)(ui->Line_Style_3->value()));
    ui->customPlot->graph()->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)(ui->Scatter_Style_3->value())));
    graphPen2.setColor(get_color(ui->color_sel_3->value()));
    graphPen2.setWidthF(ui->width_sel_3->value());
    ui->customPlot->graph()->setPen(graphPen2);

    ui->customPlot->addGraph();
    ui->customPlot->graph()->setName("Maximum");
    ui->customPlot->graph()->setData(x, max);
    ui->customPlot->graph()->setLineStyle((QCPGraph::LineStyle)(ui->Line_Style_4->value()));
    ui->customPlot->graph()->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)(ui->Scatter_Style_4->value())));
    graphPen2.setColor(get_color(ui->color_sel_4->value()));
    graphPen2.setWidthF(ui->width_sel_4->value());
    ui->customPlot->graph()->setPen(graphPen2);
    ui->customPlot->replot();

    return;
}

// Draw Graph 2 equal to Draw Graph 1
void Link_Rate::draw_graph2(quint16 Mode)
{
    data_graph l_graph=Setup->get_graph1();

    int resolution=ui->g2_histo_res->value();
    int mult=qPow(10,resolution);
    int x_resolution=((l_graph.max_bit_rate-l_graph.min_bit_rate)*mult)+1;
    // Create new Histogram local histogram
    data_graph *histogram=new data_graph(x_resolution,1);

    int n=0;
    int disp_average=0;

    // Determine size of the vectors
    switch (Mode)
    {
    case 2: n = histogram->size; break;
    default: n = l_graph.index; break;
    }

    // Clear all graphs to have a optical processing effect
    // Can be reduced to improve system performance
    if (n>0)ui->customPlot2->clearGraphs();

    QVector<double> x(n), y(n), avg(n);
    QVector<double> min(n), max(n);

    QPen graphPen1;
    QPen graphPen2;

    switch (Mode)
    {
        // MODE=2 : BIT_RATE_HISTOGRAM
    case 2:
        // Calculate Histogram
        Setup->Make_histogram(l_graph, histogram, resolution);
        // No average provided
        disp_average=0;

        for (int i=0; i<n; i++)
        {
            x[i]=histogram->x[i]*histogram->timebase;

            // dispersion ms
            y[i] = histogram->y[i];
        }
        // Put static values into vector
        min.insert(0,histogram->size,histogram->min);
        max.insert(0,histogram->size,histogram->max);

        ui->customPlot2->yAxis->setLabel("Distribution");
        ui->customPlot2->xAxis->setLabel("Bitrate (Mbit/s)");
        ui->customPlot2->xAxis->setRange(0-((double)histogram->x[histogram->size-1]*0.1),
                                         ((double)histogram->x[histogram->size-1]*1.1));
        ui->customPlot2->yAxis->setRange(0, histogram->max+(histogram->max/10));
        break;

        // MODE=1 : BIT_RATE
    case 1:
        // Display average
        disp_average=1;

        for (int i=0; i<n; i++)
        {
            x[i]=l_graph.x[i]*l_graph.timebase;

            // dispersion ms
            y[i] = l_graph.bit_rate[i];
            avg[i]=l_graph.avg_rate[i];
        }
        // Put static values into vector
        min.insert(0,l_graph.size,l_graph.min_bit_rate);
        max.insert(0,l_graph.size,l_graph.max_bit_rate);

        ui->customPlot2->yAxis->setLabel("Link rate (MBit/s)");

        ui->customPlot2->xAxis->setRange(0-((double)l_graph.index*l_graph.timebase/20), l_graph.index*l_graph.timebase+((double)l_graph.index*l_graph.timebase/20));
        ui->customPlot2->yAxis->setRange(l_graph.min_bit_rate-(l_graph.min_bit_rate/10), l_graph.max_bit_rate+(l_graph.max_bit_rate/10));

        switch (Setup->get_test_operation())
        {
        case 1: ui->customPlot2->xAxis->setLabel("Packet number"); break;
        case 2: ui->customPlot2->xAxis->setLabel("Time (ms)"); break;
        }

        break;

        // MODE=0 : dispersion
    default:
        // Display average
        disp_average=1;
        for (int i=0; i<n; i++)
        {
            x[i]=l_graph.x[i]*l_graph.timebase;

            // dispersion ms
            y[i] = l_graph.y[i];
            avg[i]=l_graph.avg[i];
        }
        // Put static values into vector
        min.insert(0,l_graph.size,l_graph.min);
        max.insert(0,l_graph.size,l_graph.max);

        ui->customPlot2->yAxis->setLabel("Dispersion (us)");

        ui->customPlot2->xAxis->setRange(0-((double)l_graph.index*l_graph.timebase/20), l_graph.index*l_graph.timebase+((double)l_graph.index*l_graph.timebase/20));
        ui->customPlot2->yAxis->setRange(l_graph.min-(l_graph.min/10), l_graph.max+(l_graph.max/10));

        switch (Setup->get_test_operation())
        {
        case 1: ui->customPlot2->xAxis->setLabel("Packet number"); break;
        case 2: ui->customPlot2->xAxis->setLabel("Time (ms)"); break;
        }

        break;
    }


    // Plot dispersion
    ui->customPlot2->addGraph();
    switch (Mode)
    {
    case 2: ui->customPlot2->graph()->setName("Bitrate Distribution"); break;
    case 1: ui->customPlot2->graph()->setName("Bitrate"); break;
    default: ui->customPlot2->graph()->setName("Dispersion"); break;
    }
    ui->customPlot2->graph()->setData(x, y);
    ui->customPlot2->graph()->setLineStyle((QCPGraph::LineStyle)(ui->Line_Style_5->value()));
    ui->customPlot2->graph()->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)(ui->Scatter_Style_5->value())));
    graphPen1.setColor(get_color(ui->color_sel_5->value()));
    graphPen1.setWidthF(ui->width_sel_5->value());
    ui->customPlot2->graph()->setPen(graphPen1);

    if (disp_average==1)
    {
        ui->customPlot2->addGraph();
        ui->customPlot2->graph()->setName("Average");
        ui->customPlot2->graph()->setData(x, avg);
        ui->customPlot2->graph()->setLineStyle((QCPGraph::LineStyle)(ui->Line_Style_6->value()));
        ui->customPlot2->graph()->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)(ui->Scatter_Style_6->value())));
        graphPen2.setColor(get_color(ui->color_sel_6->value()));
        graphPen2.setWidthF(ui->width_sel_6->value());
        ui->customPlot2->graph()->setPen(graphPen2);
    }

    // Plot average
    ui->customPlot2->addGraph();
    ui->customPlot2->graph()->setName("Minimum");
    ui->customPlot2->graph()->setData(x, min);
    ui->customPlot2->graph()->setLineStyle((QCPGraph::LineStyle)(ui->Line_Style_7->value()));
    ui->customPlot2->graph()->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)(ui->Scatter_Style_7->value())));
    graphPen2.setColor(get_color(ui->color_sel_7->value()));
    graphPen2.setWidthF(ui->width_sel_7->value());
    ui->customPlot2->graph()->setPen(graphPen2);

    ui->customPlot2->addGraph();
    ui->customPlot2->graph()->setName("Maximum");
    ui->customPlot2->graph()->setData(x, max);
    ui->customPlot2->graph()->setLineStyle((QCPGraph::LineStyle)(ui->Line_Style_8->value()));
    ui->customPlot2->graph()->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)(ui->Scatter_Style_8->value())));
    graphPen2.setColor(get_color(ui->color_sel_8->value()));
    graphPen2.setWidthF(ui->width_sel_8->value());
    ui->customPlot2->graph()->setPen(graphPen2);
    ui->customPlot2->replot();

    return;
}

void Link_Rate::set_graph1(quint8 fast_draw)
{
    // Determine data that shall be plottet to graph 1
    if (ui->Draw_diff->isChecked())
    {
        // Draw dispersion
        ui->g1_setup->setText("Dispersion");
        ui->g1_avg->setText("Dispersion average");
        ui->g1_min->setText("Dispersion minimum");
        ui->g1_max->setText("Dispersion maximum");
        draw_graph1(0);
    }
    if (ui->Draw_bitrate->isChecked())
    {
        // Draw Bitrate
        ui->g1_setup->setText("Bitrate");
        ui->g1_avg->setText("Bitrate average");
        ui->g1_min->setText("Bitrate minimum");
        ui->g1_max->setText("Bitrate maximum");
        draw_graph1(1);
    }
    if (ui->draw_Hist->isChecked() && fast_draw==1)
    {
        // Draw Histogram
        ui->g1_setup->setText("Bitrate Distribution");
        ui->g1_avg->setText("Not used");
        ui->g1_min->setText("Distribution minimum");
        ui->g1_max->setText("Distribution maximum");
        draw_graph1(2);
    }
}

void Link_Rate::set_graph2(quint8 fast_draw)
{
    // Determine data that shall be plottet to graph 1
    if (ui->Draw_diff_2->isChecked())
    {
        // Draw dispersion
        ui->g1_setup_2->setText("Dispersion");
        ui->g1_avg_2->setText("Dispersion average");
        ui->g1_min_2->setText("Dispersion minimum");
        ui->g1_max_2->setText("Dispersion maximum");
        draw_graph2(0);
    }
    if (ui->Draw_bitrate_2->isChecked())
    {
        // Draw Bitrate
        ui->g1_setup_2->setText("Bitrate");
        ui->g1_avg_2->setText("Bitrate average");
        ui->g1_min_2->setText("Bitrate minimum");
        ui->g1_max_2->setText("Bitrate maximum");
        draw_graph2(1);
    }
    if (ui->draw_Hist_2->isChecked() && fast_draw==1)
    {
        // Draw Histogram
        ui->g1_setup_2->setText("Bitrate Distribution");
        ui->g1_avg_2->setText("Not used");
        ui->g1_min_2->setText("Distribution minimum");
        ui->g1_max_2->setText("Distribution maximum");
        draw_graph2(2);
    }
}

void Link_Rate::on_Check_Server_IP_clicked()
{
    // If one socket was already setup disconnect current socket and delete it
    if (socket_setup)
    {
        Setup->disconnect_Client(this, SLOT(readPendingDatagrams()));
        Setup->delete_socket();
    }

    // Read IP and Port from user interface
    QString Server_IP=ui->Server_IP->text();
    QString Server_Port = ui->Server_Port->text();

    // Do IP Address validation (Not implemented yet)
    QHostAddress *sink  = new QHostAddress(Server_IP);
    quint16 Port = Server_Port.toInt();

    // Setup new variable of datatype socket with user Information
    Setup = new Socket(Port,*sink);

    socket_setup=Setup->setup_success();

    //Debug Network configuration
    Debug.AddText("Server IP Address: "+sink->toString()+"\n");
    Debug.AddText("Port Number: "+QString::number(Port)+"\n");

    // Chreate a new UDP socket as a child
    Setup->setup_socket(this);

    // Bind the socket if possible (return value true when successful)
    if(Setup->bind_Client(QHostAddress::Any))
        {
            // Connects udp_Client IO_Signal readyRead() to the private slot readPendingDatagrams() of link rate
            if(Setup->connect_Client(this, SLOT(readPendingDatagrams())))
            {
                Debug.AddText("\n ## UDP-Socket bind successful ##\n");
            }
            else
            {
                Debug.AddText("\n ## ERROR: UDP-Socket connect to event signal error ##\n");
            }
        }
        else
        {
            Debug.AddText("\n ## ERROR: UDP-Socket bind error ##\n");
        }

    // Test connection
    Setup->Setup_connection_test();

    // Give output values back
    ui->Console->setText(Debug.GetText());
    ui->Console->moveCursor(QTextCursor::End);
}


void Link_Rate::on_Test_data_clicked()
{
    // Check if system was setup
    if (socket_setup)
    {
        // Clear pkt loss count
        pkt_lost=0;

        //Get number of iterations that shall be executed from GUI
        M1_send_iter=ui->bottleneck_iteration->value();
        // Adjust progress bar
        ui->send_progress->setMaximum(M1_send_iter);
        ui->send_progress->setValue(0);

        // Test operation mode 1
        Setup->set_operation_mode(1);

        // Setup graph range
        Setup->set_graph_x_axis_range(M1_send_iter,1);
        // disconnect event handler routine from network signal
        Setup->disconnect_Client(this, SLOT(readPendingDatagrams()));

        Output.ClearText();
        ui->customPlot->clearGraphs();
        ui->customPlot2->clearGraphs();

        // Check if High performance mode is valid
        if (ui->HP_MODE_en->isChecked()==true)
        {
            // Perform (M1_send_iter amount of measurements, activate the output of the average data
            // when measurement is finised, no global variable shall be used, packet loss returned by function)
            Test_data_process(M1_send_iter, 2, false, &pkt_lost);

            // Provide data measured to the user by updating the GUI
            ui->send_progress->setValue(M1_send_iter);
            ui->recv_output->setText(Output.GetText());
            ui->recv_output->moveCursor(QTextCursor::End);
            ui->avg_set->setText(QString::number(Setup->graph_1->avg_rate[M1_send_iter-1]));
            ui->min_set->setText(QString::number(Setup->graph_1->min_bit_rate));
            ui->max_set->setText(QString::number(Setup->graph_1->max_bit_rate));
            ui->pkt_loss->setText(QString::number(pkt_lost));
            set_graph1(1);
            set_graph2(1);
        }
        else
        {
            // Get graph refresh rate from gui
            int graph_refresh_rate_1=ui->Graph_refresh_rate->value();
            int graph_refresh_rate_2=ui->Graph_refresh_rate_2->value();

            // Perform M1_send_iter measurements
            // (After one measurement the GUI is updated before the new sampling function is started
            for (int index=0; index<M1_send_iter; index++)
            {
                if (index<M1_send_iter-1)
                {
                    // Perform (1 measurement, no average value return, no global variable shall be used, packet loss returned by function)
                    Test_data_process(1, 1, false, &pkt_lost);

                    // Update output console
                    ui->send_progress->setValue(index+1);
                    ui->recv_output->setText(Output.GetText());
                    ui->recv_output->moveCursor(QTextCursor::End);

                    // When enough iterations were performed update display
                    if (index%graph_refresh_rate_1==0) set_graph1(0);
                    if (index%graph_refresh_rate_2==0) set_graph2(0);
                    ui->avg_set->setText(QString::number(Setup->graph_1->avg_rate[index]));
                    ui->min_set->setText(QString::number(Setup->graph_1->min_bit_rate));
                    ui->max_set->setText(QString::number(Setup->graph_1->max_bit_rate));
                    ui->pkt_loss->setText(QString::number(pkt_lost));
                }
                else
                {
                    // At last measurement activate output of average values
                    Test_data_process(1, 2, false, &pkt_lost);

                    // Update the GUI
                    ui->send_progress->setValue(index+1);
                    ui->recv_output->setText(Output.GetText());
                    ui->recv_output->moveCursor(QTextCursor::End);
                    ui->avg_set->setText(QString::number(Setup->graph_1->avg_rate[index]));
                    ui->min_set->setText(QString::number(Setup->graph_1->min_bit_rate));
                    ui->max_set->setText(QString::number(Setup->graph_1->max_bit_rate));
                    ui->pkt_loss->setText(QString::number(pkt_lost));
                    set_graph1(1);
                    set_graph2(1);
                }
            }
        }
        // Reconnect slot for event driven data handling
        Setup->connect_Client(this, SLOT(readPendingDatagrams()));


    }
    else
    {
        ui->recv_output->setText("Setup connection first!");
    }
}

void Link_Rate::Test_data_process(quint64 iterations, int output_en, bool use_global_var, quint64 *packets_lost)
{
    // Performe 2 packet time measurement
    quint64 pkt_loss = 0;
    quint64 os_exe_error=0;
    quint64 iter=iterations;
    quint64 total_iterations=iterations;
    QString result;
    QThread::currentThread()->setPriority(QThread::TimeCriticalPriority);

    // Perform the number of iterations specified by the user
    // If a packet loss is detected or the scheduler of the operating system caused an error
    // the measurement is repeated and the number of errors is incremented
    while (iter!=0)
    {
        // Repeat packet loss measurements and failed measurement due to os execution glitches
        result+=Setup->HPF_bottleneck_linkrate(iter,&pkt_loss, &os_exe_error,use_global_var);
        iter=pkt_loss+os_exe_error;
        total_iterations+=pkt_loss;
        *packets_lost+=pkt_loss;
    }    

    if (output_en>=1)
    {
        // Return the dispersion and linkrate only
        Output.AddText(result);
    }
    if (output_en>=2)
    {
        // Return Average calculation results
        Output.AddText("Average (MB/s): "+QString::number(Setup->graph_1->total_avg_rate,'f',3)+"\n");
        Output.AddText("Packets lost: "+QString::number(*packets_lost)+"\n");
        double pkt_loss_percent= (double)*packets_lost/total_iterations;
        Output.AddText("Packet loss rate: "+QString::number(pkt_loss_percent, 'f', 2)+"\n");
    }
    return;
}

/*###########################################################################################*/
// Event driven data receive function
/*###########################################################################################*/
void Link_Rate::readPendingDatagrams()
{
     while (Setup->pending_Client())
     {
         QString rts_success;
         // create local variables to store read values
         QByteArray *datagram = new QByteArray();
         datagram->resize(Setup->datasize_Client());
         QHostAddress *sender = new QHostAddress();
         quint16 *senderPort = new quint16(0);

         // store data
         int n_packets=Setup->read_Client(datagram, sender, senderPort);

         // process data
         if (0!=Setup->get_test_operation())
         {
             QString data = QString(datagram->data());
             // Data handling for test operations
             switch (Setup->get_test_operation())
             {
             // TEXT RECEIVE PROCESSING
             case 3:
                 if (!data.compare(STR_END))
                 {
                     ui->Received_text->setText(Out_Text.GetText());
                     ui->Received_text->moveCursor(QTextCursor::End);
                     ui->Text_pkt_nr->setText(QString::number(Setup->get_recv_pkt_cnt(0)));
                 }
                 else
                 {
                     Out_Text.AddText(datagram->data());
                 }
                 break;

             // MUSIC RECEIVE PROCESSING
             case 4:
                 if (!data.compare(STR_END))
                 {
                     // Get amount of packets received (dont forget to substract END)
                    quint64 l_pkt_cnt=(Setup->get_recv_pkt_cnt(1)-1);
                    // Number of Music packets received
                    quint64 l_max_Map=recv_data.size();
                    ui->Music_pkt_nr->setText(QString::number(l_max_Map));

                    // Check if all packets were received
                    // size of map equals number of highest packet?
                    if (l_max_Map==(max_index_map+1))
                    {
                        ui->Music_pkt_lost->setText("0");
                    }
                    else
                    {
                        ui->Music_pkt_lost->setText(QString::number((max_index_map+1)-l_max_Map));
                    }

                    // Check if corsstraffic influenced reception
                    // for example no packets not belonging to the same transmission were received
                    if (l_pkt_cnt==max_index_map+1)
                    {
                        // Reset Error output
                        ui->Music_cross_error->setText("NO");
                    }
                    else
                    {
                        // Give Error out
                        ui->Music_cross_error->setText("YES");
                    }

                    ui->Music_save->setDisabled(false);
                 }
                 else
                 {
                     slim_data_packet packet;

                     // Get Sequence Number of RTP packet
                     QByteArray index = datagram->mid(2,2);
                     // Calculate Packet value
                     quint64 pkt_nr=((quint8)index[0]<<8)|(quint8)index[1];
                     packet.index=pkt_nr;
                     if(pkt_nr > max_index_map)
                     {
                        max_index_map=pkt_nr;
                     }
                     // Remove RTP header
                     datagram->remove(0,12);
                     packet.data.insert(0,*datagram);
                     // Save packet into Map
                     recv_data.insert(pkt_nr, packet);
                     ui->Music_pkt_nr->setText(QString::number(recv_data.size()));
                 }
                 break;

             // VIDEO RECEIVE PROCESSING
             case 5:
                 // Enters function if end = true
                 if (!data.compare(STR_END))
                 {
                     // Get amount of packets received (dont forget to substract END)
                    quint64 l_pkt_cnt=(Setup->get_recv_pkt_cnt(1)-1);
                    // Number of Music packets received
                    quint64 l_max_Map=recv_data.size();
                    ui->Video_pkt_nr->setText(QString::number(l_max_Map));
                    missing_packets=(max_index_map+1)-l_max_Map;

                    // Check if all packets were received
                    // size of map equals number of highest packet?
                    if (l_max_Map==(max_index_map+1))
                    {
                        ui->Video_pkt_lost->setText("0");
                    }
                    else
                    {
                        ui->Video_pkt_lost->setText(QString::number(missing_packets));
                    }

                    //Search for words missing
                    for (int i=0; i<=max_index_map; i++)
                    {
                        // Data not existing in data set
                        if(!(retr_data.contains(i)))
                        {
                            QByteArray resend;
                            resend.append("R "+QString::number(i));
                            if(Setup->send_Client(resend.data(),resend.size()))
                            {
                                Debug.AddText("## Send: "+QString::fromLocal8Bit(resend.data(),resend.size())+" ##\n");
                                Debug.AddText("## Transmission successful ##\n");
                            }
                            else
                            {
                                Debug.AddText("## ERROR: Data could not be sent ##\n");
                            }
                        }
                    }

                    // WATCHDOG SETUP
                    // Start Timeout timer to ensure missing packets will be continiously polled
                    // Setup inteval timer
                    tmr_interval = new QTimer(this);
                    // connect interval timer to timer timeout event handle function
                    disconnect(tmr_interval, SIGNAL(timeout()), this, SLOT(handle_tmr_interrupt()));
                    connect(tmr_interval, SIGNAL(timeout()), this, SLOT(Recv_watchdog()));
                    tmr_interval->start(ui->Video_retrans_tmout->value());

                 }
                 // Reception of original sequence
                 // Enter when no missing packets were found!
                 else if (missing_packets==0)
                 {
                     slim_data_packet packet;

                     // Get Sequence Number of RTP packet
                     QByteArray index = datagram->mid(2,2);
                     // Calculate Packet sequence number
                     quint64 pkt_nr=((quint8)index[0]<<8)|(quint8)index[1];
                     packet.index=pkt_nr;
                     // Check if packet has highest sequence number yet
                     if(pkt_nr > max_index_map)
                     {
                        max_index_map=pkt_nr;
                     }
                     // Remove RTP header
                     datagram->remove(0,12);
                     packet.data.insert(0,*datagram);
                     //packet.data.resize(datagram->size());
                     // Save packet into Map
                     recv_data.insert(pkt_nr, packet);
                     retr_data.insert(pkt_nr, packet);
                     ui->Video_pkt_nr->setText(QString::number(recv_data.size()));
                 }
                 // Reception of retransmitted packets
                 // Enter during retransmission if the number of missing packets was already calculated
                 else
                 {
                     slim_data_packet packet;

                     // Get Sequence Number of RTP packet
                     QByteArray index = datagram->mid(2,2);
                     // Calculate Packet sequence number
                     quint64 pkt_nr=((quint8)index[0]<<8)|(quint8)index[1];
                     packet.index=pkt_nr;
                     // Remove RTP header
                     datagram->remove(0,12);
                     packet.data.insert(0,*datagram);
                     // Save packet into second memory
                     retr_data.insert(pkt_nr, packet);
                     // Give Progress out
                     ui->Video_pkt_retr->setText(QString::number(retr_data.size()-recv_data.size()));
                     // decrement the number of missing packets
                     missing_packets--;

                     // All packets received? => make data accesible
                     if(missing_packets==0)
                     {
                         //DISABLE WATCHDOG
                         disconnect(tmr_interval, SIGNAL(timeout()), this, SLOT(Recv_watchdog()));
                         delete tmr_interval;
                         ui->Video_save->setDisabled(false);
                         ui->Video_save_2->setDisabled(false);
                         ui->Video_Receive->setDisabled(false);
                     }
                 }

                 break;
             }

         }
         else
         {
             // process data and setup connection
             // Determine if wrap arround is active
             QString data = QString(datagram->data());

             // In Debug window show not more char than output_max_char
             if (data.size()>output_max_char)
             {
                data.resize(output_max_char);
             }

             // Return packet information
             QString info = ("Server: "+sender->toString()+
                            "; Port: "+QString("%1").arg(*senderPort)+
                            "; Packet size: "+QString("%1").arg(n_packets)+"\n"+
                            "Data: "+data+"\n");

             // Check if wrap arround is activated
             if(Setup->get_wrap_arround())
             {
                 // Return data to sender
                 if(Setup->send_Client(datagram->data(),datagram->size()))
                 {
                     rts_success = "Return to Sender success";
                     Setup->connection_alive(true);
                 }
                 else
                 {
                     rts_success = "## ERROR: Data could not be sent ##\n";
                     Setup->connection_alive(false);
                 }
             }
             else
             {
                 // Check if connection was set as active yet
                 if (Setup->connection_alive()==false)
                 {
                     // Verify returned data from server
                     if (Setup->verify_Recv_string(datagram->data()))
                     {
                        Setup->connection_alive(true);
                     }
                     else
                     {
                        Setup->connection_alive(false);
                     }
                 }
             }

             // Verify if connection is alive
             if(Setup->connection_alive())
             {
                 ui->connect_disp->setText("active");
             }
             else
             {
                ui->connect_disp->setText("unactive");
             }

             // Forware computed values to GUI
             Output.AddText(info);
             ui->recv_output->setText(Output.GetText());
             ui->recv_output->moveCursor(QTextCursor::End);
         }



    }

}

void Link_Rate::Recv_Timeout_setup()
{

}

void Link_Rate::Recv_Timeout_detected()
{


}

void Link_Rate::on_Server_IP_textEdited(const QString &arg1)
{

}

void Link_Rate::on_Send_clicked()
{
    if (socket_setup)
    {
        QByteArray datagram;
        datagram.append(ui->Data->text());

        // Developer Debug mode
        if (ui->Data->text()=="--d PerfTest")
        {
            // perform basic bottleneck test
            measure_bottleneck();
        }
        else
        {
            if (Setup->get_test_operation()!=0)
            {
                Output.ClearText();
            }
            Setup->set_operation_mode(0);

            if (Setup->setup_success()==1)
            {
                // Send data specified in textedit to the server
                if(Setup->send_Client(datagram.data(),datagram.size()))
                {
                    Debug.AddText("## Send: "+QString::fromLocal8Bit(datagram.data(),datagram.size())+" ##\n");
                    Debug.AddText("## Transmission successful ##\n");
                }
                else
                {
                    Debug.AddText("## ERROR: Data could not be sent ##\n");
                }
            }
            ui->Console->setText(Debug.GetText());
            ui->Console->moveCursor(QTextCursor::End);
        }
    }
}

// Set default package size to different value
void Link_Rate::on_checkBox_clicked()
{
}

void Link_Rate::on_wrap_arround_clicked()
{
    if(!socket_setup)
    {
        // Create socket if no socket was setup yet
        Setup = new Socket();
        socket_setup=Setup->setup_success();

        //Debug Network configuration
        Debug.AddText("Server_IP_Address:"+Setup->get_IP().toString()+"\n");
        Debug.AddText("Port_Number:"+QString::number(Setup->get_Port())+"\n");

        Setup->setup_socket(this);

        // Similar to function triggered by signal of button "Setup"
        if(Setup->bind_Client(QHostAddress::Any))
        {
            // Connects udp_Client IO_Signal readyRead() to the private slot readPendingDatagrams() of link rate
            if(Setup->connect_Client(this, SLOT(readPendingDatagrams())))
            {
                Debug.AddText("\n ## UDP-Socket bind successful ##\n");
            }
            else
            {
                Debug.AddText("\n ## ERROR: UDP-Socket connect to event signal error ##\n");
            }
        }
        else
        {
            Debug.AddText("\n ## ERROR: UDP-Socket bind error ##\n");
        }

        ui->Console->setText(Debug.GetText());
        ui->Console->moveCursor(QTextCursor::End);
    }

    // Set the wrap around flag as enable
    Setup->set_wrap_arround(ui->wrap_arround->isChecked());

}

void Link_Rate::on_udp_pkt_size_valueChanged(int arg1)
{
    if (!socket_setup)
    {
        ui->recv_output->setText("Please setup connection");
    }
    else
    {
        // Read data pkt size value from spinbox
        Setup->set_packet_size(arg1);
    }
}

void Link_Rate::on_bottleneck_iteration_valueChanged(int arg1)
{

}

// On "2 pkt bottleneck interval test" clicked
void Link_Rate::on_pushButton_clicked()
{
    if (socket_setup)
    {
        pkt_lost=0;
        Setup->set_operation_mode(2);

        // Get number of Samples that shall be taken
        tmr_iterations=ui->tmr_iterations->value();
        // Set limits of progress bar
        ui->send_progress->setMaximum(tmr_iterations);
        ui->send_progress->setValue(0);

        // Set differrent time base for graph
        Setup->set_graph_x_axis_range(tmr_iterations,ui->tmr_interval->value());
        // disconnect normal receiving slot
        Output.ClearText();
        ui->customPlot->clearGraphs();
        ui->customPlot2->clearGraphs();

        // Setup inteval timer
        tmr_interval = new QTimer(this);
        // connect interval timer to timer timeout event handle function
        connect(tmr_interval, SIGNAL(timeout()), this, SLOT(handle_tmr_interrupt()));

        // Verifiy if more than one sample shall be taken
        if (tmr_iterations>1)
        {
            // Yes: start timer, disconnect event driven receive function and perform first sample measurement
            tmr_iteration_cnt=0;
            tmr_interval->start(ui->tmr_interval->value());
            Setup->disconnect_Client(this, SLOT(readPendingDatagrams()));
            // Perform (1 measurement, deactivate the output of the average data
            // when measurement is finised, no global variable shall be used, packet loss returned by function)
            Test_data_process(1,1,false, &pkt_lost);
        }
        else
        {
            Setup->disconnect_Client(this, SLOT(readPendingDatagrams()));
            // Perform (1 measurement, activate the output of the average data
            // when measurement is finised, no global variable shall be used, packet loss returned by function)
            Test_data_process(1,2,false, &pkt_lost);

            // Give data out to GUI
            ui->recv_output->setText(Output.GetText());
            ui->recv_output->moveCursor(QTextCursor::End);
            Setup->connect_Client(this, SLOT(readPendingDatagrams()));
            ui->avg_set->setText(QString::number(Setup->graph_1->avg_rate[(tmr_iterations-1)]));
            ui->min_set->setText(QString::number(Setup->graph_1->min_bit_rate));
            ui->max_set->setText(QString::number(Setup->graph_1->max_bit_rate));
            ui->pkt_loss->setText(QString::number(pkt_lost));
            set_graph1(1);
            set_graph2(1);

            // reconnect event driven receive service routine
            Setup->connect_Client(this, SLOT(readPendingDatagrams()));
        }

    }
    else
    {
        ui->recv_output->setText("Setup connection first!");
    }


}

// Timer ISR -> Performs measurement when at timer overflow
void Link_Rate::handle_tmr_interrupt()
{
    // Increase iterations count
    tmr_iteration_cnt++;
    // Update progressbar
    ui->send_progress->setValue(tmr_iteration_cnt+1);
    // While not enough measurements were taken
    if ((tmr_iterations-1)==tmr_iteration_cnt)
    {
        // Deactivate timer that triggers event
        tmr_interval->stop();

        // Perform (1 measurement, activate the output of the average data
        // when measurement is finised, no global variable shall be used, packet loss returned by function)
        Test_data_process(1,2,false,&pkt_lost);

        // Update GUI
        ui->recv_output->setText(Output.GetText());
        ui->recv_output->moveCursor(QTextCursor::End);
        Setup->connect_Client(this, SLOT(readPendingDatagrams()));
        ui->avg_set->setText(QString::number(Setup->graph_1->avg_rate[tmr_iteration_cnt]));
        ui->min_set->setText(QString::number(Setup->graph_1->min_bit_rate));
        ui->max_set->setText(QString::number(Setup->graph_1->max_bit_rate));
        ui->pkt_loss->setText(QString::number(pkt_lost));
        set_graph1(1);
        set_graph2(1);
    }
    else
    {
        switch(Setup->get_test_operation())
        {

        // Perform (1 measurement, deactivate the output of the average data
        // when measurement is finised, no global variable shall be used, packet loss returned by function)
        case 2: Test_data_process(1,1,false,&pkt_lost);

            // Update GUI
            ui->recv_output->setText(Output.GetText());
            ui->recv_output->moveCursor(QTextCursor::End);
            // Reduce graph refreshment rate for slow systems
            if (tmr_iteration_cnt%(ui->Graph_refresh_rate->value())==0) set_graph1(0);
            if (tmr_iteration_cnt%(ui->Graph_refresh_rate_2->value())==0) set_graph2(0);
            ui->avg_set->setText(QString::number(Setup->graph_1->avg_rate[tmr_iteration_cnt]));
            ui->min_set->setText(QString::number(Setup->graph_1->min_bit_rate));
            ui->max_set->setText(QString::number(Setup->graph_1->max_bit_rate));
            ui->pkt_loss->setText(QString::number(pkt_lost));
            break;

        default : ;
        }
    }
    return;
}

// convert ms input into min,sec,ms format
void Link_Rate::convert_ms_to_sec(int in_ms, int *out_min, int *out_sec, int *out_msec)
{
    *out_msec=in_ms%1000;
    *out_sec=(in_ms/1000)%60;
    *out_min=(in_ms/1000)/60;
}

// Calculate the approximated processing time of the inteval measurement when number of iterations has changed
void Link_Rate::on_tmr_iterations_valueChanged(int arg1)
{
    int process_time_ms = (ui->tmr_iterations->value()*ui->tmr_interval->value());
    int min=0;
    int sec=0;
    int ms=0;
    convert_ms_to_sec(process_time_ms, &min, &sec, &ms);
    QString output = QString::number(min)+"."+QString::number(sec)+"."+QString::number(ms)+"  (min.sec.ms)";
    ui->Processing_time->setText(output);
}

// Calculate the approximated processing time of the inteval measurement when inteval has changed
void Link_Rate::on_tmr_interval_valueChanged(int arg1)
{
    int process_time_ms = (ui->tmr_iterations->value()*ui->tmr_interval->value());
    int min=0;
    int sec=0;
    int ms=0;
    convert_ms_to_sec(process_time_ms, &min, &sec, &ms);
    QString output = QString::number(min)+"."+QString::number(sec)+"."+QString::number(ms)+"  (min.sec.ms)";
    ui->Processing_time->setText(output);
}

void Link_Rate::on_pushButton_2_clicked()
{
}

// Primitive original test function to determine system configuration for best performance
// Can only be accessed through developer mode
void Link_Rate::measure_bottleneck()
{
    QByteArray datas[2];
    int t1_t;
    int t2_t;
    int diff=0;
    int read=0;
    QElapsedTimer t1;
    QElapsedTimer t2;
    QHostAddress *addr = new QHostAddress("129.187.223.200");
    int index=0;
    int error=0;
    int good=0;
    QByteArray *datagram1 = new QByteArray;
    QByteArray *datagram2 = new QByteArray;

    QByteArray *dummy=new QByteArray();
    dummy->resize(1500);

    double sum=0;
    double avg = 0;


    for (int i=0; i<2; i++)
    {
        QString nr=QString::number(i);
        datas[i].resize(800);
        datas[i].replace(0,nr.length(),nr.toUtf8());
    }

    QUdpSocket my_socket;
    my_socket.setSocketOption(QAbstractSocket::LowDelayOption,03);
    my_socket.bind(QHostAddress::Any,2000);

    while (index<100)
    {
        t1_t=0;
        t2_t=0;
        //index=0;
        t1.start();
        t2.start();
        my_socket.writeDatagram(datas[0],*addr,2000);
        my_socket.writeDatagram(datas[1],*addr,2000);

        index++;


        while (!my_socket.hasPendingDatagrams());

        t1_t=t1.nsecsElapsed();
        my_socket.readDatagram(datagram1->data(),datagram1->size());
        //read=my_socket.readDatagram(dummy->data(),dummy->size());

        //read=my_socket.bytesAvailable();
        if (my_socket.hasPendingDatagrams())
        {
            error++;
        }
        else
        {
            while (!my_socket.hasPendingDatagrams());

            t2_t=t2.nsecsElapsed();
            my_socket.readDatagram(datagram2->data(),datagram2->size());
            good++;
            diff=t2_t-t1_t;
            double result = ((double)800*8/((double)diff*qPow(10,-3)));
            //Setup->graph_1->insert_element_dispersion(diff,1,10);
            QString dif = QString::number(diff);
            QString res = QString::number(result,'f',4);

            sum+=result;
            Output.AddText(dif + "   MBPS: "+res+"\n");
            //ui->recv_output->setText(Output.GetText());

        }



    }
    avg=(double)sum/100;
    QString qavg = QString::number(avg, 'f',4);
    Output.AddText("AVG:"+qavg+"\n");
    ui->recv_output->setText(Output.GetText());

}

/* ############################################################################################*/
// TAB 3
/* ############################################################################################*/
/* GET TEXT FROM SERVER */
void Link_Rate::on_Text_Receive_clicked()
{
    if (socket_setup)
    {
        QByteArray datagram;
        datagram.append(ui->Text_Command->text());

        Setup->set_operation_mode(3);
        Out_Text.ClearText();

        if (Setup->setup_success()==1)
        {
            Setup->get_recv_pkt_cnt(1);
            // Send data specified in textedit to the server
            if(Setup->send_Client(datagram.data(),datagram.size()))
            {
                Debug.AddText("## Send: "+QString::fromLocal8Bit(datagram.data(),datagram.size())+" ##\n");
                Debug.AddText("## Transmission successful ##\n");
            }
            else
            {
                Debug.AddText("## ERROR: Data could not be sent ##\n");
            }
        }

        ui->Console->setText(Debug.GetText());
        ui->Console->moveCursor(QTextCursor::End);
        ui->Received_text->setText(Out_Text.GetText());
        ui->Received_text->moveCursor(QTextCursor::End);
    }
    else
    {
        Out_Text.AddText("Please setup socket at tab LINK RATE MEASUREMENT \n");
        ui->Received_text->setText(Out_Text.GetText());
        ui->Received_text->moveCursor(QTextCursor::End);
    }
}

void Link_Rate::on_Text_save_clicked()
{
    QString path = QFileDialog::getSaveFileName(this,
                                                tr("Save File"),
                                                "/home/Text.txt",
                                                tr("*.txt"));
    QFile Result(path);
        if (Result.open( QIODevice::ReadWrite | QIODevice::Text))
        {
            Result.write(Out_Text.GetText().toUtf8());
        }
        else
        {
            Out_Text.AddText("ERROR! Unable to save data! \n");
            ui->Received_text->setText(Out_Text.GetText());
            ui->Received_text->moveCursor(QTextCursor::End);
        }
}

/* GET MP3 FROM SERVER */
void Link_Rate::on_MP3_Receive_clicked()
{
    // Clear MAP
    recv_data.clear();
    max_index_map=0;
    ui->Music_save->setDisabled(true);

    if (socket_setup)
    {
        QByteArray datagram;
        datagram.append(ui->MP3_Command->text());

        // Operation mode 4: Receive MP3 File
        Setup->set_operation_mode(4);

        if (Setup->setup_success()==1)
        {
            Setup->get_recv_pkt_cnt(1);
            // Send data specified in textedit to the server
            if(Setup->send_Client(datagram.data(),datagram.size()))
            {
                Debug.AddText("## Send: "+QString::fromLocal8Bit(datagram.data(),datagram.size())+" ##\n");
                Debug.AddText("## Transmission successful ##\n");
            }
            else
            {
                Debug.AddText("## ERROR: Data could not be sent ##\n");
            }
        }

        ui->Console->setText(Debug.GetText());
        ui->Console->moveCursor(QTextCursor::End);
    }
    else
    {
        Out_Text.AddText("Please setup socket at tab LINK RATE MEASUREMENT \n");
        ui->Received_text->setText(Out_Text.GetText());
        ui->Received_text->moveCursor(QTextCursor::End);
    }

}

/* SAVE MUSIC FILE*/
void Link_Rate::on_Music_save_clicked()
{
    int error=0;

    QElapsedTimer access_timeout;
    // Open file dialog and setup path to save data
    QString path = QFileDialog::getSaveFileName(this,
                                                tr("Save File"),
                                                "/home/MusicFile.mp3",
                                                tr("*.mp3"));
    QFile Result(path);
    // Start open file timeout timer
    access_timeout.start();
    // Open file
    Result.open(QIODevice::WriteOnly);

    // wait until file was opened or timeout occured
    while (!Result.isOpen() && !access_timeout.hasExpired(500));

    // Timeout occured -> unable to open file
    if(access_timeout.hasExpired(500))
    {
        ui->Music_Save_suc->setText("ERROR");
    }
    else
    {
        // Go trough data map and copy all existing packets in the correct order into the
        // output file. Before copying check if packets exist or were lost(not received);
        for (int i=0; i<=max_index_map; i++)
        {
            if (recv_data.contains(i))
            {
                if (Result.write(recv_data[i].data)==-1)
                {
                    error++;
                }
            }
        }

        if(ui->Music_save_logfile->isChecked())
        {
            // Create output LOG text file
            quint64 dot_index=path.lastIndexOf(".");
            QString log_txt = path;
            // Take same data path and replace data name ending by _warn.txt
            log_txt = path.replace(dot_index,4,"_warn.txt");

            // Construct output string
            // Read values from textfields as output information to avoid more global vars
            // Not performance critical application part
            QString pkt_lost=ui->Music_pkt_lost->text();
            QString pkt_nr=ui->Music_pkt_nr->text();
            quint64 i_pkt_lost=pkt_lost.toUInt();
            quint64 i_pkt_nr=pkt_nr.toUInt();
            // output string
            QString ret = "TRANSMISSION REPORT \r\n";
            if (i_pkt_lost!=0)
            {
                ret =ret+"Transmission: ERROR \r\n";
            }
            else
            {
                ret =ret + "Transmission: SUCCESS \r\n";
            }
            // Read values from textfields as output information to avoid more global vars
            // Not performance critical application part
            int seq=i_pkt_lost+i_pkt_nr;
            ret = ret +"Expected sequences: "+(QString::number(seq))+"\r\n";
            ret = ret +"Received sequences: "+(pkt_nr)+"\r\n";
            ret = ret +"Lost sequences: "+(pkt_lost)+"\r\n";

            // Create new output file variable
            QFile Log(log_txt);
            // Start open file timeout timer
            access_timeout.start();
            // Open file
            Log.open(QIODevice::ReadWrite | QIODevice::Text);
            // wait until file was opened or timeout occured
            while (!Log.isOpen() && !access_timeout.hasExpired(500));

            if(access_timeout.hasExpired(500))
            {
                ui->Music_Save_suc->setText("ERROR");
            }
            else
            {
                // Write data into log file
                Log.write(ret.toUtf8());
            }
        }

        if(error==0)
        {
            ui->Music_Save_suc->setText("Success");
        }
        else
        {
            ui->Music_Save_suc->setText("ERROR");
        }
    }
}
/* GET VIDEO FROM SERVER */
void Link_Rate::on_Video_Receive_clicked()
{
    // Clear MAP
    recv_data.clear();
    retr_data.clear();
    max_index_map=0;
    missing_packets=0;

    ui->Video_save->setDisabled(true);
    ui->Video_save_2->setDisabled(true);
    ui->Video_Receive->setDisabled(true);

    if (socket_setup)
    {
        QByteArray datagram;
        datagram.append(ui->Video_Command->text());
        ui->Video_pkt_lost->setText("0");
        ui->Video_pkt_retr->setText("0");
        ui->Video_pkt_nr->setText("0");

        // Operation mode 4: Receive MP3 File
        Setup->set_operation_mode(5);

        if (Setup->setup_success()==1)
        {
            Setup->get_recv_pkt_cnt(1);
            // Send data specified in textedit to the server
            if(Setup->send_Client(datagram.data(),datagram.size()))
            {
                Debug.AddText("## Send: "+QString::fromLocal8Bit(datagram.data(),datagram.size())+" ##\n");
                Debug.AddText("## Transmission successful ##\n");
            }
            else
            {
                Debug.AddText("## ERROR: Data could not be sent ##\n");
            }
        }

        // Sequential non event driven data acquizition sequentially
        // Sequential operation is very similar to event driven operation
        // and can be used to get a basic understanding
        // WARNING: Sequential operation is missing retransmitting part
        /*Setup->disconnect_Client(this, SLOT(readPendingDatagrams()));

        int exit=0;
        while (exit==0)
        {
            // Check if packets are waiting to be read
            while (Setup->pending_Client())
            {
                QString rts_success;
                // create local variables to store read values
                QByteArray *dat = new QByteArray();
                dat->resize(Setup->datasize_Client());
                QHostAddress *sender = new QHostAddress();
                quint16 *senderPort = new quint16(0);

                Setup->read_Client(dat, sender, senderPort);

                QString data = QString(dat->data());
                if(!data.compare(STR_END))
                {
                    exit=1;
                }
                else
                {
                    slim_data_packet packet;

                    // Get Sequence Number of RTP packet
                    QByteArray index = dat->mid(2,2);
                    // Calculate Packet sequence number
                    quint64 pkt_nr=((quint8)index[0]<<8)|(quint8)index[1];
                    packet.index=pkt_nr;
                    // Check if packet has highest sequence number yet
                    if(pkt_nr > max_index_map)
                    {
                       max_index_map=pkt_nr;
                    }
                    // Remove RTP header
                    dat->remove(0,12);
                    packet.data.insert(0,*dat);
                    //packet.data.resize(datagram->size());
                    // Save packet into Map
                    recv_data.insert(pkt_nr, packet);
                }

            }
        }

        ui->Video_save->setDisabled(false);
        ui->Video_save_2->setDisabled(false);
        */

        ui->Console->setText(Debug.GetText());
        ui->Console->moveCursor(QTextCursor::End);
    }
    else
    {
        Out_Text.AddText("Please setup socket at tab LINK RATE MEASUREMENT \n");
        ui->Received_text->setText(Out_Text.GetText());
        ui->Received_text->moveCursor(QTextCursor::End);
    }

}

// ISR for Retransmit timeout
void Link_Rate::Recv_watchdog()
{
    //Search for words missing
    for (int i=0; i<=max_index_map; i++)
    {
        // Data not existing in data set
        if(!(retr_data.contains(i)))
        {
            QByteArray resend;
            // Prepare String of data word to by retransmitted for server
            resend.append("R "+QString::number(i));
            // request resending of missing data words
            if(Setup->send_Client(resend.data(),resend.size()))
            {
                Debug.AddText("## Send: "+QString::fromLocal8Bit(resend.data(),resend.size())+" ##\n");
                Debug.AddText("## Transmission successful ##\n");
            }
            else
            {
                Debug.AddText("## ERROR: Data could not be sent ##\n");
            }
        }
    }
}

/* SAVE VIDEO without retransmission*/
void Link_Rate::on_Video_save_clicked()
{
    int error=0;
    QElapsedTimer access_timeout;
    QString path = QFileDialog::getSaveFileName(this,
                                                tr("Save File"),
                                                "/home/lost_news.264",
                                                tr("*.264"));
    QFile Result(path);
    access_timeout.start();
    Result.open(QIODevice::WriteOnly);

    while (!Result.isOpen() && !access_timeout.hasExpired(500));

    if(access_timeout.hasExpired(500))
    {
        ui->Video_Save_suc->setText("ERROR");
    }
    else
    {
        for (int i=0; i<=(max_index_map); i++)
        {
            if (recv_data.contains(i))
            {
                if (Result.write(recv_data[i].data)==-1)
                {
                    error++;
                }
            }
        }

        if(ui->Video_save_logfile->isChecked())
        {
            // Create output LOG text file
            quint64 dot_index=path.lastIndexOf(".");
            QString log_txt = path;
            // Take same data path and replace data name ending by _warn.txt
            log_txt = path.replace(dot_index,4,"_warn.txt");

            // Construct output string
            // Read values from textfields as output information to avoid more global vars
            // Not performance critical application part
            QString pkt_lost=ui->Video_pkt_lost->text();
            QString pkt_nr=ui->Video_pkt_nr->text();
            QString pkt_retr=ui->Video_pkt_retr->text();
            quint64 i_pkt_lost=pkt_lost.toUInt();
            quint64 i_pkt_nr=pkt_nr.toUInt();
            // output string
            QString ret = "TRANSMISSION REPORT \r\n";
            if (i_pkt_lost!=0)
            {
                ret =ret+"Transmission: ERROR \r\n";
            }
            else
            {
                ret =ret + "Transmission: SUCCESS \r\n";
            }
            ret = ret+"\n"+"RETRANSMISSION OF LOST SEQUENCES APPLIED: NO \r\n\n";
            // Read values from textfields as output information to avoid more global vars
            // Not performance critical application part
            int seq=i_pkt_lost+i_pkt_nr;
            ret = ret +"Expected sequences: "+(QString::number(seq))+"\r\n";
            ret = ret +"Received sequences: "+(pkt_nr)+"\r\n";
            ret = ret +"Lost sequences: "+(pkt_lost)+"\r\n";

            // Create new output file variable
            QFile Log(log_txt);
            // Start open file timeout timer
            access_timeout.start();
            // Open file
            Log.open(QIODevice::ReadWrite | QIODevice::Text);
            // wait until file was opened or timeout occured
            while (!Log.isOpen() && !access_timeout.hasExpired(500));

            if(access_timeout.hasExpired(500))
            {
                ui->Video_Save_suc->setText("ERROR");
            }
            else
            {
                // Write data into log file
                Log.write(ret.toUtf8());
            }
        }

        if(error==0)
        {
            ui->Video_Save_suc->setText("Success");
        }
        else
        {
            ui->Video_Save_suc->setText("ERROR");
        }
    }
}

void Link_Rate::on_Video_save_2_clicked()
{
    int error=0;
    QElapsedTimer access_timeout;
    QString path = QFileDialog::getSaveFileName(this,
                                                tr("Save File"),
                                                "/home/news.264",
                                                tr("*.264"));
    QFile Result(path);
    access_timeout.start();
    Result.open(QIODevice::WriteOnly);

    while (!Result.isOpen() && !access_timeout.hasExpired(500));

    if(access_timeout.hasExpired(500))
    {
        ui->Video_Save_suc_2->setText("ERROR");
    }
    else
    {
        for (int i=0; i<=max_index_map; i++)
        {
            if (retr_data.contains(i))
            {
                if (Result.write(retr_data[i].data)==-1)
                {
                    error++;
                }
            }
        }

        if(ui->Video_save_logfile->isChecked())
        {
            // Create output LOG text file
            quint64 dot_index=path.lastIndexOf(".");
            QString log_txt = path;
            // Take same data path and replace data name ending by _warn.txt
            log_txt = path.replace(dot_index,4,"_warn.txt");

            // Construct output string
            // Read values from textfields as output information to avoid more global vars
            // Not performance critical application part
            QString pkt_lost=ui->Video_pkt_lost->text();
            QString pkt_nr=ui->Video_pkt_nr->text();
            QString pkt_retr=ui->Video_pkt_retr->text();
            quint64 i_pkt_lost=pkt_lost.toUInt();
            quint64 i_pkt_nr=pkt_nr.toUInt();
            // output string
            QString ret = "TRANSMISSION REPORT \r\n";
            if (i_pkt_lost!=0)
            {
                ret =ret+"Transmission: ERROR \r\n";
            }
            else
            {
                ret =ret + "Transmission: SUCCESS \r\n";
            }
            ret = ret+"\n"+"RETRANSMISSION OF LOST SEQUENCES APPLIED: YES \r\n\n";
            // Read values from textfields as output information to avoid more global vars
            // Not performance critical application part
            int seq=i_pkt_lost+i_pkt_nr;
            ret = ret +"Expected sequences: "+(QString::number(seq))+"\r\n";
            ret = ret +"Received sequences: "+(pkt_nr)+"\r\n";
            ret = ret +"Lost sequences: "+(pkt_lost)+"\r\n";
            ret = ret +"Retransmitted sequences: "+(pkt_retr)+"\r\n";

            // Create new output file variable
            QFile Log(log_txt);
            // Start open file timeout timer
            access_timeout.start();
            // Open file
            Log.open(QIODevice::ReadWrite | QIODevice::Text);
            // wait until file was opened or timeout occured
            while (!Log.isOpen() && !access_timeout.hasExpired(500));

            if(access_timeout.hasExpired(500))
            {
                ui->Video_Save_suc_2->setText("ERROR");
            }
            else
            {
                // Write data into log file
                Log.write(ret.toUtf8());
            }
        }

        if(error==0)
        {
            ui->Video_Save_suc_2->setText("Success");
        }
        else
        {
            ui->Video_Save_suc_2->setText("ERROR");
        }
    }
}


// If any changes are applied by the user at the graph setup tab, actualize the graphs
void Link_Rate::update_graphs()
{
    if (socket_setup)
    {
        set_graph1(1);
        set_graph2(1);
    }
}

void Link_Rate::on_Draw_bitrate_clicked()
{
    update_graphs();
    return;
}

void Link_Rate::on_draw_Hist_clicked()
{
    update_graphs();
    return;
}

void Link_Rate::on_Draw_diff_clicked()
{
    update_graphs();
    return;
}

void Link_Rate::on_Line_Style_valueChanged(int arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_Scatter_Style_valueChanged(int arg1)
{
    update_graphs();
    return;
}


void Link_Rate::on_color_sel_valueChanged(int arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_width_sel_valueChanged(double arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_Line_Style_2_valueChanged(int arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_Scatter_Style_2_valueChanged(int arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_color_sel_2_valueChanged(int arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_width_sel_2_valueChanged(double arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_Line_Style_3_valueChanged(int arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_Scatter_Style_3_valueChanged(int arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_color_sel_3_valueChanged(int arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_width_sel_3_valueChanged(double arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_Line_Style_4_valueChanged(int arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_Scatter_Style_4_valueChanged(int arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_color_sel_4_valueChanged(int arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_width_sel_4_valueChanged(double arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_Line_Style_5_valueChanged(int arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_Scatter_Style_5_valueChanged(int arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_color_sel_5_valueChanged(int arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_width_sel_5_valueChanged(double arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_Line_Style_6_valueChanged(int arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_Scatter_Style_6_valueChanged(int arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_color_sel_6_valueChanged(int arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_width_sel_6_valueChanged(double arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_Line_Style_7_valueChanged(int arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_Scatter_Style_7_valueChanged(int arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_color_sel_7_valueChanged(int arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_width_sel_7_valueChanged(double arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_Line_Style_8_valueChanged(int arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_Scatter_Style_8_valueChanged(int arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_color_sel_8_valueChanged(int arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_width_sel_8_valueChanged(double arg1)
{
    update_graphs();
    return;
}



void Link_Rate::on_Draw_bitrate_2_clicked()
{
    update_graphs();
    return;
}

void Link_Rate::on_draw_Hist_2_clicked()
{
    update_graphs();
    return;
}

void Link_Rate::on_Draw_diff_2_clicked()
{
    update_graphs();
    return;
}

void Link_Rate::on_g1_histo_res_valueChanged(int arg1)
{
    update_graphs();
    return;
}

void Link_Rate::on_g2_histo_res_valueChanged(int arg1)
{
    update_graphs();
    return;
}


void Link_Rate::on_Music_Command_editingFinished()
{

}
