#ifndef MHOMEDELEGATE_H
#define MHOMEDELEGATE_H

#include <QWidget>
#include <QStyledItemDelegate>


class mHomeDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    mHomeDelegate();
    ~mHomeDelegate();
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // MHOMEDELEGATE_H
