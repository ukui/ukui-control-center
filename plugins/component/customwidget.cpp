#include "customwidget.h"

CustomWidget::CustomWidget(QWidget *parent) :
    QWidget(parent)
{
}

CustomWidget::~CustomWidget()
{
}

void CustomWidget::emitting_toggle_signal(QString name, int type, int page){
    emit transmit(name, type, page);
}
