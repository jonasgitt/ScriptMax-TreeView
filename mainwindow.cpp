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

    OGhighlighter = new Highlighter(ui->plainTextEdit->document());
    pyhighlighter = new KickPythonSyntaxHighlighter(ui->PyScriptBox->document());

    mySampleTable = new SampleTable();

    initTree();
    parseTree();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete mySampleTable;
}

void MainWindow::updateRunTime(double uAmps){
    int secs =0 ;

    if(ui->instrumentCombo->currentText() == "CRISP" || ui->instrumentCombo->currentText() == "SURF"){
        secs += static_cast<int>(uAmps/160*3600); // TS2
    } else {
        secs += static_cast<int>(uAmps/40*3600); // TS2
    }

    runTime = runTime.addSecs(secs);
    ui->timeEdit->setTime(runTime);
    return;
}


void MainWindow::on_sampleTableButton_clicked()
{
    mySampleTable->displaySamples();
    mySampleTable->show();

}

//------------------------------------------------------------------------------------------------------//
//------------------------LOADING DATA FROM FILE--------------------------------------------------------//
void MainWindow::on_actionOpen_Script_triggered()
{


    QString defaultLocation = QStandardPaths::locate(QStandardPaths::DesktopLocation, QString(), QStandardPaths::LocateDirectory);
    QString fName = QFileDialog::getOpenFileName(this,tr("Open Script"), \
                                                 defaultLocation, tr("Script files (*.txt)"));
    QFile file(fName);
    file.open(QIODevice::ReadOnly);
    QString data = file.readAll();
    file.close();

    initTree(data);
    initSampleTable(data);
}

void MainWindow::initSampleTable(QString data){

    mySampleTable->sampleList.clear();
    int samplesStart;
    QStringList lines = data.split(QString("\n"));
    for (int i = 0; i < lines.length(); i++)
    {
        if (lines[i].contains("#SAMPLES"))
                samplesStart = i + 1;
    }

    QStringList sampleParameters;

    for (int line = samplesStart; line < lines.length(); line++){
        sampleParameters = lines[line].split(',');
        if (sampleParameters.length() > 8){
            mySampleTable->currentSample = line-samplesStart;
            NRSample newSample;
            newSample.title = sampleParameters[0];
            newSample.translation = sampleParameters[1].toDouble();
            newSample.height = sampleParameters[2].toDouble();
            newSample.phi_offset = sampleParameters[3];
            newSample.psi = sampleParameters[4].toDouble();
            newSample.footprint = sampleParameters[5].toDouble();
            newSample.resolution = sampleParameters[6].toDouble();
            newSample.s3 = sampleParameters[7].toDouble();
            newSample.s4 = sampleParameters[8].toDouble();
            newSample.knauer = sampleParameters[9].toInt();
            newSample.coarse_noMirror = sampleParameters[10].toInt();
            mySampleTable->sampleList.append(newSample);
        }
        line++;
    }

}
//------------------------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------//

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
          initTree();
          break;
      case QMessageBox::Discard:
          // Don't Save was clicked
          initTree();
          break;
      case QMessageBox::Cancel:
          // Cancel was clicked
          break;
      default:
          // should never be reached
          break;
    }
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
void MainWindow::on_actionSave_Python_Script_triggered()
{
    if (ui->PySaveCheckBox->isChecked())
        save(PYTHON);
    else{
        ui->PySaveCheckBox->setChecked(true);
        ui->tabWidget->setCurrentIndex(2);
        on_checkBox_clicked();
        QMessageBox::information(this, "Save Python file", "Please choose a file name and click 'save'.");
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

}

void MainWindow::on_actionSave_Script_As_triggered()
{
    QString defaultLocation = QStandardPaths::locate(QStandardPaths::DesktopLocation, QString(), QStandardPaths::LocateDirectory);
    fileName = QFileDialog::getSaveFileName(this,tr("Save Script As..."), \
                                           defaultLocation, tr("Script files (*.txt)"));

    QFile file(fileName);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << saveTreeString() << saveSamplesString();
    } else {
            //error message
    }
    file.close();
}

QString MainWindow::saveTreeString(){

    QAbstractItemModel *model = ui->TreeView->model();
    QModelIndex p_index, c_index;
    QString str;

    for (int p_row = 0; p_row < model->rowCount(); p_row++){

        for (int p_col = 0; p_col < 2; p_col++){
            p_index = model->index(p_row, p_col, QModelIndex());
            str += model->data(p_index).toString();
            if (p_col == 0) str += "\t";
        }
        str += "\n";

        p_index = model->index(p_row, 0);
        for (int c_row = 0; c_row < model->rowCount(p_index); c_row++){
            c_index = model->index(c_row, 0, p_index);
            str += "    " + model->data(c_index).toString();
            c_index = model->index(c_row, 1, p_index);
            str += "\t" + model->data(c_index).toString() + "\n";
        }
    }
    return str;

}

QString MainWindow::saveSamplesString(){

    QString str;
    str = "SAMPLEDATA\n\n";
    str += "#SAMPLES title, translation, height, phi_offset, psi, footprint, resolution, s3, s4, knauer, coarse_noMirror\n";
    for(int i = 0; i < mySampleTable->sampleList.length(); i++){
        str += mySampleTable->sampleList[i].title + ",";
        str += QString::number(mySampleTable->sampleList[i].translation) + ",";
        str += QString::number(mySampleTable->sampleList[i].height) + ",";
        str += mySampleTable->sampleList[i].phi_offset + ",";
        str += QString::number(mySampleTable->sampleList[i].psi) + ",";
        str += QString::number(mySampleTable->sampleList[i].footprint) + ",";
        str += QString::number(mySampleTable->sampleList[i].resolution) + ",";
        str += QString::number(mySampleTable->sampleList[i].s3) + ",";
        str += QString::number(mySampleTable->sampleList[i].s4) + ",";
        str += QString::number(mySampleTable->sampleList[i].knauer) + ",";
        str += QString::number(mySampleTable->sampleList[i].coarse_noMirror) + "\n";
    }

    return str;
}
//------------------------------------------END OF SAVE STUFF---------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------------//
//----------------------------------------------TREEVIEW------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------------//


void MainWindow::initTree(QString data){

    QStringList headers;
    headers << tr("Action") << tr("Summary of Command") << tr("Input Valid") << tr("Further Information");

    TreeModel *model;
    if (data == ""){
        model = new TreeModel(headers);
    }
    else{
        clearChildren();
        model = new TreeModel(headers , data);
    }

    ui->TreeView->setModel(model);
    ui->TreeView->setAnimated(true);

    //======DRAGDROP STUFF -- NOT WORKING===================================//
    ui->TreeView->setDragDropMode(QAbstractItemView::InternalMove);
    ui->TreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->TreeView->setDragEnabled(true);
    ui->TreeView->setDragDropOverwriteMode(false);
    ui->TreeView->setAcceptDrops(true);
    ui->TreeView->setDropIndicatorShown(true);//doesnt do anything*/
    ui->TreeView->setDefaultDropAction(Qt::MoveAction);

   //======================================================================//

    if (data == "")
       insertChild("Choose a Command...");

    ui->TreeView->setItemDelegateForColumn(0, new ComboBoxDelegate(ui->TreeView));

    bool connection = connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), SLOT(updateComboSlot(QModelIndex)));
    bool connekt = connect(mySampleTable,SIGNAL(closedSampWindow()), SLOT(updateSampleBoxes()));
    qDebug() << "MainWindow Connections: " << connection << connekt;

    ui->TreeView->setEditTriggers(QAbstractItemView::AnyKeyPressed);
    ui->TreeView->setEditTriggers(QAbstractItemView::DoubleClicked);
    ui->TreeView->setColumnWidth(0, 150);
    ui->TreeView->setColumnWidth(1, 170);
    ui->TreeView->setColumnWidth(2, 65);
}

void MainWindow::clearChildren(const QModelIndex &parent){
    while (ui->TreeView->model()->hasChildren(parent)){
        ui->TreeView->model()->removeRow(0, parent);
    }
}

void MainWindow::updateComboSlot(QModelIndex topLeft){

    if (topLeft.column() != 0){
        return;
}

    QVariant newdata = ui->TreeView->model()->data(topLeft);
    if (newdata == "Choose a Command...") return;

   clearChildren(topLeft);

   if (!ui->TreeView->model()->parent(topLeft).isValid() && topLeft.column() == 0){
       InsertParameters(parameterList(newdata));
   }

}
bool MainWindow::WarningMessage(QString message){
    QMessageBox::warning(this, tr("Max Script Maker"),
                                   message,
                                   QMessageBox::Ok);
    return true;
}

QStringList MainWindow::parameterList(QVariant runOption){

    QStringList parameters;

    if (runOption == "Run" || runOption == "Run with SM") {

        if (mySampleTable->sampleList.isEmpty()){
            WarningMessage("You must define a sample first.");
            return parameters;
        }
        parameters << "Sample" << "uAmps 3" << "Angle 3" << "uAmps 2" << "Angle 2" << "uAmps 1" << "Angle 1" << "Subtitle";
     }
    else if (runOption == "Run Transmissions"){
        parameters << "Sample" << "uAmps" << "s4vg" << "s3vg" << "s2vg" << "s1vg" << "Height Offset" << "Subtitle";
    }
    else if (runOption == "NIMA Pressure"){
        parameters << "Target Pressure";
    }
    else if (runOption == "NIMA Area"){
        parameters << "Target Area";
    }
    else if (runOption == "Contrast Change"){
        parameters << "Sample" << "Volume[mL]" << "Flow[mL/min]" << "Conc D" << "Conc C" << "Conc B" << "Conc A";
    }
    else if (runOption == "Julabo"){
        parameters << "Temperature" << "Run Control Min" << "Run Control Max";
    }
    else if (runOption == "Eurotherm"){
        parameters << "T1" << "T8" << "T7" << "T6" << "T5" << "T4" << "T3" << "T2";
    }
    else if (runOption == "Free Command")
        parameters << "Python" << "OpenGenie";

    return parameters;
}

void MainWindow::InsertParameters(QStringList parameters){

    //take the first parameter and add it as a child
    if (parameters.isEmpty()) return;

    QString parameter = parameters.takeFirst();
    insertChild(parameter);

    foreach (parameter, parameters){
        insertRow(parameter);
    }
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
    QModelIndex child = model->index(0, 0, index);

    model->setData(child, QVariant(ChildTitle), Qt::EditRole);

    if(ChildTitle == "Sample")
        setSampleComboBox(model->index(0, 1, index));

    ui->TreeView->selectionModel()->setCurrentIndex(model->index(0, 0, index),
                                            QItemSelectionModel::ClearAndSelect);

}

void MainWindow::setSampleComboBox(QModelIndex comboIndex){

    QStringList samples;

    for (int i = 0; i < mySampleTable->sampleList.length(); i++){
        samples << mySampleTable->sampleList[i].title;
    }
    QComboBox *SampleBox = new QComboBox();
    SampleBox->addItems(samples);
    ui->TreeView->setIndexWidget(comboIndex, SampleBox);
}

void MainWindow::updateSampleBoxes(){ //SLOT responding to table closure

    QAbstractItemModel *model = ui->TreeView->model();
  for (int row = 0; row < model->rowCount(); row++){

       QModelIndex RowIndex = model->index(row,0, QModelIndex()); //parent row
       QModelIndex comboIndex = model->index(0, 1, RowIndex); //
      QString option = model->data(RowIndex).toString();
        //if (qobject_cast<QComboBox*>(ui->TreeView->indexWidget(comboIndex))){
        //  setSampleComboBox(comboIndex);
        //}
        if(option.contains("Run") || option == "Contrast Change")
            setSampleComboBox(comboIndex);
    }
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
   }
   else{
       insertionRow = model->rowCount();
       model->insertRow(insertionRow);
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

    for (int column = 0; column < 2; ++column) {
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

//need to update runtime for run and runtrans
void MainWindow::parseTree(){

    ui->plainTextEdit->moveCursor(QTextCursor::Start);
    ui->PyScriptBox->moveCursor(QTextCursor::Start);

    runTime = runTime.fromString("00:00", "hh:mm");

    writeBackbone();

    samplestoPlainTextEdit();
    samplestoPyTextEdit();

    ui->plainTextEdit->moveCursor(QTextCursor::Start);
    bool foundr = ui->plainTextEdit->find("runTime=0"); qDebug() << "found runtime " << foundr;
    ui->plainTextEdit->moveCursor(QTextCursor::Down, QTextCursor::MoveAnchor);

    ui->PyScriptBox->moveCursor(QTextCursor::Start);
    bool found = ui->PyScriptBox->find("#Script body begins here:"); qDebug() << "found py " << found;
    ui->PyScriptBox->moveCursor(QTextCursor::Down, QTextCursor::MoveAnchor);
    ui->PyScriptBox->moveCursor(QTextCursor::Down, QTextCursor::MoveAnchor);

    QAbstractItemModel *model = ui->TreeView->model();

    for (int row = 0; row < model->rowCount(QModelIndex()); row++){
        QModelIndex comboIndex = model->index(row,0, QModelIndex());
        QString comboSelected = model->data(comboIndex).toString();
        printCommands(comboSelected, getChildData(row), row);//make print commands a bool?
    }

    if(ui->checkBox->isChecked()){
        save(OPENGENIE);
    }
    if(ui->PySaveCheckBox->isChecked())
        save(PYTHON);
}

QVector<QVariant> MainWindow::getChildData(int parentRow){

    QAbstractItemModel *model = ui->TreeView->model();
    QVector<QVariant> data;
    QModelIndex comboIndex = model->index(parentRow,0, QModelIndex());

    for (int childRow = 0; childRow < model->rowCount(comboIndex); childRow++){

        QModelIndex parIndex = model->index(childRow, 1, comboIndex);

        if (qobject_cast<QComboBox*>(ui->TreeView->indexWidget(parIndex)))
           data.append(readCombobox(parIndex));
        else
           data.append(model->data(parIndex, Qt::EditRole));
    }
    return data;
}

QString MainWindow::readCombobox(QModelIndex index){
    QComboBox* box = qobject_cast<QComboBox*>(ui->TreeView->indexWidget(index));
    return box->currentText();
}


//------------------------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------//
//-----------------------------------------PRINTING FUNCTIONS--------------------------------------------//
//------------------------------------------------------------------------------------------------------//
void MainWindow::writeBackbone(){

    ui->plainTextEdit->clear();
    pyWriteBackbone();

    QFile BFile(":/OGbackbone.txt");

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


void MainWindow::samplestoPlainTextEdit(){
    ui->plainTextEdit->find("#do not need to be changed during experiment."); //positions the cursor to insert instructions
    ui->plainTextEdit->moveCursor(QTextCursor::Down, QTextCursor::MoveAnchor);
    ui->plainTextEdit->moveCursor(QTextCursor::Down, QTextCursor::MoveAnchor); //move cursor down one more line
    QList<NRSample> samples = mySampleTable->sampleList;
    ui->plainTextEdit->insertPlainText(writeSamples(samples));
}

void MainWindow::samplestoPyTextEdit(){

    bool found = ui->PyScriptBox->find("def experimentsettings():");   qDebug() << "found it? " << found;
    ui->plainTextEdit->moveCursor(QTextCursor::Down, QTextCursor::MoveAnchor);
    QList<NRSample> samples = mySampleTable->sampleList;
    ui->PyScriptBox->insertPlainText(PyWriteSamples(samples));
}


void MainWindow::printCommands(QString command, QVector<QVariant> params, int row){

    if (params.isEmpty())
        return;

    runstruct runvars;
    runvars.sampNum =  findSampNum(params[0].toString());//needed for trans, run, contrast

    if (command == "NIMA Pressure"){
        printNIMA_P(runvars, row, params);
    }
    else if (command == "NIMA Area"){
        printNIMA_A(runvars, row, params);
    }
    else if (command == "Eurotherm"){
        printEuro(runvars, row, params);
    }
    else if (command == "Julabo"){
        printJulabo(runvars, row, params);
    }
    else if (command == "Run Transmissions"){
        printRunTr(runvars, row, params);
    }
    else if (command == "Contrast Change"){
        printContrast(runvars, row, params);
    }

    else if (command == "Run with SM"){

        if (mySampleTable->sampleList[runvars.sampNum.toInt() -1].coarse_noMirror == -100){
            WarningMessage("You must define coarse_noMirror for the sample in order to run with SuperMirror");
            return;
        }
        printRunSM(runvars, row, params);
    }
    else if (command == "Run"){
        printRun(runvars, row, params);
    }
    else if (command == "Free Command"){
       printFreeCommand(row, params);
    }
}

void MainWindow::printFreeCommand(int row, QVector<QVariant> &params){
    ui->PyScriptBox->insertPlainText(params[0].toString());
    ui->plainTextEdit->insertPlainText(params[1].toString());
    freeCommandSummary(row, params);
    setColor(Qt::green, row);
}

void MainWindow::freeCommandSummary(int row, QVector<QVariant> &params)
{
    QString summary;
    if (params[0].toString() != "")
        summary = params[0].toString();
    else
        summary = params[1].toString();
    QModelIndex colTwo = ui->TreeView->model()->index(row, 1, QModelIndex());
    ui->TreeView->model()->setData(colTwo, summary , Qt::DisplayRole);
}

void MainWindow::printNIMA_P(runstruct &runvars, int row, QVector<QVariant> &params){
    if (parseNIMA_P(params, runvars)){
        ui->plainTextEdit->insertPlainText(writeNIMA(runvars, Pressure, OPENGENIE));
        ui->PyScriptBox->insertPlainText(writeNIMA(runvars, Pressure, PYTHON));
        setNIMA_PSummary(runvars, row);
        setColor(Qt::green, row);
    }
    else{
        setColor(Qt::red, row);
    }
}

void MainWindow::setNIMA_PSummary(runstruct &runvars, int row){

    QString summary = "P = " + QString::number(runvars.pressure) + "mN/m";
    QModelIndex colTwo = ui->TreeView->model()->index(row, 1, QModelIndex());
    ui->TreeView->model()->setData(colTwo, summary , Qt::DisplayRole);
}

void MainWindow::printNIMA_A(runstruct &runvars, int row, QVector<QVariant> &params){
    if (parseNIMA_A(params, runvars)){
        ui->plainTextEdit->insertPlainText(writeNIMA(runvars, Area, OPENGENIE));
        ui->PyScriptBox->insertPlainText(writeNIMA(runvars, Area, PYTHON));
        setNIMA_ASummary(runvars, row);
        setColor(Qt::green, row);
    }
    else{
        setColor(Qt::red, row);
    }
}

void MainWindow::setNIMA_ASummary(runstruct &runvars, int row){

    QString summary = "A = " + QString::number(runvars.area) + "cm^2";
    QModelIndex colTwo = ui->TreeView->model()->index(row, 1, QModelIndex());
    ui->TreeView->model()->setData(colTwo, summary , Qt::DisplayRole);
}


void MainWindow::printEuro(runstruct &runvars, int row, QVector<QVariant> &params){
    if (parseEurotherm(params, runvars)){
        ui->plainTextEdit->insertPlainText(writeEuro(runvars));
        ui->PyScriptBox->insertPlainText(writeEuro(runvars));
        setColor(Qt::green, row);
    }
    else{
        setColor(Qt::red, row);
    }
}

void MainWindow::printJulabo(runstruct &runvars, int row, QVector<QVariant> params){

    bool runcontrol;

    if (parseJulabo(params, runvars, runcontrol)){
        int secs = static_cast<int>(runvars.flow/runvars.volume*60);
        ui->timeEdit->setTime(runTime.addSecs(secs));

        ui->plainTextEdit->insertPlainText(writeJulabo(runvars, runcontrol)); //implement runcontrol
        ui->PyScriptBox->insertPlainText(pyWriteJulabo(runvars, runcontrol));
        setJulaboSummary(runvars, runcontrol, row);
        setColor(Qt::green, row);
    }
    else{
        setColor(Qt::red, row);
    }
}

void MainWindow::setJulaboSummary(runstruct &runvars, bool runcontrol, int row){

    QString summary = "T = " + QString::number(runvars.JTemp) + "°C ";
    if (runcontrol)
        summary += "with Run Control";
    QModelIndex colTwo = ui->TreeView->model()->index(row, 1, QModelIndex());
    ui->TreeView->model()->setData(colTwo, summary , Qt::DisplayRole);
}

void MainWindow::printContrast(runstruct &runvars, int row, QVector<QVariant> params){

    runvars.knauer = mySampleTable->sampleList[runvars.sampNum.toInt()-1].knauer; //causes runtime crash if

    if (parseContrast(params, runvars)){
           ui->plainTextEdit->insertPlainText(writeContrast(runvars, 0, OPENGENIE)); //implement "Wait!"
           ui->PyScriptBox->insertPlainText(writeContrast(runvars, 0, PYTHON));
           setContrastSummary(runvars, row);
           setColor(Qt::green, row);
       }
    else{
        setColor(Qt::red, row);
    }
}

void MainWindow::setContrastSummary(runstruct &runvars, int row){

    QString summary = QString::number(runvars.concs[0]);
    for (int i = 1; i < 4; i++){
        summary += ":" + QString::number(runvars.concs[i]);
    }
    summary += ", " + QString::number(runvars.flow) + "mL/min";
    summary += ", " + QString::number(runvars.volume) + "mL";

    QModelIndex colTwo = ui->TreeView->model()->index(row, 1, QModelIndex());
    ui->TreeView->model()->setData(colTwo, summary , Qt::DisplayRole);
}

void MainWindow::printRunTr(runstruct &runvars, int row, QVector<QVariant> &params){

    if (parseTransm(params, runvars)){
        updateRunTime(runvars.uAmpsT);

        ui->plainTextEdit->insertPlainText(writeTransm(runvars, OPENGENIE));
        ui->PyScriptBox->insertPlainText(writeTransm(runvars, PYTHON));
        setTransmSummary(runvars, row);
        setColor(Qt::green, row);
    }
    else{
        setColor(Qt::red, row);
    }
}

void MainWindow::setTransmSummary(runstruct &runvars, int row){

    QString summary = runvars.sampName + "  " + runvars.subtitle;

    QModelIndex colTwo = ui->TreeView->model()->index(row, 1, QModelIndex());
    ui->TreeView->model()->setData(colTwo, summary , Qt::DisplayRole);

}

void MainWindow::printRun(runstruct &runvars, int row, QVector<QVariant> &params){

    if (parseRun(params, runvars)){
        for (int i= 0; i < 3; i++){
            updateRunTime(runvars.uAmps[i]);
        }
        ui->plainTextEdit->insertPlainText(writeRun(runvars, 0, OPENGENIE));
        ui->PyScriptBox->insertPlainText(writeRun(runvars, 0, PYTHON));
        setRunSummary(runvars, row);
        setColor(Qt::green, row);
    }
    else
        setColor(Qt::red, row);
}

void MainWindow::setRunSummary(runstruct &runvars, int row){

    QString summary = runvars.sampName + ", " + runvars.subtitle + ", ";
    summary += QString::number(runvars.uAmps[0] + runvars.uAmps[1] + runvars.uAmps[2]) + " uAmps";

    QModelIndex colTwo = ui->TreeView->model()->index(row, 1, QModelIndex());
    ui->TreeView->model()->setData(colTwo, summary , Qt::DisplayRole);
}

void MainWindow::printRunSM(runstruct &runvars, int row, QVector<QVariant> &params){

    if (parseRun(params, runvars)){
        for (int i= 0; i < 3; i++){
            updateRunTime(runvars.uAmps[i]);
        }
        ui->plainTextEdit->insertPlainText(writeRun(runvars, WithSM, OPENGENIE));
        ui->PyScriptBox->insertPlainText(writeRun(runvars, WithSM, PYTHON));
        setRunSummary(runvars, row);
        setColor(Qt::green, row);
    }
    else
        setColor(Qt::red, row);

}


//----------------------------------------END OF PRINTING FUNCTIONS----------------------------------------//
//------------------------------------------------------------------------------------------------------//
//---------------------------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------//
void MainWindow::setColor(Qt::GlobalColor color, int rowNumber){

    QModelIndex index = ui->TreeView->model()->index(rowNumber, 2, QModelIndex());
    bool success = ui->TreeView->model()->setData(index, QVariant(QBrush (QColor(color))), Qt::BackgroundRole);
    if (!success)
        qDebug() << "Couldn't set Color";
}


QString MainWindow::findSampNum(QString sampName){
    for (int i = 0; i < mySampleTable->sampleList.size(); i++){
        if (sampName == mySampleTable->sampleList[i].title){
            return QString::number(i+1);
        }
    }
    return "error";
}



void MainWindow::on_parseCommands_clicked()
{
    parseTree();
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



void MainWindow::on_actionExpand_triggered()
{
    ui->TreeView->expandAll();
}

void MainWindow::on_actionCollapse_triggered()
{
    ui->TreeView->collapseAll();
}




