#include <QItemDelegate>
#include "comboboxdelegate.h"
#include <QComboBox>
#include <QWidget>
#include <QDebug>
#include <tree_model.h>
#include <QObject>
ComboBoxDelegate::ComboBoxDelegate(QObject *parent)
: QItemDelegate(parent)
{
}

//creates a ComboBox
QWidget *ComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const{

    QComboBox *editor = new QComboBox(parent);

    QStringList options;
    options << "Run" << "Run with SM" << "Run Transmissions" << "NIMA Pressure" << "NIMA Area" << "Contrast Change" << "Julabo" << "Eurotherm";

    editor->addItems(options);

    if (!index.parent().isValid())
      return editor;

}

//A new editor is created every time comobobox is clicked
//this function gets the old data out of the model so that the new editor will have same content as before
void ComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  QString value = index.model()->data(index, Qt::EditRole).toString();

  QComboBox *cBox = static_cast<QComboBox*>(editor);
  cBox->setCurrentIndex(cBox->findText(value));

 connect(editor, SIGNAL(currentIndexChanged(QString)), SLOT(test(QString)));

}


//when editing is complete the model is updated
void ComboBoxDelegate::setModelData(QWidget *editor, TreeModel *model,
                               const QModelIndex &index)
{qDebug() << "now!!";
  QComboBox *cBox = static_cast<QComboBox*>(editor);
  QString value = cBox->currentText();

 // model->setData(index, value, Qt::EditRole);


}


//ensures correct dimensions in table
void ComboBoxDelegate::updateEditorGeometry(QWidget *editor,
const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
  editor->setGeometry(option.rect);
}

void ComboBoxDelegate::test(QString Action){
   QString hae = Action;
    qDebug() << "boxdebug" << Action;
    emit boxupdate(hae);
}



