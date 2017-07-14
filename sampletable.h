#ifndef SAMPLETABLE_H
#define SAMPLETABLE_H

#include <QMainWindow>
#include <QWidget>
#include <QTableWidget>
#include "sampleform.h" //why do we need this?

namespace Ui {
    class SampleTable;
}

class SampleTable : public QMainWindow
{
    Q_OBJECT
    
public:                                      //= 0 denotes a default argument which allows use of function without that argument. also means this object has no parent?
    explicit SampleTable(QMainWindow *parent = 0); //explicit forces the function to reject a function call that doesn't explicitly use the right argument
    Ui::SampleTable *ui; //declares a pointer to an object of type Sample Table

    QTableWidgetSelectionRange selectedRange() const; //const here causes error if this class function changes a member variable of the class
    ~SampleTable(); //destroys sample table

public slots:
    void fillTable();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionDelete_triggered();
    void on_actionCut_triggered();
    
private:
    //Ui::SampleTable *ui;
};

#endif // SAMPLETABLE_H
