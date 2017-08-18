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
    scriptLine +=  + "\n";
    return scriptLine;

}


QString writeRun(runstruct &runvars, int runSM, bool Python){

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

QString writeContrast(runstruct &runvars, int wait, bool Python){

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
    scriptLine += ", " + QString::number(runvars.volume) + ")"  + "\n";

    return scriptLine;

}

QString writeJulabo(runstruct &runvars, int runcont){
    QString scriptLine;

     if(runcont){
         scriptLine = "CSET /control Julabo_WB=" + QString::number(runvars.JTemp);
         scriptLine += " lowlimit=" + QString::number(runvars.JMin);
         scriptLine += " highlimit=" + QString::number(runvars.JMax);
     }

     else scriptLine = "CSET /nocontrol Julabo_WB=" + QString::number(runvars.JTemp);
     scriptLine += "\n";
     return scriptLine;
}

QString writeEuro(runstruct &runvars){
    QString scriptLine;
    scriptLine = "CSET";

    for(int i=0; i<9; i++){
        scriptLine += " temp" + QString::number(i) + "=" + QString::number(runvars.euroTemps[i]);
    }
    scriptLine += "\n";
    return scriptLine;
}

QString writeNIMA(runstruct &runvars, int mode, bool Python){

    QString scriptLine;

    if (mode == 4)
        if (!Python) scriptLine = "CSET target_pressure = " + QString::number(runvars.pressure) + "\n";
        else scriptLine = "\tgoToPressure(" + QString::number(runvars.pressure) + ")\n";
    else
        if (!Python) scriptLine = "CSET target_area = " + QString::number(runvars.area) + "\n";
        else scriptLine = "\tgoToArea(" + QString::number(runvars.area) + ")\n";

    return scriptLine;
}

QString writeTransm(runstruct &runvars, bool Python){

    QString scriptLine;

    if (!Python){
        scriptLine = "# " + runvars.sampName + "\n";
        scriptLine += "s" + runvars.sampNum + ".subtitle = \"" + runvars.subtitle + "\"" + "\n";
        scriptLine += "runTime = runTrans(s" + runvars.sampNum;
    }
    else
        scriptLine = "\ttransmission(sample[" + runvars.sampNum + "] ,\"" + runvars.sampName + "\"";
    for (int i = 0; i < 4; i++){
        scriptLine += "," + QString::number(runvars.angles[i]);
    }

    scriptLine += "," + QString::number(runvars.uAmpsT) + ")"  + "\n";
    return scriptLine;
}



runstruct parseRun(QVector<QVariant>variables){

    runstruct runvars;

    runvars.sampName = variables[0].toString();//0 is now the name - not the number

    runvars.angles[0] = variables[1].toDouble();
    runvars.uAmps[0] = variables[2].toDouble();

    runvars.angles[1] = variables[3].toDouble();
    runvars.uAmps[1] = variables[4].toDouble();

    runvars.angles[2] = variables[5].toDouble();
    runvars.uAmps[2] = variables[6].toDouble();

    qDebug() << "angle 3: " << variables[5];

    return runvars; //or call scriptlines directly?
}

//DO THE ERROR CHECKING
bool parseContrast(QVector<QVariant>variables, runstruct &runvars){

    double percentSum = 0;
    for (int i = 0; i < 4; i++){
        runvars.concs[i] = variables[i+1].toDouble();
        percentSum += variables[i+1].toDouble();
    }

    runvars.flow = variables[5].toDouble();
    runvars.volume = variables[6].toDouble();

    if (fabs(percentSum -100)>= 0.001 || runvars.flow <= 0.0 || runvars.volume <= 0.0){
        return false;
       }

    return true;
}

runstruct parseTransm(QVector<QVariant>variables){

    runstruct runvars;

    runvars.subtitle = variables[0].toString();
    runvars.heightOffsT = variables[1].toDouble();

    for (int i = 2; i < 6; i++){
        runvars.angles[i-2] = variables[i].toDouble();
    }
    runvars.uAmpsT = variables[6].toDouble();
    return runvars;
}

runstruct parseNIMA_P(QVector<QVariant>variables){

    runstruct runvars;

    runvars.pressure = variables[0].toDouble();

    return runvars;
}

runstruct parseNIMA_A(QVector<QVariant>variables){

    runstruct runvars;

    runvars.area = variables[0].toDouble();

    return runvars;
}


runstruct parseJulabo(QVector<QVariant>variables){

    runstruct runvars;

    runvars.JTemp = variables[0].toDouble();
    runvars.JMax = variables[1].toDouble();
    runvars.JMin = variables[2].toDouble();

    return runvars;
}

runstruct parseEurotherm(QVector<QVariant>variables){

    runstruct runvars;

    for (int i = 0; i < 8; i++){
        runvars.euroTemps[i] = variables[0].toDouble();
    }

    return runvars;
}
