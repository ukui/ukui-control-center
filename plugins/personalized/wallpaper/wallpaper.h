#ifndef WALLPAPER_H
#define WALLPAPER_H

#include <QWidget>
#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"
#include <QPixmap>
#include <QListWidgetItem>

#include <QGSettings/QGSettings>

#include "xmlhandle.h"
#include "component/custdomitemmodel.h"
#include "../../component/customwidget.h"

#define BACKGROUND "org.mate.background"

#define FILENAME "picture-filename" //图片文件路径
#define OPACITY "picture-opacity" //图片不透明度
#define OPTIONS "picture-options" //图片放置方式
#define PRIMARY "primary-color" //主色
#define SECONDARY "secondary-color" //副色

namespace Ui {
class Wallpaper;
}

class Wallpaper : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Wallpaper();
    ~Wallpaper();

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    CustomWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;

private:
    Ui::Wallpaper *ui;

    QString pluginName;
    int pluginType;
    CustomWidget * pluginWidget;

    QMap<QString, QString> headinfoMap;
    QMap<QString, QMap<QString, QString> > wallpaperinfosMap;

    XmlHandle * xmlhandleObj;
    QGSettings * bgsettings;
    QString localwpconf;
    QMap<QString, QListWidgetItem*> delItemsMap;

    CustdomItemModel wpListModel;

    void initData();
    void component_init();
    void init_current_status();

    //尝试mode view
    void setlistview();
    void setModeldata();

public slots:
    void wallpaper_item_clicked(QListWidgetItem *item);
    void form_combobox_changed(int index);
    void options_combobox_changed(QString op);
};

#endif // WALLPAPER_H
