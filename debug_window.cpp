#include "debug_window.h"

Debug_Window::Debug_Window()
{
    Text=new QString();
}

QString Debug_Window::GetText()
{
    return *Text;
}

QString Debug_Window::AddText(QString insert)
{
    *Text=*Text+insert;

    return *Text;
}

QString Debug_Window::ClearText()
{
    *Text="";

    return *Text;
}
