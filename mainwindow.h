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
#include <QTimer>
#include <QProgressBar>
#include <QModelIndex>

namespace Ui {
class MainWindow; //variables included in the namespace
}


class MainWindow : public QMainWindow
{
    Q_OBJECT
const bool PYTHON = true;
const bool OPENGENIE = false;
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
    void save(bool OGorPy);
    void parseTable();
    void writeBackbone();
    void pyWriteBackbone();
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
    void disableRows();

       //--------------//treeview//-----------------//
    void updateActions();
    void updateComboSlot(QModelIndex topLeft);
       //--------------//treeview//-----------------//


private slots:
    void initMainTable();
    void openSampleTable();
    void updateSubtitleSlot();
    void setHeaders(int which);
    void onRunSelected(int value);
    void onModeSelected(int value);
    void runControl(int value);
    void on_checkBox_clicked();
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
    void updateProgBar(int row);
    void on_OGButton_clicked();

    void on_PySaveCheckBox_clicked();

    void on_PyToolButton_clicked();

    void on_PySaveButton_clicked();

    //--------------//treeview//-----------------//
    void insertChild(QString ChildTitle);
    bool insertColumn();
    void insertRow(QString Action);
    bool removeColumn();
    void removeRow();
    void on_newCommand_clicked();
    void on_RemoveCommand_clicked();
    void on_parseButton_clicked();
    //--------------//treeview//-----------------//


protected slots:
    void onDeviceSelected(int value);

signals:
    void valueChanged(int newValue);
    void tableModified(int tableNumber);

private:
    QSignalMapper* signalMapper;
    //SampleForm *mySampleForm;
    Highlighter* OGhighlighter;
    KickPythonSyntaxHighlighter* pyhighlighter;
    QTimer *timer;
    QProgressBar *bar;
    int counter = 0;

    bool areyousure();
    void ProgressBar(int secs, int row);
    void SaveToolButtons(bool OGorPy);

    //--------------//treeview//-----------------//
    void initTree();
    void InsertParameters(QStringList parameters);
    QStringList parameterList(QVariant runOption);
    void parseModel();
     //--------------//treeview//-----------------//


};

void saveSettings (const QString &key, const QVariant &value, const QString &group);
QVariant loadSettings();

#endif // MAINWINDOW_H
