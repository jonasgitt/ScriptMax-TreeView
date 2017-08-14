#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sampletable.h"
#include "GCLHighLighter.h"
#include "QProcess"
#include "QLibrary"
#include <QTextStream>
#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
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
#include "tree_item.h"
#include "tree_model.h"
#include "comboboxdelegate.h"

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
    pyhighlighter = new KickPythonSyntaxHighlighter(ui->PyScriptBox->document());

    mySampleTable = new SampleTable(); // Be sure to destroy this window somewhere
    initTree();

    connect(mySampleTable,SIGNAL(closedSampWindow()), SLOT(disableRows()));

    parseTable();

}


void MainWindow::writeBackbone(){

    ui->plainTextEdit->clear();
    pyWriteBackbone();

    QString BFileName;
    //Get Backbone from .txt file
    if (ui->OGButton->isChecked())
        BFileName = ":/OGbackbone.txt";
    else if (ui->PythonButton->isChecked())
        BFileName = ":/PyBackbone.txt";

    QFile BFile(BFileName);

    if (!BFile.open(QFile::ReadOnly | QFile::Text)){
           QMessageBox::warning(this, "Error" , "Couldn't open OpenGenie Backbone File");
    }

    QTextStream in(&BFile);
    QString OGtext = in.readAll();
    ui->plainTextEdit->setPlainText(OGtext);

    BFile.close();


    ui->plainTextEdit->find("GLOBAL runTime"); //positions the cursor to insert instructions
    ui->plainTextEdit->moveCursor(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
    for (int i=0; i < mySampleTable->sampleList.length(); i++){
        ui->plainTextEdit->insertPlainText(" s" + QString::number(i+1)); // sample numbering starts with 1
    }
}

void MainWindow::pyWriteBackbone(){

    ui->PyScriptBox->clear();
    QString BFileName;
    BFileName = ":/PyBackbone.txt";

    QFile BFile(BFileName);

    if (!BFile.open(QFile::ReadOnly | QFile::Text)){
           QMessageBox::warning(this, "Error" , "Couldn't open Python Backbone File");
    }

    QTextStream in(&BFile);
    QString Pytext = in.readAll();
    ui->PyScriptBox->setPlainText(Pytext);

    BFile.close();

    ui->PyScriptBox->find("def runscript()"); //positions the cursor to insert instructions
    ui->PyScriptBox->moveCursor(QTextCursor::Down, QTextCursor::MoveAnchor);
}

void MainWindow::parseTable(){

    //finds all comboboxes that are children of the table
    //this won't include other comboxes which are themselves children of comboboxes

    QString scriptLine; // a string that temporarily stores info before adding to script
    int sendingRow;


    // initialise run time to 0:
    runTime = runTime.fromString("00:00", "hh:mm");

    // prepare script header
    writeBackbone();

    ui->plainTextEdit->find("runTime=0"); //positions the cursor to insert instructions
    ui->plainTextEdit->moveCursor(QTextCursor::Down, QTextCursor::MoveAnchor);


    //FOR LOOP THROUGH TREE
        int whatAction = 0; int row;
        switch(whatAction)
        {
            case 0:
                break;
            case 1:
                normalRun(row, false);
                break;
            case 2: // run with supermirror
               { normalRun(row, true);}
                break;
            case 3:// run kinetic
                kineticRun(row);
                break;
            case 6: // free OpenGenie command
                OGcommand(row);
                break;
            case 8: // contrastChange
                {contrastChange(row);
                break;}
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

    samplestoPlainTextEdit();
    if(ui->checkBox->isChecked()){
        save(OPENGENIE);
    }
    if(ui->PySaveCheckBox->isChecked())
        save(PYTHON);

}
//------------------------------------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------------------//
//---------------------------------RUNOPTIONS-----------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------------------//
void MainWindow::samplestoPlainTextEdit(){
    ui->plainTextEdit->find("#do not need to be changed during experiment."); //positions the cursor to insert instructions
    ui->plainTextEdit->moveCursor(QTextCursor::Down, QTextCursor::MoveAnchor);
    ui->plainTextEdit->moveCursor(QTextCursor::Down, QTextCursor::MoveAnchor); //move cursor down one more line
    QList<NRSample> samples = mySampleTable->sampleList;
    ui->plainTextEdit->insertPlainText(writeSamples(samples));
}


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


void MainWindow::openSampleTable()
{
//    if(mySampleTable->currentSample > 0){
        mySampleTable->displaySamples();
    mySampleTable->show();


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
                mySampleTable->currentSample = l-1;
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
                mySampleTable->sampleList.append(newSample);
            } else {
                // pop up some error message
            }
        }

      //INSert commands into treeview
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
    delete mySampleTable;
}

//Quits Program
void MainWindow::on_actionQuit_triggered()
{
    on_actionNew_Script_triggered();
    QApplication::quit();
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
    if (sure) initTree();
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
    if (ui->checkBox->isChecked() || mySampleTable->sampleList.isEmpty())
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
        save(OPENGENIE);
    else{
        ui->checkBox->setChecked(true);
        ui->tabWidget->setCurrentIndex(1);
        on_checkBox_clicked();
        QMessageBox::information(this, "Save GCL file", "Please choose a file name and click 'save'.");
    }
}

void MainWindow::on_saveButton_clicked()
{
    save(OPENGENIE);
}
void MainWindow::on_PySaveButton_clicked()
{
    save(PYTHON);
}


void MainWindow::save(bool OGorPy){

    QString fileName;

    if (OGorPy == OPENGENIE)
        fileName = ui->lineEdit->text();
    else
        fileName = ui->PySaveLineEdit->text();

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Error","Could not save scriptfile.");
    } else {
        QTextStream stream(&file);
        if (OGorPy == OPENGENIE) stream << ui->plainTextEdit->toPlainText();
        else stream << ui->PyScriptBox->toPlainText();
        stream.flush();
        file.close();
    }

}

void MainWindow::on_checkBox_clicked()
{
    QDateTime local(QDateTime::currentDateTime());
    QString fileLoc = loadSettings().toString();
    qDebug() << "OGFileLoc" << fileLoc;
    QString fName = fileLoc + "runscript_" + local.toString("ddMMyy_hhmm") + ".gcl";

    if (ui->checkBox->isChecked()){
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

void MainWindow::on_PySaveCheckBox_clicked()
{
    QDateTime local(QDateTime::currentDateTime());
    QString fileLoc = loadSettings().toString();
    QString fName = fileLoc + "runscript_" + local.toString("ddMMyy_hhmm") + ".py";

    if (ui->PySaveCheckBox->isChecked()){
        ui->PySaveLineEdit->setEnabled(true);
        ui->PyToolButton->setEnabled(true);
        ui->PySaveButton->setEnabled(true);
        ui->PySaveLineEdit->setText(fName);
    } else {
        ui->PySaveLineEdit->setEnabled(false);
        ui->PyToolButton->setEnabled(false);
        ui->PySaveButton->setEnabled(false);
    }

}

void MainWindow::on_toolButton_clicked()
{
    SaveToolButtons(OPENGENIE);
}
void MainWindow::on_PyToolButton_clicked()
{
    SaveToolButtons(PYTHON);
}

void MainWindow::SaveToolButtons(bool OGorPy){

    QDateTime local(QDateTime::currentDateTime());
    QString timestamp = local.toString("ddMMyy_hhmm");

    QString lastfileLoc = loadSettings().toString();

    QString fName;
    if (OGorPy == OPENGENIE){
       fName = QFileDialog::getSaveFileName(this,tr("Save GCL"), \
                        lastfileLoc + "runscript_" + timestamp, tr("GCL files (*.gcl)"));
        ui->lineEdit->setText(fName);
    }
    else{
        fName = QFileDialog::getSaveFileName(this,tr("Save GCL"), \
                                                lastfileLoc + "runscript_" + timestamp, tr("Python files (*.py)"));
        ui->PySaveLineEdit->setText(fName);
    }

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

QVariant loadSettings()
{
    QString defaultLocation = QStandardPaths::locate(QStandardPaths::DesktopLocation, QString(), QStandardPaths::LocateDirectory);
    QSettings settings;
    QVariant testloc = settings.value("lastfileloc", defaultLocation);

    return testloc;
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
            for(int i = 0; i < mySampleTable->sampleList.length(); i++){
                out << mySampleTable->sampleList[i].title << ",";
                out << QString::number(mySampleTable->sampleList[i].translation) << ",";
                out << QString::number(mySampleTable->sampleList[i].height) << ",";
                out << mySampleTable->sampleList[i].phi_offset << ",";
                out << QString::number(mySampleTable->sampleList[i].footprint) << ",";
                out << QString::number(mySampleTable->sampleList[i].resolution) << ",";
                out << QString::number(mySampleTable->sampleList[i].s3) << ",";
                out << QString::number(mySampleTable->sampleList[i].s4) << ",";
                out << QString::number(mySampleTable->sampleList[i].knauer) << "\n";
            }
            out << "#TABLE\n";
            //TODO:TREEVIEW SAVE INFO
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
    //bar->setMinimumSize(73, ui->tableWidget_1->rowHeight(1));//need this to fill box
   // ui->tableWidget_1->setCellWidget(row,10,bar);

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
      //  ui->tableWidget_1->removeCellWidget(row,10);

        //ui->tableWidget_1->setIconSize(QSize(90,ui->tableWidget_1->rowHeight(0)));
        QTableWidgetItem *icon_item = new QTableWidgetItem;

        icon_item->setIcon(QIcon(":/tick.png"));
        //ui->tableWidget_1->setItem(row, 10, icon_item);

    }

}

//--------------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------------//
//----------------------------------------------TREEVIEW------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------------//


void MainWindow::initTree(){

    QStringList headers;
    headers << tr("Action") << tr("Summary of Command");
    TreeModel *model = new TreeModel(headers);
    ui->TreeView->setModel(model);
    ui->TreeView->setAnimated(true);

    ui->TreeView->setDragDropMode(QAbstractItemView::InternalMove);
    ui->TreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->TreeView->setDragEnabled(true);
    ui->TreeView->setDragDropOverwriteMode(false);
    ui->TreeView->setAcceptDrops(true);
    ui->TreeView->setDropIndicatorShown(true);//doesnt do anything*/
    ui->TreeView->setDefaultDropAction(Qt::MoveAction);

   insertChild("Choose a Command...");


    ui->TreeView->setItemDelegateForColumn(0, new ComboBoxDelegate(ui->TreeView));

    bool connection = connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), SLOT(updateComboSlot(QModelIndex)));
    qDebug() << "MainWindow Connection: " << connection;

    for (int column = 0; column < model->columnCount(); ++column)
      ui->TreeView->resizeColumnToContents(column);
}


void MainWindow::updateComboSlot(QModelIndex topLeft){

    QVariant newdata = ui->TreeView->model()->data(topLeft);
    if (newdata == "Choose a Command...") return;

   while (ui->TreeView->model()->hasChildren(topLeft)){
       ui->TreeView->model()->removeRow(0, topLeft);
   }

   if (!ui->TreeView->model()->parent(topLeft).isValid() && topLeft.column() == 0){
       InsertParameters(parameterList(newdata));
   }

}

QStringList MainWindow::parameterList(QVariant runOption){

    QStringList parameters;

    if (runOption == "Run" || runOption == "Run with SM") {
        parameters << "Sample#" << "uAmps 3" << "Angle 3" << "uAmps 2" << "Angle 2" << "uAmps 1" << "Angle 1";
     }
    else if (runOption == "Run Transmissions"){
        parameters << "Subtitle" << "uAmps" << "s4vg" << "s3vg" << "s2vg" << "s1vg" << "Height Offset";
    }
    else if (runOption == "NIMA Pressure"){
        parameters << "Target Pressure";
    }
    else if (runOption == "NIMA Area"){
        parameters << "Target Area";
    }
    else if (runOption == "Contrast Change"){
        parameters << "Conc A" << "Volume[mL]" << "Flow[mL/min]" << "Conc D" << "Conc C" << "Conc B";;
    }
    else if (runOption == "Julabo"){
        parameters << "Temperature" << "Run Control Min" << "Run Control Max";
    }
    else if (runOption == "Eurotherm"){
        parameters << "T1" << "T8" << "T7" << "T6" << "T5" << "T4" << "T3" << "T2";
    }
    return parameters;
}

void MainWindow::insertChild(QString ChildTitle)
{
    QModelIndex index = ui->TreeView->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->TreeView->model();

    if (model->columnCount(index) == 0) {
        if (!model->insertColumn(0, index))
            return;
    }

    if (!model->insertRow(0, index))
        return;

    //for (int column = 0; column < model->columnCount(index); ++column) {
        QModelIndex child = model->index(0, 0, index);
        model->setData(child, QVariant(ChildTitle), Qt::EditRole);
        //if (!model->headerData(column, Qt::Horizontal).isValid())
          //  model->setHeaderData(column, Qt::Horizontal, QVariant("[No header]"), Qt::EditRole);
   //}

    ui->TreeView->selectionModel()->setCurrentIndex(model->index(0, 0, index),
                                            QItemSelectionModel::ClearAndSelect);

}


void MainWindow::on_newCommand_clicked()
{
   QAbstractItemModel *model = ui->TreeView->model();

   if (!model->index(0,0).isValid()){ //if table is empty
       insertChild("Choose a Command...");
       return;
   }

   int insertionRow;
  QModelIndex selectedIndex = ui->TreeView->selectionModel()->currentIndex();

   if (!ui->TreeView->model()->parent(selectedIndex).isValid()){
           insertionRow = selectedIndex.row()+1;
            model->insertRow(insertionRow);
            qDebug() << "I'm in parent";
   }
   else{
       insertionRow = model->rowCount();
       model->insertRow(insertionRow);
       qDebug() << "I's in child";
   }

   QModelIndex child = model->index(insertionRow, 0);
   model->setData(child, QVariant("Choose a Command..."));
}

//PROBLEM: this finds the index of what ever is selected (which depends on the user!!) ... is this even a problem?
void MainWindow::insertRow(QString Action)
{
    QModelIndex index = ui->TreeView->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->TreeView->model();

    if (!model->insertRow(index.row()+1, index.parent()))//this is where insertion happens
        return;



    for (int column = 0; column < model->columnCount(index.parent()); ++column) {
        QModelIndex child = model->index(index.row()+1, column, index.parent());
        model->setData(child, QVariant(Action), Qt::EditRole);
    }
}


bool MainWindow::removeColumn()
{
    QAbstractItemModel *model = ui->TreeView->model();
    int column = ui->TreeView->selectionModel()->currentIndex().column();

    // Insert columns in each child of the parent item.
    bool changed = model->removeColumn(column);

    return changed;
}

void MainWindow::InsertParameters(QStringList parameters){

    //take the first parameter and add it as a child
    QString parameter = parameters.takeFirst();
    insertChild(parameter);

    foreach (parameter, parameters){
        insertRow(parameter);
    }
}


bool MainWindow::insertColumn()
{
    QAbstractItemModel *model = ui->TreeView->model();
    int column = ui->TreeView->selectionModel()->currentIndex().column();

    // Insert a column in the parent item.
    bool changed = model->insertColumn(column + 1);
    if (changed)
        model->setHeaderData(column + 1, Qt::Horizontal, QVariant("[No header]"), Qt::EditRole);


    return changed;
}

void MainWindow::removeRow()
{
    on_removeCommands_clicked();
}


void MainWindow::parseModel(){

    QVector<QVariant> params;
    runstruct runvars;
    QAbstractItemModel *model = ui->TreeView->model();

    QModelIndex childIndex = model->index(0,0, QModelIndex());
    QModelIndex grandChild = model->index(0,1, childIndex);
    qDebug() << "Grandchild Data: " << model->data(grandChild).toString();
    qDebug() << "RootItems Child Count: " << model->rowCount(QModelIndex());

    for (int row = 0; row < model->rowCount(QModelIndex()); row++){

         QModelIndex comboIndex = model->index(row,0, QModelIndex());
         QString comboSelected = model->data(comboIndex).toString();
         qDebug() << "Combo Selected: " << comboSelected;

         for (int par = 0; par < model->rowCount(comboIndex); par++){

             QModelIndex parIndex = model->index(par, 1, comboIndex);// 0 for testing
             QString parameter = model->data(parIndex, Qt::EditRole).toString();
             qDebug() << "Data: " << parameter;

             params.append(model->data(parIndex, Qt::EditRole));
         }

         if (comboSelected == "Run")
            parseRun(params);
         else if (comboSelected == "Contrast Change")
            parseContrast(params);
         else if (comboSelected == "NIMA Pressure")
             parseNIMA_P(params);
         else if (comboSelected == "NIMA Area")
             parseNIMA_A(params);
         else if (comboSelected == "Eurotherm")
             parseEurotherm(params);
         else if (comboSelected == "Julabo")
             parseJulabo(params);
         else if (comboSelected == "Run Transmissions")
             parseTransm(params);
    }
}


//on collapse and DnD, parse all, print all


void MainWindow::on_parseCommands_clicked()
{
     parseModel();
}

void MainWindow::on_removeCommands_clicked()
{
    QAbstractItemModel *model = ui->TreeView->model();
    if (!model->index(0,0).isValid()) return;

    QModelIndex index = ui->TreeView->selectionModel()->currentIndex();
    if (!ui->TreeView->model()->parent(index).isValid()){
        if (model->removeRow(index.row(), index.parent()))
            qDebug() << "FIx this";
}
}

