#include "theme.h"
#include "ui_theme.h"

#include <QDebug>

Theme::Theme()
{
    ui = new Ui::Theme;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("theme");
    pluginType = PERSONALIZED;

    //初始化gsettings
    const QByteArray id(INTERFACE_SCHEMA);
    ifsettings = new QGSettings(id);
    const QByteArray idd(MARCO_SCHEMA);
    marcosettings = new QGSettings(idd);

    themeList << "ukui-black" << "ukui-blue";

    component_init();
    status_init();
}

Theme::~Theme()
{
    delete ui;
    delete ifsettings;
    delete marcosettings;
}

QString Theme::get_plugin_name(){
    return pluginName;
}

int Theme::get_plugin_type(){
    return pluginType;
}

QWidget * Theme::get_plugin_ui(){
    return pluginWidget;
}

void Theme::component_init(){
    QSignalMapper * setSignalMapper = new QSignalMapper();
    for (int num = 0; num < themeList.length(); num++){
        QToolButton * button = new QToolButton();
        button->setText(themeList[num]);
        connect(button, SIGNAL(released()), setSignalMapper, SLOT(map()));
        setSignalMapper->setMapping(button, themeList[num]);
        delbtnMap.insert(themeList[num], button);
        ui->themeHLayout->addWidget(button);
    }
    ui->themeHLayout->addStretch();
    connect(setSignalMapper, SIGNAL(mapped(QString)), this, SLOT(set_theme_slots(QString)));

    //
    QSize iconsize(48, 48);
    ui->wallpaperBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->wallpaperBtn->setIcon(QIcon(":/btn.svg"));
    ui->wallpaperBtn->setIconSize(iconsize);

    ui->audioToolBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->audioToolBtn->setIcon(QIcon(":/btn.svg"));
    ui->audioToolBtn->setIconSize(iconsize);

    ui->mouseToolBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->mouseToolBtn->setIcon(QIcon(":/btn.svg"));
    ui->mouseToolBtn->setIconSize(iconsize);

    ui->iconToolBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->iconToolBtn->setIcon(QIcon(":/btn.svg"));
    ui->iconToolBtn->setIconSize(iconsize);
}

void Theme::status_init(){
    //获取当前主题
    QString current_theme;
    current_theme = marcosettings->get(MARCO_THEME_KEY).toString();

    QMap<QString, QToolButton *>::iterator it = delbtnMap.begin();
    for (; it != delbtnMap.end(); it++){
        QString key = QString(it.key());
        QToolButton * tmpBtn = (QToolButton *)it.value();
        if (key == current_theme){
            tmpBtn->setIcon(QIcon(QString("://%1-select.png").arg(key)));
            tmpBtn->setIconSize(QSize(52,52));
        }
        else{
            tmpBtn->setIcon(QIcon(QString("://%1.png").arg(key)));
            tmpBtn->setIconSize(QSize(52,52));
        }
    }

    //设置当前主题预览图
    QSize size(300, 170);
    ui->previewLabel->setPixmap(QPixmap(":/preview.jpg").scaled(size));
}

void Theme::set_theme_slots(QString value){
//    ifsettings->set(INTERFACE_SCHEMA, GTK_THEME_KEY, QVariant(value));
//    marcosettings->set(MARCO_SCHEMA, MARCO_THEME_KEY, QVariant(value));
//    if (value.contains("blue"))
//        ifsettings->set(INTERFACE_SCHEMA, ICON_THEME_KEY, "ukui-icon-theme-one");
//    else
//        ifsettings->set(INTERFACE_SCHEMA, ICON_THEME_KEY, "ukui-icon-theme");
    qDebug() << value;
}
