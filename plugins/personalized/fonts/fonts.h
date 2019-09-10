#ifndef FONTS_H
#define FONTS_H

#include <QWidget>
#include <QPushButton>
#include <QAbstractButton>
#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"
#include "../../component/customwidget.h"

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

extern "C" {
#include <glib.h>
#include <gio/gio.h>
#include <stdlib.h>
}

#include <QGSettings/QGSettings>

#define INTERFACE_SCHEMA "org.mate.interface"
#define DOC_FONT_KEY "document-font-name" //用于阅读文档的默认字体的名称
#define GTK_FONT_KEY "font-name" //gkt+使用的默认字体
#define MONOSPACE_FONT_KEY "monospace-font-name" //用于终端等处的等宽字体

#define MARCO_SCHEMA "org.gnome.desktop.wm.preferences"
#define TITLEBAR_FONT_KEY "titlebar-font" //描述窗口标题栏字体的字符串。只有在"titlebar-uses-system-font"为false时有效


#define PEONY_SCHEMA "org.ukui.peony.desktop"
#define PEONY_FONT_KEY "font"  //桌面上图标描述所用的字体

#define FONT_RENDER_SCHEMA           "org.ukui.font-rendering"
#define ANTIALIASING_KEY        "antialiasing" //绘制字形时使用反锯齿类型
#define HINTING_KEY             "hinting" //绘制字形时使用微调的类型
#define RGBA_ORDER_KEY          "rgba-order" //LCD屏幕上次像素的顺序；仅在反锯齿设为"rgba"时有用
#define DPI_KEY                 "dpi" //将字体尺寸转换为像素值时所用的分辨率，以每英寸点数为单位

namespace Ui {
class Fonts;
}

class Fonts : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Fonts();
    ~Fonts();

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    CustomWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;

    void component_init();
    void status_init();
    void get_default_fontinfo();
    void get_current_fonts();
    QStringList split_fontname_size(QString value);

private:
    Ui::Fonts *ui;

    QString pluginName;
    int pluginType;
    CustomWidget * pluginWidget;

    QGSettings * ifsettings;
    QGSettings * marcosettings;
    QGSettings * peonysettings;
    QGSettings * rendersettings;

    QStringList gtkfontStrList;
    QStringList docfontStrList;
    QStringList monospacefontStrList;
    QStringList peonyfontStrList;
    QStringList titlebarfontStrList;

private slots:
    void reset_default_slot();
    void combobox_changed_slot(QString text);
    void radiobtn_clicked_slot(int indexnum);
    void pushbtn_clicked_slot(QAbstractButton * button);

};

#endif // FONTS_H
