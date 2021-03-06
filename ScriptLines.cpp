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
#include <QValidator>

//There are two Groups of functions here: "Write" functions, and "Parse Functionss"
//"Parse" functions take the QVector containing parameter data and map them to the runstruct struct.
//This runstruct is then used to create strings with appropriate syntax in "write" functions.
//Parse Functions also Check if user input is valid and thus return bools.

//-------------------------------"WRITE" FUNCTIONS------------------------------------------------------------//
//----------------------------------------------------------------------------------------------------------//

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
        scriptLine += ("          s" + QString::number(i+1) + ".title = \""+temp+"\"\n");

        temp = QString::number(samples[i].translation);
        scriptLine += ("    s" + QString::number(i+1) + ".translation = "+temp+"\n");

        temp = QString::number(samples[i].height);
        scriptLine += ("         s" + QString::number(i+1) + ".height = "+temp+"\n");

        temp = samples[i].phi_offset;
        scriptLine += ("     s" + QString::number(i+1) + ".phi_offset = "+temp+"\n");

        temp = QString::number(samples[i].psi);
        scriptLine += ("            s" + QString::number(i+1) + ".psi = "+temp+"\n");

        temp = QString::number(samples[i].footprint);
        scriptLine += ("      s" + QString::number(i+1) + ".footprint = "+temp+"\n");

        temp = QString::number(samples[i].resolution);
        scriptLine += ("     s" + QString::number(i+1) + ".resolution = "+temp+"\n");

        temp = QString::number(samples[i].s3);
        scriptLine += ("             s" + QString::number(i+1) + ".s3 = "+temp+"\n");

        temp = QString::number(samples[i].s4);
        scriptLine += ("             s" + QString::number(i+1) + ".s4 = "+temp+"\n");

        if (samples[i].knauer != -100){
            temp = QString::number(samples[i].knauer);
            scriptLine += ("         s" + QString::number(i+1) + ".knauer = "+temp+"\n");
        }

        if (samples[i].coarse_noMirror != -100){
            temp = QString::number(samples[i].knauer);
            scriptLine += ("s" + QString::number(i+1) + ".coarse_NoMirror = " + temp + "\n");
        }

        scriptLine += "#====================================\n";

    }

    scriptLine += "sample=dimensions(" + QString::number(samples.length()) + ")\n";

    for (int i=0; i < samples.length(); i++){
       scriptLine += "sample[" + QString::number(i+1) + "]=s" + QString::number(i+1) + "; ";
    }
    scriptLine +=  "\n";
    return scriptLine;

}

QString PyWriteSamples(QList <NRSample> samples){

    QString scriptLine = "def experimentsettings():\n";

    if (samples.isEmpty())
        return scriptLine;

    QString tab = "    ";
    QString tabs = tab + tab + tab;

    for (int i = 0; i < samples.length(); i++){

        scriptLine += tab + "S" + QString::number(i+1) + " = {\n";

        scriptLine += tabs + "'title'             : '" + samples[i].title + "',\n";
        scriptLine += tabs + "'subtitle'          : \"" + samples[i].subtitle + "\",\n";
        scriptLine += tabs + "'trans'             : " + QString::number(samples[i].translation) + ",\n";
        scriptLine += tabs + "'height'            : " + QString::number(samples[i].height) + ",\n";
        scriptLine += tabs + "'phi_offset'        : " + samples[i].phi_offset + ",\n";
        scriptLine += tabs + "'psi'               : " + QString::number(samples[i].psi) + ",\n";
        scriptLine += tabs + "'footprint'         : " + QString::number(samples[i].footprint) + ",\n";
        scriptLine += tabs + "'resolution'        : " + QString::number(samples[i].resolution) + ",\n";
        if (samples[i].knauer != -100)
            scriptLine += tabs + "'knauer'            : " + QString::number(samples[i].knauer) + ",\n";
        if (samples[i].coarse_noMirror != -100)
            scriptLine += tabs + "'coarse_nomirror'   : " + QString::number(samples[i].coarse_noMirror) + ",\n";

        scriptLine += tab + tab + "}\n";
        scriptLine += tab + "###########################################\n";
    }

    scriptLine += tab + "sample = [";
    for (int i=0; i < samples.length(); i++){
       scriptLine += "S" + QString::number(i+1);

       if (i != samples.length() -1)
           scriptLine + ", ";
    }
    scriptLine += "]";

    return scriptLine;
}

QString writeRun(runstruct &runvars, int runSM, bool Python){

    QString scriptLine;

    if (runSM && !Python){
        scriptLine += "# " + runvars.sampName + ":\n";
        scriptLine += "s" + runvars.sampNum + ".subtitle = \"" + runvars.subtitle + "\"" + "\n";
    }

    for (int i = 0; i < 3; i++){
       if (runvars.angles[i] != -1){
            if (runSM){
                if (!Python) scriptLine += "runTime = runAngle:SM(s";
                else if (Python) scriptLine += "    runAngle_SM(sample[";
            }
            else {
                if (!Python) scriptLine += "runTime = runAngle(s";
                else if (Python) scriptLine += "    runAngle(sample[";
            }
            scriptLine += runvars.sampNum;
            if (Python) scriptLine += "]";
            scriptLine += "," + QString::number(runvars.angles[i]);
            scriptLine += "," + QString::number(runvars.uAmps[i]) + ")" + "\n";
        }
    }

    return scriptLine;
}

QString writeContrast(runstruct &runvars, int wait, bool Python){

    QString scriptLine;

    if (!Python) scriptLine  = "runTime = ";
    else scriptLine = "    ";

    if (wait) scriptLine += "contrastChange:wait(";
    else scriptLine += "contrastChange(";

    scriptLine += QString::number(runvars.knauer) + ", ";

    for (int i = 0; i < 5; i++){
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

QString pyWriteJulabo(runstruct &runvars, bool runcont){

    QString scriptLine = "    cset(Julabo_WB=" + QString::number(runvars.JTemp);

    if (runcont){
        scriptLine += "runcontrol=1, lowlimit=" + QString::number(runvars.JMin) + ", ";
        scriptLine += "highlimit=" + QString::number(runvars.JMax);
    }
    scriptLine += ")\n";
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
        else scriptLine = "    goToPressure(" + QString::number(runvars.pressure) + ")\n";
    else
        if (!Python) scriptLine = "CSET target_area = " + QString::number(runvars.area) + "\n";
        else scriptLine = "    goToArea(" + QString::number(runvars.area) + ")\n";

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
        scriptLine = "    transmission(sample[" + runvars.sampNum + "] ,\"" + runvars.sampName + "\"";
    for (int i = 0; i < 4; i++){
        scriptLine += "," + QString::number(runvars.angles[i]);
    }

    scriptLine += "," + QString::number(runvars.uAmpsT) + ")"  + "\n";
    return scriptLine;
}

//-------------------------------------------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------------------------------------------//
//------------------------------------------"PARSE" FUNCTIONS--------------------------------------------------------//
//-------------------------------------------------------------------------------------------------------------------//



//Only parses the run into the runstruct if:
//a.) there is a uAmp value for every Angle b.) uAmps, angle both greater than 0. c.) no inputs other than numbers or "uAmps"/"Angle"
bool parseRun(QVector<QVariant>variables, runstruct &runvars){

    if (!checkifDoubles(variables, 2, variables.size()))
        return false;

    runvars.sampName = variables[0].toString();
    runvars.subtitle = variables[1].toString();

    double ang; double amp; bool ok; bool empty; QString ampStr, angStr;

    for (int i = 1; i < 4; i++){

        ampStr = variables[2*i+1].toString();
        angStr = variables[2*i].toString();
        if ((angStr == "" &&  ampStr == "") || (angStr.contains("Angle") && ampStr.contains("uAmps") && i!=1)){
            runvars.angles[i-1] = -1;
            runvars.uAmps[i-1] = -1;
            ok = true;
            empty = true;
        }
        else if (angStr != "" && ampStr != ""){
            ang = variables[2*i].toDouble();
            amp = variables[2*i+1].toDouble();
            ok = true;
        }
        else
            return false;
        if (ang <= 0.0 || amp <= 0.0 || ang >= 5)
            return false;
        else if (ok && !empty) {
            runvars.angles[i-1] = ang;
            runvars.uAmps[i-1] = amp;
            ok = true;
        }
    }

    return ok;
}


bool parseContrast(QVector<QVariant>variables, runstruct &runvars){

    if(!checkifDoubles(variables, 1, variables.size()))
        return false;

    double percentSum = 0;
    for (int i = 0; i < 4; i++){
        runvars.concs[i] = variables[i+1].toDouble();
        percentSum += variables[i+1].toDouble();
    }

    runvars.flow = variables[5].toDouble();
    runvars.volume = variables[6].toDouble();

    if (fabs(percentSum -100)>= 0.1 || runvars.flow <= 0.0 || runvars.volume <= 0.0){
        return false;
       }

    return true;
}


bool parseTransm(QVector<QVariant>variables, runstruct &runvars){

    runvars.sampName = variables[0].toString();

    if(!checkifDoubles(variables, 2, variables.size()))
        return false;

    runvars.subtitle = variables[1].toString();
    runvars.heightOffsT = variables[2].toDouble();

    for (int i = 2; i < 7; i++){
        runvars.angles[i-2] = variables[i].toDouble();
    }
    runvars.uAmpsT = variables[7].toDouble();
    return true;
}



bool parseNIMA_P(QVector<QVariant>variables, runstruct &runvars){

    if(!checkifDoubles(variables, 0, variables.size()))
        return false;

    runvars.pressure = variables[0].toDouble();
    return true;
}

bool parseNIMA_A(QVector<QVariant>variables, runstruct &runvars){

    if(!checkifDoubles(variables, 0, variables.size()))
        return false;
    runvars.area = variables[0].toDouble();

    return true;
}

//For now, runcontrol(y/n) depends only if the user has used the available field.
bool parseJulabo(QVector<QVariant>&variables, runstruct &runvars, bool &runcontrol){

    int last = 3;
    runcontrol = true;
    QString max = variables[1].toString();
    QString min = variables[2].toString();

    if ((max == "Run Control Max" && min == "Run Control Min") || (max == "" && min == "")){
        last = 1;
        runcontrol = false;
    }

    if(!checkifDoubles(variables, 0, last, -5, 95))
        return false;

    runvars.JTemp = variables[0].toDouble();
    runvars.JMax = variables[1].toDouble();
    runvars.JMin = variables[2].toDouble();

    if (runcontrol && (runvars.JMax < runvars.JTemp || runvars.JMin > runvars.JTemp)){
        return false;
    }

    return true;
}

bool parseEurotherm(QVector<QVariant>variables, runstruct &runvars){

    if(!checkifDoubles(variables, 0, variables.size()))
        return false;

    for (int i = 0; i < 8; i++){
        runvars.euroTemps[i] = variables[0].toDouble();
    }

    return true;
}

//checks if input in range is positive doubles, default lower = 0, upper = 1000
bool checkifDoubles(QVector<QVariant>&vars, int first, int last, int lower, int upper){

    QDoubleValidator v(lower, upper, 16);
    QString input; int pos = 0;
    for (int i = first; i < last; i++){
        input = vars[i].toString();
        if (v.validate(input, pos) != QValidator::Acceptable)
            return false;
    }
    return true;
}
