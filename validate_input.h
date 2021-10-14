#ifndef VALIDATE_INPUT_H
#define VALIDATE_INPUT_H

#include <QString>
#include <QStringList>


class Validate_Input
{
public:
    Validate_Input();

    QString IP_validate(QString IP);

private:
    QString point;
    quint8 max_length;
};

#endif // VALIDATE_INPUT_H
