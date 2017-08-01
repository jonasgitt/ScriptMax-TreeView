#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sampleform.h"
#include "sampletable.h"
#include "GCLHighLighter.h"
#include "QProcess"
#include "QLibrary"
#include <QTextStream>
#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
//#include "C:\\Users\\ktd43279\\Documents\\PROGS\\MaxSCriptMaker_laptop\\include\\genie_data_access.h"
#include <iostream>
#include <QHostInfo>
#include <QDesktopServices>
#include "ScriptLines.h"
#include <QFile>
#include <QTextStream>
#include <QLineEdit>
#include <QStandardPaths>
#include <QSettings>
#include "pyhighlighter.h"
#include <QProgressBar>
#include <QTimer>
#include <QSize>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(0);

    fileName = "";
    ui->saveButton->setEnabled(false);

    //Script is in OpenGenie at launch
    ui->OGButton->setChecked(true);
    OGhighlighter = new Highlighter(ui->plainTextEdit->document());

    mySampleTable = new SampleTable(); // Be sure to destroy this window somewhere
    initMainTable();
    ProgressBar(10, 1);//EVENTUALLY BELONGS SOMEWHERE ELSE

    connect(ui->tableWidget_1,SIGNAL(currentCellChanged(int,int,int,int)),SLOT(parseTableSlot()));
    connect(mySampleTable,SIGNAL(closedSampWindow()), SLOT(disableRows()));

    parseTable();

}


void MainWindow::initMainTable(){

    actions << " " << "Run" << "Run with SM" << "Kinetic run" << "Run PNR" << "Run PA" \
            << "Free text (OG)"\
            << "--------------"<< "contrastchange" << "Set temperature" << "NIMA" \
            << "--------------"<< "Set Field" << "Run Transmissions";


    QList<QTableWidget*> tables = this->findChildren<QTableWidget*>(); //finds all children of type QTableWidget

    //save number of rows/cols for the 0th entry in qlist, would probs be main table
    const int ROWS = tables[0]->rowCount();
    const int COLS = tables[0]->columnCount();

    int i=0;

    foreach (QTableWidget *table, tables)
    {
        for (int r = 0; r < ROWS; r++) {
            QComboBox *combo = new QComboBox(); //combobox on each line
            //combo->setParent(table);
            table->setCellWidget (r, 0, combo); //combobox in row r and column 0
            combo->addItems(actions);           //puts runoptions in combobox
            combo->setProperty("row", r);   //sets the row property to hold row number r
            connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(onRunSelected(int)));
            i++;
            combo->setDisabled(mySampleForm->sampleList.isEmpty());
        }
        for (int row = 0; row< ROWS; row++){
            for (int col = 1; col< COLS; col++){

                //if there is a combobox in a cell remove the cell widget?
                if(qobject_cast<QComboBox*>(ui->tableWidget_1->cellWidget(row,col)))
                    ui->tableWidget_1->removeCellWidget(row,col);
                QTableWidgetItem *newItem = new QTableWidgetItem;
                newItem->setText("");
                table->setItem(row,col,newItem); //puts/replaces an empty cell where the widget was removed

            }
            if (mySampleForm->sampleList.isEmpty())
                table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        }
        //don't these two commands work against eachother
        table->resizeColumnToContents(0);
        table->setColumnWidth(10,40);
    }

    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(openSampleForm()));
    connect(ui->pushButton_3, SIGNAL(clicked()), this, SLOT(openSampleTable()));
    connect(ui->pushButton_2, SIGNAL(clicked()), this, SLOT(runGenie())); //PLAY BUTTON
    ui->tableWidget_1->setColumnWidth(0,120);

    //Shows Context Menu
    ui->tableWidget_1->verticalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableWidget_1->verticalHeader(), SIGNAL(customContextMenuRequested(const QPoint&)),
        this, SLOT(ShowContextMenu(const QPoint&)));

    // initialise run time to 0:
    QTime time = QTime::fromString("00:00", "hh:mm");
    runTime = time;

}


void MainWindow::parseTableSlot(){
    parseTable();
}


void MainWindow::writeBackbone(){

    QString BFileName;
    //Get Backbone from .txt file
    if (ui->OGButton->isChecked())
        BFileName = ":/OGbackbone.txt";
    else if (ui->PythonButton->isChecked())
        BFileName = ":/PyBackbone.txt";

    else ui->plainTextEdit->setPlainText("Neither are checked");

    QFile BFile(BFileName);

    if (!BFile.open(QFile::ReadOnly | QFile::Text)){
           QMessageBox::warning(this, "Error" , "Couldn't open Backbone File");
    }

    QTextStream in(&BFile);
    QString OGtext = in.readAll();
    ui->plainTextEdit->setPlainText(OGtext);

    BFile.close();


    ui->plainTextEdit->find("GLOBAL runTime"); //positions the cursor to insert instructions
    ui->plainTextEdit->moveCursor(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
    for (int i=0; i < mySampleForm->sampleList.length(); i++){
        ui->plainTextEdit->insertPlainText(" s" + QString::number(i+1)); // sample numbering starts with 1
    }
}


void MainWindow::parseTable(){

    //finds all comboboxes that are children of the table
    //this won't include other comboxes which are themselves children of comboboxes
    QList<QComboBox*> combo = ui->tableWidget_1->findChildren<QComboBox*>();
    QString scriptLine; // a string that temporarily stores info before adding to script
    int sendingRow;

    // change column headers according to row being edited
    sendingRow = ui->tableWidget_1->currentRow();
    if(sendingRow>-1)
        setHeaders(combo[sendingRow]->currentIndex());

    // initialise run time to 0:
    runTime = runTime.fromString("00:00", "hh:mm");

    // prepare script header
    ui->plainTextEdit->clear();
    writeBackbone();

    ui->plainTextEdit->find("runTime=0"); //positions the cursor to insert instructions
    ui->plainTextEdit->moveCursor(QTextCursor::Down, QTextCursor::MoveAnchor);

    // process each table row
    for (int row=0; row < ui->tableWidget_1->rowCount(); row++){
        //ui->tableWidget_1->item(row, 8)->setBackground(Qt::white);
        int whatAction = combo[row]->currentIndex();
        switch(whatAction)
        {
            case 0:
                break;
            case 1: // normal run - extract into void with options for runangle_SM, kinetic etc...
                normalRun(row, false);
                break;
            case 2: // run with supermirror
                normalRun(row, true);
                break;
            case 3:// run kinetic
                kineticRun(row);
                break;
            case 6: // free OpenGenie command
                OGcommand(row);
                break;
            case 8: // contrastChange
                contrastChange(row);
                break;
            case 9:// set temperature
                setTemp(row);
                break;
            case 10: // NIMA control
                setNIMA(row);
                break;
            case 13: // run transmissions
                runTrans(row);
                break;
        }
    }
    samplestoPlainTextEdit();
    if(ui->checkBox->isChecked()){
        save();
    }


}
//------------------------------------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------------------//
//---------------------------------RUNOPTIONS-----------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------------------//
void MainWindow::samplestoPlainTextEdit(){
    ui->plainTextEdit->find("#do not need to be changed during experiment."); //positions the cursor to insert instructions
    ui->plainTextEdit->moveCursor(QTextCursor::Down, QTextCursor::MoveAnchor);
    ui->plainTextEdit->moveCursor(QTextCursor::Down, QTextCursor::MoveAnchor); //move cursor down one more line
    QList<NRSample> samples = mySampleForm->sampleList;
    ui->plainTextEdit->insertPlainText(writeSamples(samples));
}

void MainWindow::normalRun(int row, bool runSM){

    bool ok;
    QComboBox* whichSamp = new QComboBox;
    runstruct runvars;

    if(mySampleForm->sampleList.length()){ //if there is a sample

            whichSamp = (QComboBox*)ui->tableWidget_1->cellWidget(row, 1);
            runvars.sampName = mySampleForm->sampleList[whichSamp->currentIndex()].title;

            mySampleForm->sampleList[whichSamp->currentIndex()].subtitle = ui->tableWidget_1->item(row,2)->text();
            runvars.subtitle = ui->tableWidget_1->item(row,2)->text();

            runvars.sampNum = QString::number(whichSamp->currentIndex()+1);

            for(int cell = 0; cell < 3; cell++){
                ok = false;
                runvars.angles[cell] = (ui->tableWidget_1->item(row,2*cell+3)->text()).toDouble(&ok);
                runvars.uAmps[cell] = (ui->tableWidget_1->item(row,2*cell+4)->text()).toDouble(&ok);

                if (runvars.angles[cell] > 0.0 && runvars.uAmps[cell] > 0.0) // apply filter on 'reasonable' angles and run times
                {
                    ok = true;
                    updateRunTime(runvars.uAmps[cell]);
                } else if  (!(ui->tableWidget_1->item(row,2*cell+3)->text().contains("Angle")\
                            && ui->tableWidget_1->item(row,2*cell+4)->text().contains("uAmps"))\
                        || !(ui->tableWidget_1->item(row,2*cell+3)->text() == ""\
                                && ui->tableWidget_1->item(row,2*cell+4)->text() == ""))
                    {
                        ok = false; break;
                    }
            };

            if (ok){
                ui->tableWidget_1->item(row, 10)->setBackground(Qt::green);
            } else {
                ui->tableWidget_1->item(row, 10)->setBackground(Qt::red);
            }
        }

    QString scriptText = writeRun(runvars, runSM);
    ui->plainTextEdit->insertPlainText(scriptText);
    return;
}

void MainWindow::kineticRun(int row){

    QComboBox* whichSamp = new QComboBox;
    QString scriptLine;

    whichSamp=(QComboBox*)ui->tableWidget_1->cellWidget(row, 1);
    scriptLine = "runTime = runKinetic(s" + QString::number(whichSamp->currentIndex()+1) \
            + "," + ui->tableWidget_1->item(row,3)->text() + "," \
            + ui->tableWidget_1->item(row,4)->text() + ")";
    ui->plainTextEdit->insertPlainText(scriptLine+ "\n");
    return;
}

void MainWindow::OGcommand(int row){

      ui->plainTextEdit->insertPlainText(ui->tableWidget_1->item(row,1)->text()+ "\n");
}

void MainWindow::contrastChange(int row){

    int percentSum = 0;
    int secs;
    double angle1;
    bool ok, wait;
    QString scriptLine;
    QComboBox* whichSamp = new QComboBox;
    QComboBox* continueRun = new QComboBox;

    runstruct runvars;

    whichSamp = (QComboBox*)ui->tableWidget_1->cellWidget(row, 1);
    continueRun = (QComboBox*)ui->tableWidget_1->cellWidget(row, 8);

    // check if A-D are integers and sum to 100:
    for (int i=0; i < 4; i++){
        runvars.concs[i] = (ui->tableWidget_1->item(row,i+2)->text()).toInt(&ok);

        if (runvars.concs[i] >= 0){
            percentSum += runvars.concs[i];
        } else {
            percentSum = 0;
            break;
        }
    }

    runvars.flow = (ui->tableWidget_1->item(row,6)->text()).toDouble();
    runvars.volume = (ui->tableWidget_1->item(row,7)->text()).toDouble();
    runvars.knauer = mySampleForm->sampleList[whichSamp->currentIndex()].knauer;

    if (percentSum != 100 || runvars.flow <= 0.0 || runvars.volume <= 0.0){
        ui->tableWidget_1->item(row, 10)->setBackground(Qt::red);
        return;
       }
    else ui->tableWidget_1->item(row, 10)->setBackground(Qt::green);

    if (continueRun->currentIndex()){

        angle1 = runvars.volume/runvars.flow; //pump time in minutes
        secs = static_cast<int>(angle1*60); //for TS2 current
        ui->timeEdit->setTime(runTime.addSecs(secs));//whichSamp->currentIndex()+1)

        wait = true;
        scriptLine = writeContrast(runvars, wait);

    }else {
        wait = false;
        scriptLine = writeContrast(runvars, wait);
    }
    ui->plainTextEdit->insertPlainText(scriptLine + "\n");
    return;
}

void MainWindow::setTemp(int row){

    QString scriptLine = "";
    bool ok;
    QComboBox* whichTemp = new QComboBox;
    QComboBox* runControl = new QComboBox;
    runstruct runvars;

    ui->tableWidget_1->item(row, 10)->setBackground(Qt::red);

    whichTemp = (QComboBox*)ui->tableWidget_1->cellWidget(row, 1);
    switch (whichTemp->currentIndex())
    {
        case 0:{ //Julabo control
            ok=true;
            runControl = (QComboBox*)ui->tableWidget_1->cellWidget(row, 3);
            //int runCont = 1;//TEMP
            runvars.JTemp = ui->tableWidget_1->item(row,2)->text().toDouble(&ok);

            int runCont;
            if (runvars.JTemp > -5.0 && runvars.JTemp < 95.0 && ok)
                    runCont = runControl->currentIndex();

            if (runCont && ok){
                runvars.JMin = ui->tableWidget_1->item(row,4)->text().toDouble(&ok);
                runvars.JMax = ui->tableWidget_1->item(row,5)->text().toDouble(&ok);

                if (runCont && runvars.JMax > runvars.JTemp && runvars.JMin < runvars.JTemp){
                    scriptLine = writeJulabo(runvars, runCont);
                    ui->tableWidget_1->item(row, 10)->setBackground(Qt::green);
                }
             }

            else if (!runCont && ok) {
                scriptLine = writeJulabo(runvars, runCont);
                ui->tableWidget_1->item(row, 10)->setBackground(Qt::green);
            }

            break;}
    case 1:{ // Eurotherms control
            ok = true;
            int i = 0;

            while(i < 9 && ok){
                runvars.euroTemps[i] = (ui->tableWidget_1->item(row,i+2)->text()).toDouble(&ok);
                i++;
            }
            if (ok) ui->tableWidget_1->item(row, 10)->setBackground(Qt::green);
            scriptLine = writeEuro(runvars);

            break;}
    case 2:{ // Peltier control
            break;}
    }

    ui->plainTextEdit->insertPlainText(scriptLine + "\n");
    return;
}

void MainWindow::setNIMA(int row){

      QString scriptLine;
      QComboBox* box1 = new QComboBox;
      bool PorA, ok;
      runstruct runvars;

      box1 = (QComboBox*)ui->tableWidget_1->cellWidget(row, 1);
      ui->tableWidget_1->item(row, 10)->setBackground(Qt::red);

      if(box1->currentText().contains("Pressure")){
          PorA = false;
          runvars.pressure = (ui->tableWidget_1->item(row,2)->text()).toDouble(&ok);
       }

      else if (box1->currentText().contains("Area"))
          PorA = true;
          runvars.area = (ui->tableWidget_1->item(row,2)->text()).toDouble(&ok);

      if (ok) {
          scriptLine = writeNIMA(runvars, PorA);
          ui->tableWidget_1->item(row, 10)->setBackground(Qt::green);
      }

      ui->plainTextEdit->insertPlainText(scriptLine + "\n");
      return;
}

void MainWindow::runTrans(int row){

    double angle2;
    bool ok;
    QComboBox* whichSamp = new QComboBox;
    runstruct runvars;

    if(mySampleForm->sampleList.length()){
        whichSamp=(QComboBox*)ui->tableWidget_1->cellWidget(row, 1);
        runvars.sampName = mySampleForm->sampleList[whichSamp->currentIndex()].title;

        mySampleForm->sampleList[whichSamp->currentIndex()].subtitle = ui->tableWidget_1->item(row,2)->text();
        runvars.subtitle = ui->tableWidget_1->item(row,2)->text();

        runvars.sampNum = QString::number(whichSamp->currentIndex()+1);
        runvars.heightOffsT = (ui->tableWidget_1->item(row,3)->text()).toDouble(&ok);

        for (int i = 0; i < 3; i++){
            runvars.angles[i] = (ui->tableWidget_1->item(row,i+4)->text()).toDouble(&ok);
        }
        runvars.uAmpsT = (ui->tableWidget_1->item(row,7)->text()).toDouble(&ok);
        angle2 = (ui->tableWidget_1->item(row,8)->text()).toDouble(&ok);

        if (ok){
            ui->tableWidget_1->item(row, 10)->setBackground(Qt::green);
            updateRunTime(angle2);
            ui->plainTextEdit->insertPlainText(writeTransm(runvars) + "\n");
        } else ui->tableWidget_1->item(row, 10)->setBackground(Qt::red);

    }

    return;
}

//-----------------------------------------RUNOPTIONS OVER----------------------------------------------------------//
//------------------------------------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------------------//
void MainWindow::updateRunTime(double angle){
    int secs;
    if(ui->instrumentCombo->currentText() == "CRISP" || ui->instrumentCombo->currentText() == "SURF"){
        secs = static_cast<int>(angle/160*3600); // TS2
    } else {
        secs = static_cast<int>(angle/40*3600); // TS2
    }
    runTime = runTime.addSecs(secs);
    ui->timeEdit->setTime(runTime);
    return;
}

//why is it only displaying one specific sample and not the whole table?
void MainWindow::openSampleForm()
{
    if(mySampleForm->currentSample > 0){
        mySampleForm->displaySample(mySampleForm->currentSample);
    }
    mySampleForm->show();
   // ...
}


void MainWindow::updateSamplesSlot(){
    bool ok;

    //what do these expressions mean?
    QRegExp isSum("^[+-]?\\d*\\.?\\d*([+-]\\d*\\.?\\d*)?$");
    QRegExp isDouble("^[+-]?[0-9]+([\\,\\.][0-9]+)?$");

    //tableWidget is part of SampleTable
    for(int row=0; row<mySampleTable->ui->tableWidget->rowCount(); row++){
        ok = true;
        // ####### check if it's only edit or new sample!!!!!
        //if there is already an entry in the list
        if(row < mySampleForm->sampleList.length()){
            //if first column is empty
            if(mySampleTable->ui->tableWidget->item(row,0)->text()=="")
                ok = false;
            //if the field does not match isSum exactly
            if(!isSum.exactMatch(mySampleTable->ui->tableWidget->item(row,3)->text())){
                ok = false;
                //make third column empty?
                mySampleTable->ui->tableWidget->item(row,3)->setText("");
            }
            //in every column
            for(int col=1; col < mySampleTable->ui->tableWidget->columnCount()-1; col++){
                //what is special about the third column?
                if(col!=3)
                    if (!isDouble.exactMatch(mySampleTable->ui->tableWidget->item(row,col)->text())){
                        ok = false;
                        mySampleTable->ui->tableWidget->item(row,col)->setText("");
                        mySampleTable->ui->tableWidget->setCurrentCell(row,col);
                    }                
            }
            //add: check if translation values are the same...

            //the above checked if a cell was empty or of the wrong type
            //if input was ok save the inputted data in the samplelist
            if (ok){
                mySampleForm->sampleList[row].title = mySampleTable->ui->tableWidget->item(row,0)->text();
                mySampleForm->sampleList[row].translation = mySampleTable->ui->tableWidget->item(row,1)->text().toDouble();
                mySampleForm->sampleList[row].height = mySampleTable->ui->tableWidget->item(row,2)->text().toDouble();
                mySampleForm->sampleList[row].phi_offset = mySampleTable->ui->tableWidget->item(row,3)->text();
                mySampleForm->sampleList[row].footprint = mySampleTable->ui->tableWidget->item(row,4)->text().toDouble();
                mySampleForm->sampleList[row].resolution = mySampleTable->ui->tableWidget->item(row,5)->text().toDouble();
                mySampleForm->sampleList[row].s3 = mySampleTable->ui->tableWidget->item(row,6)->text().toDouble();
                mySampleForm->sampleList[row].s4 = mySampleTable->ui->tableWidget->item(row,7)->text().toDouble();
                mySampleForm->sampleList[row].knauer = mySampleTable->ui->tableWidget->item(row,8)->text().toDouble();
            }else {
                QMessageBox msgBox;
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText("One or more values are missing, of the wrong type or duplicates (e.g. translation or switch!");
                msgBox.exec();
            }
        }
         //if a new sample is being created
         else {
            if(mySampleTable->ui->tableWidget->item(row,0)->text()=="")
                ok = false;
            if(!isSum.exactMatch(mySampleTable->ui->tableWidget->item(row,3)->text())){
                ok = false;
                mySampleTable->ui->tableWidget->item(row,3)->setText("");
            }
            for(int col=1; col < mySampleTable->ui->tableWidget->columnCount(); col++){
                if(col!=3)
                    if (!isDouble.exactMatch(mySampleTable->ui->tableWidget->item(row,col)->text())){
                        ok = false;
                        mySampleTable->ui->tableWidget->item(row,col)->setText("");
                        //mySampleTable->ui->tableWidget->setCurrentCell(row,col);
                    }
            }
            if (ok){ // create new smaple in sampleList
                NRSample newSample;

                newSample.title = mySampleTable->ui->tableWidget->item(row,0)->text();
                newSample.translation = mySampleTable->ui->tableWidget->item(row,1)->text().toDouble();
                newSample.height = mySampleTable->ui->tableWidget->item(row,2)->text().toDouble();
                newSample.phi_offset = mySampleTable->ui->tableWidget->item(row,3)->text();
                newSample.footprint = mySampleTable->ui->tableWidget->item(row,4)->text().toDouble();
                newSample.resolution = mySampleTable->ui->tableWidget->item(row,5)->text().toDouble();
                newSample.s3 = mySampleTable->ui->tableWidget->item(row,6)->text().toDouble();
                newSample.s4 = mySampleTable->ui->tableWidget->item(row,7)->text().toDouble();
                newSample.knauer = mySampleTable->ui->tableWidget->item(row,8)->text().toDouble();
                mySampleForm->sampleList.append(newSample);
                if(mySampleForm->sampleList.length() == 1){
                    mySampleForm->displaySample(0);
                    mySampleForm->currentSample = 0;
                }

            }
        }
    }
    parseTable();

}


void MainWindow::updateSubtitleSlot(){
    /*QComboBox *comb = (QComboBox *)sender();
    int nRow = comb->property("row").toInt();
    ui->tableWidget_1->item(nRow,2)->setText(mySampleForm->sampleList[comb->currentIndex()].subtitle);
    */
    parseTable();
}


//reads from table and stores in mysampleform
void MainWindow::openSampleTable()
{
    //mySampleTable = new SampleTable();
    // initialize table
    for (int row = mySampleForm->sampleList.length(); row< mySampleTable->ui->tableWidget->rowCount(); row++){
        for (int col = 0; col< mySampleTable->ui->tableWidget->columnCount(); col++){
            QTableWidgetItem *newItem = new QTableWidgetItem;
            newItem->setText("");
            mySampleTable->ui->tableWidget->setItem(row,col,newItem);

        }
    }

    for (int i=0; i < mySampleForm->sampleList.length(); i++){
        mySampleTable->ui->tableWidget->setVerticalHeaderItem(i,new QTableWidgetItem("S"+QString::number(i+1)));

        QTableWidgetItem *title = new QTableWidgetItem;
        title->setText(mySampleForm->sampleList[i].title);
        mySampleTable->ui->tableWidget->setItem(i,0,title);

        QTableWidgetItem *trans = new QTableWidgetItem;
        trans->setText(QString::number(mySampleForm->sampleList[i].translation));
        mySampleTable->ui->tableWidget->setItem(i,1,trans);

        QTableWidgetItem *height = new QTableWidgetItem;
        height->setText(QString::number(mySampleForm->sampleList[i].height));
        mySampleTable->ui->tableWidget->setItem(i,2,height);

        QTableWidgetItem *phioff = new QTableWidgetItem;
        phioff->setText(mySampleForm->sampleList[i].phi_offset);
        mySampleTable->ui->tableWidget->setItem(i,3,phioff);

        QTableWidgetItem *footprint = new QTableWidgetItem;
        footprint->setText(QString::number(mySampleForm->sampleList[i].footprint));
        mySampleTable->ui->tableWidget->setItem(i,4,footprint);

        QTableWidgetItem *res = new QTableWidgetItem;
        res->setText(QString::number(mySampleForm->sampleList[i].resolution));
        mySampleTable->ui->tableWidget->setItem(i,5,res);

        QTableWidgetItem *s3 = new QTableWidgetItem;
        s3->setText(QString::number(mySampleForm->sampleList[i].s3));
        mySampleTable->ui->tableWidget->setItem(i,6,s3);

        QTableWidgetItem *s4 = new QTableWidgetItem;
        s4->setText(QString::number(mySampleForm->sampleList[i].s4));
        mySampleTable->ui->tableWidget->setItem(i,7,s4);

        QTableWidgetItem *knauer = new QTableWidgetItem;
        knauer->setText(QString::number(mySampleForm->sampleList[i].knauer));
        mySampleTable->ui->tableWidget->setItem(i,8,knauer);
    }
    connect(mySampleTable->ui->tableWidget,SIGNAL(currentCellChanged(int,int,int,int)),SLOT(updateSamplesSlot()));
    mySampleTable->show();
   // ...
}

//changes the table when we select run, run transmissions, contrast change
void MainWindow::setHeaders(int which){
    switch (which) {
    case 0: // empty
        for(int i=2; i<12; i++){
            ui->tableWidget_1->setHorizontalHeaderItem(i-1,new QTableWidgetItem(QString::number(i)));
        }
        break;
    case 1: // Run
        ui->tableWidget_1->setHorizontalHeaderItem(2,new QTableWidgetItem("Subtitle"));
        ui->tableWidget_1->setHorizontalHeaderItem(3,new QTableWidgetItem("Angle 1"));
        ui->tableWidget_1->setHorizontalHeaderItem(4,new QTableWidgetItem("uAmps 1"));
        ui->tableWidget_1->setHorizontalHeaderItem(5,new QTableWidgetItem("Angle 2"));
        ui->tableWidget_1->setHorizontalHeaderItem(6,new QTableWidgetItem("uAmps 2"));
        ui->tableWidget_1->setHorizontalHeaderItem(7,new QTableWidgetItem("Angle 3"));
        ui->tableWidget_1->setHorizontalHeaderItem(8,new QTableWidgetItem("uAmps 3"));
        break;
    case 8: // contrastChange
        ui->tableWidget_1->setHorizontalHeaderItem(2,new QTableWidgetItem("concA"));
        ui->tableWidget_1->setHorizontalHeaderItem(3,new QTableWidgetItem("concB"));
        ui->tableWidget_1->setHorizontalHeaderItem(4,new QTableWidgetItem("concC"));
        ui->tableWidget_1->setHorizontalHeaderItem(5,new QTableWidgetItem("concD"));
        ui->tableWidget_1->setHorizontalHeaderItem(6,new QTableWidgetItem("Flow [mL/min]"));
        ui->tableWidget_1->setHorizontalHeaderItem(7,new QTableWidgetItem("Volume [mL]"));
        break;
    case 13: // Run transmissions
        ui->tableWidget_1->setHorizontalHeaderItem(2,new QTableWidgetItem("Subtitle"));
        ui->tableWidget_1->setHorizontalHeaderItem(3,new QTableWidgetItem("height offset"));
        ui->tableWidget_1->setHorizontalHeaderItem(4,new QTableWidgetItem("s1vg"));
        ui->tableWidget_1->setHorizontalHeaderItem(5,new QTableWidgetItem("s2vg"));
        ui->tableWidget_1->setHorizontalHeaderItem(6,new QTableWidgetItem("s3vg"));
        ui->tableWidget_1->setHorizontalHeaderItem(7,new QTableWidgetItem("s4vg"));
        ui->tableWidget_1->setHorizontalHeaderItem(8,new QTableWidgetItem("uamps"));
        break;
    default:
        break;
    }
}


void MainWindow::runControl(int value){
    QComboBox *comb = (QComboBox *)sender();
    QTableWidget *tabl = (QTableWidget *)comb->parentWidget()->parent();
    int nRow = comb->property("row").toInt();

    switch(value)
    {
        case 0:
            tabl->item(nRow,4)->setText("");
            tabl->item(nRow,5)->setText("");
            break;
        case 1:
            tabl->item(nRow,4)->setText("MIN");
            tabl->item(nRow,5)->setText("MAX");
            break;
    }
}


void  MainWindow::onDeviceSelected(int value)
{
    QComboBox *comb = (QComboBox *)sender();
    QTableWidget *tabl = (QTableWidget *)comb->parentWidget()->parent();
    QComboBox* runControl = new QComboBox;
    QStringList yesNo;
    int nRow = comb->property("row").toInt();

    QComboBox* box = qobject_cast<QComboBox*>(ui->tableWidget_1->cellWidget(nRow,3));
    if(box)
        tabl->removeCellWidget(nRow,3); // remove run control box
    for(int i=1;i<10;i++){
        tabl->item(nRow,i)->setText("");
    }
    switch(value)
    {
        case 0:
            tabl->item(nRow,2)->setText("Julabo Temp");            
            yesNo << "no runcontrol" << "RUNCONTROL";
            runControl->addItems(yesNo);
            tabl->setCellWidget(nRow,3,runControl);
            runControl->setProperty("row", nRow);
            connect(runControl, SIGNAL(activated(int)), this, SLOT(runControl(int)));
            break;
        case 1:
            tabl->item(nRow,2)->setText("T1");
            tabl->item(nRow,3)->setText("T2");
            tabl->item(nRow,4)->setText("T3");
            tabl->item(nRow,5)->setText("T4");
            tabl->item(nRow,6)->setText("T5");
            tabl->item(nRow,7)->setText("T6");
            tabl->item(nRow,8)->setText("T7");
            break;
    }
}


void  MainWindow::onModeSelected(int value)
{
    QComboBox *comb = (QComboBox *)sender();
    QTableWidget *tabl = (QTableWidget *)comb->parentWidget()->parent();
    int nRow = comb->property("row").toInt();

    for(int i=1;i<10;i++){
        tabl->item(nRow,i)->setText("");
    }
    switch(value)
    {
        case 0:
            tabl->item(nRow,2)->setText("Target p");
            break;
        case 1:
            tabl->item(nRow,2)->setText("Target A");
            break;
    }
}


void  MainWindow::onRunSelected(int value)
{
    QComboBox *comb = (QComboBox *)sender();
    QTableWidget *tabl = (QTableWidget *)comb->parentWidget()->parent();

    int nRow = comb->property("row").toInt();
    for(int i=1;i < ui->tableWidget_1->columnCount();i++){
        tabl->item(nRow,i)->setText("");
    }
    QComboBox* comboTemp = new QComboBox();
    QStringList devices;
    devices << "Julabo Waterbath" << "Eurotherm 8x" << "Peltier x4";
    comboTemp->addItems(devices);

    QComboBox* combo = new QComboBox();
    QStringList modes;
    modes << "Pressure Ctrl" << "Area Ctrl";
    combo->addItems(modes);

    comb->setStyleSheet("QComboBox { background-color: lightGray; }");

    QComboBox* samplesCombo = new QComboBox();
    QString s;
    samples.clear();
    for (int i=0;i<mySampleForm->sampleList.length();i++){
        s = mySampleForm->sampleList[i].title;
        samples << s;
    }

    samplesCombo->addItems(samples);

    QComboBox* waitCombo = new QComboBox();
    QStringList wait;
    wait << "CONTINUE" << "WAIT";
    waitCombo->addItems(wait);

    tabl->removeCellWidget(nRow,1);
    tabl->removeCellWidget(nRow,3);
    tabl->removeCellWidget(nRow,8);
    setHeaders(0);

    switch(value)
    {
    case 1: // run
            if(mySampleForm->sampleList.length()){
                setHeaders(1);
                comb->setStyleSheet("QComboBox { background-color: lightGreen; }");
                tabl->removeCellWidget(nRow,1);
                tabl->setCellWidget (nRow, 1, samplesCombo);
                tabl->item(nRow,2)->setText(mySampleForm->sampleList[samplesCombo->currentIndex()].subtitle);
                tabl->item(nRow,2)->setToolTip("Subtitle");
                tabl->item(nRow,3)->setText("Angle 1");
                tabl->item(nRow,3)->setToolTip("Angle 1");
                tabl->item(nRow,4)->setText("uAmps 1");
                tabl->item(nRow,4)->setToolTip("uAmps 1");
                tabl->item(nRow,5)->setText("Angle 2");
                tabl->item(nRow,5)->setToolTip("Angle 2");
                tabl->item(nRow,6)->setText("uAmps 2");
                tabl->item(nRow,6)->setToolTip("uAmps 2");
                tabl->item(nRow,7)->setText("Angle 3");
                tabl->item(nRow,7)->setToolTip("Angle 3");
                tabl->item(nRow,8)->setText("uAmps 3");
                tabl->item(nRow,8)->setToolTip("uAmps 3");
                connect(samplesCombo, SIGNAL(activated(int)), this, SLOT(updateSubtitleSlot()));
            } else {
                QMessageBox msgBox;
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText("There are no samples to run. Please define at least one!");
                msgBox.exec();
            }
            break;
        case 2: // run with SM
            setHeaders(1);
            comb->setStyleSheet("QComboBox { background-color: lightGreen; }");
            tabl->removeCellWidget(nRow,1);
            tabl->setCellWidget (nRow, 1, samplesCombo);
            tabl->item(nRow,2)->setText(mySampleForm->sampleList[samplesCombo->currentIndex()].subtitle);
            tabl->item(nRow,2)->setToolTip("Subtitle");
            tabl->item(nRow,3)->setText("Angle 1");
            tabl->item(nRow,3)->setToolTip("Angle 1");
            tabl->item(nRow,4)->setText("uAmps 1");
            tabl->item(nRow,4)->setToolTip("uAmps 1");
            tabl->item(nRow,5)->setText("Angle 2");
            tabl->item(nRow,5)->setToolTip("Angle 2");
            tabl->item(nRow,6)->setText("uAmps 2");
            tabl->item(nRow,6)->setToolTip("uAmps 2");
            tabl->item(nRow,7)->setText("Angle 3");
            tabl->item(nRow,7)->setToolTip("Angle 3");
            tabl->item(nRow,8)->setText("uAmps 3");
            tabl->item(nRow,8)->setToolTip("uAmps 3");
            connect(samplesCombo, SIGNAL(activated(int)), this, SLOT(updateSubtitleSlot()));
            break;
        case 3: // run kinetic
            comb->setStyleSheet("QComboBox { background-color: lightGreen; }");
            tabl->removeCellWidget(nRow,1);
            tabl->setCellWidget (nRow, 1, samplesCombo);
            tabl->item(nRow,3)->setText("Angle");
            tabl->item(nRow,3)->setToolTip("Angle");
            tabl->item(nRow,4)->setText("time");
            tabl->item(nRow,4)->setToolTip("time per step");
            tabl->item(nRow,5)->setText("No of steps");
            tabl->item(nRow,4)->setToolTip("No of steps");
            connect(samplesCombo, SIGNAL(activated(int)), this, SLOT(parseTableSlot()));
            break;
        case 8: // contrastChange
            setHeaders(8);
            tabl->removeCellWidget(nRow,1);
            tabl->setCellWidget (nRow, 1, samplesCombo);
            tabl->item(nRow,2)->setText("conc A");
            tabl->item(nRow,2)->setToolTip("conc A");
            tabl->item(nRow,3)->setText("conc B");
            tabl->item(nRow,3)->setToolTip("conc B");
            tabl->item(nRow,4)->setText("conc C");
            tabl->item(nRow,4)->setToolTip("conc C");
            tabl->item(nRow,5)->setText("conc D");
            tabl->item(nRow,5)->setToolTip("conc D");
            tabl->item(nRow,6)->setText("Flow[mL/min]");
            tabl->item(nRow,6)->setToolTip("Flow[mL/min]");
            tabl->item(nRow,7)->setText("Volume[mL]");
            tabl->item(nRow,6)->setToolTip("Volume[mL]");
            tabl->setCellWidget (nRow, 8, waitCombo);
            connect(samplesCombo, SIGNAL(activated(int)), this, SLOT(parseTableSlot()));
            connect(waitCombo, SIGNAL(activated(int)), this, SLOT(parseTableSlot()));
            break;
        case 9: // set temperature
            comb->setStyleSheet("QComboBox { background-color: orange; }");
            tabl->setCellWidget (nRow, 1, comboTemp);
            comboTemp->setProperty("row", nRow);
            connect(comboTemp, SIGNAL(currentIndexChanged(int)), this, SLOT(onDeviceSelected(int)));
            connect(comboTemp, SIGNAL(activated(int)), this, SLOT(onDeviceSelected(int)));
            //comboTemp->setCurrentIndex(1);
            //comboTemp->setCurrentIndex(0);
            break;
        case 10: // NIMA control
            tabl->setCellWidget (nRow, 1, combo);
            combo->setProperty("row", nRow);
            connect(combo, SIGNAL(activated(int)), this, SLOT(onModeSelected(int)));
            break;
        case 13: // Run Transmissions
            setHeaders(13);
            tabl->removeCellWidget(nRow,1);
            tabl->setCellWidget (nRow, 1, samplesCombo);
            tabl->item(nRow,2)->setText("Subtitle");
            tabl->item(nRow,2)->setToolTip("Subtitle");
            tabl->item(nRow,3)->setText("height offset");
            tabl->item(nRow,3)->setToolTip("height offset");
            tabl->item(nRow,4)->setText("s1vg");
            tabl->item(nRow,4)->setToolTip("s1vg");
            tabl->item(nRow,5)->setText("s2vg");
            tabl->item(nRow,5)->setToolTip("s2vg");
            tabl->item(nRow,6)->setText("s3vg");
            tabl->item(nRow,6)->setToolTip("s3vg");
            tabl->item(nRow,7)->setText("s4vg");
            tabl->item(nRow,7)->setToolTip("s4vg");
            tabl->item(nRow,7)->setText("uAmps");
            tabl->item(nRow,7)->setToolTip("uAmps");
            connect(samplesCombo, SIGNAL(activated(int)), this, SLOT(parseTableSlot()));
            break;
    }

    parseTable();
    //emit valueChanged(value);
}

//-------------------------------------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------------------------------------//
//-----------------------------------COPY CUT PASTE ETC--------------------------------------------------------//
//-------------------------------------------------------------------------------------------------------------//

//this is a right-click
void MainWindow::ShowContextMenu(const QPoint& pos) // this is a slot
{
    // for most widgets
    QPoint globalPos = ui->tableWidget_1->mapToGlobal(pos);
    // for QAbstractScrollArea and derived classes you would use:
    // QPoint globalPos = myWidget->viewport()->mapToGlobal(pos);

    QMenu myMenu;
    myMenu.addAction(new QAction("Insert Empty Row", this));
    //connect(newAction, SIGNAL(triggered()), this, SLOT(insertEmptyRow()));
    myMenu.addAction(new QAction("Delete Row", this));
    // ...

    QAction* selectedItem = myMenu.exec(globalPos);
    if (selectedItem)
    {
        int row = ui->tableWidget_1->row(ui->tableWidget_1->itemAt(pos));
        ui->tableWidget_1->item(2,2)->setText(QString::number(row));
        // something was chosen, do stuff
    }
    else
    {
        // nothing was chosen
    }
}

QTableWidgetSelectionRange MainWindow::selectedRange() const
{
    QList<QTableWidgetSelectionRange> ranges = ui->tableWidget_1->selectedRanges();
    if (ranges.isEmpty())
        return QTableWidgetSelectionRange();
    return ranges.first();
}

//copies table content to clipboard for pasting
void MainWindow::on_actionCopy_triggered()
{
    QTableWidgetSelectionRange range = selectedRange();
    QString str;

    for (int i = 0; i < range.rowCount(); ++i) {
        if (i > 0)
            str += "\n";
        for (int j = 0; j < range.columnCount(); ++j) {
            if (j > 0)
                str += "\t";
            str += ui->tableWidget_1->item(range.topRow() + i, range.leftColumn() + j)->text();
        }
    }
    QApplication::clipboard()->setText(str);

}


//let's user paste into other row/column
void MainWindow::on_actionPaste_triggered()
{
    QTableWidgetSelectionRange range = selectedRange();
    QString str = QApplication::clipboard()->text();
    QStringList rows = str.split('\n');
    int numRows = rows.count();
    int numColumns = rows.first().count('\t') + 1;

    if (range.rowCount() * range.columnCount() != 1
            && (range.rowCount() != numRows
                || range.columnCount() != numColumns)) {

        /*QMessageBox::information(this, tr("Spreadsheet"),
                tr("The information cannot be pasted because the copy "
                   "and paste areas aren't the same size."));
                   */
        return;
    }

    for (int i = 0; i < numRows; ++i) {
        QStringList columns = rows[i].split('\t');
        for (int j = 0; j < numColumns; ++j) {
            int row = range.topRow() + i;
            int column = range.leftColumn() + j;
            if (row < ui->tableWidget_1->rowCount() && column < ui->tableWidget_1->columnCount())
                ui->tableWidget_1->item(row, column)->setText(columns[j]);
        }
    }
    //somethingChanged();
}

//lets user delete
void MainWindow::on_actionDelete_triggered()
{
    QList<QTableWidgetItem *> items = ui->tableWidget_1->selectedItems();
    if (!items.isEmpty()) {
        foreach (QTableWidgetItem *item, items)
            delete item;
        //somethingChanged();
    }
}

//Copies and Deletes at the same time
void MainWindow::on_actionCut_triggered()
{
    on_actionCopy_triggered();
    on_actionDelete_triggered();
}





//checkbox2 is "use actual SE actions" on interface. If clicked, Temperature Options disappear from Run Options Combo Box
void MainWindow::on_checkBox_2_clicked(bool checked)
{
    QStringList list;
    QStringList newActions;
    list = searchDashboard(ui->instrumentCombo->currentText());

    //creates the combo box on the mainwindow containing run options
    //only difference between this and initmaintable (line 50) is the temperature options are missing
    if (checked){
        newActions << " " << "Run" << "Run with SM" << "Kinetic run" << "Run PNR" << "Run PA" \
                << "Free text (OG)"\
                << "--------------"<< list \
                << "--------------"<< "Set Field" << "Run Transmissions";
       // QList<QComboBox*> combo = ;
        for (int r = 0; r < ui->tableWidget_1->rowCount(); r++) {
            QComboBox* box = qobject_cast<QComboBox*>(ui->tableWidget_1->cellWidget(r,0));
            box->clear();
            box->addItems(newActions);
        }

    } else {
        for (int row = 0; row < ui->tableWidget_1->rowCount(); row++) {
            QComboBox* box = qobject_cast<QComboBox*>(ui->tableWidget_1->cellWidget(row,0));
            box->clear();
            box->addItems(actions); //actions is a QStringList also used in line 50 that standard run options are written to
        }
    }
}



//??saves parameter in NRSample struct, don't get what it is doing to contrast change and temperature information
void MainWindow::on_actionOpen_Script_triggered()
{
    int line_count=0;
    int tableStart;
    QStringList sampleParameters;
    QStringList rowEntries;
    QString line[150];

    QString defaultLocation = QStandardPaths::locate(QStandardPaths::DesktopLocation, QString(), QStandardPaths::LocateDirectory);
    fileName = QFileDialog::getOpenFileName(this,tr("Open Script"), \
                                           defaultLocation, tr("Script files (*.scp)"));
    if (fileName!=""){
        QFile file(fileName);
        file.open(QIODevice::ReadOnly);
        QTextStream in(&file);
        //QString line = in.readAll();
        while(!in.atEnd())
        {
            line[line_count]=in.readLine(); //line Qstring temporarily stores file content
            if (line[line_count].contains("#TABLE"))
                    tableStart = line_count;
            line_count++;
        }
        file.close();
        QTextCharFormat boldFormat;
        boldFormat.setFontWeight(QFont::Bold);
        setWindowTitle("ScriptMax - " + file.fileName());
        for(int l=1; l < tableStart; l++){
            sampleParameters = line[l].split(','); //sampleParameters stores the parameters that are listed at the start of line Qstring
            if (sampleParameters.length() == 9){   //these parameters are then passed to a NRSample struct
                mySampleForm->currentSample = l-1;
                NRSample newSample;
                newSample.title = sampleParameters[0];
                newSample.translation = sampleParameters[1].toDouble();
                newSample.height = sampleParameters[2].toDouble();
                newSample.phi_offset = sampleParameters[3].toDouble();
                newSample.footprint = sampleParameters[4].toDouble();
                newSample.resolution = sampleParameters[5].toDouble();
                newSample.s3 = sampleParameters[6].toDouble();
                newSample.s4 = sampleParameters[7].toDouble();
                newSample.knauer = sampleParameters[8].toInt();
                mySampleForm->sampleList.append(newSample);
            } else {
                // pop up some error message
            }
        }

        //after the parameters the tablestarts, this info is now read
        //??don't really understand what this is doing
        for(int row=0; row < line_count; row++ ){
            QComboBox* box1 = qobject_cast<QComboBox*>(ui->tableWidget_1->cellWidget(row,0));
            rowEntries = line[row+tableStart+1].split(',');
            int index1 = box1->findText(rowEntries[0]); //findText returns index of item containing whatever is at rowEntries[0]
            if(index1>0){
                box1->setCurrentIndex(index1);
                if(box1->currentText() == "contrastchange"){
                    QComboBox* box2 = qobject_cast<QComboBox*>(ui->tableWidget_1->cellWidget(row,1));
                    box2->setCurrentIndex(box2->findText(rowEntries[1]));
                    for(int col = 2; col < 8; col++){
                        ui->tableWidget_1->item(row,col)->setText(rowEntries[col]);
                    }
                    QComboBox* box3 = qobject_cast<QComboBox*>(ui->tableWidget_1->cellWidget(row,8));
                    int index2 = box3->findText(rowEntries[8]);
                    box3->setCurrentIndex(index2);
                } else if(box1->currentText().contains("Set temperature")\
                          && rowEntries[1] == "Julabo Waterbath"){
                    QComboBox* box2 = qobject_cast<QComboBox*>(ui->tableWidget_1->cellWidget(row,1));
                    box2->setCurrentIndex(1);
                    box2->setCurrentIndex(0);
                    ui->tableWidget_1->item(row,2)->setText(rowEntries[2]);
                    QComboBox* box4 = qobject_cast<QComboBox*>(ui->tableWidget_1->cellWidget(row,3));
                    if (box4){
                        int index2 = box4->findText(rowEntries[3]);
                        box4->setCurrentIndex(index2);
                    }
                    for(int col = 4; col < 9; col++){
                        ui->tableWidget_1->item(row,col)->setText(rowEntries[col]);
                    }

                } else if(rowEntries.length()){
                    QComboBox* box2 = qobject_cast<QComboBox*>(ui->tableWidget_1->cellWidget(row,1));
                    box2->setCurrentIndex(box2->findText(rowEntries[1]));
                    for(int col = 2; col < 9; col++){
                        ui->tableWidget_1->item(row,col)->setText(rowEntries[col]);
                    }
                }
            }
        }
    }
}

//Asks if changes to be saved, then initMainTable()
void MainWindow::on_actionNew_Script_triggered()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("All changes will be lost.");
    msgBox.setInformativeText("Do you want to save your changes?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    int ret = msgBox.exec();

    switch (ret) {
      case QMessageBox::Save:
          // Save was clicked
          on_actionSave_Script_triggered();
          initMainTable();
          break;
      case QMessageBox::Discard:
          // Don't Save was clicked
          initMainTable();
          break;
      case QMessageBox::Cancel:
          // Cancel was clicked
          break;
      default:
          // should never be reached
          break;
    }
}

//this destructor is never actually used
MainWindow::~MainWindow()
{
    delete ui;
    delete mySampleForm;
}

//Quits Program
void MainWindow::on_actionQuit_triggered()
{
    on_actionNew_Script_triggered();
    QApplication::quit();
}



//Nothing Here
void MainWindow::on_instrumentCombo_activated(const QString &arg1)
{

}

//Reveals Documentation
void MainWindow::on_actionHow_To_triggered()
{
    QString str(QDir::currentPath());
    str.append("/ScriptMax Version 1_dcumentation.pdf");
    QDesktopServices::openUrl(QUrl::fromLocalFile(str));
    //QDesktopServices::openUrl(QUrl("file:///", QUrl::TolerantMode));
}

//Shows About Box
void MainWindow::on_actionAbout_ScriptMax_triggered()
{
    QMessageBox msgBox;
    msgBox.setText("ScriptMax v1.1.\nPlease use at your own risk!");
    msgBox.exec();
}

//Clears Table
void MainWindow::on_clearTableButton_clicked()
{
    bool sure;
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("Warning");
    msgBox.setInformativeText("Are you sure you want to clear the table? All entries will be lost."\
    "to clear the script, go to File>New Script");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Yes);
    int ret = msgBox.exec();
    switch (ret) {
       case QMessageBox::Yes:
           sure = true;
           break;
       case QMessageBox::Cancel:
           sure = false;
           break;
       default:
           // should never be reached
           break;
     }
    if (sure) initMainTable();
}



void MainWindow::closeEvent(QCloseEvent *event)
{
    if (areyousure()) {
        event->accept();
    } else {
        event->ignore();
    }
}

bool MainWindow::areyousure()
{
    if (ui->checkBox->isChecked() || mySampleForm->sampleList.isEmpty())
        return true;
    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, tr("Application"),
                               tr("The document has been modified.\n"
                                  "Are you sure you want to leave without saving?"),
                               QMessageBox::Yes | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Yes:
        return true;
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

//-------------------------------------------------------------------------------------------------------//
//--------------------------------SAVE STUFF-------------------------------------------------------------//
//-------------------------------------------------------------------------------------------------------//

void MainWindow::on_actionSave_GCL_file_triggered()
{
    if (ui->checkBox->isChecked())
        save();
    else{
        ui->checkBox->setChecked(true);
        ui->tabWidget->setCurrentIndex(1);
        on_checkBox_clicked(true);
        QMessageBox::information(this, "Save GCL file", "Please choose a file name and click 'save'.");
    }
}

void MainWindow::on_saveButton_clicked()
{
    save();
}

//used only once, at the end of Parsetable
void MainWindow::save(){


    //lineEdit is the Save As box,
    QString fileName = ui->lineEdit->text();

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Error","Could not save scriptfile.");
    } else {
        QTextStream stream(&file);
        stream << ui->plainTextEdit->toPlainText();
        stream.flush();
        file.close();
    }

}


//Tells the widget if it has been enabled or not in response to status of checkbox
void MainWindow::on_checkBox_clicked(bool checked)
{
    QString defaultLocation = QStandardPaths::locate(QStandardPaths::DesktopLocation, QString(), QStandardPaths::LocateDirectory);
    QDateTime local(QDateTime::currentDateTime());
    QString lastfileLoc = loadSettings("lastfileloc", defaultLocation, "savegroup").toString();
    QString fName = lastfileLoc + "runscript_" + local.toString("ddMMyy_hhmm");

    if (ui->OGButton->isChecked()) fName += ".gcl";
    else fName += ".py";

    if (checked){
        ui->lineEdit->setEnabled(true);
        ui->toolButton->setEnabled(true);
        ui->saveButton->setEnabled(true);
        ui->lineEdit->setText(fName);
    } else {
        ui->lineEdit->setEnabled(false);
        ui->toolButton->setEnabled(false);
        ui->saveButton->setEnabled(false);
    }
}

//"..." opens file Dialog
void MainWindow::on_toolButton_clicked()
{
    QDateTime local(QDateTime::currentDateTime());
    QString timestamp = local.toString("ddMMyy_hhmm");

    QString defaultLocation = QStandardPaths::locate(QStandardPaths::DesktopLocation, QString(), QStandardPaths::LocateDirectory);
    QString lastfileLoc = loadSettings("lastfileloc", defaultLocation, "savegroup").toString();

    QString fName;
    if (ui->OGButton->isChecked())
       fName = QFileDialog::getSaveFileName(this,tr("Save GCL"), \
                        lastfileLoc + "runscript_" + timestamp, tr("GCL files (*.gcl)"));
    else
        fName = QFileDialog::getSaveFileName(this,tr("Save GCL"), \
                                             lastfileLoc + "runscript_" + timestamp, tr("Python files (*.py)"));
    ui->lineEdit->setText(fName);
    QString saveloc = fName.left(fName.lastIndexOf("/") + 1);
    saveSettings("lastfileloc", saveloc, "savegroup");
}

void saveSettings(const QString &key, const QVariant &value, const QString &group)
{
    QSettings settings;
    settings.beginGroup(group);
    settings.setValue(key, value);
    settings.endGroup();
}

QVariant loadSettings(const QString &key, const QVariant &defaultValue = QVariant(), const QString &group = "default")
{
    QSettings settings;
    settings.beginGroup(group);
    QVariant value = settings.value(key, defaultValue);
    settings.endGroup();
    return value;
}

//makes document with only the most important infos. Need to delete or more clearly distinguish from save().
void MainWindow::on_actionSave_Script_triggered()
{
    //the fileName is obtained in line 1247 i think. interface does not give option to name. must be automatic
    if (fileName != "") {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            // error message
        } else {
            // SAVE SAMPLE INFORMATION...
            //================================================
            QTextStream out(&file);
            out << "#SAMPLES title, translation, height, phi_offset, footprint, resolution, s3, s4\n";
            for(int i = 0; i < mySampleForm->sampleList.length(); i++){
                out << mySampleForm->sampleList[i].title << ",";
                out << QString::number(mySampleForm->sampleList[i].translation) << ",";
                out << QString::number(mySampleForm->sampleList[i].height) << ",";
                out << mySampleForm->sampleList[i].phi_offset << ",";
                out << QString::number(mySampleForm->sampleList[i].footprint) << ",";
                out << QString::number(mySampleForm->sampleList[i].resolution) << ",";
                out << QString::number(mySampleForm->sampleList[i].s3) << ",";
                out << QString::number(mySampleForm->sampleList[i].s4) << ",";
                out << QString::number(mySampleForm->sampleList[i].knauer) << "\n";
            }
            out << "#TABLE\n";
            for(int row = 0; row < ui->tableWidget_1->rowCount(); row++){
                QComboBox* box1 = qobject_cast<QComboBox*>(ui->tableWidget_1->cellWidget(row,0));
                QComboBox* box2 = qobject_cast<QComboBox*>(ui->tableWidget_1->cellWidget(row,1));
                if(box1 && box2){
                        out << box1->currentText() << "," << box2->currentText(); // which action and which sample etc.
                    if(box1->currentText() == "contrastchange"){
                        for(int col = 2; col < 8; col++){
                            out << "," << ui->tableWidget_1->item(row,col)->text();
                        }
                        QComboBox* box3 = qobject_cast<QComboBox*>(ui->tableWidget_1->cellWidget(row,8));
                        out << "," << box3->currentText();
                        out << "\n";
                    } else if(box2->currentText().contains("Julabo Waterbath")){
                        out << "," << ui->tableWidget_1->item(row,2)->text();
                        QComboBox* box4 = qobject_cast<QComboBox*>(ui->tableWidget_1->cellWidget(row,3));
                        out << "," << box4->currentText();
                        for(int col = 4; col < 9; col++){
                            out << "," << ui->tableWidget_1->item(row,col)->text();
                        }
                        out << "\n";
                    } else {
                        for(int col = 2; col < 9; col++){
                            out << "," << ui->tableWidget_1->item(row,col)->text();
                        }
                        out << "\n";
                    }
                }
            }
        }
        file.close();
    }
    else{
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("You haven't specified a filename or directory.");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();

        switch (ret) {
          case QMessageBox::Save:
              // Save was clicked
              on_actionSave_Script_As_triggered();
              initMainTable();
              break;
          case QMessageBox::Cancel:
              // Cancel was clicked
              break;
          default:
              // should never be reached
              break;
        }
    }
}

//??Obtains filename and then executes onactionsavescripttriggered. Not sure how filename is obtained
void MainWindow::on_actionSave_Script_As_triggered()
{
    QString defaultLocation = QStandardPaths::locate(QStandardPaths::DesktopLocation, QString(), QStandardPaths::LocateDirectory);
    fileName = QFileDialog::getSaveFileName(this,tr("Save Script As..."), \
                                           defaultLocation, tr("Script files (*.scp)"));
    on_actionSave_Script_triggered();
}

//--------------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------------//
//----------------------------------------------NOT IN USE------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------------//

//opens genie and hands it the script using GX library
void MainWindow::runGenie(){

    QString qtStrData;
    QByteArray inBytes;
    const char *cStrData;


    //-------------------------
    char    s[256];

    int*    b;
    int dims[1];

    char bb[5];
        //float   cc[2];
        //double  dd[2];

    int ndims = 1;
    QString itemText = "";
/*
    qtStrData = ui->tableWidget_1->item(0,1)->text();
    inBytes = qtStrData.toUtf8();
    cStrData = inBytes.constData();
    GX_ASSIGN_HANDLE(cStrData,"");
*/
/*
    QTableWidgetItem* item = new QTableWidgetItem();
    item->setIcon(*(new QIcon("C:/Users/ktd43279/Documents/PROGS/MaxSCriptMaker_laptop/large_gears.gif")));
    ui->tableWidget_1->setVerticalHeaderItem(1,item);

    GX_select_source("C:/Users/ktd43279/Documents/PROGS/MaxSCriptMaker_laptop/INTER00025720.raw");
    GX_get("VV", "SPEC", 0);            // Integer array
    GX_assign_handle("_a", "printn(VV)");
    GX_ASSIGN_HANDLE("begin; waitfor seconds=10;abort","");
    GX_ASSIGN_HANDLE("rs", "GETRUNSTATE()");
    GX_ASSIGN_HANDLE("printn rs", "");


    while (itemText!="SETUP"){
        GX_ASSIGN_HANDLE("rs", "GETRUNSTATE()");
        dims[0] = 256;
        GX_transfer("rs", "-->", "STRING", s, &ndims, dims );
        itemText = QString::fromStdString(s);
        ui->tableWidget_1->item(5,1)->setText(itemText);
        printf("%s", "waiting...\n");
    }
    printf("%s", "FINISHED\n");
    itemText = QString::fromStdString(s);
    ui->tableWidget_1->item(5,1)->setText(itemText);

    for(int col=1; col<ui->tableWidget_1->columnCount();++col){
        ui->tableWidget_1->item(1,col)->setBackground(Qt::Dense4Pattern);
    }
    qtStrData = "VV["+ui->tableWidget_1->item(0,1)->text()+"]";
    inBytes = qtStrData.toUtf8();
    cStrData = inBytes.constData();


    dims[0]=5;
    ndims=1;
    GX_transfer("VV", "-->", "INT32[]", bb, &ndims, dims );
    printf("ARRAY: %c\n", bb[0]);
    QString txt = QString().sprintf("%c",bb[0]);
    ui->tableWidget_1->item(3,3)->setText(txt);
    //-------------------------


    QStringList SECIblocks = searchDashboard(ui->instrumentCombo->currentText());
    ui->comboBox->addItems(SECIblocks);

*/


    //GX_ASSIGN_HANDLE(cStrData,"");

    //GX_get("V1", "", 2);            // Integer#
    //GX_assign_handle("VV=42", "");
    //GX_GET("VV","",0);
    //GX_ASSIGN_HANDLE("__th", "printn(VV)");
    //GX_transfer("VV", "-->", "INT64", &b, 0, 0);

    //ui->tableWidget_1->item(0,8)->setText(QString::number(b));



    /*
    QProcess *process = new QProcess(this);
    //QString file = "%GENIE_DIR%\\tkgenie32.exe -l";
    save();
    QString file = "\"C:\\Users\\ktd43279\\Max_tkgenie32.bat\"";
    process->start(file);
    //system("start "\"C:\\Program Files (x86)\\CCLRC ISIS Facility\\Open GENIE\\tkgenie32.bat\""");
    */


}

//Customise Commands Field
/*
void MainWindow::on_OG_checkBox_stateChanged(int arg1)
{
    int handle;
    switch (arg1){
        case 0: // unchecked
            handle = GX_DEACTIVATE_SESSION();
            break;
        case 2: // checked
            // ACTIVATE GENIE SESSION
            handle = GX_ACTIVATE_SESSION("og",1,1);
            GX_ASSIGN_HANDLE("ISC_SETUP(\"INTER\")","");

            //GX_ASSIGN_HANDLE("w","get(1,\"C:/Users/ktd43279/Documents/PROGS/MaxSCriptMaker_laptop/INTER00025720.raw\")");
            //GX_ASSIGN_HANDLE("printn w.y[1]","");
            break;
    }
}


void MainWindow::on_OGcmd_pushButton_clicked()
{
    QString qtStrData;
    QByteArray inBytes;
    const char *cStrData;


    qtStrData = ui->OGcmd_lineEdit->text();
    inBytes = qtStrData.toUtf8();
    cStrData = inBytes.constData();
    GX_ASSIGN_HANDLE(cStrData,"");

}
*/

//searches web dashboard for SE options
QStringList MainWindow::searchDashboard(QString instrument){

    //these are unused as of now. QRegExp is usually used for searching, validating
    QRegExp groups("<span style=\"font-weight:bold;\">([a-zA-Z0-9 ]+)</span>");
    QRegExp blocks("<li>([a-zA-Z0-9 _]+):");

    QStringList list;
    QStringList list2;

   //accidently removed a line

    //qWarning() << list;
    return list;


}

void MainWindow::on_PythonButton_clicked()
{
    pyhighlighter = new KickPythonSyntaxHighlighter(ui->plainTextEdit->document());
    writeBackbone();
}


void MainWindow::on_OGButton_clicked()
{
    OGhighlighter = new Highlighter(ui->plainTextEdit->document());
    writeBackbone();
}


void MainWindow::ProgressBar(int secs, int row){


    bar = new QProgressBar();
    bar->setMinimumSize(73, ui->tableWidget_1->rowHeight(1));//need this to fill box
    ui->tableWidget_1->setCellWidget(row,10,bar);

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateProgBar(row)));
    timer->start(secs*10);

}

void MainWindow::updateProgBar(int row){

    if(counter <= 100)
    {
        counter++;
        bar->setValue(counter);
    }
    else
    {
        timer->stop();
        delete timer;
        ui->tableWidget_1->removeCellWidget(row,10);

        ui->tableWidget_1->setIconSize(QSize(90,ui->tableWidget_1->rowHeight(0)));
        QTableWidgetItem *icon_item = new QTableWidgetItem;

        for (int col = 1; col < 10; col++)
               ui->tableWidget_1->item(row,col)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

        icon_item->setIcon(QIcon(":/tick.png"));
        ui->tableWidget_1->setItem(row, 10, icon_item);

    }

}
