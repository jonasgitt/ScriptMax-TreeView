

#ifndef SAMPLEFORM_H //checks if a flag has been set. these are known as flags btw
#define SAMPLEFORM_H

#include <QWidget>


namespace Ui {
    class SampleForm;
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

//sampleForm is the new class which inherits all the properties of QWidget
class SampleForm : public QWidget
{
    Q_OBJECT //a macro that always needs to be there if we create a class containing signals and slots

public:
    QStringList samples;
    QList<NRSample> sampleList;
    int currentSample;
    explicit SampleForm(QWidget *parent = 0);
    ~SampleForm();


signals:
    void button3Clicked();

public slots:
    void displaySample(int which);
    void updateSample();
    void saveSample();
    void prevSample();
    void nextSample();

private:
    Ui::SampleForm *ui; //defines a pointer at ui that is very often used in sampleform.cpp
};

#endif // SAMPLEFORM_H
