#include "sampleform.h"
#include "sampletable.h"
#include "ui_sampletable.h"
#include "GCLHighLighter.h"
#include <QMainWindow>
#include <QComboBox>
#include <QSignalMapper>
#include <QTime>
#include "mainwindow.h"
#include "runoptions.h"
#include <QApplication>


QString writeRun(runstruct runvars, bool runSM){

    QString scriptLine, ang, amp;
    scriptLine = "# " + runvars.sampName + ":\n";
    scriptLine += "s" + runvars.sampNum + ".subtitle = \"" + runvars.subtitle + "\"" + "\n";

    if (runSM){
        for (int i = 0; i < 3; i++){
            ang = QString::number(runvars.angles[i]);
            amp = QString::number(runvars.uAmps[i]);
            scriptLine += "runTime = runAngle_SM(s" + runvars.sampNum + "," + ang + "," + amp + ")" + "\n";
        }
    }
    else {
        for (int i = 0; i < 3; i++){
            ang = QString::number(runvars.angles[i]);
            amp = QString::number(runvars.uAmps[i]);
            scriptLine += "runTime = runAngle(s" + runvars.sampNum + "," + ang + "," + amp + ")" + "\n";
        }
    }
    return scriptLine;
}

QString writeContrast(runstruct runvars, bool wait){

    QString scriptLine;

    if (wait)
        scriptLine = "runTime = contrastChange:wait(";
    else
        scriptLine = "runTime = contrastChange(";

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
/*
void SMRun(int row){

    QComboBox* whichSamp = new QComboBox;
    QString scriptLine;

    whichSamp=(QComboBox*)ui->tableWidget_1->cellWidget(row, 1);
    mySampleForm->sampleList[whichSamp->currentIndex()].subtitle = ui->tableWidget_1->item(row,2)->text();
    scriptLine = "s" + QString::number(whichSamp->currentIndex()+1) + ".subtitle = \"" + ui->tableWidget_1->item(row,2)->text() + "\"";
    ui->plainTextEdit->insertPlainText(scriptLine+ "\n");
    for(int angle=0; angle<3; angle++){
        if (!(ui->tableWidget_1->item(row,2*angle+3)->text().contains("Angle")) \
                && !(ui->tableWidget_1->item(row,2*angle+4)->text().contains("uAmps")))
        {
            scriptLine = "runTime = runAngle_SM(s" + QString::number(whichSamp->currentIndex()+1) \
                + "," + ui->tableWidget_1->item(row,2*angle+3)->text() + "," \
                + ui->tableWidget_1->item(row,2*angle+4)->text() + ")";
            ui->plainTextEdit->insertPlainText(scriptLine+ "\n");
        }
    }

}

void kineticRun(int row){

    QComboBox* whichSamp = new QComboBox;
    QString scriptLine;

    whichSamp=(QComboBox*)ui->tableWidget_1->cellWidget(row, 1);
    scriptLine = "runTime = runKinetic(s" + QString::number(whichSamp->currentIndex()+1) \
            + "," + ui->tableWidget_1->item(row,3)->text() + "," \
            + ui->tableWidget_1->item(row,4)->text() + ")";
    ui->plainTextEdit->insertPlainText(scriptLine+ "\n");
}

void OGcommand(int row){

      ui->plainTextEdit->insertPlainText(ui->tableWidget_1->item(row,1)->text()+ "\n");
}

void contrastChange(int row){

    int percentSum = 0;
    int secs;
    double angle1;
    bool ok;
    QString scriptLine;
    QComboBox* whichSamp = new QComboBox;
    QComboBox* continueRun = new QComboBox;

    whichSamp = (QComboBox*)ui->tableWidget_1->cellWidget(row, 1);
    continueRun = (QComboBox*)ui->tableWidget_1->cellWidget(row, 8);

    // check if A-D are integers and sum to 100:
    for (int col=2; col < 6; col++){
        if ((ui->tableWidget_1->item(row,col)->text()).toInt(&ok) > 0 \
                && !(ui->tableWidget_1->item(row,col)->text().contains("."))\
                || (ui->tableWidget_1->item(row,col)->text() == "0")){
            percentSum += (ui->tableWidget_1->item(row,col)->text()).toInt(&ok);
        } else {
            percentSum = 0;
            break;
        }
    }
    // set flag:
    if (percentSum != 100 || (ui->tableWidget_1->item(row,6)->text()).toDouble() <= 0.0\
            || (ui->tableWidget_1->item(row,7)->text()).toDouble() <= 0.0){
        ui->tableWidget_1->item(row, 10)->setBackground(Qt::red);
        return;
        }

    ui->tableWidget_1->item(row, 10)->setBackground(Qt::green);
    if (continueRun->currentIndex()){
        // increase run time and update display
        angle1 = ui->tableWidget_1->item(row,7)->text().toDouble()/ui->tableWidget_1->item(row,6)->text().toDouble(); //pump time in minutes
        secs = static_cast<int>(angle1*60); //for TS2 current
        runTime = runTime.addSecs(secs);
        ui->timeEdit->setTime(runTime);//whichSamp->currentIndex()+1)
        scriptLine = "runTime = contrastChange:wait(" + QString::number(mySampleForm->sampleList[whichSamp->currentIndex()].knauer) \
            + "," + ui->tableWidget_1->item(row,2)->text() \
            + "," + ui->tableWidget_1->item(row,3)->text() \
            + "," + ui->tableWidget_1->item(row,4)->text() \
            + "," + ui->tableWidget_1->item(row,5)->text() \
            + "," + ui->tableWidget_1->item(row,6)->text() \
            + "," + ui->tableWidget_1->item(row,7)->text() \
                + ")";
    }else{
        scriptLine = "runTime = contrastChange(" + QString::number(mySampleForm->sampleList[whichSamp->currentIndex()].knauer) \
            + "," + ui->tableWidget_1->item(row,2)->text() \
            + "," + ui->tableWidget_1->item(row,3)->text() \
            + "," + ui->tableWidget_1->item(row,4)->text() \
            + "," + ui->tableWidget_1->item(row,5)->text() \
            + "," + ui->tableWidget_1->item(row,6)->text() \
            + "," + ui->tableWidget_1->item(row,7)->text() \
                + ")";
    }

    ui->plainTextEdit->insertPlainText(scriptLine+ "\n");
}

void setTemp(int row){

    QString scriptLine;
    bool ok;
    QComboBox* whichTemp = new QComboBox;
    QComboBox* runControl = new QComboBox;

    whichTemp = (QComboBox*)ui->tableWidget_1->cellWidget(row, 1);
    switch (whichTemp->currentIndex())
    {
        case 0: //Julabo control
            ok=true;
            runControl = (QComboBox*)ui->tableWidget_1->cellWidget(row, 3);
            // Waterbath limits?!!!!!!!!!!!!!!!!!!!!!!!!
            if(ui->tableWidget_1->item(row,2)->text().toDouble(&ok)> -5.0 \
                    && ui->tableWidget_1->item(row,2)->text().toDouble(&ok)<95.0 \
                    && ok)
            {
                ui->tableWidget_1->item(row, 10)->setBackground(Qt::green);
                switch(runControl->currentIndex())
                {
                    case 0: // no runcontrol
                        scriptLine = "CSET /nocontrol Julabo_WB=" + ui->tableWidget_1->item(row,2)->text();
                        ui->plainTextEdit->insertPlainText(scriptLine+ "\n");
                        break;
                    case 1: // activate runcontrol
                        if(ui->tableWidget_1->item(row,4)->text().toDouble(&ok) < ui->tableWidget_1->item(row,2)->text().toDouble()\
                                && ui->tableWidget_1->item(row,5)->text().toDouble(&ok) > ui->tableWidget_1->item(row,2)->text().toDouble()\
                                && ok)
                        {
                            ui->tableWidget_1->item(row, 10)->setBackground(Qt::green);
                            scriptLine = "CSET /control Julabo_WB=" + ui->tableWidget_1->item(row,2)->text()\
                                    + " lowlimit=" + ui->tableWidget_1->item(row,4)->text()\
                                    + " highlimit=" + ui->tableWidget_1->item(row,5)->text();
                            ui->plainTextEdit->insertPlainText(scriptLine+ "\n");
                        } else {
                            ui->tableWidget_1->item(row, 10)->setBackground(Qt::red);
                        }
                        break;
                }
            } else {
                ui->tableWidget_1->item(row, 10)->setBackground(Qt::red);
            }
            break;
        case 1: // Eurotherms control
            ok = true;
            scriptLine = "CSET";
            ui->plainTextEdit->insertPlainText(scriptLine);
            for(int i=0;i<9;i++){
                scriptLine = " temp" + QString::number(i) + "=" + ui->tableWidget_1->item(row,i+2)->text();
                ui->plainTextEdit->insertPlainText(scriptLine);
            }
            ui->plainTextEdit->insertPlainText("\n");
            break;
        case 2: // Peltier control
            break;
    }
}

void setNIMA(int row){

      QString scriptLine;
      QComboBox* box1 = new QComboBox;

      box1 = (QComboBox*)ui->tableWidget_1->cellWidget(row, 1);
      //QComboBox* box1 = qobject_cast<QComboBox*>(ui->tableWidget_1->cellWidget(row,1));
      if(box1->currentText().contains("Pressure"))
          scriptLine = "CSET target_pressure = " + ui->tableWidget_1->item(row,2)->text();
      else if (box1->currentText().contains("Area"))
          scriptLine = "CSET target_area = " + ui->tableWidget_1->item(row,2)->text();
      ui->plainTextEdit->insertPlainText(scriptLine + "\n");
}

void runTrans(int row){

    int secs;
    double angle1, angle2;
    bool ok;
    QString scriptLine;
    QComboBox* whichSamp = new QComboBox;

    if(mySampleForm->sampleList.length()){
        whichSamp=(QComboBox*)ui->tableWidget_1->cellWidget(row, 1);
        scriptLine = "# " + mySampleForm->sampleList[whichSamp->currentIndex()].title;
        ui->plainTextEdit->insertPlainText(scriptLine+ ":\n");
        mySampleForm->sampleList[whichSamp->currentIndex()].subtitle = ui->tableWidget_1->item(row,2)->text();

        scriptLine = "s" + QString::number(whichSamp->currentIndex()+1) + ".subtitle = \"" \
                + ui->tableWidget_1->item(row,2)->text() + "\"";
        ui->plainTextEdit->insertPlainText(scriptLine+ "\n");
        ok = false;
        angle2 = (ui->tableWidget_1->item(row,8)->text()).toDouble(&ok);

        // increase run time and update display
        if(ui->instrumentCombo->currentText() == "CRISP" || ui->instrumentCombo->currentText() == "SURF"){
            secs = static_cast<int>(angle2/160*3600); // TS2
        } else {
            secs = static_cast<int>(angle2/40*3600); // TS2
        }
        runTime = runTime.addSecs(secs);
        ui->timeEdit->setTime(runTime);
        ok=true;
        scriptLine = "runTime = runTrans(s" + QString::number(whichSamp->currentIndex()+1) \
                    + "," + ui->tableWidget_1->item(row,3)->text() + "," \
                    + ui->tableWidget_1->item(row,4)->text() + "," \
                    + ui->tableWidget_1->item(row,5)->text() + "," \
                    + ui->tableWidget_1->item(row,6)->text() + "," \
                    + ui->tableWidget_1->item(row,7)->text() + ")";

                ui->plainTextEdit->insertPlainText(scriptLine+ "\n");

        if (ok){
            ui->tableWidget_1->item(row, 10)->setBackground(Qt::green);
        } else {
            ui->tableWidget_1->item(row, 10)->setBackground(Qt::red);
        }
    }
}

*/
