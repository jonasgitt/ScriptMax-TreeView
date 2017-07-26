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


QString writeRun(runstruct &runvars, bool runSM){

    QString scriptLine;

    if (runSM){
        for (int i = 0; i < 3; i++){
            scriptLine += "runTime = runAngle_SM(s";
            scriptLine = "# " + runvars.sampName + ":\n";
            scriptLine += "s" + runvars.sampNum + ".subtitle = \"" + runvars.subtitle + "\"" + "\n";
        }
    }
    else{
        for (int i = 0; i < 3; i++){
            scriptLine += "runTime = runAngle(s";
            scriptLine += runvars.sampNum + "," + QString::number(runvars.angles[i]);
            scriptLine += "," + QString::number(runvars.uAmps[i]) + ")" + "\n";
        }
    }
    return scriptLine;
}

QString writeContrast(runstruct runvars, bool wait){

    QString scriptLine;

    if (wait) scriptLine = "runTime = contrastChange:wait(";
    else scriptLine = "runTime = contrastChange(";

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

QString writeNIMA(runstruct runvars, bool PorA){

    QString scriptLine;

    if (!PorA)
        scriptLine = "CSET target_pressure = " + QString::number(runvars.pressure);
    else
        scriptLine = "CSET target_area = " + QString::number(runvars.area);

    return scriptLine;
}

QString writeTransm(runstruct runvars){

    QString scriptLine;
    scriptLine = "# " + runvars.sampName + "\n";
    scriptLine += "s" + runvars.sampNum + ".subtitle = \"" + runvars.subtitle + "\"" + "\n";
    scriptLine += "runTime = runTrans(s" + runvars.sampNum;

    for (int i = 0; i < 3; i++){
        scriptLine += "," + QString::number(runvars.angles[i]);
    }

    scriptLine += "," + QString::number(runvars.uAmpsT) + ")";
    return scriptLine;
}

//=================================================================================================================//
//======================================PYTHON=====================================================================//
//=================================================================================================================//

QString PyContrast(runstruct runvars, bool wait){


}
