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
#include "mainwindow.h"
#include <QWidget>
#include "ui_mainwindow.h"
#include "border_shadow_effect.h"
#include "utils/keyvalueconverter.h"
#include <QLinearGradient>
#include "utils/functionselect.h"
#include <QFont>
#include <QSettings>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QPluginLoader>
#include <QPainter>
#include <QPainterPath>
#include <QProcess>
#include <libmatemixer/matemixer.h>
#include <QDebug>
#include <QMessageBox>
#include <QGSettings>
#include <KWindowEffects>
#include <KWindowSystem>
#include "customstyle.h"

#define MODE_QT_KEY  "style-name"
#define PERSONALISE  "org.ukui.control-center.personalise"
#define QueryLineEditBackground "#FFFFFF" //搜索框背景
#define QueryLineEditClickedBackground "#FFFFFF" //搜索框背景选中
#define QueryLineEditClickedBorder "rgba(61, 107, 229, 1)" //搜索框背景选中边框
/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

extern "C" {
#include <glib.h>
#include <gio/gio.h>
}

#define THEME_QT_SCHEMA "org.ukui.style"


const int dbWitdth = 50;
extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_searchWidget(nullptr)

{
    m_effect = new BorderShadowEffect(this);
    m_effect->setPadding(10);
    m_effect->setBorderRadius(16);
    m_effect->setBlurRadius(16);

    ui->setupUi(this);
    // 初始化mixer
    mate_mixer_init();
    this->installEventFilter(this);
    setAttribute(Qt::WA_TranslucentBackground, true);

    if (QGSettings::isSchemaInstalled(PERSONALISE) && QGSettings::isSchemaInstalled("org.ukui.style")){
        gsettings = new QGSettings(PERSONALISE);
        themeSetting = new QGSettings("org.ukui.style");
    }
    setAppStyle();
    initTileBar();
    initConnection();
    initStyleSheet();
    initShortcutKey();


    //初始化功能列表数据
    FunctionSelect::initValue();
    //构建枚举键值转换对象
    kvConverter = new KeyValueConverter(); //继承QObject，No Delete
    //加载插件
    loadPlugins();
    ui->leftsidebarWidget->setVisible(false);
    bIsFullScreen = false;
    //加载功能页Widget
    modulepageWidget = new ModulePageWidget(this);
    ui->stackedWidget->addWidget(modulepageWidget);
    //快捷参数
    if (QApplication::arguments().length() > 1){
        bootOptionsFilter(QApplication::arguments().at(1));
    }
    this->setFocus();

    connect(modulepageWidget,SIGNAL(pageChangeSignal()),this,SLOT(pageChangeSlot()));
}
void MainWindow::setAppStyle()
{
    QString currentThemeMode = themeSetting->get("style-name").toString();
    qApp->setStyle(new InternalStyle(currentThemeMode));
    connect(themeSetting, &QGSettings::changed, this, [=] (const QString &key) {
        if (key == "styleName") {
            QString themeMode = themeSetting->get("style-name").toString();
            qApp->setStyle(new InternalStyle("ukui"));
        }
    });
}
MainWindow::~MainWindow()
{
    delete ui;
    if (gsettings){
        delete gsettings;
    }
    if (themeSetting)
        delete themeSetting;
}
void MainWindow::initShortcutKey() {
    QAction *action = new QAction(this);
    action->setShortcut(tr("F1"));
    this->addAction(action);
    connect(action, &QAction::triggered,this,[=]{
        QProcess process(this);
        process.startDetached("kylin-user-guide");
    });
}
void MainWindow::initConnection() {
    connect(themeSetting, &QGSettings::changed,[=](QString key){
        if ("systemFont" == key || "systemFontSize" ==key) {
            QFont font = this->font();
            int width = font.pointSize();
            for (auto widget : qApp->allWidgets()) {
                widget->setFont(font);
            }
            ui->leftsidebarWidget->setMaximumWidth(width * 10 +20);
        }
    });
    //监听平板模式切换
    m_statusSessionDbus = new QDBusInterface("com.kylin.statusmanager.interface",
                                              "/",
                                              "com.kylin.statusmanager.interface",
                                              QDBusConnection::sessionBus(),this);

    is_tabletmode = m_statusSessionDbus->call("get_current_tabletmode");

    if(is_tabletmode){
        minBtn->hide();
        maxBtn->hide();
        closeBtn->hide();
        blankLabel1->show();
        blankLabel2->show();
        blankLabel3->show();
    } else {
        minBtn->show();
        maxBtn->show();
        closeBtn->show();
        blankLabel1->hide();
        blankLabel2->hide();
        blankLabel3->hide();
    }
    connect(m_statusSessionDbus, SIGNAL(mode_change_signal(bool)), this, SLOT(mainWindow_statusDbusSlot(bool)));
    connect(minBtn, SIGNAL(clicked()), this, SLOT(showMinimized()));
//    connect(ui->minBtn, &QPushButton::clicked, [=]{
//        KWindowSystem::minimizeWindow(this->winId());
//    });
    connect(maxBtn, &QPushButton::clicked, this, [=]{
        if (isMaximized()){
            bIsFullScreen = false;
            showNormal();
            maxBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
        } else {
            bIsFullScreen = true;
            showMaximized();
            maxBtn->setIcon(QIcon::fromTheme("window-restore-symbolic"));
        }
    });
    connect(closeBtn, &QPushButton::clicked, this, [=]{
        close();
    });
}

void MainWindow::mainWindow_statusDbusSlot(bool tablet_mode)
{
    if(tablet_mode){
        minBtn->hide();
        maxBtn->hide();
        closeBtn->hide();
        blankLabel1->show();
        blankLabel2->show();
        blankLabel3->show();
    } else {
        minBtn->show();
        maxBtn->show();
        closeBtn->show();
        blankLabel1->hide();
        blankLabel2->hide();
        blankLabel3->hide();
    }
}

void MainWindow::initStyleSheet() {
    //设置顶部搜索栏
    m_queryWid->setGeometry(QRect((m_searchWidget->width()-(m_queryIcon->width()+m_queryText->width()+10))/2,0,
                                  m_queryIcon->width()+m_queryText->width()+10,35));
    m_queryWid->show();

    ui->titleLayout->setContentsMargins(400,0,0,0);
    ui->titleLayout->setAlignment(m_searchWidget,Qt::AlignCenter);
    // 设置panel图标
    QIcon panelicon;
    if (QIcon::hasThemeIcon("ukui-control-center"))
        panelicon = QIcon::fromTheme("ukui-control-center");

    this->setWindowIcon(panelicon);
    this->setWindowTitle(tr("System Settings"));
    minBtn->setProperty("useIconHighlightEffect", true);
    minBtn->setProperty("iconHighlightEffectMode", 1);
    minBtn->setStyleSheet("QPushButton:hover{background:rgba(47,179,232,10%); border-radius: 4px;}"
                          "QPushButton:pressed{background-color:rgba(47,179,232,20%); border-radius: 4px;}");
    minBtn->setFlat(true);
    maxBtn->setProperty("useIconHighlightEffect", true);
    maxBtn->setProperty("iconHighlightEffectMode", 1);
    maxBtn->setStyleSheet("QPushButton:hover{background:rgba(47,179,232,10%); border-radius: 4px;}"
                          "QPushButton:pressed{background-color:rgba(47,179,232,20%); border-radius: 4px;}");
    maxBtn->setFlat(true);
    closeBtn->setProperty("useIconHighlightEffect", true);
    closeBtn->setProperty("iconHighlightEffectMode", 1);
    closeBtn->setFlat(true);
    closeBtn->setStyleSheet("QPushButton:hover{background:rgba(251,80,80,10%); border-radius: 4px;}"
                            "QPushButton:pressed{background-color:rgba(251,80,80,20%); border-radius: 4px;}");

    // 设置右上角按钮图标
    minBtn->setIcon(renderSvg(QIcon::fromTheme("window-minimize-symbolic"), "blue"));
    maxBtn->setIcon(renderSvg(QIcon::fromTheme("window-maximize-symbolic"), "blue"));
    closeBtn->setIcon(renderSvg(QIcon::fromTheme("window-close-symbolic"),"red"));
}

void MainWindow::sltMessageReceived(const QString &msg) {

    //showNormal();
    KWindowSystem::forceActiveWindow(this->winId());
    this->show(); //显示最近一次的大小
    bootOptionsFilter(msg);

//    Qt::WindowFlags flags = windowFlags();
//    flags |= Qt::WindowStaysOnTopHint;
//    setWindowFlags(flags);
//    flags &= ~Qt::WindowStaysOnTopHint;
//    setWindowFlags(flags);
}

void MainWindow::bootOptionsFilter(QString opt){
    if (opt == "-m") {
        //显示器
        bootOptionsSwitch(DEVICES, DISPLAYS);

    } else if (opt == "-b") {
        //背景
        bootOptionsSwitch(PERSONALIZED, WALLPAPER);

    } else if (opt == "-u") {
        //账户
        bootOptionsSwitch(ACCOUNT, USERINFO);

    } else if (opt == "-a") {
        //关于
        bootOptionsSwitch(OTHER, ABOUT);

    } else if (opt == "-p") {
        //电源
        bootOptionsSwitch(SYSTEM, BATTERY);
    } else if (opt == "-t") {
        // Datetime moudle
        bootOptionsSwitch(SYSTEM, DATTIME);
    } else if (opt == "-s") {
        // Audio module
        bootOptionsSwitch(SYSTEM, SOUNDS);
    } else if (opt == "--userinfo") {
        bootOptionsSwitch(ACCOUNT, USERINFO);
    } else if (opt == "--network") {
        bootOptionsSwitch(NETWORK, NETCONNECT);
    } else if (opt == "--bluetooth") {
        bootOptionsSwitch(NETWORK, BLUETOOTH);
    }  else if (opt == "--proxy") {
        bootOptionsSwitch(NETWORK, PROXY);
    } else if (opt == "--theme") {
        bootOptionsSwitch(PERSONALIZED, THEME);
    } else if (opt == "--wallpaper") {
        bootOptionsSwitch(PERSONALIZED, WALLPAPER);
    } else if (opt == "--screenlock") {
        bootOptionsSwitch(PERSONALIZED, LOCKSCREEN);
    } else if (opt == "--font") {
        bootOptionsSwitch(PERSONALIZED, FONTS);
    } else if (opt == "--startup") {
        bootOptionsSwitch(SYSTEM, STARTUP);
    } else if (opt == "--battery") {
        bootOptionsSwitch(SYSTEM, BATTERY);
    } else if (opt == "--opmode") {
        bootOptionsSwitch(SYSTEM, OPERATIONMODE);
    } else if (opt == "--audio") {
        bootOptionsSwitch(SYSTEM, SOUNDS);
    } else if (opt == "--datetime") {
        bootOptionsSwitch(SYSTEM, DATTIME);
    } else if (opt == "--language") {
        bootOptionsSwitch(SYSTEM, LANGUAGEREAGION);
    } else if (opt == "--recovery") {
        bootOptionsSwitch(SYSTEM, SYSTEMRECOVERY);
    } else if (opt == "--displays") {
        bootOptionsSwitch(DEVICES, DISPLAYS);
    } else if (opt == "--printer") {
        bootOptionsSwitch(DEVICES, PRINTERS);
    } else if (opt == "--mouse") {
        bootOptionsSwitch(DEVICES, MOUSE);
    } else if (opt == "--trackpad") {
        bootOptionsSwitch(DEVICES, TRACKPAD);
    } else if (opt == "--pen") {
        bootOptionsSwitch(DEVICES, PEN);
    } else if (opt == "--gesture") {
        bootOptionsSwitch(OTHER, GESTURE);
    } else if (opt == "--about") {
        bootOptionsSwitch(OTHER, ABOUT);
    }
}

void MainWindow::bootOptionsSwitch(int moduleNum, int funcNum){

    QList<FuncInfo> pFuncStructList = FunctionSelect::funcinfoList[moduleNum];
    QString funcStr = pFuncStructList.at(funcNum).namei18nString;
    qDebug() << "moduleNum is" << moduleNum << " " << funcNum << " " << funcStr << endl;

    QMap<QString, QObject *> pluginsObjMap = modulesList.at(moduleNum);

    if (pluginsObjMap.keys().contains(funcStr)){
        //开始跳转
        ui->stackedWidget->setCurrentIndex(1);
        modulepageWidget->switchPage(pluginsObjMap.value(funcStr));
    }
}

void MainWindow::paintEvent(QPaintEvent *event) {
    validBorder();
    QColor color = this->palette().window().color();
    QColor colorBase = this->palette().base().color();

    int R1 = color.red();
    int G1 = color.green();
    int B1 = color.blue();
    qreal a1 = 0.3;

    int R2 = colorBase.red();
    int G2 = colorBase.green();
    int B2 = colorBase.blue();

    qreal a2 = 1;
    qreal a = 1 - (1 - a1)*(1 - a2);
    qreal R = (a1*R1 + (1 - a1)*a2*R2) / a;
    qreal G = (a1*G1 + (1 - a1)*a2*G2) / a;
    qreal B = (a1*B1 + (1 - a1)*a2*B2) / a;

    colorBase.setRed(R);
    colorBase.setGreen(G);
    colorBase.setBlue(B);

    colorBase.setAlphaF(0);

    QPainterPath sidebarPath;
    sidebarPath.setFillRule(Qt::FillRule::WindingFill);
    QPainterPath deletePath;
    QPainterPath tmpPath;
    //color.setAlphaF(0.5);
    m_effect->setWindowBackground(color);
    QPainter p(this);
    if(!this->isMaximized()){
        tmpPath.addRoundedRect(rect().adjusted(0, 0, 0, 0),16,16);
        deletePath.addRoundedRect(rect().adjusted(305, 0, 0, 0), 16, 16);
        sidebarPath = tmpPath - deletePath;
        m_effect->m_transparent_path.addRect(0,0,0,0);
        m_effect->setTransParentPath(sidebarPath);
        m_effect->setTransParentAreaBg(colorBase);
        m_effect->drawWindowShadowManually(&p, this->rect(),true,false);
    } else {
        tmpPath.addRoundedRect(rect().adjusted( 0, 1, -10,0),0,0);
        deletePath.addRoundedRect(rect().adjusted(305, 1, 0, 0), 16, 16);
        sidebarPath = tmpPath - deletePath;
        m_effect->m_transparent_path.addRect(0,0,0,0);
        m_effect->setTransParentPath(sidebarPath);
        m_effect->setTransParentAreaBg(colorBase);
        m_effect->drawWindowShadowManually(&p, this->rect(),true,true);
    }
    QMainWindow::paintEvent(event);
    p.save();
    p.restore();
    connect(gsettings,&QGSettings::changed,[=]{
        this->update();
    });
}
void MainWindow::validBorder(){
    if (this->isMaximized()) {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QPainterPath rectPath;
        rectPath.addRoundedRect(this->rect(), 0, 0);
        p.save();
        p.restore();
        setContentsMargins(0, 0, 0, 0);
        m_effect->setPadding(0);
        KWindowEffects::enableBlurBehind(this->winId(), true);

    } else {
        //适配窗管，无需再画阴影
        m_effect->setPadding(0);

        QPainterPath path;
        auto rect = this->rect();
//        rect.adjust(4, 4, -4, -4);
        path.addRoundedRect(rect, 16, 16);
        setProperty("blurRegion", QRegion(path.toFillPolygon().toPolygon()));
        //use KWindowEffects
        KWindowEffects::enableBlurBehind(this->winId(), true, QRegion(path.toFillPolygon().toPolygon()));
    }

}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    if (this == watched) {
        if (event->type() == QEvent::WindowStateChange) {
            if (this->windowState() == Qt::WindowMaximized) {
                QFont font = this->font();
                maxBtn->setIcon(QIcon::fromTheme("window-restore-symbolic"));
                maxBtn->setToolTip(tr("Restore"));
            } else {
                maxBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
                maxBtn->setToolTip(tr("Maximize"));
            }
        } else if (event->type() == QEvent::MouseButtonDblClick) {
            is_tabletmode = m_statusSessionDbus->call("get_current_tabletmode");
            if (!is_tabletmode) {
                bool res = dblOnEdge(dynamic_cast<QMouseEvent*>(event));
                if (res) {
                    if (this->windowState() == Qt::WindowMaximized) {
                        bIsFullScreen = false;
                        this->showNormal();
                    } else {
                        bIsFullScreen = true;
                        this->showMaximized();
                    }
                }
            }
        }
    }
//    if (closeBtn == watched) {
//        if (event->type() == QEvent::Enter) {
//            closeBtn->setIcon(renderSvg(QIcon::fromTheme("window-close-symbolic"),"white"));
//        } else if (event->type() == QEvent::Leave) {
//            closeBtn->setIcon(renderSvg(QIcon::fromTheme("window-close-symbolic"),"default"));
//        }
//    }
    if (watched==m_searchWidget) {
        if (event->type()==QEvent::FocusIn) {
             if (m_searchWidget->text().isEmpty()) {
                 m_animation->stop();
                 m_animation->setStartValue(QRect((320-(m_queryIcon->width()+m_queryText->width()+10))/2,0,
                                                m_queryIcon->width()+m_queryText->width()+10,35));
                 m_animation->setEndValue(QRect(0,0,
                                               m_queryIcon->width()+5,35));
                 m_animation->setEasingCurve(QEasingCurve::OutQuad);
                 m_animation->start();
                 m_searchWidget->setTextMargins(30,1,0,1);
             }
             m_isSearching=true;
        } else if (event->type()==QEvent::FocusOut) {
            m_searchKeyWords.clear();
            if (m_searchWidget->text().isEmpty()) {
                if (m_isSearching) {
                    m_animation->stop();
                    m_queryText->adjustSize();
                    m_animation->setStartValue(QRect(0,0,
                                                    m_queryIcon->width()+5,35));
                    m_animation->setEndValue(QRect((320-(m_queryIcon->width()+m_queryText->width()+10))/2,0,
                                                 m_queryIcon->width()+m_queryText->width()+10,35));
                    m_animation->setEasingCurve(QEasingCurve::InQuad);
                    m_animation->start();
                }
                m_isSearching=false;
            }
        }
    }
    return QObject::eventFilter(watched, event);
}

void MainWindow::initTileBar() {
    m_searchWidget = new SearchWidget(this);
    QString m_themeMode = themeSetting->get(MODE_QT_KEY).toString();
    if ("ukui-white" == m_themeMode || "ukui-default" == m_themeMode || "ukui-light" == m_themeMode || "ukui-white-unity" == m_themeMode) {
        sprintf(style, "SearchWidget{border:0px;background-color:#FFFFFF;border-radius:8px;}",
                QueryLineEditBackground);
        m_searchWidget->setStyleSheet(style);
    } else if ("ukui-dark" == m_themeMode || "ukui-black" == m_themeMode || "ukui-black-unity" == m_themeMode){
        sprintf(style, "SearchWidget{border:0px;background-color:#2C2F33;border-radius:8px;}",
                QueryLineEditBackground);
        m_searchWidget->setStyleSheet(style);
    }
    connect(themeSetting, &QGSettings::changed, this, [=](const QString &key){
        if (key == "styleName") {
            QString themeMode = themeSetting->get(MODE_QT_KEY).toString();
            if ("ukui-white" == themeMode || "ukui-default" == themeMode || "ukui-light" == themeMode || "ukui-white-unity" == themeMode) {
                sprintf(style, "SearchWidget{border:0px;background-color:#FFFFFF;color:#000000;border-radius:8px;}",
                        QueryLineEditBackground);
                m_searchWidget->setStyleSheet(style);
            } else if ("ukui-dark" == themeMode || "ukui-black" == themeMode || "ukui-black-unity" == themeMode){
                sprintf(style, "SearchWidget{border:0px;background-color:#2C2F33;color:#FFFFFF;border-radius:8px;}",
                        QueryLineEditBackground);
                m_searchWidget->setStyleSheet(style);
            }
        }
    });

    m_searchWidget->setFocusPolicy(Qt::ClickFocus);
    m_searchWidget->installEventFilter(this);

    m_queryWid=new QWidget;
    m_queryWid->setParent(m_searchWidget);
    m_queryWid->setFocusPolicy(Qt::NoFocus);
    m_queryWid->setStyleSheet("border:0px;background:transparent");

    QHBoxLayout* queryWidLayout=new QHBoxLayout;
    queryWidLayout->setContentsMargins(4,8,0,0);
    queryWidLayout->setAlignment(Qt::AlignJustify);
    queryWidLayout->setSpacing(5);
    m_queryWid->setLayout(queryWidLayout);


    QPixmap pixmap=loadSvg(QString("://img/dropArrow/search.svg"),"default");
    m_queryIcon=new QLabel;
    m_queryIcon->setStyleSheet("background:transparent");
    m_queryIcon->setFixedSize(pixmap.size());
    m_queryIcon->setPixmap(pixmap);

    m_queryText=new QLabel;
    m_queryText->setText(tr("Search"));
    m_queryText->setStyleSheet("background:transparent;color:#626c6e;");
    m_queryText->adjustSize();

    titleLabel  = new QLabel(tr("System Settings"), this);
    titleLabel->setStyleSheet("QLabel{font-weight: 400;line-height: 24px;font-size: 16px;}");

    icon =new QLabel(this);
    icon->setPixmap(QIcon::fromTheme("ukui-control-center").pixmap(QSize(32, 32)));
    icon->setStyleSheet("border-radius:8px;background:transparent;");
    icon->resize(32,32);

    const QByteArray id(THEME_QT_SCHEMA);
    QGSettings *mQtGsetting = new QGSettings(id, QByteArray(), this);
    connect(mQtGsetting, &QGSettings::changed, this, [=](QString key) {
        if (key == "iconThemeName") {
            icon->setPixmap(QIcon::fromTheme("ukui-control-center").pixmap(QSize(32, 32)));
        }
    });



    queryWidLayout->addStretch();
    queryWidLayout->addWidget(m_queryIcon);
    queryWidLayout->addWidget(m_queryText);
    queryWidLayout->addStretch();


    m_searchWidget->setContextMenuPolicy(Qt::NoContextMenu);
    m_animation= new QPropertyAnimation(m_queryWid,"geometry");
    m_animation->setDuration(100);
//    m_searchWidget->setGeometry();
    ui->titleLayout->addWidget(m_searchWidget,Qt::AlignCenter);
    connect(m_animation,&QPropertyAnimation::finished,this,&MainWindow::animationFinishedSlot);
    connect(m_searchWidget, &SearchWidget::notifyModuleSearch, this, &MainWindow::switchPage);

    minBtn      = new QPushButton(this);
    maxBtn      = new QPushButton(this);
    closeBtn     = new QPushButton(this);
    blankLabel1 = new QLabel(this);
    blankLabel2 = new QLabel(this);
    blankLabel3 = new QLabel(this);


    blankLabel1->setFixedSize(48,48);
    blankLabel2->setFixedSize(48,48);
    blankLabel3->setFixedSize(48,48);

    minBtn->setFixedSize(48, 48);
    maxBtn->setFixedSize(48, 48);
    closeBtn->setFixedSize(48, 48);

    minBtn->setToolTip(tr("Minimize"));
    maxBtn->setToolTip(tr("Maximize"));
    closeBtn->setToolTip(tr("Close"));

    m_searchWidget->setMinimumWidth(320);
    m_searchWidget->setMinimumHeight(40);
    m_searchWidget->setMaximumWidth(320);
    m_searchWidget->setMaximumHeight(40);

    icon->setGeometry(rect().x()+34, rect().y()+21,32, 32);
    titleLabel->setGeometry(rect().x()+74, rect().y()+25.01,120, 24);
    icon->setParent(this);
    titleLabel->setParent(this);
    ui->titleLayout->addStretch();
    //平板模式空白占位，避免搜索框右移
    ui->titleLayout->addWidget(blankLabel1);
    ui->titleLayout->addWidget(blankLabel2);
    ui->titleLayout->addWidget(blankLabel3);

    ui->titleLayout->addWidget(minBtn);
    ui->titleLayout->addWidget(maxBtn);
    ui->titleLayout->addWidget(closeBtn);
    ui->titleLayout->setAlignment(minBtn, Qt::AlignTop);
    ui->titleLayout->setAlignment(maxBtn, Qt::AlignTop);
    ui->titleLayout->setAlignment(closeBtn, Qt::AlignTop);
}
void MainWindow::animationFinishedSlot()
{
    if (m_isSearching) {
        m_queryWid->layout()->removeWidget(m_queryText);
        m_queryText->setParent(nullptr);
        m_searchWidget->setTextMargins(30,1,0,1);
        if (!m_searchKeyWords.isEmpty()) {
            m_searchWidget->setText(m_searchKeyWords);
            m_searchKeyWords.clear();
        }
    } else {
        m_queryWid->layout()->addWidget(m_queryText);
    }
}
void MainWindow::setBtnLayout(QPushButton * &pBtn){
    QLabel * imgLabel = new QLabel(pBtn);
    QSizePolicy imgLabelPolicy = imgLabel->sizePolicy();
    imgLabelPolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    imgLabelPolicy.setVerticalPolicy(QSizePolicy::Fixed);
    imgLabel->setSizePolicy(imgLabelPolicy);
    imgLabel->setScaledContents(true);

    QVBoxLayout * baseVerLayout = new QVBoxLayout(pBtn);

    QHBoxLayout * contentHorLayout = new QHBoxLayout();
    contentHorLayout->addStretch();
    contentHorLayout->addWidget(imgLabel);
    contentHorLayout->addStretch();

    baseVerLayout->addStretch();
    baseVerLayout->addLayout(contentHorLayout);
    baseVerLayout->addStretch();

    pBtn->setLayout(baseVerLayout);
}

void MainWindow::loadPlugins(){
    for (int index = 0; index < TOTALMODULES; index++){
        QMap<QString, QObject *> pluginsMaps;
        modulesList.append(pluginsMaps);
    }

    static bool installed = (QCoreApplication::applicationDirPath() == QDir(("/usr/bin")).canonicalPath());

    if (installed)
        pluginsDir = QDir(PLUGIN_INSTALL_DIRS);
    else {
        pluginsDir = QDir(qApp->applicationDirPath() + "/plugins");
    }

    foreach (QString fileName, pluginsDir.entryList(QDir::Files)){
        if (!fileName.endsWith(".so"))
            continue;
        if (fileName == "liboperatingmode.so" || fileName == "libautoboot.so" || fileName == "libfonts.so" || fileName == "libexperienceplan.so" || fileName == "libarea.so")
            continue;
        qDebug() << "Scan Plugin: " << fileName;

        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QObject * plugin = loader.instance();
        if (plugin) {
            CommonInterface * pluginInstance = qobject_cast<CommonInterface *>(plugin);
            modulesList[pluginInstance->get_plugin_type()].insert(pluginInstance->get_plugin_name(), plugin);

            qDebug() << "Load Plugin :" << kvConverter->keycodeTokeyi18nstring(pluginInstance->get_plugin_type()) << "->" << pluginInstance->get_plugin_name() ;

            m_searchWidget->addModulesName(pluginInstance->name(), pluginInstance->get_plugin_name(), pluginInstance->translationPath());

            int moduletypeInt = pluginInstance->get_plugin_type();
            if (!moduleIndexList.contains(moduletypeInt))
                moduleIndexList.append(moduletypeInt);
        } else {
            //如果加载错误且文件后缀为so，输出错误
            if (fileName.endsWith(".so"))
                qDebug() << fileName << "Load Failed: " << loader.errorString() << "\n";
        }
    }
    m_searchWidget->setLanguage(QLocale::system().name());
}

const QPixmap MainWindow::loadSvg(const QString &fileName, QString color)
{
    int size = 16;
    const auto ratio = qApp->devicePixelRatio();
    if ( 2 == ratio) {
        size = 48;
    } else if (3 == ratio) {
        size = 96;
    }
    QPixmap pixmap(size, size);
    QSvgRenderer renderer(fileName);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);
    renderer.render(&painter);
    painter.end();

    pixmap.setDevicePixelRatio(ratio);
    return drawSymbolicColoredPixmap(pixmap, color);
}
const QPixmap loadSvg_1(const QString &fileName, const int size)
{
    QPixmap pixmap(size, size);
    QSvgRenderer renderer(fileName);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);
    renderer.render(&painter);
    painter.end();

    return pixmap;
}
QPixmap MainWindow::drawSymbolicColoredPixmap(const QPixmap &source, QString cgColor)
{
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                if ("white" == cgColor) {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                } else if ("black" == cgColor) {
                    color.setRed(0);
                    color.setGreen(0);
                    color.setBlue(0);
//                    color.setAlpha(0.1);
                    color.setAlphaF(0.9);
                    img.setPixelColor(x, y, color);
                } else if ("gray" == cgColor) {
                    color.setRed(152);
                    color.setGreen(163);
                    color.setBlue(164);
                    img.setPixelColor(x, y, color);
                } else if ("blue" == cgColor){
                    color.setRed(61);
                    color.setGreen(107);
                    color.setBlue(229);
                    img.setPixelColor(x, y, color);
                } else {
                    return source;
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}

bool MainWindow::dblOnEdge(QMouseEvent *event)
{
    QPoint pos = event->globalPos();
    int globalMouseY = pos.y();

    int frameY = this->y();

    bool onTopEdges = (globalMouseY >= frameY &&
                  globalMouseY <= frameY + dbWitdth);
    return onTopEdges;
}

QMap<QString, QObject *> MainWindow::exportModule(int type){
    QMap<QString, QObject *> emptyMaps;
    if (type < modulesList.length())
        return modulesList[type];
    else
        return emptyMaps;
}

void MainWindow::switchPage(QString moduleName) {

    for (int i = 0; i < modulesList.length(); i++) {
        auto modules = modulesList.at(i);
        //开始跳转
        if (modules.keys().contains(moduleName)) {
            ui->stackedWidget->setCurrentIndex(1);
            modulepageWidget->switchPage(modules.value(moduleName));
        }
    }
}

const QPixmap MainWindow::renderSvg(const QIcon &icon, QString cgColor) {
    int size = 24;
    const auto ratio = qApp->devicePixelRatio();
    if ( 2 == ratio) {
        size = 48;
    } else if (3 == ratio) {
        size = 96;
    }
    QPixmap iconPixmap = icon.pixmap(size,size);
    iconPixmap.setDevicePixelRatio(ratio);
    QImage img = iconPixmap.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                if ("white" == cgColor) {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                } else if ("black" == cgColor) {
                    color.setRed(0);
                    color.setGreen(0);
                    color.setBlue(0);
//                    color.setAlpha(0.1);
                    color.setAlphaF(0.9);
                    img.setPixelColor(x, y, color);
                } else if ("gray" == cgColor) {
                    color.setRed(152);
                    color.setGreen(163);
                    color.setBlue(164);
                    img.setPixelColor(x, y, color);
                } else if ("blue" == cgColor){
                    color.setRed(61);
                    color.setGreen(107);
                    color.setBlue(229);
                    img.setPixelColor(x, y, color);
                } else if ("red" == cgColor) {
                    color.setRed(251);
                    color.setGreen(80);
                    color.setBlue(80);
                    img.setPixelColor(x, y, color);
                } else {
                    return iconPixmap;
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}


void MainWindow::pageChangeSlot()
{
    if (this->m_searchWidget->text().count()) {
        this->m_searchWidget->setText(QString::fromUtf8(""));
        this->m_searchWidget->clearFocus();
    }
}
