#-------------------------------------------------
#
# Project created by QtCreator 2011-06-07T09:09:19
#
#-------------------------------------------------

QT       += core gui
QT       += network

TARGET = qmodbus
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    win_qextserialport.cpp \
    qextserialbase.cpp \
    qextserialport.cpp \
    aboutdialog.cpp \
    qiplineedit.cpp

HEADERS  += mainwindow.h \
    qextserialbase.h \
    win_qextserialport.h \
    qextserialport.h \
    aboutdialog.h \
    qiplineedit.h

FORMS    += \
    mainwindow.ui \
    aboutdialog.ui

RESOURCES += \
    src.qrc

RC_FILE += myico.rc

OTHER_FILES += \
    myico.rc

















