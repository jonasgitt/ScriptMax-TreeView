#ifndef SCRIPTLINES_H
#define SCRIPTLINES_H

#include "sampleform.h"
#include "sampletable.h"
#include "ui_sampletable.h"
#include "mainwindow.h"
#include "GCLHighLighter.h"
#include <QMainWindow>
#include <QComboBox>
#include <QSignalMapper>
#include <QTime>

class runstruct{

public:
    double angles[3];
    double uAmps[3];
    QString subtitle;
    QString sampNum;
    QString sampName;

    double concs[3];
    double flow, volume;
    int knauer;

    double JTemp, JMin, JMax;

    double area, pressure;

    double euroTemps[9];

    double heightOffsT, uAmpsT;

};


runstruct parseRun(QVector<QVariant>variables);
runstruct parseContrast(QVector<QVariant>variables);
runstruct parseTransm(QVector<QVariant>variables);
runstruct parseNIMA_A(QVector<QVariant>variables);
runstruct parseNIMA_P(QVector<QVariant>variables);
runstruct parseJulabo(QVector<QVariant>variables);
runstruct parseEurotherm(QVector<QVariant>variables);

//OpenGenie Strings
QString writeSamples(QList<NRSample> samples);
QString writeRun(runstruct &runvars, bool runSM, bool Python);
QString writeContrast(runstruct runvars, bool wait, bool Python);
QString writeJulabo(runstruct runvars, int runCont);
QString writeEuro(runstruct runvars);
QString writeNIMA(runstruct runvars, bool Pressure, bool Python);
QString writeTransm(runstruct runvars, bool Python);

//Python Strings
QString PyContrast(runstruct runvars, bool wait);

const bool PYTHON = true;
const bool OPENGENIE = false;

#endif // SCRIPTLINES_H

