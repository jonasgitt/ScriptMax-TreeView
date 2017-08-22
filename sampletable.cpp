#include "sampletable.h"
#include "ui_sampletable.h"
#include <QClipboard>
#include <QTableWidgetItem>
#include <QTableWidget>
#include <QMessageBox>
#include <QMainWindow>
#include "qmainwindow.h"
#include <QDebug>
#include <mainwindow.h>
#include <QCloseEvent>

SampleTable::SampleTable(QMainWindow *parent) :
    QMainWindow(parent),
    ui(new Ui::SampleTable)
{
    ui->setupUi(this);

    //is this really needed??
    for (int row = 0; row< ui->tableWidget->rowCount(); row++){
        for (int col = 1; col< ui->tableWidget->columnCount(); col++){
             QTableWidgetItem *newItem = new QTableWidgetItem;
            newItem->setText(" ");
            ui->tableWidget->setItem(row,col,newItem);

        }
    }

    QStringList headers;
    headers << "Title" << "Translation" << "Height" << "Phi-Offset" << "Psi" << "Footprint" << "Resolution" << "Slit 3" << "Slit 4 (detector)" << "Switch Position" << "Coarse_NoMirror";

    for (int i = 0; i < headers.size(); i++){
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(headers[i]));
    }
}


void SampleTable::displaySamples(){

    for (int row = sampleList.length(); row<   ui->tableWidget->rowCount(); row++){
        for (int col = 0; col <   ui->tableWidget->columnCount(); col++){
            QTableWidgetItem *newItem = new QTableWidgetItem;
            newItem->setText("");
            ui->tableWidget->setItem(row,col,newItem);

        }
    }

    for (int i=0; i <  sampleList.length(); i++){
        ui->tableWidget->setVerticalHeaderItem(i,new QTableWidgetItem("S"+QString::number(i+1)));

        QTableWidgetItem *title = new QTableWidgetItem;
        title->setText( sampleList[i].title);
        ui->tableWidget->setItem(i,0,title);

        QTableWidgetItem *trans = new QTableWidgetItem;
        trans->setText(QString::number( sampleList[i].translation));
        ui->tableWidget->setItem(i,1,trans);

        QTableWidgetItem *height = new QTableWidgetItem;
        height->setText(QString::number(sampleList[i].height));
        ui->tableWidget->setItem(i,2,height);

        QTableWidgetItem *phioff = new QTableWidgetItem;
        phioff->setText(sampleList[i].phi_offset);
        ui->tableWidget->setItem(i,3,phioff);

        QTableWidgetItem *psi = new QTableWidgetItem;
        psi->setText(QString::number(sampleList[i].psi));
        ui->tableWidget->setItem(i,4,psi);

        QTableWidgetItem *footprint = new QTableWidgetItem;
        footprint->setText(QString::number(sampleList[i].footprint));
        ui->tableWidget->setItem(i,5,footprint);

        QTableWidgetItem *res = new QTableWidgetItem;
        res->setText(QString::number(sampleList[i].resolution));
        ui->tableWidget->setItem(i,6,res);

        QTableWidgetItem *s3 = new QTableWidgetItem;
        s3->setText(QString::number(sampleList[i].s3));
        ui->tableWidget->setItem(i,7,s3);

        QTableWidgetItem *s4 = new QTableWidgetItem;
        s4->setText(QString::number(sampleList[i].s4));
        ui->tableWidget->setItem(i,8,s4);

        QTableWidgetItem *knauer = new QTableWidgetItem; qDebug() << "prin" << sampleList[i].knauer;
        if (sampleList[i].knauer == -100)
            knauer->setText("");
        else
            knauer->setText(QString::number(sampleList[i].knauer));
        ui->tableWidget->setItem(i,9,knauer);

        QTableWidgetItem *noMirror = new QTableWidgetItem;
        if (sampleList[i].coarse_noMirror == -100)
            noMirror->setText("");
        else
            noMirror->setText(QString::number(sampleList[i].coarse_noMirror));
        ui->tableWidget->setItem(i,10,noMirror);
    }
    connect(ui->tableWidget,SIGNAL(currentCellChanged(int,int,int,int)),SLOT(updateSamplesSlot()));
}

void SampleTable::updateSamplesSlot(){


    for(int row=0; row< ui->tableWidget->rowCount(); row++){

        if(ui->tableWidget->item(row,0)->text() != "")
            ui->tableWidget->item(row,0)->setBackground(Qt::red);

        //if there is already an entry in the list
        if(row < sampleList.length()){

            if (validateSample(row)){
                NRSample sample = sampleList[row];
                parseRow(sample, row);
            }
        }
        //if a new sample is being created
        else if (validateSample(row)){
                NRSample newSample;
                parseRow(newSample, row); qDebug() << "straight after parsing: " << newSample.knauer;
                sampleList.append(newSample);if (sampleList.length()) qDebug() << "after" << sampleList[0].knauer;
        }

    }

}

void SampleTable::parseRow(NRSample &sample, int row){

    sample.title =  ui->tableWidget->item(row,0)->text();
    sample.translation =  ui->tableWidget->item(row,1)->text().toDouble();
    sample.height =  ui->tableWidget->item(row,2)->text().toDouble();
    sample.phi_offset =  ui->tableWidget->item(row,3)->text();
    sample.psi = ui->tableWidget->item(row, 4)->text().toDouble();
    sample.footprint =  ui->tableWidget->item(row,5)->text().toDouble();
    sample.resolution =  ui->tableWidget->item(row,6)->text().toDouble();
    sample.s3 =  ui->tableWidget->item(row,7)->text().toDouble();
    sample.s4 =  ui->tableWidget->item(row,8)->text().toDouble();

    if (sampleList.length()){
        if (ui->tableWidget->item(row,9)->text() == "")
           sampleList[row].knauer = -100; //needed so that sample can be created without these values
        else
           sampleList[row].knauer =  ui->tableWidget->item(row,9)->text().toDouble();

        if (ui->tableWidget->item(row,10)->text() == "")
           sampleList[row].coarse_noMirror = -100; //needed so that sample can be created without these values
        else
           sampleList[row].coarse_noMirror =  ui->tableWidget->item(row,10)->text().toDouble();
    }

    ui->tableWidget->item(row,0)->setBackground(Qt::green);
}

bool SampleTable::validateSample(int row){

    QTableWidget *t = ui->tableWidget;

    for (int col = 0; col < t->columnCount(); col++){

        if (t->item(row, 0)->text() == ""){
            return false;}

        if (!validateDoubles(t->item(row, col)->text()) && col != 0 && col < 9){
            return false;
        }

        if (t->item(row,9)->text() != "" && !validateDoubles(t->item(row,9)->text()))
            return false;

        if (t->item(row,10)->text() != "" && !validateDoubles(t->item(row,10)->text()))
            return false;
    }

    return true;

}
bool SampleTable::validateDoubles(QString input){

    QDoubleValidator v(0, 1000, 16);
    int pos = 0;

    if (v.validate(input, pos) != QValidator::Acceptable)
        return false;


    return true;

}

QTableWidgetSelectionRange SampleTable::selectedRange() const
{
    QList<QTableWidgetSelectionRange> ranges = ui->tableWidget->selectedRanges();
    if (ranges.isEmpty())
        return QTableWidgetSelectionRange();
    return ranges.first();
}


void SampleTable::on_actionCopy_triggered()
{
    QTableWidgetSelectionRange range = selectedRange();
    QString str;

    for (int i = 0; i < range.rowCount(); ++i) {
        if (i > 0)
            str += "\n";
        for (int j = 0; j < range.columnCount(); ++j) {
            if (j > 0)
                str += "\t";
            str += ui->tableWidget->item(range.topRow() + i, range.leftColumn() + j)->text();
        }
    }
    QApplication::clipboard()->setText(str);

}



void SampleTable::on_actionPaste_triggered()
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
            if (row < ui->tableWidget->rowCount() && column < ui->tableWidget->columnCount())
                ui->tableWidget->item(row, column)->setText(columns[j]);
        }
    }
    //somethingChanged();
}

//change to deleteRow
//causes segfault when table tries to parse deleted widgetitems?
void SampleTable::on_actionDelete_triggered()
{qDebug() << "is updatesampleslot called after deletion?";
    QList<QTableWidgetItem *> items = ui->tableWidget->selectedItems();
    if (!items.isEmpty()) {
        foreach (QTableWidgetItem *item, items)
            item->setText(".");
        //somethingChanged();
    }
}

void SampleTable::on_actionCut_triggered()
{
    on_actionCopy_triggered();
    on_actionDelete_triggered();
}



SampleTable::~SampleTable()
{
    delete ui;
}

void SampleTable::on_actionSave_and_Close_triggered()
{

    this->close();
    //To do...
    //emit closed sampwindow, save, parse sampletable?
}

void SampleTable::closeEvent(QCloseEvent *event)
{
    updateSamplesSlot();

    if (areyousure()) {
            emit closedSampWindow();
            event->accept();

        } else {
            event->ignore();
        }
}

bool SampleTable::areyousure()
{

    bool missing = false;

    int row = 0;
    while (ui->tableWidget->item(row, 0)->text() != ""){
        if (ui->tableWidget->item(row, 0)->background() == Qt::red)
            missing = true;
        row++;
    }

    if (missing){
        const QMessageBox::StandardButton ret
                = QMessageBox::warning(this, tr("Sample Table"),
                                       tr("You missed something!\n"
                                          "Are you sure you want to leave?"),
                                       QMessageBox::Yes | QMessageBox::Cancel);
        switch (ret) {
        case QMessageBox::Yes:
            return true;
        case QMessageBox::Cancel:
            return false;
        default:
            break;
        }
    }
    return true;

}
