#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "sampleform.h"
#include "sampletable.h"
#include "ui_sampletable.h"
#include "GCLHighLighter.h"
#include <QMainWindow>
#include <QComboBox>
#include <QSignalMapper>
#include <QTime>
#include <QSettings>
#include "pyhighlighter.h"


namespace Ui {
class MainWindow; //variables included in the namespace
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

protected:
    void closeEvent(QCloseEvent *event) override;
    
public:
    explicit MainWindow(QWidget *parent = 0);
    Ui::MainWindow *ui;
    QString fileName;
    QStringList samples;
    QStringList actions;
    SampleTable *mySampleTable;
    QTime runTime;
    QTableWidgetSelectionRange selectedRange() const;
    void save();
    void parseTable();
    void writeBackbone();
    QString makeLine(QString action, QStringList args);
    ~MainWindow();

    void defineSamples();
    //outsourced cases
    void normalRun(int row, bool runSM);
    void kineticRun(int row);
    void OGcommand(int row);
    void contrastChange(int row);
    void setTemp(int row);
    void setNIMA(int row);
    void runTrans(int row);

    void updateRunTime(double angle);

    void samplestoPlainTextEdit();




public slots:
    void parseTableSlot();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionDelete_triggered();
    void on_actionCut_triggered();
    void ShowContextMenu(const QPoint& pos);
    QStringList searchDashboard(QString path);



private slots:
    void initMainTable();
    void openSampleForm();
    void openSampleTable();
    void updateSamplesSlot();
    void updateSubtitleSlot();
    void runGenie();
    void setHeaders(int which);
    void onRunSelected(int value);
    void onModeSelected(int value);
    void runControl(int value);
    void on_checkBox_clicked(bool checked);
    void on_checkBox_2_clicked(bool checked);
    void on_actionSave_Script_triggered();
    void on_actionSave_Script_As_triggered();
    void on_actionOpen_Script_triggered();
    void on_actionNew_Script_triggered();
    void on_actionQuit_triggered();
    void on_toolButton_clicked();
    void on_instrumentCombo_activated(const QString &arg1);
    //void on_OG_checkBox_stateChanged(int arg1);
    //void on_OGcmd_pushButton_clicked();
    void on_actionHow_To_triggered();
    void on_actionAbout_ScriptMax_triggered();
    void on_clearTableButton_clicked();
    void on_actionSave_GCL_file_triggered();

    void on_saveButton_clicked();

    void on_PythonButton_clicked();

    void on_OGButton_clicked();

protected slots:
    void onDeviceSelected(int value);

signals:
    void valueChanged(int newValue);
    void tableModified(int tableNumber);

private:
    QSignalMapper* signalMapper;
    SampleForm *mySampleForm;
    Highlighter* OGhighlighter;
    KickPythonSyntaxHighlighter* pyhighlighter;
    bool areyousure();

};

void saveSettings (const QString &key, const QVariant &value, const QString &group);
QVariant loadSettings(const QString &key, const QVariant &defaultValue, const QString &group);

#endif // MAINWINDOW_H
