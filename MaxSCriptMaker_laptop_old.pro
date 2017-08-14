#-------------------------------------------------
#
# Project created by QtCreator 2014-12-23T12:08:47
#
#-------------------------------------------------

QT       += core gui
QT       += core network
QT       += widgets
QT       += core


#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MaxSCriptMaker_laptop
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    sampletable.cpp \
    GCLHighLighter.cpp \
    translator.cpp \
    ScriptLines.cpp \
    pyhighlighter.cpp \
    tree_item.cpp \
    tree_model.cpp

HEADERS  += mainwindow.h \
    sampletable.h \
    GCLHighLighter.h \
    translator.h \
    ScriptLines.h \
    pyhighlighter.h \
    tree_item.h \
    tree_model.h

FORMS    += mainwindow.ui \
    sampletable.ui

RESOURCES += \
    ScriptMax_resource.qrc





symbian: LIBS += -lgeniedll
else:unix|win32: LIBS += -L$$PWD/ -lgeniedll

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

DISTFILES += \
    SampTable.ui.qml \
    SampTable.qml \
    KkkForm.ui.qml \
    Kkk.qml
