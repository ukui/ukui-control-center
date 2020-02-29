/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "fonts.h"
#include "ui_fonts.h"

#include <QDebug>

#define N 3
#define SMALL 1.00
#define MEDIUM 1.25
#define LARGE 1.50


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

/*
  设置字体，每套字体包括5个部件，应用程序字体、文档字体、等宽字体、桌面字体和窗口标题字体。
  字体设置为预设值，每套字体除大小不固定，其他字体类别固定。
  目前有
  ubuntu字体，分别为"ubuntu" "sans" "ubuntu mono" "' '" "ubuntu Bold"
  mate字体，分别为"ubuntu" "ubuntu" "ubuntu mono" "sans" "ubuntu Bold"
  ukui字体，分别为"ubuntu","ubuntu","ubuntu mono","ubuntu","ubuntu medium"
*/

typedef struct _FontInfo FontInfo;
struct _FontInfo{
    QString type;
    QString gtkfont;
    QString docfont;
    QString monospacefont;
    QString peonyfont;
    QString titlebarfont;
    int gtkfontsize;
    int docfontsize;
    int monospacefontsize;
    int peonyfontsize;
    int titlebarfontsize;
};

FontInfo defaultfontinfo;

FontInfo fontinfo[N] = {
    {"Ubuntu",
    "Ubuntu",
    "Sans",
    "Ubuntu Mono",
    "Ubuntu",
    "Ubuntu Bold",
    0, 0, 0, 0, 0},
    {"Mate",
    "Ubuntu",
    "Ubuntu",
    "Ubuntu Mono",
    "Sans",
    "Ubuntu Bold",
    0, 0, 0, 0, 0},
    {"Ukui",
    "Ubuntu",
    "Ubuntu",
    "Ubuntu Mono",
    "Ubuntu",
    "Ubuntu Medium",
    0, 0, 0, 0, 0}
};

//字体效果
typedef enum{
    ANTIALIASING_NONE,
    ANTIALIASING_GRAYSCALE,
    ANTIALIASING_RGBA
}Antialiasing;

typedef enum{
    HINT_NONE,
    HINT_SLIGHT,
    HINT_MEDIUM,
    HINT_FULL
}Hinting;

struct FontEffects : QObjectUserData {
    Antialiasing antial;
    Hinting hinting;
};

QList<int> defaultsizeList = {6, 7, 8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72};

Fonts::Fonts()
{
    ui = new Ui::Fonts;
    itemDelege = new QStyledItemDelegate();
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("fonts");
    pluginType = PERSONALIZED;


    pluginWidget->setStyleSheet("background: #ffffff;");
    ui->fontSizeWidget->setStyleSheet("QWidget{background: #F4F4F4; border-radius: 4px;}");
//    ui->fontSizeLabel->setStyleSheet("QLabel{background: #F4F4F4;}");

    ui->fontTypeWidget->setStyleSheet("QWidget#fontTypeWidget{background: #F4F4F4; border-radius: 4px;}");
    ui->fontSelectLabel->setStyleSheet("QLabel{background: #F4F4F4;}");
    ui->monoSelectLabel->setStyleSheet("QLabel{background: #F4F4F4;}");


    ui->advancedBtn->setStyleSheet("QPushButton#advancedBtn{background: #F4F4F4; border: none;}");

    ui->advancedWidget->setStyleSheet("QWidget#advancedWidget{background: #F4F4F4; border-radius: 4px;}");
    ui->defaultFontLabel->setStyleSheet("QLabel{background: #F4F4F4;}");
    ui->monoFontLabel->setStyleSheet("QLabel{background: #F4F4F4;}");
    ui->docFontLabel->setStyleSheet("QLabel{background: #F4F4F4;}");
    ui->peonyFontLabel->setStyleSheet("QLabel{background: #F4F4F4;}");
    ui->titleFontLabel->setStyleSheet("QLabel{background: #F4F4F4;}");


    ui->sampleBtn1->setStyleSheet("QPushButton{background: #F0F0F0; border-radius: 4px;}"
                                  "QPushButton:checked{background: #F0F3FD; border-radius: 4px;}");
    ui->sampleBtn2->setStyleSheet("QPushButton{background: #F0F0F0; border-radius: 4px;}"
                                  "QPushButton:checked{background: #F0F3FD; border-radius: 4px;}");
    ui->sampleBtn3->setStyleSheet("QPushButton{background: #F0F0F0; border-radius: 4px;}"
                                  "QPushButton:checked{background: #F0F3FD; border-radius: 4px;}");
    ui->sampleBtn4->setStyleSheet("QPushButton{background: #F0F0F0; border-radius: 4px;}"
                                  "QPushButton:checked{background: #F0F3FD; border-radius: 4px;}");

    ui->fontSizeSlider->setMinimum(100);
    ui->fontSizeSlider->setMaximum(275);
    ui->fontSizeSlider->setSingleStep(25);
    ui->fontSizeSlider->setPageStep(25);
    ui->fontSizeSlider->setTickPosition(QSlider::TicksBelow);

    const QByteArray id(INTERFACE_SCHEMA);
    ifsettings = new QGSettings(id);
    const QByteArray idd(MARCO_SCHEMA);
    marcosettings = new QGSettings(idd);
    const QByteArray iddd(PEONY_SCHEMA);
    peonysettings = new QGSettings(iddd);
    const QByteArray iid(FONT_RENDER_SCHEMA);
    rendersettings = new QGSettings(iid);

    _getDefaultFontinfo();

    initComponent();
    initFontStatus();

}

Fonts::~Fonts()
{
    delete ui;
    delete ifsettings;
    delete marcosettings;
    delete peonysettings;
    delete rendersettings;
}

QString Fonts::get_plugin_name(){
    return pluginName;
}

int Fonts::get_plugin_type(){
    return pluginType;
}

QWidget *Fonts::get_plugin_ui(){
    return pluginWidget;
}

void Fonts::plugin_delay_control(){

}

void Fonts::_getDefaultFontinfo(){
    GVariant * value;
    const char * font_value;
    gsize size; gint length;

    //GTK FONT
    GSettings * ifgsettings;
    ifgsettings = g_settings_new(INTERFACE_SCHEMA);
    value = g_settings_get_default_value(ifgsettings, GTK_FONT_KEY);
    size = g_variant_get_size(value);
    font_value = g_variant_get_string(value, &size);
    length = (gint)strlen(font_value);
    if (font_value[length-2] == ' '){
        defaultfontinfo.gtkfontsize = atoi(&font_value[length - 1]);
    }
    else
        defaultfontinfo.gtkfontsize = atoi(&font_value[length - 2]);
    g_variant_unref(value);

    //DOC FONT
    value = g_settings_get_default_value(ifgsettings, DOC_FONT_KEY);
    size = g_variant_get_size(value);
    font_value = g_variant_get_string(value, &size);
    length = (gint)strlen(font_value);
    if (font_value[length -2] == ' ')
        defaultfontinfo.docfontsize = atoi(&font_value[length -1]);
    else
        defaultfontinfo.docfontsize = atoi(&font_value[length -2]);
    g_variant_unref(value);

    //MON
    value = g_settings_get_default_value(ifgsettings, MONOSPACE_FONT_KEY);
    size = g_variant_get_size(value);
    font_value = g_variant_get_string(value, &size);
    length = (gint)strlen(font_value);
    if (font_value[length -2] == ' ')
        defaultfontinfo.monospacefontsize = atoi(&font_value[length -1]);
    else
        defaultfontinfo.monospacefontsize = atoi(&font_value[length -2]);
    g_variant_unref(value);

    //PEONY
    GSettings * peonygsettings;
    peonygsettings = g_settings_new(PEONY_SCHEMA);
    value = g_settings_get_default_value(peonygsettings, PEONY_FONT_KEY);
    size = g_variant_get_size(value);
    font_value = g_variant_get_string(value, &size);
    length = (gint)strlen(font_value);
    if (font_value[length -2] == ' ')
        defaultfontinfo.peonyfontsize = atoi(&font_value[length -1]);
    else
        defaultfontinfo.peonyfontsize = atoi(&font_value[length -2]);
    g_variant_unref(value);

    //TITLE
    GSettings * marcogsettings;
    marcogsettings = g_settings_new(MARCO_SCHEMA);
    value = g_settings_get_default_value(marcogsettings, TITLEBAR_FONT_KEY);
    size = g_variant_get_size(value);
    font_value = g_variant_get_string(value, &size);
    length = (gint)strlen(font_value);
    if (font_value[length -2] == ' ')
        defaultfontinfo.titlebarfontsize = atoi(&font_value[length -1]);
    else
        defaultfontinfo.titlebarfontsize = atoi(&font_value[length -2]);
    g_variant_unref(value);

    g_object_unref(ifgsettings);
    g_object_unref(peonygsettings);
    g_object_unref(marcogsettings);
}

void Fonts::_getCurrentFontInfo(){
    gtkfontStrList = _splitFontNameSize(ifsettings->get(GTK_FONT_KEY).toString());
    docfontStrList = _splitFontNameSize(ifsettings->get(DOC_FONT_KEY).toString());
    monospacefontStrList = _splitFontNameSize(ifsettings->get(MONOSPACE_FONT_KEY).toString());
    peonyfontStrList = _splitFontNameSize(peonysettings->get(PEONY_FONT_KEY).toString());
    titlebarfontStrList = _splitFontNameSize(marcosettings->get(TITLEBAR_FONT_KEY).toString());
}

QStringList Fonts::_splitFontNameSize(QString value){
    QStringList valueStringList;
    if (value.right(1) >= '0' && value.right(1) <= '9'){
        QStringList tmpStringList = value.split(' ');
        QString::SectionFlag flag = QString::SectionSkipEmpty;
        valueStringList << value.section(' ', 0, tmpStringList.length() - 2, flag);
        valueStringList << value.section(' ', tmpStringList.length() - 1, tmpStringList.length() - 1, flag);
    }
    else{
        valueStringList << value << "10";
    }
    return valueStringList;
}

void Fonts::get_current_fonts(){

}

void Fonts::initComponent(){
    ui->fontSelectComBox->setItemDelegate(itemDelege);
    ui->fontSelectComBox->setMaxVisibleItems(5);

    ui->monoSelectComBox->setItemDelegate(itemDelege);
    ui->monoSelectComBox->setMaxVisibleItems(5);

    ui->defaultFontComBox->setItemDelegate(itemDelege);
    ui->defaultFontComBox->setMaxVisibleItems(5);

    ui->docFontComBox->setItemDelegate(itemDelege);
    ui->docFontComBox->setMaxVisibleItems(5);

    ui->peonyFontComBox->setItemDelegate(itemDelege);
    ui->peonyFontComBox->setMaxVisibleItems(5);

    ui->monoFontComBox->setItemDelegate(itemDelege);
    ui->monoFontComBox->setMaxVisibleItems(5);

    ui->titleFontComBox->setItemDelegate(itemDelege);
    ui->titleFontComBox->setMaxVisibleItems(5);

    ui->defaultSizeComBox->setItemDelegate(itemDelege);
    ui->defaultSizeComBox->setMaxVisibleItems(5);

    ui->docSizeComBox->setItemDelegate(itemDelege);
    ui->docSizeComBox->setMaxVisibleItems(5);

    ui->MonoSizeComBox->setItemDelegate(itemDelege);
    ui->MonoSizeComBox->setMaxVisibleItems(5);

    ui->peonySizeComBox->setItemDelegate(itemDelege);
    ui->peonySizeComBox->setMaxVisibleItems(5);

    ui->titleSizeComBox->setItemDelegate(itemDelege);
    ui->titleSizeComBox->setMaxVisibleItems(5);

    //导入系统字体列表
    QStringList fontfamiles = fontdb.families();
    for (QString font : fontfamiles){
        ////通用设置
        //字体
        ui->fontSelectComBox->addItem(font);
        //等宽字体
        ui->monoSelectComBox->addItem(font);
        ////高级设置
        // gtk default
        ui->defaultFontComBox->addItem(font);
        //doc font
        ui->docFontComBox->addItem(font);
        // peony font
        ui->peonyFontComBox->addItem(font);
        //monospace font
        ui->monoFontComBox->addItem(font);
        //title font
        ui->titleFontComBox->addItem(font);
    }

    //高级字体按钮状态
    ui->advancedWidget->setVisible(ui->advancedBtn->isChecked());
    connect(ui->advancedBtn, &QPushButton::clicked, this, [=]{
        ui->advancedWidget->setVisible(ui->advancedBtn->isChecked());
    });

    ////导入字体大小列表
    //获取当前字体
    QStringList gtkfontStrList = _splitFontNameSize(ifsettings->get(GTK_FONT_KEY).toString());
    QStringList docfontStrList = _splitFontNameSize(ifsettings->get(DOC_FONT_KEY).toString());
    QStringList monospacefontStrList = _splitFontNameSize(ifsettings->get(MONOSPACE_FONT_KEY).toString());
    QStringList peonyfontStrList = _splitFontNameSize(peonysettings->get(PEONY_FONT_KEY).toString());
    QStringList titlebarfontStrList = _splitFontNameSize(marcosettings->get(TITLEBAR_FONT_KEY).toString());

    QList<int> gtksizeList = fontdb.pointSizes(gtkfontStrList.at(0));
    QList<int> docsizeList = fontdb.pointSizes(docfontStrList.at(0));
    QList<int> monosizeList = fontdb.pointSizes(monospacefontStrList.at(0));
    QList<int> peonysizeList = fontdb.pointSizes(peonyfontStrList.at(0));
    QList<int> titlesizeList = fontdb.pointSizes(titlebarfontStrList.at(0));

    if (gtksizeList.length() == 0)
        gtksizeList = defaultsizeList;
    for (int size : gtksizeList)
        ui->defaultSizeComBox->addItem(QString::number(size));
    if (docsizeList.length() == 0)
        docsizeList = defaultsizeList;
    for (int size : docsizeList)
        ui->docSizeComBox->addItem(QString::number(size));
    if (monosizeList.length() == 0)
        monosizeList = defaultsizeList;
    for (int size : monosizeList)
        ui->MonoSizeComBox->addItem(QString::number(size));
    if (peonysizeList.length() == 0)
        peonysizeList = defaultsizeList;
    for (int size : peonysizeList)
        ui->peonySizeComBox->addItem(QString::number(size));
    if (titlesizeList.length() == 0)
        titlesizeList = defaultsizeList;
    for (int size : titlesizeList)
        ui->titleSizeComBox->addItem(QString::number(size));

    //init sample button
    FontEffects * example1 = new FontEffects();
    FontEffects * example2 = new FontEffects();
    FontEffects * example3 = new FontEffects();
    FontEffects * example4 = new FontEffects();
    example1->antial = ANTIALIASING_NONE; example1->hinting = HINT_FULL;
    example2->antial = ANTIALIASING_GRAYSCALE; example2->hinting = HINT_FULL;
    example3->antial = ANTIALIASING_GRAYSCALE; example3->hinting = HINT_MEDIUM;
    example4->antial = ANTIALIASING_RGBA; example4->hinting = HINT_FULL;

    ui->sampleBtn1->setUserData(Qt::UserRole, example1);
    ui->sampleBtn2->setUserData(Qt::UserRole, example2);
    ui->sampleBtn3->setUserData(Qt::UserRole, example3);
    ui->sampleBtn4->setUserData(Qt::UserRole, example4);


    ////绑定信号
    //字体效果按钮
    connect(ui->sampleBtnGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(setFontEffectSlot(QAbstractButton*)));


}

void Fonts::initFontStatus(){
    //初始化字体ComBox状态
    initGeneralFontStatus();

    //初始化高级字体ComBox状态
    initAdvancedFontStatus();

    //初始化示例按钮状态
    initSampleFontStatus();
}

void Fonts::initGeneralFontStatus(){
    //获取当前字体信息
    _getCurrentFontInfo();

    //初始化字体选择ComBox(选择gtk默认字体作为标准)
    QString currentfonts = gtkfontStrList[0];
    ui->fontSelectComBox->blockSignals(true);
    ui->fontSelectComBox->setCurrentText(currentfonts);
    ui->fontSelectComBox->blockSignals(true);

    //初始化等宽字体ComBox
    QString currentmonofont = monospacefontStrList[0];
    ui->monoSelectComBox->blockSignals(true);
    ui->monoSelectComBox->setCurrentText(currentmonofont);
    ui->monoSelectComBox->blockSignals(false);

    //获取字体大小倍率,选择文档字体大小作为标准，来自gtk控制面板的逻辑
    float res = QString(docfontStrList[1]).toFloat() / (float)defaultfontinfo.docfontsize;
    int value = int(res * 100);

    ui->fontSizeSlider->blockSignals(true);

    ui->fontSizeSlider->setValue(value);

    ui->fontSizeSlider->blockSignals(false);

}

void Fonts::initAdvancedFontStatus(){
    //获取当前字体信息
    _getCurrentFontInfo();

    //阻塞信号
    ui->defaultFontComBox->blockSignals(true);
    ui->docFontComBox->blockSignals(true);
    ui->monoFontComBox->blockSignals(true);
    ui->peonyFontComBox->blockSignals(true);
    ui->titleFontComBox->blockSignals(true);

    //初始化高级字体ComBox
    ui->defaultFontComBox->setCurrentText(gtkfontStrList.at(0));
    ui->docFontComBox->setCurrentText(docfontStrList.at(0));
    ui->monoFontComBox->setCurrentText(monospacefontStrList.at(0));
    ui->peonyFontComBox->setCurrentText(peonyfontStrList.at(0));
    ui->titleFontComBox->setCurrentText(titlebarfontStrList.at(0));

    //释放信号
    ui->defaultFontComBox->blockSignals(false);
    ui->docFontComBox->blockSignals(false);
    ui->monoFontComBox->blockSignals(false);
    ui->peonyFontComBox->blockSignals(false);
    ui->titleFontComBox->blockSignals(false);

    //阻塞字体大小ComBox信号
    ui->defaultSizeComBox->blockSignals(true);
    ui->docSizeComBox->blockSignals(true);
    ui->MonoSizeComBox->blockSignals(true);
    ui->peonySizeComBox->blockSignals(true);
    ui->titleSizeComBox->blockSignals(true);

    ui->defaultSizeComBox->setCurrentText(gtkfontStrList.at(1));
    ui->docSizeComBox->setCurrentText(docfontStrList.at(1));
    ui->MonoSizeComBox->setCurrentText(monospacefontStrList.at(1));
    ui->peonySizeComBox->setCurrentText(peonyfontStrList.at(1));
    ui->titleSizeComBox->setCurrentText(titlebarfontStrList.at(1));

    //阻塞字体大小ComBox信号
    ui->defaultSizeComBox->blockSignals(false);
    ui->docSizeComBox->blockSignals(false);
    ui->MonoSizeComBox->blockSignals(false);
    ui->peonySizeComBox->blockSignals(false);
    ui->titleSizeComBox->blockSignals(false);

}

void Fonts::initSampleFontStatus(){
    //初始化字体效果按钮
    GSettings * settings = g_settings_new(FONT_RENDER_SCHEMA);
    int currentantial = g_settings_get_enum(settings, ANTIALIASING_KEY);
    int currenthinting = g_settings_get_enum(settings, HINTING_KEY);
    QList<QAbstractButton *> buttonsList = ui->sampleBtnGroup->buttons();
    for (int num = 0; num < buttonsList.size(); num++){
        FontEffects * btnFontEffects = (FontEffects *)((QPushButton *)buttonsList[num])->userData(Qt::UserRole);
        if (currentantial == btnFontEffects->antial && currenthinting == btnFontEffects->hinting){
            QPushButton * button = ((QPushButton *)buttonsList[num]);
            button->blockSignals(true);
            button->setChecked(true);
            button->blockSignals(false);
        }
    }
    g_object_unref(settings);
}

void Fonts::refresh_mainpage_status(){
    //获取当前字体集合
    QString currentfonts = "";
    QStringList gtkfontStrList = _splitFontNameSize(ifsettings->get(GTK_FONT_KEY).toString());
    QStringList docfontStrList = _splitFontNameSize(ifsettings->get(DOC_FONT_KEY).toString());
    QStringList monospacefontStrList = _splitFontNameSize(ifsettings->get(MONOSPACE_FONT_KEY).toString());
    QStringList peonyfontStrList = _splitFontNameSize(peonysettings->get(PEONY_FONT_KEY).toString());
    QStringList titlebarfontStrList = _splitFontNameSize(marcosettings->get(TITLEBAR_FONT_KEY).toString());

    //设置当前字体集
    for (int i = 0; i < N; i++){
        if (fontinfo[i].docfont != docfontStrList[0])
            continue;
        if (fontinfo[i].gtkfont != gtkfontStrList[0])
            continue;
        if (fontinfo[i].monospacefont != monospacefontStrList[0])
            continue;
        if (fontinfo[i].titlebarfont != titlebarfontStrList[0])
            continue;
        if (fontinfo[i].peonyfont != peonyfontStrList[0])
            continue;
        currentfonts = fontinfo[i].type;
    }

//    ui->fontsComboBox->blockSignals(true);
//    if (currentfonts == "") //未匹配上，自定义
//        ui->fontsComboBox->setCurrentIndex(0);
//    else
//        ui->fontsComboBox->setCurrentText(currentfonts);
//    ui->fontsComboBox->blockSignals(false);

//    //设置字体大小,选择文档字体大小作为标准，来自gtk控制面板的逻辑
//    float res = QString(docfontStrList[1]).toFloat() / (float)defaultfontinfo.docfontsize;

//    if (res == SMALL){
//        ui->smallRadioBtn->blockSignals(true);
//        ui->smallRadioBtn->setChecked(true);
//        ui->smallRadioBtn->blockSignals(false);
//    }
//    else if (res > SMALL && res <= MEDIUM){
//        ui->mediumRadioBtn->blockSignals(true);
//        ui->mediumRadioBtn->setChecked(true);
//        ui->mediumRadioBtn->blockSignals(false);
//    }
//    else if (res > MEDIUM){
//        ui->largerRadioBtn->blockSignals(true);
//        ui->largerRadioBtn->setChecked(true);
//        ui->largerRadioBtn->blockSignals(false);
//    }

    //设置当前字体效果
    GSettings * settings = g_settings_new(FONT_RENDER_SCHEMA);
//    int currentantial = g_settings_get_enum(settings, ANTIALIASING_KEY);
//    int currenthinting = g_settings_get_enum(settings, HINTING_KEY);
//    QList<QAbstractButton *> buttonsList = ui->pushBtnbuttonGroup->buttons();
//    for (int num = 0; num < buttonsList.size(); num++){
//        FontEffects * btnFontEffects = (FontEffects *)((QPushButton *)buttonsList[num])->userData(Qt::UserRole);
//        if (currentantial == btnFontEffects->antial && currenthinting == btnFontEffects->hinting){
//            QPushButton * button = ((QPushButton *)buttonsList[num]);
//            button->blockSignals(true);
//            button->setChecked(true);
//            button->blockSignals(false);
//        }
//    }
    g_object_unref(settings);
}

void Fonts::refresh_subpage_status(){

    //获取当前字体集合
    QString currentfonts = "";
    QStringList gtkfontStrList = _splitFontNameSize(ifsettings->get(GTK_FONT_KEY).toString());
    QStringList docfontStrList = _splitFontNameSize(ifsettings->get(DOC_FONT_KEY).toString());
    QStringList monospacefontStrList = _splitFontNameSize(ifsettings->get(MONOSPACE_FONT_KEY).toString());
    QStringList peonyfontStrList = _splitFontNameSize(peonysettings->get(PEONY_FONT_KEY).toString());
    QStringList titlebarfontStrList = _splitFontNameSize(marcosettings->get(TITLEBAR_FONT_KEY).toString());

    //
//    ui->defaultfontCbx->blockSignals(true);
//    ui->docfontCbx->blockSignals(true);
//    ui->monofontCbx->blockSignals(true);
//    ui->peonyfontCbx->blockSignals(true);
//    ui->titlefontCbx->blockSignals(true);
//    ui->defaultfontCbx->setCurrentText(gtkfontStrList.at(0));
//    ui->docfontCbx->setCurrentText(docfontStrList.at(0));
//    ui->monofontCbx->setCurrentText(monospacefontStrList.at(0));
//    ui->peonyfontCbx->setCurrentText(peonyfontStrList.at(0));
//    ui->titlefontCbx->setCurrentText(titlebarfontStrList.at(0));
//    ui->defaultfontCbx->blockSignals(false);
//    ui->docfontCbx->blockSignals(false);
//    ui->monofontCbx->blockSignals(false);
//    ui->peonyfontCbx->blockSignals(false);
//    ui->titlefontCbx->blockSignals(false);

//    ui->defaultsizeCbx->blockSignals(true);
//    ui->docsizeCbx->blockSignals(true);
//    ui->monosizeCbx->blockSignals(true);
//    ui->peonysizeCbx->blockSignals(true);
//    ui->titlesizeCbx->blockSignals(true);
//    ui->defaultsizeCbx->setCurrentText(gtkfontStrList.at(1));
//    ui->docsizeCbx->setCurrentText(docfontStrList.at(1));
//    ui->monosizeCbx->setCurrentText(monospacefontStrList.at(1));
//    ui->peonysizeCbx->setCurrentText(peonyfontStrList.at(1));
//    ui->titlesizeCbx->setCurrentText(titlebarfontStrList.at(1));
//    ui->defaultsizeCbx->blockSignals(false);
//    ui->docsizeCbx->blockSignals(false);
//    ui->monosizeCbx->blockSignals(false);
//    ui->peonysizeCbx->blockSignals(false);
//    ui->titlesizeCbx->blockSignals(false);
}

void Fonts::refresh_status(){
//    refresh_mainpage_status();
//    refresh_subpage_status();
}

void Fonts::status_init(){
//    refresh_status();
    //设置状态后绑定slot
//    connect(ui->fontsComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(combobox_changed_slot(QString)));
//    connect(ui->radioBtnbuttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(radiobtn_clicked_slot(int)));
//    connect(ui->pushBtnbuttonGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(pushbtn_clicked_slot(QAbstractButton*)));
//    connect(ui->resetBtn, SIGNAL(clicked()), this, SLOT(reset_default_slot()));
//    connect(ui->advancedBtn, &QPushButton::clicked, this, [=]{ui->StackedWidget->setCurrentIndex(1);});

//    connect(ui->defaultfontCbx, &ComboBox::currentTextChanged, this, [=](QString text){
//        rebuild_size_combo(ui->defaultsizeCbx, text);
//        ifsettings->set(GTK_FONT_KEY, QVariant(QString("%1 %2").arg(text).arg(ui->defaultsizeCbx->currentText())));
//        refresh_mainpage_status();
//    });
//    connect(ui->docfontCbx, &ComboBox::currentTextChanged, this, [=](QString text){
//        rebuild_size_combo(ui->docsizeCbx, text);
//        ifsettings->set(DOC_FONT_KEY, QVariant(QString("%1 %2").arg(text).arg(ui->docsizeCbx->currentText())));
//        refresh_mainpage_status();
//    });
//    connect(ui->monofontCbx, &ComboBox::currentTextChanged, this, [=](QString text){
//        rebuild_size_combo(ui->monosizeCbx, text);
//        ifsettings->set(MONOSPACE_FONT_KEY, QVariant(QString("%1 %2").arg(text).arg(ui->monosizeCbx->currentText())));
//        refresh_mainpage_status();
//    });
//    connect(ui->peonyfontCbx, &ComboBox::currentTextChanged, this, [=](QString text){
//        rebuild_size_combo(ui->peonysizeCbx, text);
//        peonysettings->set(PEONY_FONT_KEY, QVariant(QString("%1 %2").arg(text).arg(ui->peonysizeCbx->currentText())));
//        refresh_mainpage_status();
//    });
//    connect(ui->titlefontCbx, &ComboBox::currentTextChanged, this, [=](QString text){
//        rebuild_size_combo(ui->titlesizeCbx, text);
//        marcosettings->set(TITLEBAR_FONT_KEY, QVariant(QString("%1 %2").arg(text).arg(ui->titlesizeCbx->currentText())));
//        refresh_mainpage_status();
//    });

//    connect(ui->defaultsizeCbx, &ComboBox::currentTextChanged, this, [=](QString text){
//        ifsettings->set(GTK_FONT_KEY, QVariant(QString("%1 %2").arg(ui->defaultfontCbx->currentText()).arg(text)));
//        refresh_mainpage_status();
//    });
//    connect(ui->docsizeCbx, &ComboBox::currentTextChanged, this, [=](QString text){
//        ifsettings->set(DOC_FONT_KEY, QVariant(QString("%1 %2").arg(ui->docfontCbx->currentText()).arg(text)));
//        refresh_mainpage_status();
//    });
//    connect(ui->monosizeCbx, &ComboBox::currentTextChanged, this, [=](QString text){
//        ifsettings->set(MONOSPACE_FONT_KEY, QVariant(QString("%1 %2").arg(ui->monofontCbx->currentText()).arg(text)));
//        refresh_mainpage_status();
//    });
//    connect(ui->peonysizeCbx, &ComboBox::currentTextChanged, this, [=](QString text){
//        peonysettings->set(PEONY_FONT_KEY, QVariant(QString("%1 %2").arg(ui->peonyfontCbx->currentText()).arg(text)));
//        refresh_mainpage_status();
//    });
//    connect(ui->titlesizeCbx, &ComboBox::currentTextChanged, this, [=](QString text){
//        marcosettings->set(TITLEBAR_FONT_KEY, QVariant(QString("%1 %2").arg(ui->titlefontCbx->currentText()).arg(text)));
//        refresh_mainpage_status();
//    });

//    connect(ui->defaultBtn, &QPushButton::clicked, [=]{
//        ifsettings->reset(GTK_FONT_KEY);
//        refresh_status();
//    });
//    connect(ui->docBtn, &QPushButton::clicked, [=]{
//        ifsettings->reset(DOC_FONT_KEY);
//        refresh_status();
//    });
//    connect(ui->monoBtn, &QPushButton::clicked, [=]{
//        ifsettings->reset(MONOSPACE_FONT_KEY);
//        refresh_status();
//    });
//    connect(ui->peonyBtn, &QPushButton::clicked, [=]{
//        peonysettings->reset(PEONY_FONT_KEY);
//        refresh_status();
//    });
//    connect(ui->titleBtn, &QPushButton::clicked, [=]{
//        marcosettings->reset(TITLEBAR_FONT_KEY);
//        refresh_status();
//    });
}

//void Fonts::rebuild_size_combo(ComboBox *combo, QString text){
//    combo->blockSignals(true);
//    QString oldsize = combo->currentText();
//    combo->removewidgetItems();
//    QList<int> sizeList = fontdb.pointSizes(text);
//    if (sizeList.length() == 0)
//        sizeList = defaultsizeList;
//    for (int size : sizeList)
//        combo->addwidgetItem(QString::number(size));
//    combo->setCurrentText(oldsize);

//    combo->blockSignals(false);
//}

void Fonts::setFontEffectSlot(QAbstractButton *button){
    QPushButton * btnclicked = (QPushButton *)button;
    FontEffects * setFontEffects = (FontEffects *)(btnclicked->userData(Qt::UserRole));

    GSettings * settings = g_settings_new(FONT_RENDER_SCHEMA);
    g_settings_set_enum(settings, ANTIALIASING_KEY, setFontEffects->antial);
    g_settings_set_enum(settings, HINTING_KEY, setFontEffects->hinting);
    g_object_unref(settings);
}

void Fonts::resetDefaultSlot(){
    //reset font
    ifsettings->reset(GTK_FONT_KEY);
    ifsettings->reset(DOC_FONT_KEY);
    ifsettings->reset(MONOSPACE_FONT_KEY);
    peonysettings->reset(PEONY_FONT_KEY);
    marcosettings->reset(TITLEBAR_FONT_KEY);

    //reset font render
    rendersettings->reset(ANTIALIASING_KEY);
    rendersettings->reset(HINTING_KEY);

//    refresh_status();
}

void Fonts::radiobtn_clicked_slot(int indexnum){
//    float level = 1;
//    if (indexnum == SID){
//        level = SMALL;
//    }
//    else if (indexnum == MID){
//        level = MEDIUM;
//    }
//    else if (indexnum == LID){
//        level = LARGE;
//    }

//    //获取当前字体
//    get_current_fonts();
//    //设置字体大小
//    ifsettings->set(GTK_FONT_KEY, QVariant(QString("%1 %2").arg(gtkfontStrList[0]).arg(defaultfontinfo.gtkfontsize * level)));
//    ifsettings->set(DOC_FONT_KEY, QVariant(QString("%1 %2").arg(docfontStrList[0]).arg(defaultfontinfo.docfontsize * level)));
//    ifsettings->set(MONOSPACE_FONT_KEY, QVariant(QString("%1 %2").arg(monospacefontStrList[0]).arg(defaultfontinfo.monospacefontsize * level)));
//    peonysettings->set(PEONY_FONT_KEY, QVariant(QString("%1 %2").arg(peonyfontStrList[0]).arg(defaultfontinfo.monospacefontsize * level)));
//    marcosettings->set(TITLEBAR_FONT_KEY, QVariant(QString("%1 %2").arg(titlebarfontStrList[0]).arg(defaultfontinfo.titlebarfontsize * level)));

//    refresh_subpage_status();
}

void Fonts::combobox_changed_slot(QString text){
    get_current_fonts();
    for (int i = 0; i < N; i++){
        if (fontinfo[i].type == text){
            ifsettings->set(GTK_FONT_KEY, QVariant(QString("%1 %2").arg(fontinfo[i].gtkfont).arg(gtkfontStrList[1])));
            ifsettings->set(DOC_FONT_KEY, QVariant(QString("%1 %2").arg(fontinfo[i].docfont).arg(docfontStrList[1])));
            ifsettings->set(MONOSPACE_FONT_KEY, QVariant(QString("%1 %2").arg(fontinfo[i].monospacefont).arg(monospacefontStrList[1])));
            peonysettings->set(PEONY_FONT_KEY, QVariant(QString("%1 %2").arg(fontinfo[i].peonyfont).arg(peonyfontStrList[1])));
            marcosettings->set(TITLEBAR_FONT_KEY, QVariant(QString("%1 %2").arg(fontinfo[i].titlebarfont).arg(titlebarfontStrList[1])));
        }
    }
    refresh_subpage_status();
}
