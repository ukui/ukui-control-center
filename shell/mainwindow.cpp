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
#include "ui_mainwindow.h"
#include "utils/keyvalueconverter.h"
#include "utils/functionselect.h"
#include "utils/utils.h"
#include <ukcc/widgets/imageutil.h>
#include "ukccabout.h"
#include "devicesmonitor.h"

#include <libmatemixer/matemixer.h>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QPluginLoader>
#include <QPainter>
#include <QPainterPath>
#include <QProcess>
#include <QDebug>
#include <QMessageBox>
#include <QGSettings>
#include <QMenu>
#include <QShortcut>
#include <QMouseEvent>
#include <QScrollBar>
#include <KWindowSystem>
#include "component/leftwidgetitem.h"
#include "iconbutton.h"
#include <ukcc/widgets/lightlabel.h>

#define STYLE_FONT_SCHEMA  "org.ukui.style"

#define THEME_QT_SCHEMA  "org.ukui.style"


#ifdef WITHKYSEC
#include <kysec/libkysec.h>
#include <kysec/status.h>
#endif

const QByteArray kVinoSchemas    = "org.gnome.Vino";

#define KYLIN_USER_GUIDE_PATH              "/"
#define KYLIN_USER_GUIDE_SERVICE           "com.kylinUserGuide.hotel"
#define KYLIN_USER_GUIDE_INTERFACE         "com.guide.hotel"

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

const int dbWitdth = 50;
extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_searchWidget(nullptr)
{
    mate_mixer_init();
    qApp->installEventFilter(this);
    is_ExitPower = isExitsPower();
    initUI();
    hideComponent();
}

MainWindow::~MainWindow()
{
    delete ui;
    ui = nullptr;
}

void MainWindow::bootOptionsFilter(QString opt) {
    int moduleNum;
    bool isExitsModule = false;
    QString funcStr;
    QList<FuncInfo> pFuncStructList;
    for (int i = 0; i < FunctionSelect::funcinfoList.size(); i++) {
        for (int j = 0; j < FunctionSelect::funcinfoList[i].size(); j++) {
            if (!FunctionSelect::funcinfoList[i][j].nameString.compare(opt, Qt::CaseInsensitive)) {
                moduleNum = FunctionSelect::funcinfoList[i][j].type;
                funcStr = FunctionSelect::funcinfoList[i][j].namei18nString;
                pFuncStructList = FunctionSelect::funcinfoList[i];
                isExitsModule = true;
                break;
            }
        }
    }
    if (!isExitsModule) {
        return ;
    }

    QMap<QString, QObject *> pluginsObjMap = modulesList.at(moduleNum);

    if (pluginsObjMap.keys().contains(funcStr)){
        //开始跳转
        functionBtnClicked(pluginsObjMap.value(funcStr));
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    /* clear m_searchWidget's focus
     * MouseButtonPress event happened but not in m_searchWidget
     */
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mEvent = static_cast<QMouseEvent*>(event);
        QWidget *searchParentWid = static_cast<QWidget*>(m_searchWidget->parent());
        QPoint  searchPoint      = searchParentWid->mapFromGlobal(mEvent->globalPos());
        //qDebug()<<m_searchWidget->geometry()<<  mWindowGlobalPoint << mouseGlobalPoint << tPoint;
        if (!m_searchWidget->geometry().contains(searchPoint)) {
            if (m_isSearching == true) {
                m_searchWidget->setFocus();
                m_searchWidget->clearFocus();
            }
        }
    }

    if (event->type() == QEvent::MouseMove) {
        if (scrollArea->geometry().contains(this->mapFromGlobal(QCursor::pos()))) {
            scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        } else {
            scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
    }

    if (this == watched) {
        if (event->type() == QEvent::WindowStateChange) {
            if (this->windowState() == Qt::WindowMaximized) {
                maxBtn->setIcon(QIcon::fromTheme("window-restore-symbolic"));
                maxBtn->setToolTip(tr("Normal"));
            } else {
                maxBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
                maxBtn->setToolTip(tr("Maximize"));
            }
        } else if (event->type() == QEvent::MouseButtonDblClick) {
            bool res = dblOnEdge(dynamic_cast<QMouseEvent*>(event));
            if (res) {
                if (this->windowState() == Qt::WindowMaximized) {
                    this->showNormal();
                } else {
                    this->showMaximized();
                }
            }
        }
    }
    if (watched == m_searchWidget) {
        if (event->type() == QEvent::FocusIn) {
            if (m_searchWidget->text().isEmpty()) {
                m_animation->stop();
                m_queryWid->layout()->removeWidget(m_queryText);
                m_animation->setStartValue(m_queryWid->pos());
                if (m_queryWid->pos().x() > 20) {
                    queryWidCenterPos = QPoint(m_queryWid->pos());
                }
                m_animation->setEndValue(QPoint(8,0));
                m_animation->setEasingCurve(QEasingCurve::OutQuad);
                m_animation->start();
            }
            m_isSearching = true;
        } else if (event->type() == QEvent::FocusOut) {
            m_searchKeyWords.clear();
            if (m_searchWidget->text().isEmpty()) {
                if (m_isSearching) {
                    m_animation->setStartValue(m_queryWid->pos());
                    m_animation->setEndValue(queryWidCenterPos);
                    m_animation->setEasingCurve(QEasingCurve::InQuad);
                    m_animation->start();
                    m_queryWid->layout()->addWidget(m_queryText);
                }
            }
            m_isSearching = false;
        }
    }

    if (watched == homepageWidget) {
        if (event->type() == QEvent::Paint) {
            QTimer::singleShot(1, this, [=]() {
                m_searchWidget->setFixedWidth(350 > mOptionBtn->x() - titleLabel->x() - titleLabel->width() ? (mOptionBtn->x() - m_searchWidget->x() - 16) : 350);
            });
        }
    }
    return QObject::eventFilter(watched, event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F1) {
        if (!event->isAutoRepeat()) //避免重复触发该事件
            onF1ButtonClicked();
    }
}

void MainWindow::initUI() {
    ui->setupUi(this);
    QRect screenSize = Utils::sizeOnCursor();
    this->setMinimumSize(978, 630);
    if (screenSize.width() > 1440)
        this->resize(1160,720);
    m_ModuleMap = Utils::getModuleHideStatus();

    const QByteArray id("org.ukui.style");
    m_fontSetting = new QGSettings(id, QByteArray(), this);
    connect(m_fontSetting, &QGSettings::changed, this, [=](QString key) {
        if ("systemFont" == key || "systemFontSize" ==key) {
            changeSearchSlot();
            QFont font = this->font();
            int width = font.pointSize();
            for (auto widget : qApp->allWidgets()) {
                QString className(widget->metaObject()->className());
                if (widget->objectName() == "timeClockLable") {
                    QFont fontTime;
                    fontTime.setBold(true);
                    fontTime.setWeight(QFont::Medium);
                    fontTime.setPixelSize(font.pointSize() * 28 / 11);
                    widget->setFont(fontTime);
                } else if(widget->objectName() == "DateTime_Info") {
                    QFont fontTitle;
                    fontTitle.setPixelSize(font.pointSize() * 18 / 11);
                    fontTitle.setWeight(QFont::Medium);
                    widget->setFont(fontTitle);
                } else {
                    font.setWeight(QFont::Normal);
                    widget->setFont(font);
                }
            }
        }
    });

    initTileBar();
    initStyleSheet();

    //初始化功能列表数据
    FunctionSelect::initValue();

    //构建枚举键值转换对象
    kvConverter = new KeyValueConverter(); //继承QObject，No Delete

    //加载插件
     loadPlugins();

    connect(minBtn, SIGNAL(clicked()), this, SLOT(showMinimized()));
    connect(maxBtn, &QPushButton::clicked, this, [=] {
        if (isMaximized()) {
            showNormal();
            maxBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
        } else {
            showMaximized();
            maxBtn->setIcon(QIcon::fromTheme("window-restore-symbolic"));
        }
    });
    connect(closeBtn, &QPushButton::clicked, this, [=] {
        qApp->quit();
    });

    connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, [=](int index){
        ui->centralWidget->setVisible(false);    //避免出现明显的卡顿现象，在选择进入屏保界面之后这个问题比较明显，这种做法只是优化
        if (index){ //次页部分组件样式
            titleLabel->setHidden(true);
            mTitleIcon->setHidden(true);
            ui->leftsidebarWidget->setVisible(true);
            //左上角显示字符/返回按钮
            backBtn->setVisible(true);
            ui->stackedWidget->status = false;
            ui->titleWidget->status = false;
        } else { //首页部分组件样式
            //左侧边栏显示/不显示
            ui->leftsidebarWidget->setHidden(true);
            titleLabel->setVisible(true);
            mTitleIcon->setVisible(true);
            //左上角显示字符/返回按钮
            backBtn->setHidden(true);
            if (modulepageWidget) {
                modulepageWidget->pluginLeave();
            }
            ui->stackedWidget->status = true;
            ui->titleWidget->status = true;
        }
        ui->centralWidget->setVisible(true);
    });

    //加载首页Widget
    homepageWidget = new HomePageWidget(this);
    homepageWidget->installEventFilter(this);
    ui->stackedWidget->addWidget(homepageWidget);

    //加载左侧边栏一级菜单
    initLeftsideBar();

    //加载功能页Widget
    modulepageWidget = new ModulePageWidget(this);
    ui->stackedWidget->addWidget(modulepageWidget);

    connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, [=] (int id){
        if (id == 0) {
            modulepageWidget->mCurrentPluName =  "";
        }
    });
    connect(modulepageWidget, &ModulePageWidget::hScrollBarHide, this, [=]() {
        m_searchWidget->setFixedWidth(350);
    });
    connect(modulepageWidget, &ModulePageWidget::hScrollBarShow, this, [=]() {
        QTimer::singleShot(1, this, [=]() {
            if (m_searchWidget->width() > backBtn->x() - mOptionBtn->x()) {
                m_searchWidget->setFixedWidth(mOptionBtn->x() - backBtn->x() - mOptionBtn->width() - 16);
            }
        });
    });

    //top left return button
    connect(backBtn, &QPushButton::clicked, this, [=]{
        ui->stackedWidget->setCurrentIndex(0);
    });

    // 快捷参数
    if (QApplication::arguments().length() > 2) {
        QTimer::singleShot(2, this, [=](){  //延时保证菜单栏选中插件居中，控件未画出来，无法准确居中
            bootOptionsFilter(QApplication::arguments().at(2));
        });
    }
}

void MainWindow::initTileBar() {
    QLabel  *logoSetLabel  = new QLabel(this);
    QLabel  *textSetLable  = new QLabel(this);
    ui->leftTopWidget->layout()->addWidget(logoSetLabel);
    ui->leftTopWidget->layout()->addWidget(textSetLable);
    logoSetLabel->setFixedSize(24,24);
    logoSetLabel->setPixmap(QPixmap::fromImage(QIcon::fromTheme("ukui-control-center").pixmap(24,24).toImage()));
    const QByteArray id(THEME_QT_SCHEMA);
    QGSettings *mQtSettings = new QGSettings(id, QByteArray(), this);
    connect(mQtSettings, &QGSettings::changed, this, [=](QString key) {
        if (key == "iconThemeName")
            logoSetLabel->setPixmap(QPixmap::fromImage(QIcon::fromTheme("ukui-control-center").pixmap(24,24).toImage()));
    });

    textSetLable->setText(tr("Settings"));
    textSetLable->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    titleLayout = new QHBoxLayout(ui->titleWidget);
    ui->titleWidget->setLayout(titleLayout);
    ui->titleWidget->setObjectName("titleWidget");
    titleLayout->setContentsMargins(8, 4, 4, 0);
    titleLayout->setSpacing(0);
    m_searchWidget = new SearchWidget(this);
    m_searchWidget->setStyleSheet("background-color:palette(windowtext)");
    m_searchWidget->setFocusPolicy(Qt::ClickFocus);
    m_searchWidget->setTextMargins(30, 1, 0, 1);
    QHBoxLayout *searchLayout = new QHBoxLayout(m_searchWidget);
    searchLayout->setMargin(0);

    m_queryWid = new QWidget;
    searchLayout->addWidget(m_queryWid);
    searchLayout->setAlignment(m_queryWid, Qt::AlignHCenter);
    m_queryWid->setFocusPolicy(Qt::NoFocus);

    QHBoxLayout* queryWidLayout = new QHBoxLayout;
    queryWidLayout->setContentsMargins(0, 0, 0, 0);
    queryWidLayout->setAlignment(Qt::AlignJustify);
    queryWidLayout->setSpacing(0);
    m_queryWid->setLayout(queryWidLayout);

    QIcon searchIcon = QIcon::fromTheme("edit-find-symbolic");
    m_queryIcon = new QLabel(this);
    m_queryIcon->setPixmap(searchIcon.pixmap(searchIcon.actualSize(QSize(16, 16))));
    m_queryIcon->setProperty("useIconHighlightEffect",0x02);

    m_queryText = new QLabel(this);
    m_queryText->setText(tr("Search"));
    m_queryText->setStyleSheet("background:transparent;color:#626c6e;");

    queryWidLayout->addWidget(m_queryIcon);
    queryWidLayout->addWidget(m_queryText);

    m_searchWidget->setContextMenuPolicy(Qt::NoContextMenu);
    m_animation = new QPropertyAnimation(m_queryWid, "pos", this);
    m_animation->setDuration(100);
    titleLayout->addWidget(m_searchWidget,Qt::AlignCenter);
    connect(m_animation,&QPropertyAnimation::finished,this,&MainWindow::animationFinishedSlot);

    connect(m_searchWidget, &SearchWidget::notifyModuleSearch, this, &MainWindow::switchPage);

    backBtn     = new QPushButton(this);
    mOptionBtn  = new QToolButton(this);
    minBtn      = new QPushButton(this);
    maxBtn      = new QPushButton(this);
    closeBtn    = new QPushButton(this);
    mTitleIcon  = new QLabel(this);
    titleLabel  = new QLabel(tr("Settings"), this);

    backBtn->setFixedSize(30, 30);
    mOptionBtn->setFixedSize(30, 30);
    minBtn->setFixedSize(30, 30);
    maxBtn->setFixedSize(30, 30);
    closeBtn->setFixedSize(30, 30);
    mTitleIcon->setFixedSize(24, 24);

    mOptionBtn->setToolTip(tr("Main menu"));
    minBtn->setToolTip(tr("Minimize"));
    maxBtn->setToolTip(tr("Maximize"));
    closeBtn->setToolTip(tr("Close"));

    QIcon titleIcon = QIcon::fromTheme("ukui-control-center");
    mTitleIcon->setPixmap(titleIcon.pixmap(titleIcon.actualSize(QSize(24, 24))));

    changeSearchSlot();
    m_searchWidget->setFixedWidth(350);

    titleLayout->addWidget(mTitleIcon);
    titleLayout->addSpacing(8);
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(backBtn);
    titleLayout->addStretch();
    titleLayout->addWidget(m_searchWidget);
    titleLayout->addStretch();
    titleLayout->addWidget(mOptionBtn);
    titleLayout->addSpacing(4);
    titleLayout->addWidget(minBtn);
    titleLayout->addSpacing(4);
    titleLayout->addWidget(maxBtn);
    titleLayout->addSpacing(4);
    titleLayout->addWidget(closeBtn);

    initUkccAbout();
}
void MainWindow::animationFinishedSlot()
{
    if (m_isSearching && !m_searchKeyWords.isEmpty()) {
       m_searchWidget->setText(m_searchKeyWords);
       m_searchKeyWords.clear();
    }
}

void MainWindow::onF1ButtonClicked() {
    QString pluName = "ukui-control-center";
    if (!modulepageWidget->mCurrentPluName.isEmpty()) {
        pluName = QString("%1%2%3").arg(pluName).arg("/").arg(modulepageWidget->mCurrentPluName);
    }
    qDebug()<<pluName;
    showGuide(pluName);
}

void MainWindow::initUkccAbout() {
    mOptionBtn->setPopupMode(QToolButton::InstantPopup);
    QMenu* ukccMain = new QMenu(this);
    ukccMain->setObjectName("mainMenu");
    mOptionBtn->setMenu(ukccMain);

    QAction* ukccHelp = new QAction(tr("Help"),this);
    ukccMain->addAction(ukccHelp);
    QAction* ukccAbout = new QAction(tr("About"),this);
    ukccMain->addAction(ukccAbout);
    QAction* ukccExit = new QAction(tr("Exit"),this);
    ukccMain->addAction(ukccExit);

    connect(ukccExit, SIGNAL(triggered()), this, SLOT(close()));

    connect(ukccAbout, &QAction::triggered, this, [=] {
        UkccAbout *ukcc = new UkccAbout(this);
        ukcc->exec();
    });

     connect(ukccHelp, &QAction::triggered, this, &MainWindow::onF1ButtonClicked);
}
void MainWindow::setBtnLayout(QPushButton * &pBtn) {
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

    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {

        //三权分立开启
#ifdef WITHKYSEC
        if (!kysec_is_disabled() && kysec_get_3adm_status() && (getuid() || geteuid())){
            //时间和日期 | 用户帐户 | 电源管理 |网络连接 |网络代理
            if (fileName.contains("datetime") || fileName.contains("userinfo") || fileName.contains("power") || \
                    fileName.contains("netconnect") || fileName.contains("proxy") || fileName.contains("update") || \
                    fileName.contains("upgrade") || fileName.contains("backup") || fileName.contains("vino") ||
                    fileName.contains("printer") || fileName.contains("bluetooth", Qt::CaseInsensitive) || fileName.contains("mobilehotspot", Qt::CaseInsensitive) ||
                    fileName.contains("vpn") || fileName.contains("autoboot") || fileName.contains("wlanconnect") ||
                    fileName.contains("projection") || fileName.contains("area"))
                continue;
        }
#endif
        qDebug() << "Scan Plugin: " << fileName;
        if (!fileName.endsWith(".so")
                || ("libpower.so" == fileName && !is_ExitPower)
                || ("libtouchpad.so" == fileName && Utils::isWayland())
                ) {
            continue;
        }
#ifdef __sw_64__
        if ("libpower.so" == fileName) {
            continue;
        }
#endif

        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QObject * plugin = loader.instance();
        if (plugin) {
            CommonInterface * pluginInstance = qobject_cast<CommonInterface *>(plugin);
            // 插件是否启用
            if (!pluginInstance || !pluginInstance->isEnable() || !fileName.endsWith("so")) {
                continue;
            }

            modulesList[pluginInstance->pluginTypes()].insert(pluginInstance->plugini18nName(), plugin);

            qDebug() << "Load Plugin :" << kvConverter->keycodeTokeyi18nstring(pluginInstance->pluginTypes()) << "->" << pluginInstance->plugini18nName() ;

            m_searchWidget->addModulesName(pluginInstance->name(), pluginInstance->plugini18nName(), pluginInstance->translationPath());

            int moduletypeInt = pluginInstance->pluginTypes();
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

void MainWindow::initLeftsideBar(){

    leftBtnGroup = new QButtonGroup();
    leftMicBtnGroup = new QButtonGroup();

    scrollArea = new QScrollArea(ui->leftBotWidget);
    scrollArea->horizontalScrollBar()->setVisible(false);
    // scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    QWidget *menuWidget = new QWidget(this);
    QVBoxLayout *menuLayout = new QVBoxLayout(menuWidget);
    menuLayout->setContentsMargins(8,0,0,8);
    menuLayout->setSpacing(4);

    scrollArea->viewport()->setAttribute(Qt::WA_TranslucentBackground);
    scrollArea->setStyleSheet("QScrollArea{background-color: transparent;}");
    scrollArea->viewport()->setStyleSheet("background-color: transparent;");
    scrollArea->verticalScrollBar()->setProperty("drawScrollBarGroove", false);



    for(int type = 0; type < TOTALMODULES; type++) {
        //循环构建左侧边栏一级菜单按钮
        if (moduleIndexList.contains(type)){
            QString mnameString = kvConverter->keycodeTokeystring(type);
            QString mnamei18nString  = kvConverter->keycodeTokeyi18nstring(type); //设置TEXT

            if (m_ModuleMap.keys().contains(mnameString.toLower())) {
                if (!m_ModuleMap[mnameString.toLower()].toBool()) {
                    continue;
                }
            }

            QWidget *typeWidget     = new QWidget(menuWidget);
            QHBoxLayout *typeLayout = new QHBoxLayout(typeWidget);
            LightLabel *typeLabel       = new LightLabel(typeWidget);
            if (type != 0) {
                typeLayout->setContentsMargins(18,20,0,0);
            } else {
                typeLayout->setContentsMargins(18,10,0,0);
            }
            typeLayout->addWidget(typeLabel);
            typeLabel->setText(mnamei18nString);
            menuLayout->addWidget(typeWidget);

            QMap<QString, QObject *> moduleMap;
            moduleMap = this->exportModule(type);
            QVariantMap mModuleMap;
            mModuleMap = Utils::getModuleHideStatus();

            QList<FuncInfo> functionStructList = FunctionSelect::funcinfoList[type];
            for (int funcIndex = 0; funcIndex < functionStructList.size(); funcIndex++) {
                FuncInfo single = functionStructList.at(funcIndex);
                //跳过插件不存在的功能项
                if (!moduleMap.contains(single.namei18nString) || !single.isEnable) {
                    continue;
                }

                if (mModuleMap.keys().contains(single.nameString.toLower())) {
                    if (!mModuleMap[single.nameString.toLower()].toBool()) {
                        continue;
                    }
                }
                // intel与sp1做区分
                if ((Utils::isTablet() && single.nameString == "Userinfo")
                    || (!Utils::isTablet() && single.nameString == "Userinfointel")) {
                    continue;
                }

                //填充左侧菜单
                CommonInterface * pluginInstance = qobject_cast<CommonInterface *>(moduleMap.value(single.namei18nString));
                QPushButton *pluginBtn = buildLeftsideBtn(single.nameString, single.namei18nString, pluginInstance->icon());
                leftBtnGroup->addButton(pluginBtn, type);
                pluginBtn->setProperty("useButtonPalette", true);

                QHBoxLayout *pluginLayout = new QHBoxLayout();
                menuLayout->addLayout(pluginLayout);
                pluginLayout->setContentsMargins(0, 0, 0, 0);
//                pluginBtn->setStyleSheet("QPushButton:checked{background-color: palette(highlight);border-radius: 6px;}");

                pluginLayout->addWidget(pluginBtn);
                pluginInstance->pluginBtn = pluginBtn;
                // 初始化插件状态
                QGSettings *msettings = nullptr;
                if (homepageWidget->vecGsettins.contains(single.nameString)) {
                    msettings = homepageWidget->vecGsettins[single.nameString];
                    if (msettings) {
                        pluginBtn->setVisible(msettings->get(SHOW_KEY).toBool());
                        m_searchWidget->hiddenSearchItem(QLocale::system().name() == "zh_CN" ? single.namei18nString : single.nameString , msettings->get(SHOW_KEY).toBool());
                    }
                }

                // 监听该插件是否启用
                if (msettings) {
                    connect(msettings , &QGSettings::changed,[=](QString key){
                        if (key == SHOW_KEY) {
                            if ( !msettings->get(SHOW_KEY).toBool() && pluginBtn->isChecked()) {
                                ui->stackedWidget->setCurrentIndex(0);
                            }
                            pluginBtn->setVisible( msettings->get(SHOW_KEY).toBool());
                            m_searchWidget->hiddenSearchItem(QLocale::system().name() == "zh_CN" ? single.namei18nString : single.nameString , msettings->get(SHOW_KEY).toBool());
                        } else {
                             m_searchWidget->hiddenSearchItem(QLocale::system().name() == "zh_CN" ? single.namei18nString : single.nameString , msettings->get(SHOW_KEY).toBool());
                        }
                    });
                }

                connect(pluginBtn, &QPushButton::clicked, this, [=](){
                    modulepageWidget->refreshPluginWidget(pluginInstance);
                });
            }
        }
    }
    menuLayout->addStretch();
    scrollArea->setWidget(menuWidget);
    ui->leftBotLayout->addWidget(scrollArea);
}

QPushButton * MainWindow::buildLeftsideBtn(QString bname,QString tipName, QIcon icon) {
    int itype = kvConverter->keystringTokeycode(bname);

    QPushButton * leftsidebarBtn = new QPushButton();
    leftsidebarBtn->setAttribute(Qt::WA_DeleteOnClose);
    leftsidebarBtn->setCheckable(true);
    leftsidebarBtn->setFixedSize(230,40);  //一级菜单按钮显示的宽度

    IconButton * iconBtn = new IconButton(bname, icon, leftsidebarBtn);
    iconBtn->setCheckable(true);
    iconBtn->setFixedSize(QSize(16, 16));
    iconBtn->setFocusPolicy(Qt::NoFocus);
    iconBtn->reLoadIcon();
    static QString hoverColor;
    static QString clickColor;

    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        QGSettings *qtSettings = new QGSettings("org.ukui.style", QByteArray(), this);
        if (qtSettings->keys().contains("styleName")) {
            hoverColor = pluginBtnHoverColor(qtSettings->get("style-name").toString(), true);
            clickColor = pluginBtnHoverColor(qtSettings->get("style-name").toString(), false);
            if (!leftsidebarBtn->isChecked())
                leftsidebarBtn->setStyleSheet(QString("QPushButton:hover{background-color:%1;border-radius: 6px;}"
                                                      "QPushButton:pressed{background-color:%2;border-radius: 6px;}").arg(hoverColor).arg(clickColor));
        }

        connect(qtSettings, &QGSettings::changed, this, [=,&hoverColor](const QString &key) {
            if (key == "styleName") {
                iconBtn->reLoadIcon();
                hoverColor = this->pluginBtnHoverColor(qtSettings->get("style-name").toString(), true);
                clickColor = pluginBtnHoverColor(qtSettings->get("style-name").toString(), false);
                if (!leftsidebarBtn->isChecked())
                    leftsidebarBtn->setStyleSheet(QString("QPushButton:hover{background-color:%1;border-radius: 6px;}"
                                                          "QPushButton:pressed{background-color:%2;border-radius: 6px;}").arg(hoverColor).arg(clickColor));
            }
        });
    }

    QLabel * textLabel = new QLabel(leftsidebarBtn);
    textLabel->setFixedWidth(leftsidebarBtn->width() - 40);
    QFontMetrics  fontMetrics(textLabel->font());
    int fontSize = fontMetrics.width(tipName);
    if (fontSize > textLabel->width()) {
        textLabel->setText(fontMetrics.elidedText(tipName, Qt::ElideRight, textLabel->width()));
    } else {
        textLabel->setText(tipName);
    }
    const QByteArray styleID(STYLE_FONT_SCHEMA);
    QGSettings *stylesettings = new QGSettings(styleID, QByteArray(), this);
    connect(stylesettings,&QGSettings::changed,[=](QString key)
    {
        if("systemFont" == key || "systemFontSize" == key)
        {
            QFontMetrics  fontMetrics_1(textLabel->font());
            int fontSize_1 = fontMetrics_1.width(tipName);

            if (fontSize_1 > textLabel->width()) {
                qDebug()<<textLabel->width();
                textLabel->setText(fontMetrics_1.elidedText(tipName, Qt::ElideRight, textLabel->width()));
            } else {
                textLabel->setText(tipName);
            }
        }
    });


    QSizePolicy textLabelPolicy = textLabel->sizePolicy();
    textLabelPolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    textLabelPolicy.setVerticalPolicy(QSizePolicy::Fixed);
    textLabel->setSizePolicy(textLabelPolicy);
    textLabel->setScaledContents(true);

    leftMicBtnGroup->addButton(iconBtn, itype);

    connect(iconBtn, &QPushButton::toggled, this, [=] (bool checked) {
        iconBtn->reLoadIcon();
        if (checked) {
            textLabel->setStyleSheet("color:white");
        } else {
            textLabel->setStyleSheet("color:palette(windowText)");
        }
    });

    connect(iconBtn, &QPushButton::clicked, leftsidebarBtn, &QPushButton::click);

    connect(leftsidebarBtn, &QPushButton::toggled, this, [=](bool checked) {
        iconBtn->setChecked(checked);
        if (checked) {
            leftsidebarBtn->setStyleSheet("QPushButton:checked{background-color: palette(highlight);border-radius: 6px;}");
            textLabel->setStyleSheet("color:white");
        } else {
            leftsidebarBtn->setStyleSheet(QString("QPushButton:hover{background-color:%1;border-radius: 6px;}"
                                                  "QPushButton:pressed{background-color:%2;border-radius: 6px;}").arg(hoverColor).arg(clickColor));
            textLabel->setStyleSheet("color:palette(windowText)");
        }
    });

    QHBoxLayout * btnHorLayout = new QHBoxLayout();
    btnHorLayout->setContentsMargins(14,0,0,0);
    btnHorLayout->addWidget(iconBtn, Qt::AlignCenter);
    btnHorLayout->addWidget(textLabel);
    btnHorLayout->addStretch();
    btnHorLayout->setSpacing(8);

    leftsidebarBtn->setLayout(btnHorLayout);

    return leftsidebarBtn;
}

bool MainWindow::isExitsCloudAccount() {
    QProcess *wifiPro = new QProcess();
    QString shellOutput = "";
    wifiPro->start("dpkg -l  | grep kylin-sso-client");
    wifiPro->waitForFinished();
    QString output = wifiPro->readAll();
    shellOutput += output;
    QStringList slist = shellOutput.split("\n");

    for (QString res : slist) {
        if (res.contains("kylin-sso-client")) {
            return true;
        }
    }
    return false;
}

bool MainWindow::isExitsPower()
{
    QProcess *process = new QProcess;
    process->start("dpkg -l ukui-power-manager");
    process->waitForFinished();

    QByteArray ba = process->readAllStandardOutput();
    delete process;
    QString mOutput = QString(ba.data());

    return mOutput.contains("ii", Qt::CaseSensitive) ? true : false;
}

bool MainWindow::isExitWirelessDevice()
{
    QDBusInterface *interface = new QDBusInterface("com.kylin.network", "/com/kylin/network",
                                     "com.kylin.network",
                                     QDBusConnection::sessionBus());
    if (!interface->isValid()) {
        qDebug() << "/com/kylin/network is invalid";
        return false;
    }

    QDBusMessage result = interface->call(QStringLiteral("getDeviceListAndEnabled"),1);
    if(result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "getWirelessDeviceList error:" << result.errorMessage();
        return false;
    }

    auto dbusArg =  result.arguments().at(0).value<QDBusArgument>();
    QMap<QString, bool> deviceListMap;
    dbusArg >> deviceListMap;


    if (deviceListMap.isEmpty()) {
        qDebug() << "no wireless device";
        return false;
    }
    return true;
}

bool MainWindow::dblOnEdge(QMouseEvent *event) {
    QPoint pos = event->globalPos();
    int globalMouseY = pos.y();

    int frameY = this->y();

    bool onTopEdges = (globalMouseY >= frameY &&
                       globalMouseY <= frameY + dbWitdth);
    return onTopEdges;
}

void MainWindow::initStyleSheet() {
    // 设置panel图标
    QIcon panelicon;
    if (QIcon::hasThemeIcon("ukui-control-center"))
        panelicon = QIcon::fromTheme("ukui-control-center");

    qApp->setWindowIcon(panelicon);
    this->setWindowTitle(tr("Settings"));
    ui->stackedWidget->setAutoFillBackground(true);

    // 左上角返回按钮
    backBtn->setProperty("useIconHighlightEffect", true);
    backBtn->setProperty("iconHighlightEffectMode", 1);
    backBtn->setFlat(true);

    mOptionBtn->setProperty("useIconHighlightEffect", 0x2);
    mOptionBtn->setProperty("isWindowButton", 0x01);
    mOptionBtn->setAutoRaise(true);

    minBtn->setProperty("useIconHighlightEffect", 0x2);
    minBtn->setProperty("isWindowButton", 0x01);
    minBtn->setFlat(true);

    maxBtn->setProperty("useIconHighlightEffect", 0x2);
    maxBtn->setProperty("isWindowButton", 0x1);
    maxBtn->setFlat(true);

    closeBtn->setProperty("isWindowButton", 0x02);
    closeBtn->setProperty("useIconHighlightEffect", 0x08);
    closeBtn->setFlat(true);

    ui->centralWidget->setAttribute(Qt::WA_TranslucentBackground);
    ui->leftsidebarWidget->setFixedWidth(260); //一级菜单宽度

    // 设置左上角按钮图标
    backBtn->setIcon(QIcon::fromTheme("go-home-symbolic"));
    backBtn->setIconSize(QSize(24, 24));

    // 设置右上角按钮图标
    mOptionBtn->setIcon(QIcon::fromTheme("open-menu-symbolic"));
    minBtn->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    maxBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
    closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
}

void MainWindow::changeSearchSlot() {
//    int fontSize = m_fontSetting->get("system-font-size").toInt();
//    m_searchWidget->setFixedHeight((fontSize - 11) * 2 + 32);
}

void MainWindow::showGuide(QString pluName)
{
    QString service_name = "com.kylinUserGuide.hotel_" + QString::number(getuid());
    qDebug()<<service_name<<"---------------"<<pluName;
    QDBusInterface *interface = new QDBusInterface(service_name,
                                                   KYLIN_USER_GUIDE_PATH,
                                                   KYLIN_USER_GUIDE_INTERFACE,
                                                   QDBusConnection::sessionBus(),
                                                   this);
    QDBusMessage msg = interface->call("showGuide" , pluName);
}

void MainWindow::hideComponent()
{
    QTimer::singleShot(100, this, [=]() {
        if(Utils::isTablet()) {
            bootOptionsFilter("userinfointel");
            backBtn->hide();
        }
    });
}

void MainWindow::setModuleBtnHightLight(int id) {
    leftBtnGroup->button(id)->setChecked(true);
    leftMicBtnGroup->button(id)->setChecked(true);
}

QMap<QString, QObject *> MainWindow::exportModule(int type) {
    QMap<QString, QObject *> emptyMaps;
    if (type < modulesList.length())
        return modulesList[type];
    else
        return emptyMaps;
}

void MainWindow::functionBtnClicked(QObject *plugin) {
    if (!scrollArea)
        return;

    ui->stackedWidget->setCurrentIndex(1);
    modulepageWidget->switchPage(plugin);

    CommonInterface * pluginInstance = qobject_cast<CommonInterface *>(plugin);
    int value = pluginInstance->pluginBtn->pos().y() + pluginInstance->pluginBtn->height() - scrollArea->verticalScrollBar()->pageStep();
    value = value + scrollArea->height()/2; //尽量让选中的显示在中间位置
    if (value <= 0) {
        scrollArea->verticalScrollBar()->setValue(0);
    } else if (value > scrollArea->verticalScrollBar()->maximum()){
        scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->maximum());
    } else {
        scrollArea->verticalScrollBar()->setValue(value);
    }
}

void MainWindow::sltMessageReceived(const QString &msg) {
    KWindowSystem::forceActiveWindow(this->winId());
    this->show();
    bootOptionsFilter(msg);
}

void MainWindow::switchPage(QString moduleName, QString jumpMoudle) {

    for (int i = 0; i < modulesList.length(); i++) {
        auto modules = modulesList.at(i);
        //开始跳转
        if (modules.keys().contains(moduleName)) {
            if (m_ModuleMap.isEmpty() || m_ModuleMap[jumpMoudle.toLower()].toBool()) {
                functionBtnClicked(modules.value(moduleName));
                return ;
            }
        }
    }
    QMessageBox::information(this, tr("Warning"), tr("This function has been controlled"));
    return;
}

void MainWindow::moveEvent(QMoveEvent *event)
{
    Q_UNUSED(event);
    Q_EMIT posChanged();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    Q_EMIT posChanged();
}

QString MainWindow::pluginBtnHoverColor(QString styleName, bool hoverFlag)
{
    QColor color1 = palette().color(QPalette::Active, QPalette::Button);
    QColor color2 = palette().color(QPalette::Active, QPalette::BrightText);
    QColor color;
    qreal r,g,b,a;
    QString hoverColor;
    if (((styleName.contains("dark") || styleName.contains("black")) && hoverFlag) ||
        ((!styleName.contains("dark") && !styleName.contains("black")) && !hoverFlag)) {
        r = color1.redF() * 0.8 + color2.redF() * 0.2;
        g = color1.greenF() * 0.8 + color2.greenF() * 0.2;
        b = color1.blueF() * 0.8 + color2.blueF() * 0.2;
        a = color1.alphaF() * 0.8 + color2.alphaF() * 0.2;
    } else {
        r = color1.redF() * 0.95 + color2.redF() * 0.05;
        g = color1.greenF() * 0.95 + color2.greenF() * 0.05;
        b = color1.blueF() * 0.95 + color2.blueF() * 0.05;
        a = color1.alphaF() * 0.95 + color2.alphaF() * 0.05;
    }
    color = QColor::fromRgbF(r, g, b, a);
    hoverColor = QString("rgba(%1, %2, %3, %4)").arg(color.red())
                                                .arg(color.green())
                                                .arg(color.blue())
                                                .arg(color.alpha());
    return hoverColor;
}
