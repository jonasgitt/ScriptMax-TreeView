#include "tree_model.h"

#include <QtWidgets>
#include "tree_item.h"
#include "tree_model.h"
#include "scriptlines.h"
#include <QAbstractItemModel>


TreeModel::TreeModel(const QStringList &headers, const QString &data, QObject *parent)
    : QAbstractItemModel(parent)
{
    QVector<QVariant> rootData;
    foreach (QString header, headers)
        rootData << header;

    rootItem = new TreeItem(rootData);

    if (data != ""){
        setupModelData(data.split(QString("\n")), rootItem);
    }
}


//we give it the ComboBox cell as a parent!
void TreeModel::runSelected(TreeItem *comboParent)
{
    QStringList parameters;
    parameters << "Angle 1" << "uAmps 1" << "Angle 2" << "uAmps 2" << "Angle 3" << "uAmps 3";

    QString parameter;
    int parNum = 0;
    foreach (parameter, parameters){
        parameter = parameters[parNum];
        comboParent->insertChildren(comboParent->childCount(), 1, rootItem->columnCount());
        comboParent->child(comboParent->childCount() - 1)->setData(0, parameter);
        parNum++;
    }

}

TreeModel::~TreeModel()
{
    delete rootItem;
}

int TreeModel::columnCount(const QModelIndex & /* parent */) const
{
    return rootItem->columnCount();
}


//this gets the data out of the table (e.g. item->data(0) gave me which option was selected in combobox)
QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    TreeItem *item = getItem(index);

    if (!index.isValid())
        return QVariant();

    if (role == Qt::BackgroundRole)
         return item->data(index.column());


    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    return item->data(index.column());

}

//Enables editing of items
Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    //if we in first col of second child --> not editable
    if ((index.parent() != QModelIndex() && index.column() == 0) || index.column() == 2)
        return Qt::ItemIsEnabled;

    return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

TreeItem *TreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}


QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

//gives index of child based on parent index, row and column
QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex(); //empty, represents root modelindex

    TreeItem *parentItem = getItem(parent);

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);//creates a model index
    else
        return QModelIndex();
}


bool TreeModel::insertColumns(int position, int columns, const QModelIndex &parent)
{
    bool success;

    beginInsertColumns(parent, position, position + columns - 1);
    success = rootItem->insertColumns(position, columns); //any additions to model are also refleced in view? (treeitem?)
    endInsertColumns();

    return success;
}

bool TreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    bool success;

    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(position, rows, rootItem->columnCount());
    endInsertRows();

    return success;
}

//Returns index of parent node
QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = getItem(index);
    TreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

bool TreeModel::removeColumns(int position, int columns, const QModelIndex &parent)
{
    bool success;

    beginRemoveColumns(parent, position, position + columns - 1);
    success = rootItem->removeColumns(position, columns);
    endRemoveColumns();

    if (rootItem->columnCount() == 0)
        removeRows(0, rowCount());

    return success;
}

bool TreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}


int TreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem = getItem(parent);

    return parentItem->childCount(); //number of children of root = number of rows
}

//ensure data is stored in model correctly (after editing)
//This results in any and all data written to the table to be stored in column...
//...specified in setData()
bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    /*if (role != Qt::EditRole)
        return false;
*/
    TreeItem *item = getItem(index); //gets item a given index
    bool result = item->setData(index.column(), value);

    if (result)
        emit dataChanged(index, index);

    return result;
}

bool TreeModel::setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    bool result = rootItem->setData(section, value);

    if (result)
        emit headerDataChanged(orientation, section, section);

    return result;
}

Qt::DropActions TreeModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;//deleting this results in being unable to drop
}

bool TreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent){

    if(!canDropMimeData(data, action, row, column, parent))
            return false;

    if (action == Qt::IgnoreAction)
        return true;

    int beginRow;

    if (row != -1)
        beginRow = row;
    else if (parent.isValid()){//will be true
        beginRow = parent.row();
     qDebug() << "BEGINROW: " << beginRow;
    }
    else
        beginRow = rowCount(QModelIndex());


    //CONTINUE FROM HERE
    QModelIndex grandParent = parent.parent();
    //insertRow(beginRow, grandParent);
    //setData(parent.child(beginRow, 0), data);

    //moveRow(parent, 3, QModelIndex(), parent.parent().child(0,0));


    qDebug() << "when is this even called!?!?";

    return true;

}

bool TreeModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(action);
    Q_UNUSED(row);
    Q_UNUSED(data);

    if (column > 0){
        qDebug() << "Error here";
        return false;
    }
qDebug() << "Dropped into row: " << parent.row();
qDebug() << "Target row: " << row << "Target Column: " << column;

    if (rowCount(parent) <= 0){
        //return false;
    }

    if (row < 0){ //row = -1 means item dropped directly on parent
       //return false;
}
    if (parent.parent() != QModelIndex()){
        qDebug() << "oder hier??";
        return false;
    }
    return true;
}

void TreeModel::setupModelData(const QStringList &lines, TreeItem *parent)
{

    QList<TreeItem*> parents;
    QList<int> indentations;
    parents << parent;
    indentations << 0;

    int number = 0;

    while (number < lines.count()) {
        int position = 0;
        while (position < lines[number].length()) {
            if (lines[number].at(position) != ' ')
                break;
            ++position;
        }

        QString lineData = lines[number].mid(position).trimmed();

        if (!lineData.isEmpty()) {
            // Read the column data from the rest of the line.
            QStringList columnStrings = lineData.split("\t", QString::SkipEmptyParts);
            QVector<QVariant> columnData;

            for (int column = 0; column < columnStrings.count(); ++column){
                if (columnStrings[0] == "SAMPLEDATA"){
                     return;
                }
                columnData << columnStrings[column];
            }
            if (position > indentations.last()) {
                // The last child of the current parent is now the new parent
                // unless the current parent has no children.

                if (parents.last()->childCount() > 0) {
                    parents << parents.last()->child(parents.last()->childCount()-1);
                    indentations << position;
                }
            } else {
                while (position < indentations.last() && parents.count() > 0) {
                    parents.pop_back();
                    indentations.pop_back();
                }
            }

            // Append a new item to the current parent's list of children.
            TreeItem *parent = parents.last();
            parent->insertChildren(parent->childCount(), 1, rootItem->columnCount());
            for (int column = 0; column < columnData.size(); ++column){
                parent->child(parent->childCount() - 1)->setData(column, columnData[column]);
            }
        }

        ++number;
    }
}
