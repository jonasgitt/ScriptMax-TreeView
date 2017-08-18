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
#include "ScriptLines.h"

namespace Ui {
class MainWindow; //variables included in the namespace
}


class MainWindow : public QMainWindow
{
    Q_OBJECT


enum {OPENGENIE, PYTHON, WithSM, wait, Pressure, Area, runContr};

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
    void writeBackbone();
    void pyWriteBackbone();
    QString makeLine(QString action, QStringList args);
    ~MainWindow();

    void defineSamples();

    void updateRunTime(double angle);

    void samplestoPlainTextEdit();

    QString findSampNum(QString sampName);
    void printCommands(QString command, QVector<QVariant> params, int row);
    void printContrast(runstruct &runvars, int row, QVector<QVariant> params);
    void printRun(runstruct &runvars, int row, QVector<QVariant> &params);
    void printRunSM(runstruct &runvars, int row, QVector<QVariant> &params);
    void printRunTr(runstruct &runvars, int row, QVector<QVariant> &params);

public slots:

       //--------------//treeview//-----------------//
    void updateComboSlot(QModelIndex topLeft);
       //--------------//treeview//-----------------//


private slots:
    void on_checkBox_clicked();
    void on_actionSave_Script_triggered();
    void on_actionSave_Script_As_triggered();
    void on_actionOpen_Script_triggered();
    void on_actionNew_Script_triggered();
    void on_actionQuit_triggered();
    void on_toolButton_clicked();
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
    void on_removeCommands_clicked();
    void on_parseCommands_clicked();
    void updateSampleBoxes();
    //--------------//treeview//-----------------//


    void on_actionExpand_triggered();

    void on_actionCollapse_triggered();



    void on_sampleTableButton_clicked();


private:
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
    void parseTree();
    QVector<QVariant> getChildData(int parentRow);
    void setColor(Qt::GlobalColor color, int rowNumber);

     //--------------//treeview//-----------------//

    void setSampleComboBox(QModelIndex comboIndex);
    QString readCombobox(QModelIndex index);

};

void saveSettings (const QString &key, const QVariant &value, const QString &group);
QVariant loadSettings();

#endif // MAINWINDOW_H
