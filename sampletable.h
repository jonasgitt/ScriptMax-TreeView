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
    int knauer; //Switch Position
    double s3; //slit 3
    double s4; //slit 4
    double coarse_noMirror; //so that we know if it hasnt been initialized yet
    double psi;
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

    void displaySamples();
    void updateSamplesSlot();
    
private:
    Ui::SampleTable *ui;
    void emit_closeSignal();
    bool areyousure();
    bool validateDoubles(QString input);
    void parseRow(NRSample &sample, int row);
    bool validateSample(int row);
signals:
    void closedSampWindow();
private slots:
    void on_actionSave_and_Close_triggered();
    void on_actionDeleteRow_triggered();
};

#endif // SAMPLETABLE_H
