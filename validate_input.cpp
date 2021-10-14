#include "validate_input.h"

Validate_Input::Validate_Input()
{
    point=".";
    max_length=15;
}

QString Validate_Input::IP_validate(QString IP)
{
    // String processing still needs to be done!


    QString ret_string="";

    if(IP.contains(point))
    {
        IP.resize(max_length);
        // Devide the String into subsegements of 3 numbers each
        QStringList segments = IP.split(point);

        //for (segments.count()





    }
    else
    {
        return ret_string;
    }

}
