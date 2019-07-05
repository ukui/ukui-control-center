#ifndef AUDIO_H
#define AUDIO_H

#include <QWidget>
#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"

namespace Ui {
class Audio;
}

class Audio : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Audio();
    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    QWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    ~Audio();

private:
    Ui::Audio *ui;
    QString pluginName;
    int pluginType;
    QWidget * pluginWidget;

public slots:
    void change_soundtheme_page();
    void change_preference_page();
};

#endif // AUDIO_H
