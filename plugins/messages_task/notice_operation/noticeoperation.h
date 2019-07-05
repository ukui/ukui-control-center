#ifndef NOTICEOPERATION_H
#define NOTICEOPERATION_H

#include <QWidget>
#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"

namespace Ui {
class NoticeOperation;
}

class NoticeOperation : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    NoticeOperation();
    ~NoticeOperation();

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    QWidget * get_plugin_ui() Q_DECL_OVERRIDE;

private:
    Ui::NoticeOperation *ui;

    QString pluginName;
    int pluginType;
    QWidget * pluginWidget;
};

#endif // NOTICEOPERATION_H
