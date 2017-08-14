#ifndef COMBODELEGATE_H
#define COMBODELEGATE_H

#include <QItemDelegate>
#include <tree_model.h>
#include <QObject>

class ComboBoxDelegate : public QItemDelegate
{
   Q_OBJECT

public:
   ComboBoxDelegate(QObject *parent = 0);

   QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                      const QModelIndex &index) const;

   void setEditorData(QWidget *editor, const QModelIndex &index) const;
   void setModelData(QWidget *editor, TreeModel *model,
                  const QModelIndex &index);

   void updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &index) const;
public Q_SLOTS:
   void test(QString Action);
signals:
   void boxupdate(QString Action);
};


#endif // COMBODELEGATE_H
