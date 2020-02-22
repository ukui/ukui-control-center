#include "poplistdelegate.h"

PopListDelegate::PopListDelegate(QWidget* parent) : QStyledItemDelegate (parent)
{

}

// 委托类，主要绘制鼠标划过时候的状态
void PopListDelegate::paint(QPainter *painter, const QStyleOptionViewItem& option,const QModelIndex& index) const{
    painter->save();

    QRect rect(option.rect);

    if(option.state.testFlag(QStyle::State_MouseOver)) {
        QColor hoverColor(44, 167, 248);
        painter->fillRect(rect, QBrush(hoverColor));
    }

    QColor textColor(48, 48, 48);
    if(option.state.testFlag(QStyle::State_MouseOver)) {
        textColor = Qt::white;
    }

    painter->setPen(QPen(textColor));
    QString text = index.model()->data(index, Qt::DisplayRole).toString();

      painter->drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, text);
    painter->restore();
}
