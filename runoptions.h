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
/*


    //outsourced cases
    void normalRun(int row, SampleForm *mySampleForm);
    void SMRun(int row);
    void kineticRun(int row);
    void OGcommand(int row);
    void contrastChange(int row);
    void setTemp(int row);
    void setNIMA(int row);
    void runTrans(int row);
*/

class runstruct{

public:
    double angles[3];
    //double ang1, ang2, ang3;
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

};

QString writeRun(runstruct runvars, bool runSM);
QString writeContrast(runstruct runvars, bool wait);
QString writeJulabo(runstruct runvars, int runCont);
QString writeEuro(runstruct runvars);
QString writeNIMA(runstruct runvars, bool PorA);

#endif // RUNOPTIONS_H

