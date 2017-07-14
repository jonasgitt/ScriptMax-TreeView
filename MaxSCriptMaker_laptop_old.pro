#-------------------------------------------------
#
# Project created by QtCreator 2014-12-23T12:08:47
#
#-------------------------------------------------

QT       += core gui
QT       += core network
QT       += widgets



#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MaxSCriptMaker_laptop
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
       sampleform.cpp \
    sampletable.cpp \
    GCLHighLighter.cpp \
    translator.cpp \
    runoptions.cpp

HEADERS  += mainwindow.h \
    sampleform.h \
    sampletable.h \
    GCLHighLighter.h \
    translator.h \
    runoptions.h

FORMS    += mainwindow.ui \
    sampleform.ui \
    sampletable.ui

RESOURCES += \
    ScriptMax_resource.qrc





symbian: LIBS += -lgeniedll
else:unix|win32: LIBS += -L$$PWD/ -lgeniedll

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

DISTFILES +=
