#ifndef DEBUG_WINDOW_H
#define DEBUG_WINDOW_H

#include <QString>


class Debug_Window
{
public:
    Debug_Window();
    QString GetText();
    QString AddText(QString insert);
    QString ClearText();

private:
    QString* Text;
};

#endif // DEBUG_WINDOW_H
