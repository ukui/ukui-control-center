#include "poplistdelegate.h"

PopListDelegate::PopListDelegate(QWidget* parent) : QStyledItemDelegate (parent)
{

}

// 委托类，主要绘制鼠标划过时候的状态
void PopListDelegate::paint(QPainter *painter, const QStyleOptionViewItem& option,const QModelIndex& index) const{
    painter->save();

    painter->setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter->setPen(Qt::transparent);
    QRect rect(option.rect);
    rect.setWidth(rect.width() - 1);
    rect.setHeight(rect.height() - 1);

    if(option.state.testFlag(QStyle::State_MouseOver)) {
        QColor hoverColor(44, 167, 248);
        painter->setBrush(QBrush(hoverColor));
        painter->drawRoundedRect(rect, 4, 4);
    }

    QPalette pal;
    QBrush brush = pal.buttonText();
    QColor textColor = brush.color();
    if(option.state.testFlag(QStyle::State_MouseOver)) {
        textColor = Qt::white;
    }

    painter->setPen(QPen(textColor));
    QString text = index.model()->data(index, Qt::DisplayRole).toString();
    painter->drawText(QRect(rect.x() + 8,rect.y(),rect.width(),rect.height()), Qt::AlignLeft | Qt::AlignVCenter, text);
    painter->restore();
}
