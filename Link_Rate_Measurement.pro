#-------------------------------------------------
#
# Project created by QtCreator 2014-04-27T20:31:15
#
#-------------------------------------------------

QT       += core gui
QT       += widgets
QT       += network
QT       += printsupport

TARGET = Link_Rate_Measurement
TEMPLATE = app

SOURCES += main.cpp\
        link_rate.cpp \
    qcustomplot.cpp \
    socket.cpp \
    debug_window.cpp \
    counter.cpp \
    time_struct.cpp \
    validate_input.cpp \
    data_packet.cpp

HEADERS  += link_rate.h \
    qcustomplot.h \
    socket.h \
    debug_window.h \
    counter.h \
    time_struct.h \
    validate_input.h \
    data_packet.h

FORMS    += link_rate.ui


