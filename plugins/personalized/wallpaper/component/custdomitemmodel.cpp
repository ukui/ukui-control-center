#include "custdomitemmodel.h"

#include <QDebug>


CustdomItemModel::CustdomItemModel()
{
}

CustdomItemModel::~CustdomItemModel()
{
}

QModelIndex CustdomItemModel::index(int row, int column, const QModelIndex &parent) const {
    if (row >= 0 && row < rowCount() && column >= 0 && column < columnCount()){
        QStandardItem * item = itemList.at(row);
        return createIndex(row, column, (void *)(item));
    }
    return QModelIndex();
}

int CustdomItemModel::columnCount(const QModelIndex &parent) const {
    return 1;
}

int CustdomItemModel::rowCount(const QModelIndex &parent) const {
    return itemList.count();
}

QVariant CustdomItemModel::data(const QModelIndex &index, int role) const {
    qDebug() << role << "**********role***";
    if (!index.isValid())
        return QVariant();
    if (role == Qt::DisplayRole){
        QStandardItem * item = itemList.at(index.row());
        return QVariant(item->text());
    }
    else if (role == Qt::DecorationRole){
        QStandardItem * item = itemList.at(index.row());
        return QVariant(item->icon());
    }
    else if (role == Qt::ToolTipRole){
        QStandardItem * item = itemList.at(index.row());
        return item->toolTip();
    }
    else
        return QVariant();
}

bool CustdomItemModel::setData(const QModelIndex &index, const QVariant &value, int role){
    if (index.isValid() && role == Qt::EditRole){
        QStandardItem * item = itemList.at(index.row());
        item->setText(value.toString());
        emit dataChanged(index, index);
        return true;
    }
    else if (role == Qt::DecorationRole){
        QStandardItem * item = itemList.at(index.row());
        item->setIcon(value.value<QIcon>());
        return true;
    }
    else if (role == Qt::ToolTipRole){
        QStandardItem * item = itemList.at(index.row());
        item->setToolTip(value.toString());
        return true;
    }
    else
        return false;
}

bool CustdomItemModel::insertRows(int row, int count, const QModelIndex &parent){
    beginInsertRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; i++){
        QStandardItem * item = new QStandardItem();
        item->setSizeHint(QSize(160, 160));
        itemList.insert(row, item);
    }
    endInsertRows();
    return true;
}
