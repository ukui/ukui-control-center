#ifndef NOFOCUSFRAMEDELEGATE_H
#define NOFOCUSFRAMEDELEGATE_H

#include <QPainter>
#include <QStyledItemDelegate>

class NoFocusFrameDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit NoFocusFrameDelegate(QWidget *parent = 0);

    void paint(QPainter * painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

};

#endif // NOFOCUSFRAMEDELEGATE_H
