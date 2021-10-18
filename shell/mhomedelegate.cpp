#include "mhomedelegate.h"

mHomeDelegate::mHomeDelegate()
{

}

mHomeDelegate::~mHomeDelegate()
{

}

void mHomeDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
        // option.rect属性中保存了Item的位置
        editor->setGeometry(option.rect);
}
