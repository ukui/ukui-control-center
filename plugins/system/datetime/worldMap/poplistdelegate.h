#ifndef POPLISTDELEGATE_H
#define POPLISTDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QColor>

// 委托类，使用当前的样式绘制list
class PopListDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    PopListDelegate(QWidget* parent = nullptr);

protected:
    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
};

#endif // POPLISTDELEGATE_H
