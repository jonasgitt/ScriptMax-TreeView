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
}



void SampleTable::displaySamples(){
    // initialize table
    for (int row = sampleList.length(); row<   ui->tableWidget->rowCount(); row++){
        for (int col = 0; col<   ui->tableWidget->columnCount(); col++){
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

        QTableWidgetItem *footprint = new QTableWidgetItem;
        footprint->setText(QString::number(sampleList[i].footprint));
        ui->tableWidget->setItem(i,4,footprint);

        QTableWidgetItem *res = new QTableWidgetItem;
        res->setText(QString::number(sampleList[i].resolution));
          ui->tableWidget->setItem(i,5,res);

        QTableWidgetItem *s3 = new QTableWidgetItem;
        s3->setText(QString::number(sampleList[i].s3));
          ui->tableWidget->setItem(i,6,s3);

        QTableWidgetItem *s4 = new QTableWidgetItem;
        s4->setText(QString::number(sampleList[i].s4));
          ui->tableWidget->setItem(i,7,s4);

        QTableWidgetItem *knauer = new QTableWidgetItem;
        knauer->setText(QString::number(sampleList[i].knauer));
          ui->tableWidget->setItem(i,8,knauer);
    }
    connect(ui->tableWidget,SIGNAL(currentCellChanged(int,int,int,int)),SLOT(updateSamplesSlot()));
}

void SampleTable::updateSamplesSlot(){
    bool ok;


    QRegExp isSum("^[+-]?\\d*\\.?\\d*([+-]\\d*\\.?\\d*)?$");
    QRegExp isDouble("^[+-]?[0-9]+([\\,\\.][0-9]+)?$");


    for(int row=0; row< ui->tableWidget->rowCount(); row++){
        ok = true;


        //if there is already an entry in the list
        if(row < sampleList.length()){
                //if first column is empty
                if( ui->tableWidget->item(row,0)->text()=="")
                    ok = false;

                if(!isSum.exactMatch( ui->tableWidget->item(row,3)->text())){
                    ok = false;
                     ui->tableWidget->item(row,3)->setText("");
                }

                for(int col=1; col <  ui->tableWidget->columnCount()-1; col++){
                    //what is special about the third column?
                    if(col!=3)
                        if (!isDouble.exactMatch( ui->tableWidget->item(row,col)->text())){
                            ok = false;
                             ui->tableWidget->item(row,col)->setText("");
                             ui->tableWidget->setCurrentCell(row,col);
                        }
                }


                //the above checked if a cell was empty or of the wrong type
                //if input was ok save the inputted data in the samplelist
                if (ok){
                    sampleList[row].title =  ui->tableWidget->item(row,0)->text();
                    sampleList[row].translation =  ui->tableWidget->item(row,1)->text().toDouble();
                    sampleList[row].height =  ui->tableWidget->item(row,2)->text().toDouble();
                    sampleList[row].phi_offset =  ui->tableWidget->item(row,3)->text();
                    sampleList[row].footprint =  ui->tableWidget->item(row,4)->text().toDouble();
                    sampleList[row].resolution =  ui->tableWidget->item(row,5)->text().toDouble();
                    sampleList[row].s3 =  ui->tableWidget->item(row,6)->text().toDouble();
                    sampleList[row].s4 =  ui->tableWidget->item(row,7)->text().toDouble();
                    sampleList[row].knauer =  ui->tableWidget->item(row,8)->text().toDouble();
                }else {
                    QMessageBox msgBox;
                    msgBox.setIcon(QMessageBox::Warning);
                    msgBox.setText("One or more values are missing, of the wrong type or duplicates (e.g. translation or switch!");
                    msgBox.exec();
                }
        }
         //if a new sample is being created
         else {
                if( ui->tableWidget->item(row,0)->text()==""){//only create sample if it has name
                    ok = false;}
                if(!isSum.exactMatch( ui->tableWidget->item(row,3)->text())){
                    ok = false;qDebug() << "not exact match";
                     ui->tableWidget->item(row,3)->setText("");
                }
                for(int col=1; col <  ui->tableWidget->columnCount(); col++){
                    if(col!=3)
                        if (!isDouble.exactMatch( ui->tableWidget->item(row,col)->text())){
                            ok = false;
                             ui->tableWidget->item(row,col)->setText("");
                            // ui->tableWidget->setCurrentCell(row,col);
                        }
                }
                if (ok){ // create new smaple in sampleList
                    NRSample newSample;

                    newSample.title =  ui->tableWidget->item(row,0)->text();
                    newSample.translation =  ui->tableWidget->item(row,1)->text().toDouble();
                    newSample.height =  ui->tableWidget->item(row,2)->text().toDouble();
                    newSample.phi_offset =  ui->tableWidget->item(row,3)->text();
                    newSample.footprint =  ui->tableWidget->item(row,4)->text().toDouble();
                    newSample.resolution =  ui->tableWidget->item(row,5)->text().toDouble();
                    newSample.s3 =  ui->tableWidget->item(row,6)->text().toDouble();
                    newSample.s4 =  ui->tableWidget->item(row,7)->text().toDouble();
                    newSample.knauer =  ui->tableWidget->item(row,8)->text().toDouble();
                    sampleList.append(newSample);
                    if(sampleList.length() == 1){
                        //displaySample(0);
                        currentSample = 0;
                    }

            }
        }
    }

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
    if (areyousure()) {
            event->accept();
            emit closedSampWindow();
        } else {
            event->ignore();
        }
}
//NEEDS FIXING!
bool SampleTable::areyousure()
{
    /*
    bool missing = false;
    for (int row = 0; row < sampleList.length(); row++){
        qDebug() << "row: " << row;
        for (int col = 0; col < ui->tableWidget->columnCount(); col++){
            if(ui->tableWidget->item(row,col)->text().isEmpty()){
             qDebug() << "somethings missing";   missing = true;
            }
        }
    }
    if (missing == false)
        return true;
qDebug() << "Has it not returned??";
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
    }*/
    return true;

}
