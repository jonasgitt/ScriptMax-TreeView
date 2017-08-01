#ifndef SAMPLETABLE_H
#define SAMPLETABLE_H

#include <QMainWindow>
#include <QWidget>
#include <QTableWidget>
#include "sampleform.h" //why do we need this?

namespace Ui {
    class SampleTable;
}

struct NRSample{

public:

    QString title;
    QString subtitle;
    double translation;
    double height;
    QString phi_offset;
    double footprint;
    double resolution;
    int knauer;
    double s3;
    double s4;
};

class SampleTable : public QMainWindow
{
    Q_OBJECT
protected:
    void closeEvent(QCloseEvent *event) override;
    
public:
    QStringList samples;
    QList<NRSample> sampleList;
    int currentSample;
    QTableWidgetSelectionRange selectedRange() const;

    explicit SampleTable(QMainWindow *parent = 0);
   // Ui::SampleTable *ui;

    ~SampleTable();

public slots:
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionDelete_triggered();
    void on_actionCut_triggered();

    void displaySamples();
    void updateSamplesSlot();
    
private:
    Ui::SampleTable *ui;
    void emit_closeSignal();
signals:
    void closedSampWindow();
};

#endif // SAMPLETABLE_H
