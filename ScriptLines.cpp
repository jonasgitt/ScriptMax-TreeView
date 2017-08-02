#include "sampleform.h"
#include "sampletable.h"
#include "ui_sampletable.h"
#include "GCLHighLighter.h"
#include <QMainWindow>
#include <QComboBox>
#include <QSignalMapper>
#include <QTime>
#include "mainwindow.h"
#include "ScriptLines.h"
#include <QApplication>

#include <QDebug>
QString writeSamples(QList<NRSample> samples){

    QString scriptLine;
     for (int i=0; i < samples.length(); i++){
        scriptLine += "s" + QString::number(i+1) + "=fields(); ";
     }

     scriptLine += "\n#====================================\n";

     for (int i=0; i < samples.length(); i++){

        //scriptLine += ("            s" + QString::number(i+1) + " = create(\"NR_sample\")\n"); //sample numbering starts at 1

        QString temp = samples[i].title;
        scriptLine += ("      s" + QString::number(i+1) + ".title = \""+temp+"\"\n");

        temp = QString::number(samples[i].translation);
        scriptLine += ("s" + QString::number(i+1) + ".translation = "+temp+"\n");

        temp = QString::number(samples[i].height);
        scriptLine += ("     s" + QString::number(i+1) + ".height = "+temp+"\n");

        temp = samples[i].phi_offset;
        scriptLine += (" s" + QString::number(i+1) + ".phi_offset = "+temp+"\n");

        temp = QString::number(samples[i].footprint);
        scriptLine += ("  s" + QString::number(i+1) + ".footprint = "+temp+"\n");

        temp = QString::number(samples[i].resolution);
        scriptLine += (" s" + QString::number(i+1) + ".resolution = "+temp+"\n");

        temp = QString::number(samples[i].s3);
        scriptLine += ("         s" + QString::number(i+1) + ".s3 = "+temp+"\n");

        temp = QString::number(samples[i].s4);
        scriptLine += ("         s" + QString::number(i+1) + ".s4 = "+temp+"\n");

        temp = QString::number(samples[i].knauer);
        scriptLine += ("     s" + QString::number(i+1) + ".knauer = "+temp+"\n");

        scriptLine += "#====================================\n";

    }

    scriptLine += "sample=dimensions(" + QString::number(samples.length()) + ")\n";

    for (int i=0; i < samples.length(); i++){
       scriptLine += "sample[" + QString::number(i+1) + "]=s" + QString::number(i+1) + "; ";
    }

    return scriptLine;

}


QString writeRun(runstruct &runvars, bool runSM, bool Python){

    QString scriptLine;

    if (runSM && !Python){
        scriptLine += "# " + runvars.sampName + ":\n";
        scriptLine += "s" + runvars.sampNum + ".subtitle = \"" + runvars.subtitle + "\"" + "\n";
    }

    for (int i = 0; i < 3; i++){
        if (runSM){
            if (!Python) scriptLine += "runTime = runAngle_SM(s";
            else if (Python) scriptLine += "\trunAngle_SM(sample[";
        }
        else {
            if (!Python) scriptLine += "runTime = runAngle(s";
            else if (Python) scriptLine += "\trunAngle(sample[";
        }
        scriptLine += runvars.sampNum;
        if (Python) scriptLine += "]";
        scriptLine += "," + QString::number(runvars.angles[i]);
        scriptLine += "," + QString::number(runvars.uAmps[i]) + ")" + "\n";
    }

    return scriptLine;
}

QString writeContrast(runstruct runvars, bool wait, bool Python){

    QString scriptLine;

    if (!Python) scriptLine  = "runTime = ";
    else scriptLine = "\t";

    if (wait) scriptLine += "contrastChange:wait(";
    else scriptLine += "contrastChange(";

    scriptLine += QString::number(runvars.knauer) + ", ";

    for (int i = 0; i < 4; i++){
        scriptLine += QString::number(runvars.concs[i]);
        scriptLine += ", ";
    }

    scriptLine += QString::number(runvars.flow);
    scriptLine += ", " + QString::number(runvars.volume) + ")";

    return scriptLine;

}

QString writeJulabo(runstruct runvars, int runcont){
    QString scriptLine;

     if(runcont){
         scriptLine = "CSET /control Julabo_WB=" + QString::number(runvars.JTemp);
         scriptLine += " lowlimit=" + QString::number(runvars.JMin);
         scriptLine += " highlimit=" + QString::number(runvars.JMax);
     }

     else scriptLine = "CSET /nocontrol Julabo_WB=" + QString::number(runvars.JTemp);

     return scriptLine;
}

QString writeEuro(runstruct runvars){
    QString scriptLine;
    scriptLine = "CSET";

    for(int i=0; i<9; i++){
        scriptLine += " temp" + QString::number(i) + "=" + QString::number(runvars.euroTemps[i]);
    }
    return scriptLine;
}

QString writeNIMA(runstruct runvars, bool Pressure, bool Python){

    QString scriptLine;

    if (Pressure)
        if (!Python) scriptLine = "CSET target_pressure = " + QString::number(runvars.pressure);
        else scriptLine = "\tgoToPressure(" + QString::number(runvars.pressure) + ")";
    else
        if (!Python) scriptLine = "CSET target_area = " + QString::number(runvars.area);
        else scriptLine = "\tgoToArea(" + QString::number(runvars.area) + ")";

    return scriptLine;
}

QString writeTransm(runstruct runvars, bool Python){

    QString scriptLine;

    if (!Python){
        scriptLine = "# " + runvars.sampName + "\n";
        scriptLine += "s" + runvars.sampNum + ".subtitle = \"" + runvars.subtitle + "\"" + "\n";
        scriptLine += "runTime = runTrans(s" + runvars.sampNum;
    }
    else
        scriptLine = "\ttransmission(sample[" + runvars.sampNum + "] ,\"" + runvars.sampName + "\"";
    for (int i = 0; i < 3; i++){
        scriptLine += "," + QString::number(runvars.angles[i]);
    }

    scriptLine += "," + QString::number(runvars.uAmpsT) + ")";
    return scriptLine;
}

