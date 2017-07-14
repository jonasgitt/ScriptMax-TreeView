#include "sampletable.h"
#include "sampleform.h"
#include "ui_sampletable.h"
#include <QClipboard>

SampleTable::SampleTable(QMainWindow *parent) :
    QMainWindow(parent),
    ui(new Ui::SampleTable) //"new" has something to do with how memory is allocated
{
    ui->setupUi(this);
    for (int row = 0; row< ui->tableWidget->rowCount(); row++){
        for (int col = 1; col< ui->tableWidget->columnCount(); col++){
             QTableWidgetItem *newItem = new QTableWidgetItem;
            newItem->setText(" ");
            ui->tableWidget->setItem(row,col,newItem);

        }
    }
}

void SampleTable::fillTable(){
    /*for (int i=0; i < mySampleForm->sampleList.length(); i++){
        ui->tableWidget->item(0,0)->setText("TEST");
    }*/

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


void SampleTable::on_actionDelete_triggered()
{
    QList<QTableWidgetItem *> items = ui->tableWidget->selectedItems();
    if (!items.isEmpty()) {
        foreach (QTableWidgetItem *item, items)
            delete item;
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
