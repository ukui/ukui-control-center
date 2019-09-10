#ifndef MOUSE_H
#define MOUSE_H

#include <QObject>
#include <QtPlugin>
#include <gio/gio.h>
#include "mainui/interface.h"
#include "ui_keyboard.h"
#include "keyboard_xkb.h"
namespace Ui {
    class KeyboardWidget;
}

class Keyboard : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface" )
    Q_INTERFACES(CommonInterface)

public:
    Keyboard(); 
    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    QWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    ~Keyboard();
private Q_SLOTS:
    void on_checkBox_stateChanged(int arg1);

    void on_horizontalSlider_valueChanged(int value);

    void on_horizontalSlider_2_valueChanged(int value);

    void on_checkBox_2_stateChanged(int arg1);

    void on_horizontalSlider_3_valueChanged(int value);

    void on_pushButton_3_clicked();

    void on_treeView_clicked(const QModelIndex &index);

    void getCountries();

    void on_pushButton_5_clicked();

private:
    Ui::KeyboardWidget * ui;
    QString pluginName;
    int pluginType;
    QWidget * pluginWidget;
    KeyboardXkb *keyboardXkb;
    GSettings *layout_settings;
    void init_setting();
    void setup_dialog();
    void set_kb_repeat_visible();
    void set_kb_cursorblink_visible();

};

#endif
