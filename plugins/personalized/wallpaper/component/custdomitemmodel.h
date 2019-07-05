#ifndef CUSTDOMITEMMODEL_H
#define CUSTDOMITEMMODEL_H

#include <QAbstractTableModel>
#include <QStandardItem>
//#include <QMap>

class CustdomItemModel : public QAbstractTableModel
{
public:
    CustdomItemModel();
    ~CustdomItemModel();

//    virtual QModelIndex index(int row, int column = 0, const QModelIndex & parent = QModelIndex()) const;
//    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) const;
//    void setCurrentMap(const QMap<QString, QMap<QString, QString> > &map);
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

//    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex());
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());

private:
//    QString current_key(int offset) const;
    QStringList rowStringList;
//    QMap<QString, QMap<QString, QString> > currentMap;
    QList<QStandardItem *> itemList;

};

#endif // CUSTDOMITEMMODEL_H
