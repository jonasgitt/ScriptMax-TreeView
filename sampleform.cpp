/*
#include "sampleform.h"
#include "ui_sampleform.h"
#include <QMainWindow>
#include "mainwindow.h"



SampleForm::SampleForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SampleForm)
{
    ui->setupUi(this);

    ui->pushButton->setEnabled(0);

}


void SampleForm::updateSample(){

    sampleList[currentSample].title = ui->titleEdit->text();
    sampleList[currentSample].translation = ui->lineEdit_3->text().toDouble();
    sampleList[currentSample].height = ui->lineEdit_4->text().toDouble();
    sampleList[currentSample].phi_offset = ui->lineEdit_5->text().toDouble();
    sampleList[currentSample].footprint = ui->lineEdit_6->text().toDouble();
    sampleList[currentSample].resolution = ui->lineEdit_7->text().toDouble();
    sampleList[currentSample].s3 = ui->lineEdit_s3->text().toDouble();
    sampleList[currentSample].s4 = ui->lineEdit_s4->text().toDouble();
    sampleList[currentSample].knauer = ui->spinBox->value();
}

void SampleForm::displaySample(int which){

    if (currentSample < sampleList.length()){
        ui->titleEdit->setText(sampleList[which].title);
        ui->lineEdit_3->setText(QString::number(sampleList[which].translation));
        ui->lineEdit_4->setText(QString::number(sampleList[which].height));
        ui->lineEdit_5->setText(sampleList[which].phi_offset);
        ui->lineEdit_6->setText(QString::number(sampleList[which].footprint));
        ui->lineEdit_7->setText(QString::number(sampleList[which].resolution));
        ui->lineEdit_s3->setText(QString::number(sampleList[which].s3));
        ui->lineEdit_s4->setText(QString::number(sampleList[which].s4));
        ui->spinBox->setValue(sampleList[which].knauer);
        ui->label_10->setText(QString::number(currentSample));
    } else {
        ui->titleEdit->setText("");
        ui->lineEdit_3->setText("");
        ui->lineEdit_4->setText("");
        ui->lineEdit_5->setText("");
        ui->lineEdit_6->setText("");
        ui->lineEdit_7->setText("");
        ui->lineEdit_s3->setText("");
        ui->lineEdit_s4->setText("");
        ui->spinBox->setValue(0);
        ui->label_10->setText(QString::number(currentSample));
    }

}

void SampleForm::saveSample(){

    emit button3Clicked();
    SampleForm::hide();
    nextSample();
    prevSample();

}

void SampleForm::prevSample(){

    if (currentSample > 0){
        //updateSample();
        currentSample--;
        displaySample(currentSample);
        if (currentSample == 0){
            displaySample(0);
            ui->pushButton->setEnabled(0);
        }
    }
}

void SampleForm::nextSample(){

    bool ok;
    ok = true;

    ui->pushButton->setEnabled(1);
    QList<QLineEdit*> entries = this->findChildren<QLineEdit*>();
    foreach (QLineEdit *txt, entries){
        if(txt->text() == ""){ok=false;}
    }

    // initialize sampleList
    if (sampleList.length() == 0 && ok){
        NRSample newSample;
        sampleList.append(newSample);
        updateSample();
        currentSample = 1;
        displaySample(currentSample);
    } else {
        if(currentSample < sampleList.length()){
            if(ok){updateSample();} // save displayed changes
            currentSample++;
            displaySample(currentSample);
        } else {
            if(ok){
                NRSample newSample;
                sampleList.append(newSample);
                updateSample();
                currentSample++;
                displaySample(currentSample);
            }
        }
    }

}

SampleForm::~SampleForm()
{
    delete ui;
}
*/
