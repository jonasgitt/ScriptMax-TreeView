#ifndef RUNOPTIONS_H
#define RUNOPTIONS_H

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

QString writeSamples(QList<NRSample> samples);
QString writeRun(runstruct &runvars, bool runSM);
QString writeContrast(runstruct runvars, bool wait);
QString writeJulabo(runstruct runvars, int runCont);
QString writeEuro(runstruct runvars);
QString writeNIMA(runstruct runvars, bool PorA);
QString writeTransm(runstruct runvars);


#endif // RUNOPTIONS_H

