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
    parseTree();

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


void MainWindow::on_sampleTableButton_clicked()
{
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
    headers << tr("Action") << tr("Summary of Command") << tr("Further Information");
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
    bool connekt = connect(mySampleTable,SIGNAL(closedSampWindow()), SLOT(updateSampleBoxes()));
    qDebug() << "MainWindow Connections: " << connection << connekt;

    ui->TreeView->setEditTriggers(QAbstractItemView::AnyKeyPressed);
    ui->TreeView->setEditTriggers(QAbstractItemView::DoubleClicked);
    ui->TreeView->resizeColumnToContents(0);
      ui->TreeView->setColumnWidth(1, 150);//dont work
}


void MainWindow::updateComboSlot(QModelIndex topLeft){

    if (topLeft.column() != 0){
        return;
}

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
        parameters << "Sample" << "uAmps 3" << "Angle 3" << "uAmps 2" << "Angle 2" << "uAmps 1" << "Angle 1";
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
        if (qobject_cast<QComboBox*>(ui->TreeView->indexWidget(comboIndex))){
          setSampleComboBox(comboIndex);
        }
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

    runTime = runTime.fromString("00:00", "hh:mm");

    writeBackbone();

    ui->plainTextEdit->find("runTime=0"); //positions the cursor to insert instructions
    ui->plainTextEdit->moveCursor(QTextCursor::Down, QTextCursor::MoveAnchor);

    QAbstractItemModel *model = ui->TreeView->model();

    for (int row = 0; row < model->rowCount(QModelIndex()); row++){
        QModelIndex comboIndex = model->index(row,0, QModelIndex());
         QString comboSelected = model->data(comboIndex).toString();
         printCommands(comboSelected, getChildData(row), row);//make print commands a bool?

    }

    samplestoPlainTextEdit();
    if(ui->checkBox->isChecked()){
        save(OPENGENIE);
    }
    if(ui->PySaveCheckBox->isChecked())
        save(PYTHON);
}

QVector<QVariant> MainWindow::getChildData(int parentRow){

    QAbstractItemModel *model = ui->TreeView->model();
    QVector<QVariant> data;
    QModelIndex comboIndex = model->index(parentRow,0, QModelIndex());//come along

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

void MainWindow::printCommands(QString command, QVector<QVariant> params, int row){

    if (params.isEmpty())
        return;

    runstruct runvars;
    runvars.sampNum =  findSampNum(params[0].toString());//needed for trans, run, contrast

    if (command == "NIMA Pressure"){
        runvars = parseNIMA_P(params);
        ui->plainTextEdit->insertPlainText(writeNIMA(runvars, Pressure, OPENGENIE));
        ui->PyScriptBox->insertPlainText(writeNIMA(runvars, Pressure, PYTHON));
    }
    else if (command == "NIMA Area"){
        runvars = parseNIMA_A(params);
        ui->plainTextEdit->insertPlainText(writeNIMA(runvars, Area, OPENGENIE));
        ui->PyScriptBox->insertPlainText(writeNIMA(runvars, Area, PYTHON));
    }
    else if (command == "Eurotherm"){
        runvars = parseEurotherm(params);
        ui->plainTextEdit->insertPlainText(writeEuro(runvars));
        ui->PyScriptBox->insertPlainText(writeEuro(runvars));
    }
    else if (command == "Julabo"){
        runvars = parseJulabo(params);
        ui->plainTextEdit->insertPlainText(writeJulabo(runvars, 0)); //implement runcontrol
        ui->PyScriptBox->insertPlainText(writeJulabo(runvars, 0)); //is this python compatible?
    }
    else if (command == "Run Transmissions"){
        printRunTr(runvars, row, params);

    }
    else if (command == "Contrast Change"){
        printContrast(runvars, row, params);
    }

    else if (command == "Run with SM"){
        printRunSM(runvars, row, params);
    }
    else if (command == "Run"){
        printRun(runvars, row, params);
    }
}

void MainWindow::printContrast(runstruct &runvars, int row, QVector<QVariant> params){


    runvars.knauer = mySampleTable->sampleList[runvars.sampNum.toInt()-1].knauer; //causes runtime crash if

    if (parseContrast(params, runvars)){
           ui->plainTextEdit->insertPlainText(writeContrast(runvars, 0, OPENGENIE)); //implement "Wait!"
           ui->PyScriptBox->insertPlainText(writeContrast(runvars, 0, PYTHON));
           setColor(Qt::green, row);
       }
    else{
        setColor(Qt::red, row);
    }
}

void MainWindow::printRunTr(runstruct &runvars, int row, QVector<QVariant> &params){

    if (parseTransm(params, runvars)){
        ui->plainTextEdit->insertPlainText(writeTransm(runvars, OPENGENIE));
        ui->PyScriptBox->insertPlainText(writeTransm(runvars, PYTHON));
           setColor(Qt::green, row);
       }
    else{
        setColor(Qt::red, row);
    }
}

void MainWindow::printRun(runstruct &runvars, int row, QVector<QVariant> &params){

    if (parseRun(params, runvars)){
        ui->plainTextEdit->insertPlainText(writeRun(runvars, 0, OPENGENIE));
        ui->PyScriptBox->insertPlainText(writeRun(runvars, 0, PYTHON));
        setColor(Qt::green, row);
    }
    else
        setColor(Qt::red, row);
}

void MainWindow::printRunSM(runstruct &runvars, int row, QVector<QVariant> &params){

    if (parseRun(params, runvars)){
        ui->plainTextEdit->insertPlainText(writeRun(runvars, WithSM, OPENGENIE));
        ui->PyScriptBox->insertPlainText(writeRun(runvars, WithSM, PYTHON));
        setColor(Qt::green, row);
    }
    else
        setColor(Qt::red, row);

}

void MainWindow::setColor(Qt::GlobalColor color, int rowNumber){

    QModelIndex index = ui->TreeView->model()->index(rowNumber, 2, QModelIndex());
    bool success = ui->TreeView->model()->setData(index, QVariant(QBrush (QColor(color))), Qt::BackgroundRole);
    if (!success)
        qDebug() << "Couldn't set Color";
}


QString MainWindow::findSampNum(QString sampName){
    for (int i = 0; i < mySampleTable->sampleList.size(); i++){
        if (sampName == mySampleTable->sampleList[i].title){
            //qDebug() << "sampnum: " << QString::number(i+1);
            return QString::number(i+1);
    }
        }
    qDebug() << "no luck";
    return "error";
}


void MainWindow::on_parseCommands_clicked()
{
    /*if (mySampleTable->sampleList.isEmpty()){
         QMessageBox::warning(this, "Error" , "You must define a sample first.");
        return;
    }*/
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


