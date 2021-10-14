#include <QtWidgets/QApplication>
#include "link_rate.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // Set thread priority to time critical to improve system performance
    QThread::currentThread()->setPriority(QThread::TimeCriticalPriority);
    Link_Rate w;
    w.show();
    
    return a.exec();
}
