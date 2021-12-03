#include "widget.h"
#include "controlpanel.h"
#include "declarative/qmloutput.h"
#include "declarative/qmlscreen.h"
#include "utils.h"
#include "ui_display.h"
#include "displayperformancedialog.h"

#include <QHBoxLayout>
#include <QTimer>
#include <QLabel>
#include <QVBoxLayout>
#include <QSplitter>
#include <QtGlobal>
#include <QQuickView>
#include <qquickitem.h>
#include <QDebug>
#include <QPushButton>
#include <QProcess>
#include <QtAlgorithms>
#include <QtXml>
#include <QDomDocument>
#include <QDir>
#include <QStandardPaths>
#include <QComboBox>
#include <QQuickWidget>
#include <QElapsedTimer>

#include <KAboutData>
#include <KMessageBox>
#include <KF5/KScreen/kscreen/output.h>
#include <KF5/KScreen/kscreen/edid.h>
#include <KF5/KScreen/kscreen/mode.h>
#include <KF5/KScreen/kscreen/config.h>
#include <KF5/KScreen/kscreen/getconfigoperation.h>
#include <KF5/KScreen/kscreen/configmonitor.h>
#include <KF5/KScreen/kscreen/setconfigoperation.h>
#include <KF5/KScreen/kscreen/edid.h>

#ifdef signals
#undef signals
#endif

extern "C" {
#define MATE_DESKTOP_USE_UNSTABLE_API
#include <libmate-desktop/mate-rr.h>
#include <libmate-desktop/mate-rr-config.h>
#include <libmate-desktop/mate-rr-labeler.h>
#include <libmate-desktop/mate-desktop-utils.h>
#include <math.h>
}

enum eScreenMode {
    firstScreenMode = 0,
    cloneScreenMode,
    extendScreenMode,
    secondScreenMode
};

#define QML_PATH "kcm_kscreen/qml/"

#define UKUI_CONTORLCENTER_PANEL_SCHEMAS "org.ukui.control-center.panel.plugins"
#define NIGHT_MODE_KEY                   "nightmodestatus"

#define FONT_RENDERING_DPI               "org.ukui.SettingsDaemon.plugins.xsettings"
#define SCALE_KEY                        "scaling-factor"

#define DPI_SCHEMAS                      "org.ukui.font-rendering"
#define DPI_KEY                          "dpi"

#define MOUSE_SIZE_SCHEMAS               "org.ukui.peripherals-mouse"
#define CURSOR_SIZE_KEY                  "cursor-size"

#define POWER_SCHMES                     "org.ukui.power-manager"
#define POWER_KEY                        "brightness-ac"

#define ADVANCED_SCHEMAS                 "org.ukui.session.required-components"
#define ADVANCED_KEY                     "windowmanager"

#define SETTINGS_DAEMON_COLOR_SCHEMAS    "org.ukui.SettingsDaemon.plugins.color"
#define AUTO_KEY                         "night-light-schedule-automatic"
#define AllDAY_KEY                       "night-light-allday"
#define NIGHT_ENABLE_KEY                 "night-light-enabled"
#define NIGHT_FROM_KEY                   "night-light-schedule-from"
#define NIGHT_TO_KEY                     "night-light-schedule-to"
#define AUTO_NIGHT_FROM_KEY              "night-light-schedule-automatic-from"
#define AUTO_NIGHT_TO_KEY                "night-light-schedule-automatic-to"
#define NIGHT_TEMPERATURE_KEY            "night-light-temperature"

#define SETTINGS_XRANDR_SCHEMAS          "org.ukui.SettingsDaemon.plugins.xrandr"
#define XRANDR_MIRROR_MODE               "xrandr-mirror-mode"
#define XRANDR_CLONE                     "xrandr-clone"

#define CONTROL_CENTER_MODE              "org.ukui.control-center.personalise"
void value_to_hour_minute(double value, int *hour, int *minute)
{
    double hours;
    double mins = 0.f;
    /* display the right thing for AM/PM */
    mins = modf (value, &hours) * 60.f;
    *hour = int(hours+0.5);
    *minute = int(mins+0.5);
}

Q_DECLARE_METATYPE(KScreen::OutputPtr)
MateRRScreen          *mmScreen;
DisplayWidget::DisplayWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DisplayWindow()), slider(new Slider())
{
    qRegisterMetaType<QQuickView*>();
    gdk_init(NULL, NULL);
    ui->setupUi(this);
    m_initPath = QString("%1/%2/%3").arg(QDir::homePath()).arg(".config/ukui").arg("config.ini");//构造函数中初始化设置信息
    ui->quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    initUI();
#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)
    oriApply = true;
#else
    oriApply = false;
    ui->quickWidget->setAttribute(Qt::WA_AlwaysStackOnTop);
    ui->quickWidget->setClearColor(Qt::transparent);
#endif

    ui->quickWidget->setContentsMargins(0,0,0,0);

    ui->showMonitorframe->hide();
    closeScreenButton = new SwitchButton;
    ui->showScreenLayout->addWidget(closeScreenButton);
    m_unifybutton = new SwitchButton;
//    m_unifybutton->setEnabled(false);
    ui->unionLayout->addWidget(m_unifybutton);
    QHBoxLayout *lightLayout = new QHBoxLayout(ui->lightframe);
    QHBoxLayout *nightLayout = new QHBoxLayout(ui->nightframe);

    lightButton = new SwitchButton;

    lightLayout->addWidget(lightLabel);
    lightLayout->addStretch();
    lightLayout->addWidget(lightButton);


    nightButton = new SwitchButton;
    nightLayout->addWidget(nightLabel);
    nightLayout->addStretch();
    nightLayout->addWidget(nightButton);

    QProcess * process = new QProcess;
    process->start("lsb_release -r");
    process->waitForFinished();

    QByteArray ba = process->readAllStandardOutput();
    QString osReleaseCrude = QString(ba.data());
    QStringList res = osReleaseCrude.split(":");
    QString osRelease = res.length() >= 2 ?  res.at(1) : "";
    osRelease = osRelease.simplified();

    if (QGSettings::isSchemaInstalled(POWER_SCHMES)
     && QGSettings::isSchemaInstalled(SETTINGS_XRANDR_SCHEMAS)
     && QGSettings::isSchemaInstalled(CONTROL_CENTER_MODE)) {
        powerSettings = new QGSettings(POWER_SCHMES);
        xrandrSettings = new QGSettings(SETTINGS_XRANDR_SCHEMAS);
        m_settings = new QGSettings(CONTROL_CENTER_MODE);
    }

    initGSettings();
    initTemptSlider();
    initConfigFile();
    initUiComponent();
    initBrightnessUI();
    hasAutoLight();
    initUnifybuttonStatus();
//    nightButton->setVisible(this->m_redshiftIsValid);

#if QT_VERSION <= QT_VERSION_CHECK(5,12,0)
    ui->nightframe->setVisible(false);
#else
    ui->nightframe->setVisible(true);
#endif
    if (QGSettings::isSchemaInstalled(SETTINGS_DAEMON_AUTOBRIGHTNESS)) {
        m_brilight =new QGSettings(SETTINGS_DAEMON_AUTOBRIGHTNESS);
        if (m_brilight != nullptr) {
            connect(m_brilight, &QGSettings::changed, this, &DisplayWidget::slotChangeAutoBrightness);
        }
    }
    bool autobristatus = false;
    if (m_brilight && m_brilight->keys().contains("autoBrightness")) {
        autobristatus = m_brilight->get(AUTO_BRIGHTINESS_KEY).toBool();
    }
    lightButton->setChecked(autobristatus);
    connect(nightButton,SIGNAL(checkedChanged(bool)),this,SLOT(showNightWidget(bool)));
    connect(lightButton,SIGNAL(checkedChanged(bool)),this,SLOT(showlightWidget(bool)));
    connect(singleButton, SIGNAL(buttonClicked(int)), this, SLOT(showCustomWiget(int)));
    //是否禁用主显示器确认按钮
    connect(ui->primaryCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &DisplayWidget::mainScreenButtonSelect);
    //主屏确认按钮
    connect(ui->mainScreenButton, SIGNAL(clicked()),this, SLOT(primaryButtonEnable()));
    mControlPanel = new ControlPanel(this);
    connect(mControlPanel, &ControlPanel::changed, this, [ = ] () {
        Q_EMIT changed();
        apply();
    });
    connect(mControlPanel, &ControlPanel::scaleChanged, this, &DisplayWidget::scaleChangedSlot);

    ui->controlPanelLayout->addWidget(mControlPanel);
    m_unifyTimer  =new QTimer();
    connect(m_unifyTimer,SIGNAL(timeout()),this,SLOT(m_unifybuttonChanged()));
    connect(m_unifybutton,SIGNAL(checkedChanged(bool)),this,SLOT(startUnifyButton()));

    mOutputTimer = new QTimer(this);
    mScreenRemoveTimer = new QTimer(this);
    connect(mOutputTimer, &QTimer::timeout,
            this, &DisplayWidget::clearOutputIdentifiers);

    loadQml();
    mScreen->setDisplayWidget(this);
    connect(mScreen, &QMLScreen::changed, this, [ = ]() {
        apply();
    });

}
DisplayWidget::~DisplayWidget()
{
    clearOutputIdentifiers();
    delete ui;
    if (powerSettings) {
        delete powerSettings;
    }
    if (m_colorSettings) {
        delete m_colorSettings;
    }
    if (xrandrSettings) {
        delete xrandrSettings;
    }
}

void DisplayWidget::startUnifyButton() {
    m_unifyTimer->start(700);
}

void DisplayWidget::initUnifybuttonStatus() {
    mScreenAddTimer = new QTimer();
    connect(m_statusSessionDbus, SIGNAL(mode_change_signal(bool)), this, SLOT(widget_DbusSlot(bool)));

    connect(qApp, SIGNAL(screenAdded(QScreen *)),this,SLOT(screenAddedTimer()));
    connect(mScreenAddTimer,SIGNAL(timeout()),this,SLOT(screenAddedProcess()));
//    connect(mScreenRemoveTimer,SIGNAL(timeout()),this,SLOT(screenRemovedTimer));
    connect(qApp, SIGNAL(screenRemoved(QScreen *)),this, SLOT(screenRemovedProcess()));
    connect(m_ScreenModeDbus, SIGNAL(screenModeChanged(int)), this, SLOT(screenModeChangeSlot(int)));
}

void DisplayWidget::widget_DbusSlot(bool tablet_mode)
{
    QDBusReply<int> pcMode = m_ScreenModeDbus->call("getScreenMode", "ukcc");
    QMLOutput *base = mScreen->primaryOutput();
    int enabledOutputsCount = 0;
    Q_FOREACH (const KScreen::OutputPtr &output, mConfig->outputs()) {
        if (output->isEnabled()) {
            ++enabledOutputsCount;
        }
        if (enabledOutputsCount > 1) {
            break;
        }
    }
    if (enabledOutputsCount > 1 && tablet_mode ) {
        m_unifybutton->setChecked(true);
        m_unifybutton->setEnabled(false);
    } else if (enabledOutputsCount > 1 && !tablet_mode && pcMode == extendScreenMode){
        m_unifybutton->setEnabled(true);
        m_unifybutton->setChecked(false);
    } else if (enabledOutputsCount > 1 && !tablet_mode && pcMode == cloneScreenMode){
        m_unifybutton->setEnabled(true);
        m_unifybutton->setChecked(true);
    }
}

void DisplayWidget::screenModeChangeSlot(int screenMode)
{
    QDBusReply<bool> tablet_mode = m_statusSessionDbus->call("get_current_tabletmode");
    QMLOutput *base = mScreen->primaryOutput();
    int enabledOutputsCount = 0;
    Q_FOREACH (const KScreen::OutputPtr &output, mConfig->outputs()) {
        if (output->isEnabled()) {
            ++enabledOutputsCount;
        }
        if (enabledOutputsCount > 1) {
            break;
        }
    }
    if (enabledOutputsCount > 1 && tablet_mode ) {
        m_unifybutton->setChecked(true);
        m_unifybutton->setEnabled(false);
    } else if (enabledOutputsCount > 1 && !tablet_mode && screenMode == extendScreenMode){
        m_unifybutton->setEnabled(true);
        m_unifybutton->setChecked(false);
    } else if (enabledOutputsCount > 1 && !tablet_mode && screenMode == cloneScreenMode){
        m_unifybutton->setEnabled(true);
        m_unifybutton->setChecked(true);
    }
}

void DisplayWidget::screenAddedProcess() {
    mScreenAddTimer->stop();
    QMLOutput *base = mScreen->primaryOutput();
    QDBusReply<bool> tb_mode = m_statusSessionDbus->call("get_current_tabletmode");
    if (tb_mode) {
//        base->setIsCloneMode(true);
        m_unifybutton->setChecked(true);
//        m_unifybutton->setEnabled(false);
    }
    apply();
}

void DisplayWidget::screenAddedTimer (){
    mScreenAddTimer->start(800);
}

void DisplayWidget::m_unifybuttonChanged() {
    m_unifyTimer->stop();
    unifyChecked = true;
    m_ScreenModeDbus = new QDBusInterface("org.ukui.SettingsDaemon",
                                           "/org/ukui/SettingsDaemon/xrandr",
                                           "org.ukui.SettingsDaemon.xrandr",
                                           QDBusConnection::sessionBus(), this);
    QDBusReply<bool> tb_mode = m_statusSessionDbus->call("get_current_tabletmode");
    if (!tb_mode) {
        if (m_unifybutton->isChecked()) {
            m_ScreenModeDbus->call("setScreenMode", "cloneScreenMode", "ukcc");
        } else {
            m_ScreenModeDbus->call("setScreenMode", "extendScreenMode", "ukcc");
        }
    }

    slotUnifyOutputs();
}
void DisplayWidget::initUI(){

    ui->darkLabel->setPixmap(QPixmap("://img/plugins/display/dark.svg"));
    ui->brightLabel->setPixmap(QPixmap("://img/plugins/display/bright.svg"));

    //~ contents_path /display/Screen mirror
    ui->unifyLabel->setText(tr("Screen mirror"));
    ui->toolTip->setText(tr("Tablet mode only supports mirroring"));
    ui->toolTip->setEnabled(false);
    ui->label->setText(tr("Open this monitor"));
    lightLabel = new QLabel(tr("auto light"));
    ////~ contents_path /display/enable night Mode
    nightLabel = new QLabel(tr("enable night Mode"));
    //~ contents_path /display/screen brightness
    ui->birghtnessLabel->setText(tr("screen brightness"));
    //~ contents_path /display/Night Mode
    ui->nightModeLabel->setText(tr("Night Mode"));
    //~ contents_path /display/all time
    ui->alltimelabel->setText(tr("all time"));
    //~ contents_path /display/color temperature
    ui->label_18->setText(tr("color temperature"));

    ui->sunradioBtn->setStyleSheet("QRadioButton::indicator:unchecked {image: url(:/img/plugins/theme/notselected_default.svg);}"
                                   "QRadioButton::indicator:unchecked:hover {image: url(:/img/plugins/theme/selected_hover.svg);}"
                                   "QRadioButton::indicator:unchecked:pressed {image: url(:/img/plugins/theme/selected_default.svg);}"
                                   "QRadioButton::indicator:checked {image: url(:/img/plugins/theme/selected_click.svg);}");
    ui->alldayradioBtn->setStyleSheet("QRadioButton::indicator:unchecked {image: url(:/img/plugins/theme/notselected_default.svg);}"
                                      "QRadioButton::indicator:unchecked:hover {image: url(:/img/plugins/theme/selected_hover.svg);}"
                                      "QRadioButton::indicator:unchecked:pressed {image: url(:/img/plugins/theme/selected_default.svg);}"
                                      "QRadioButton::indicator:checked {image: url(:/img/plugins/theme/selected_click.svg);}");
    ui->brightnessSlider->setFixedHeight(60);
    ui->brightnessSlider->setStyleSheet( "QSlider::groove:horizontal {"
                                       "border: 0px none;"
                                       "background: palette(button);"
                                       "height: 8px;"
                                       "border-radius: 5px;"
                                       "}"

                                       "QSlider::handle:horizontal {"

                                       "height: 40px;"
                                       "width: 36px;"
                                       "margin: 30px;"
                                       "border-image: url(://img/plugins/mouse/slider.svg);"
                                       "margin: -20 -4px;"
                                       "}"

                                       "QSlider::add-page:horizontal {"
                                       "background: palette(button);"
                                       "border-radius: 20px;"
                                       "}"

                                       "QSlider::sub-page:horizontal {"
                                       "background: #2FB3E8;"
                                       "border-radius: 5px;"
                                       "}");
    ui->temptSlider->setFixedHeight(60);
    ui->temptSlider->setStyleSheet( "QSlider::groove:horizontal {"
                                    "border: 0px none;"
                                    "background: palette(button);"
                                    "height: 8px;"
                                    "border-radius: 5px;"
                                    "}"

                                    "QSlider::handle:horizontal {"

                                    "height: 40px;"
                                    "width: 36px;"
                                    "margin: 30px;"
                                    "border-image: url(://img/plugins/mouse/slider.svg);"
                                    "margin: -20 -4px;"
                                    "}"

                                    "QSlider::add-page:horizontal {"
                                    "background: palette(button);"
                                    "border-radius: 20px;"
                                    "}"

                                    "QSlider::sub-page:horizontal {"
                                    "background: #2FB3E8;"
//                                    "background: #DDDDDD;"
                                    "border-radius: 5px;"
                                    "}");
}

void DisplayWidget::hasAutoLight()
{
    bool hasAutoLighted;
    m_widgetInterface = new QDBusInterface("net.hadess.SensorProxy",
                                         "/net/hadess/SensorProxy",
                                         "org.freedesktop.DBus.Properties",
                                         QDBusConnection::systemBus());
    if(!m_widgetInterface->isValid()) {
        qCritical() << "Create Client Interface Failed: " << QDBusConnection::systemBus().lastError().message();
        return;
    }
    QDBusReply<QMap<QString, QVariant> > reply = m_widgetInterface->call("GetAll","net.hadess.SensorProxy");
    if (reply.isValid()) {
        QMap<QString, QVariant> propertyMap;
        propertyMap = reply.value();
        hasAutoLighted = propertyMap.find("HasAmbientLight").value().toBool();
    } else {
        hasAutoLighted = false;
        qDebug() << "reply failed";
    }
    if (hasAutoLighted) {
        qDebug()<<"检测到该设备支持自动亮度调节";
        ui->lightframe->setVisible(true);
    } else {
        qDebug()<<"检测到该设备不支持自动亮度调节";
        ui->lightframe->setVisible(false);
    }
}

void DisplayWidget::OnRandrEvent()
{
    QDBusInterface *tabletModeDbus;
    QGSettings * xrandrGSettings;
    MateRRConfig *rr_config;
    bool isClone = false;
    if (QGSettings::isSchemaInstalled(SETTINGS_XRANDR_SCHEMAS)) {
        xrandrGSettings = new QGSettings(SETTINGS_XRANDR_SCHEMAS);
    }
    tabletModeDbus = new QDBusInterface("com.kylin.statusmanager.interface",
                                       "/",
                                       "com.kylin.statusmanager.interface",
                                       QDBusConnection::sessionBus());
    if (!mmScreen) {
        return;
    }

    m_ScreenModeDbus = new QDBusInterface("org.ukui.SettingsDaemon",
                                           "/org/ukui/SettingsDaemon/xrandr",
                                           "org.ukui.SettingsDaemon.xrandr",
                                           QDBusConnection::sessionBus());

    rr_config = mate_rr_config_new_current (mmScreen, NULL);
    isClone =  mate_rr_config_get_clone(rr_config);  
    QDBusReply<bool> is_tabletMode = tabletModeDbus->call("get_current_tabletmode");
    if (isClone) {
        if (!is_tabletMode) {
            m_ScreenModeDbus->call("setScreenMode", "cloneScreenMode", "ukcc");
        }
        xrandrGSettings->set(XRANDR_CLONE,true);
    } else {
        if (!is_tabletMode) {
            m_ScreenModeDbus->call("setScreenMode", "extendScreenMode", "ukcc");
        }
        xrandrGSettings->set(XRANDR_CLONE,false);
    }
}

bool DisplayWidget::writeInit(QString group, QString key, bool value)
{
    if(group.isEmpty() || key.isEmpty())
    {
        return false;
    }else
    {
        //创建配置文件操作对象,Iniformat存储数据到INI文件中
        QSettings config(m_initPath, QSettings::IniFormat);

        //将信息写入配置文件
        config.beginGroup(group);
        config.setValue(key, value);
        config.endGroup();

        return true;
    }
}

void DisplayWidget::apply() {
    save();
    if (oriApply) {
        QTimer::singleShot(1, this,
            [this] () {
               save();
            }
        );
    }
}

bool DisplayWidget::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::Resize) {
        if (mOutputIdentifiers.contains(qobject_cast<QQuickView*>(object))) {
            QResizeEvent *e = static_cast<QResizeEvent*>(event);
            const QRect screenSize = object->property("screenSize").toRect();
            QRect geometry(QPoint(0, 0), e->size());
            geometry.moveCenter(screenSize.center());
            static_cast<QQuickView*>(object)->setGeometry(geometry);
            // Pass the event further
        }
    }
    return QObject::eventFilter(object, event);
}


void DisplayWidget::setConfig(const KScreen::ConfigPtr &config)
{
    if (mConfig) {
        KScreen::ConfigMonitor::instance()->removeConfig(mConfig);
        for (const KScreen::OutputPtr &output : mConfig->outputs()) {
            output->disconnect(this);
        }
        mConfig->disconnect(this);
    }

    mConfig = config;

    KScreen::ConfigMonitor::instance()->addConfig(mConfig);
    resetPrimaryCombo();
    connect(mConfig.data(), &KScreen::Config::outputAdded,
            this, &DisplayWidget::outputAdded);
    connect(mConfig.data(), &KScreen::Config::outputRemoved,
            this, &DisplayWidget::outputRemoved);
    connect(mConfig.data(), &KScreen::Config::primaryOutputChanged,
            this, &DisplayWidget::primaryOutputChanged);

    // 上面屏幕拿取配置
    mScreen->setConfig(mConfig);
    mControlPanel->setConfig(mConfig);
//    ui->unifyButton->setEnabled(mConfig->outputs().count() > 1);
    m_unifybutton->setEnabled(mConfig->outputs().count() > 1);

    //ui->scaleAllOutputsButton->setVisible(!mConfig->supportedFeatures().testFlag(KScreen::Config::Feature::PerOutputScaling));
    for (const KScreen::OutputPtr &output : mConfig->outputs()) {
        outputAdded(output);
    }
    // 选择主屏幕输出
    QMLOutput *qmlOutput = mScreen->primaryOutput();
    mmScreen = mate_rr_screen_new (gdk_screen_get_default (),NULL);
    if(mmScreen == nullptr){
        qDebug()<<"Could not initialize the RANDR plugin";
        return;
    }
    g_signal_connect(mmScreen,"changed", G_CALLBACK (&DisplayWidget::OnRandrEvent), this);

    if (qmlOutput) {
        mScreen->setActiveOutput(qmlOutput);
    } else {
        if (!mScreen->outputs().isEmpty()) {
            mScreen->setActiveOutput(mScreen->outputs().at(0));
            //选择一个主屏幕，避免闪退现象
            primaryButtonEnable();
        }
    }
    slotOutputEnabledChanged();

    if (!unifyChecked) {
        if (isCloneMode()) {
            m_unifybutton->setChecked(true);
            slotUnifyOutputs();
        }
    }
    QDBusReply<bool> tb_mode = m_statusSessionDbus->call("get_current_tabletmode");

    QDBusReply<int> pcmode = m_ScreenModeDbus->call("getScreenMode", "ukcc");

    int enabledOutputsCount = 0;
    Q_FOREACH (const KScreen::OutputPtr &output, mConfig->outputs()) {
        if (output->isEnabled()) {
            ++enabledOutputsCount;
        }
        if (enabledOutputsCount > 1) {
            break;
        }
    }
//    mScreen->setDisplayWidget(this);
    if (tb_mode && enabledOutputsCount > 1) {
        m_unifybutton->setChecked(true);
        m_unifybutton->setEnabled(false);
    } else {
        m_unifybutton->setEnabled(true);
        if (pcmode == cloneScreenMode) {
            m_unifybutton->setChecked(true);
        } else if (pcmode == extendScreenMode) {
            m_unifybutton->setChecked(false);
        }
    }
}
void DisplayWidget::screenRemovedProcess()
{
    //由于setting daemon对插上显示器后一直为扩展模式，所以需要控制面板按钮保持同步
    QMLOutput *base = mScreen->primaryOutput();
    base->setIsCloneMode(false);
    m_unifybutton->setChecked(false);
    const KScreen::OutputPtr newPrimary = mConfig->output(ui->primaryCombo->itemData(0).toInt());
   // qDebug()<<"按下主屏按钮---->"<<newPrimary<<"index ----"<<index<<endl;
    mConfig->setPrimaryOutput(newPrimary);
    Q_EMIT changed();
    Q_EMIT primaryChange();
    apply();
}

void DisplayWidget::screenRemovedTimer()
{
    mScreenRemoveTimer->stop();

}
KScreen::ConfigPtr DisplayWidget::currentConfig() const
{
    return mConfig;
}
bool DisplayWidget::isCloneMode()
{
    MateRRScreen *rr_screen;
    MateRRConfig *rr_config;

    rr_screen = mate_rr_screen_new (gdk_screen_get_default (), NULL); /* NULL-GError */
    if (!rr_screen)
            return false;

    rr_config = mate_rr_config_new_current (rr_screen, NULL);

    return mate_rr_config_get_clone(rr_config);
}
void DisplayWidget::loadQml()
{
    qmlRegisterType<QMLOutput>("org.kde.kscreen", 1, 0, "QMLOutput");
    qmlRegisterType<QMLScreen>("org.kde.kscreen", 1, 0, "QMLScreen");

    qmlRegisterType<KScreen::Output>("org.kde.kscreen", 1, 0, "KScreenOutput");
    qmlRegisterType<KScreen::Edid>("org.kde.kscreen", 1, 0, "KScreenEdid");
    qmlRegisterType<KScreen::Mode>("org.kde.kscreen", 1, 0, "KScreenMode");

    ui->quickWidget->setSource(QUrl("qrc:/qml/main.qml"));

    QQuickItem* rootObject = ui->quickWidget->rootObject();
    mScreen = rootObject->findChild<QMLScreen*>(QStringLiteral("outputView"));
    if (!mScreen) {
        return;
    }
    connect(mScreen, &QMLScreen::focusedOutputChanged,
            this, &DisplayWidget::slotFocusedOutputChanged);
}

void DisplayWidget::resetPrimaryCombo()
{
    //qDebug()<<"resetPrimaryCombo----->"<<endl;
#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)

#else
    bool isPrimaryDisplaySupported = mConfig->supportedFeatures().testFlag(KScreen::Config::Feature::PrimaryDisplay);
    ui->primaryLabel->setVisible(isPrimaryDisplaySupported);
    ui->primaryCombo->setVisible(isPrimaryDisplaySupported);
#endif

    // Don't emit currentIndexChanged when resetting
    bool blocked = ui->primaryCombo->blockSignals(true);
    ui->primaryCombo->clear();
    //ui->primaryCombo->addItem(i18n("无主显示输出"));
    ui->primaryCombo->blockSignals(blocked);

    if (!mConfig) {
        return;
    }
//    int output_num = 0;
    for (auto &output: mConfig->outputs()) {
        addOutputToPrimaryCombo(output);
//        output_num ++;
    }
    //显示器仅有一个时隐藏显示器选择框、统一输出、开启屏幕
//    qDebug()<<"显示器数量："<<ui->primaryCombo->count();
    if(ui->primaryCombo->count() == 1){
        ui->screenframe->setVisible(false);
        ui->showMonitorframe->setVisible(false);
        ui->unionframe->setVisible(false);
        ui->line->setVisible(false);
        ui->line_2->setVisible(false);
        ui->line_3->setVisible(false);
    } else if(ui->primaryCombo->count() > 1) {
        ui->screenframe->setVisible(true);
        //暂时隐藏打开显示器按钮
        ui->showMonitorframe->setVisible(false);
        ui->unionframe->setVisible(true);
        ui->line->setVisible(true);
        ui->line_2->setVisible(true);
        ui->line_3->setVisible(true);
    }
}

void DisplayWidget::addOutputToPrimaryCombo(const KScreen::OutputPtr &output)
{
    // 注释后让他显示全部屏幕下拉框
    if (!output->isConnected()) {
        return;
    }
    ui->primaryCombo->addItem(Utils::outputName(output), output->id());
    if (output->isPrimary()) {
        Q_ASSERT(mConfig);
        int lastIndex = ui->primaryCombo->count() - 1;
        ui->primaryCombo->setCurrentIndex(lastIndex);
    }
}

//这里从屏幕点击来读取输出
void DisplayWidget::slotFocusedOutputChanged(QMLOutput *output)
{
    mControlPanel->activateOutput(output->outputPtr());

    //读取屏幕点击选择下拉框
    Q_ASSERT(mConfig);
    int index = output->outputPtr().isNull() ? 0 : ui->primaryCombo->findData(output->outputPtr()->id());
    if (index == -1 || index == ui->primaryCombo->currentIndex()) {
        return;
    }
    //qDebug()<<"下拉框id----->"<<index<<endl;
    ui->primaryCombo->setCurrentIndex(index);
}

void DisplayWidget::slotFocusedOutputChangedNoParam()
{
    //qDebug()<<"slotFocusedOutputChangedNoParam-------->"<<res<<endl;
    mControlPanel->activateOutput(res);
}


void DisplayWidget::slotOutputEnabledChanged()
{
    // 这里为点击禁用屏幕输出后的改变
    resetPrimaryCombo();
    int enabledOutputsCount = 0;
    Q_FOREACH (const KScreen::OutputPtr &output, mConfig->outputs()) {
        if (output->isEnabled()) {
            ++enabledOutputsCount;
        }
        if (enabledOutputsCount > 1) {
            break;
        }
    }
    m_unifybutton->setEnabled(enabledOutputsCount > 1);
    ui->unionframe->setVisible(enabledOutputsCount > 1);
}

void DisplayWidget::slotOutputConnectedChanged()
{
    resetPrimaryCombo();
}

void DisplayWidget::slotUnifyOutputs()
{
    QMLOutput *base = mScreen->primaryOutput();
    //qDebug()<<"primaryOutput---->"<<base<<endl;
    QList<int> clones;

    if (!base) {
        for (QMLOutput *output: mScreen->outputs()) {
            if (output->output()->isConnected() && output->output()->isEnabled()) {
                base = output;
                break;
            }
        }
        if (!base) {
            // WTF?
            return;
        }
    }

    if (base->isCloneMode() && !m_unifybutton->isChecked()) {
        qDebug()<<"取消clone------------>"<<endl;

        QPoint secPoint;
        KScreen::OutputList screens =  mPrevConfig->connectedOutputs();

        QMap<int, KScreen::OutputPtr>::iterator it = screens.begin();
        while (it != screens.end()) {

            KScreen::OutputPtr screen= it.value();
//            qDebug()<<"screens is-------->"<<screen<<endl;
            if (screen->isPrimary()) {

                secPoint = QPoint(screen->size().width(),0);
            }
            it++;
        }

        QMap<int, KScreen::OutputPtr>::iterator secIt = screens.begin();
        while (secIt != screens.end()) {
            KScreen::OutputPtr screen= secIt.value();
            qDebug()<<"screens is-------->"<<screen<<endl;
            if (!screen->isPrimary()) {
                screen->setPos(secPoint);
            }
            secIt++;
        }

        setConfig(mPrevConfig);
        mPrevConfig.clear();
        ui->primaryCombo->setEnabled(true);
        //开启开关
//        ui->checkBox->setEnabled(true);
//        closeScreenButton->setEnabled(true);
        ui->primaryCombo->setEnabled(true);
//        ui->unifyButton->setText(tr("统一输出"));
    } else if (!base->isCloneMode() && m_unifybutton->isChecked()){
        // Clone the current config, so that we can restore it in case user
        // breaks the cloning
        qDebug()<<"点击统一输出---->"<<endl;
        //保存之前的配置
        mPrevConfig = mConfig->clone();

        for (QMLOutput *output: mScreen->outputs()) {

            if (!output->output()->isConnected()) {
                continue;
            }

            if (!output->output()->isEnabled()) {

                output->setVisible(false);
                continue;
            }

            if (!base) {
                base = output;
            }

            output->setOutputX(0);
            output->setOutputY(0);
            output->output()->setPos(QPoint(0, 0));
//            output->output()->setClones(QList<int>());

            if (base != output) {
                clones << output->output()->id();
                output->setCloneOf(base);
                //镜像模式后外显隐藏
                output->setVisible(false);
            }
        }

        base->output()->setClones(clones);
        base->setIsCloneMode(true);

        mScreen->updateOutputsPlacement();


        //关闭开关
        closeScreenButton->setEnabled(false);
        ui->primaryCombo->setEnabled(false);
        ui->mainScreenButton->setEnabled(false);

        //qDebug()<<"输出---->"<<base->outputPtr()<<endl;
        mControlPanel->setUnifiedOutput(base->outputPtr());
    }
    Q_EMIT changed();
    apply();
//    MateRRConfig *rr_config;
//    bool isClone = false;
//    if (!mmScreen) {
//        return;
//    }
//     rr_config = mate_rr_config_new_current (mmScreen, NULL);
//     isClone =  mate_rr_config_get_clone(rr_config);
//     while (isClone != m_unifybutton->isChecked()) {
//         apply();
//     }
//     if (isClone && !m_unifybutton->isChecked()) {
//         m_unifybutton->setChecked(true);
//     } else if (!isClone && m_unifybutton->isChecked()) {
//         m_unifybutton->setChecked(false);
//     }
    //改为xrandr+mate的方法应用镜像模式
//    applyConifg();
}

void DisplayWidget::applyConifg()
{
    MateRRConfig *config;
    MateRROutputInfo **outputs;
    MateRROutput *rr_output;


    //QString secondName;
    QString runCommand;
    // QString primaryName;
    guint32 timestamp;
    char *monitorName = NULL;
    char *secondScreenName;
    int screenCount = 0;

    int primaryWidth;
    int primaryHeight;

    int secondWidth;
    int secondHeight;
    QSettings settings(m_initPath, QSettings::IniFormat);
    int begin_x = settings.value("qmlOutput_Position/start_x").toInt();
//    int begin_y = settings.value("qmlOutput_Position/start_y").toInt();
//    mmScreen = mate_rr_screen_new (gdk_screen_get_default (),NULL);
    config = mate_rr_config_new_current (mmScreen, NULL);
    outputs = mate_rr_config_get_outputs (config);

    for (int i = 0; outputs[i] != NULL; i++) {
        MateRROutputInfo *output = outputs[i];
        rr_output = mate_rr_screen_get_output_by_name (mmScreen, mate_rr_output_info_get_name (output));

        if (mate_rr_output_info_is_active(output) && mate_rr_output_get_is_primary(rr_output)) {
            primaryWidth = mate_rr_output_info_get_preferred_width(output);
            primaryHeight = mate_rr_output_info_get_preferred_height(output);
            monitorName = mate_rr_output_info_get_name(output);

        } else if (mate_rr_output_info_is_connected(output) && !mate_rr_output_get_is_primary(rr_output)) {
            if (NULL == monitorName) {
                //sometime can't find primary screen..so must save a value to monitor name
                monitorName = mate_rr_output_info_get_name(output);
            }
            secondScreenName = mate_rr_output_info_get_name(output);
            secondWidth = mate_rr_output_info_get_preferred_width(output);
            secondHeight = mate_rr_output_info_get_preferred_height(output);
        }
        if (mate_rr_output_info_is_connected (output)) {
            screenCount++;
        }
    }
    char cmd[128]="";
    if (m_unifybutton->isChecked()) {
        mate_rr_config_set_clone(config,true);
        sprintf(cmd,"xrandr --output  %s  --same-as %s --auto --mode %dx%d", secondScreenName, monitorName, primaryWidth, primaryHeight);
    } else {
        if (begin_x > 360) {
            mate_rr_config_set_clone(config,false);
            sprintf(cmd,"xrandr --output  %s  --right-of %s --auto --mode %dx%d", secondScreenName, monitorName, secondWidth, secondHeight);
            qDebug()<<"在右边";
        } else {
            mate_rr_config_set_clone(config,false);
            sprintf(cmd,"xrandr --output  %s  --left-of %s --auto --mode %dx%d", secondScreenName, monitorName, secondWidth, secondHeight);
            qDebug()<<"在左边";
        }
    }
    mate_rr_screen_get_timestamps (mmScreen, NULL, &timestamp);
    mate_rr_config_apply_with_time (config, mmScreen, timestamp, NULL);
    QProcess::execute(cmd);
    g_object_unref (config);
}

// FIXME: Copy-pasted from KDED's Serializer::findOutput()
KScreen::OutputPtr DisplayWidget::findOutput(const KScreen::ConfigPtr &config, const QVariantMap &info)
{
    KScreen::OutputList outputs = config->outputs();
    Q_FOREACH(const KScreen::OutputPtr &output, outputs) {
        if (!output->isConnected()) {
            continue;
        }

        const QString outputId = (output->edid() && output->edid()->isValid()) ? output->edid()->hash() : output->name();
        if (outputId != info[QStringLiteral("id")].toString()) {
            continue;
        }

        QVariantMap posInfo = info[QStringLiteral("pos")].toMap();
        QPoint point(posInfo[QStringLiteral("x")].toInt(), posInfo[QStringLiteral("y")].toInt());
        output->setPos(point);
        output->setPrimary(info[QStringLiteral("primary")].toBool());
        output->setEnabled(info[QStringLiteral("enabled")].toBool());
        output->setRotation(static_cast<KScreen::Output::Rotation>(info[QStringLiteral("rotation")].toInt()));

        QVariantMap modeInfo = info[QStringLiteral("mode")].toMap();
        QVariantMap modeSize = modeInfo[QStringLiteral("size")].toMap();
        QSize size(modeSize[QStringLiteral("width")].toInt(), modeSize[QStringLiteral("height")].toInt());

        const KScreen::ModeList modes = output->modes();
        Q_FOREACH(const KScreen::ModePtr &mode, modes) {
            if (mode->size() != size) {
                continue;
            }
            if (QString::number(mode->refreshRate()) != modeInfo[QStringLiteral("refresh")].toString()) {
                continue;
            }

            output->setCurrentModeId(mode->id());
            break;
        }
        return output;
    }

    return KScreen::OutputPtr();
}

void DisplayWidget::writeScale(int scale) {
    if (isScaleChanged) {
        KMessageBox::information(this,tr("Some applications need to be logouted to take effect"));
    }
    isScaleChanged = false;
    int cursize;
    QGSettings * dpiSettings;
    QGSettings * cursorSettings;
    QGSettings * fontSettings;
    QByteArray id(FONT_RENDERING_DPI);
    QByteArray iid(MOUSE_SIZE_SCHEMAS);
    QByteArray iiid(DPI_SCHEMAS);
    if (QGSettings::isSchemaInstalled(FONT_RENDERING_DPI) && QGSettings::isSchemaInstalled(MOUSE_SIZE_SCHEMAS)
            && QGSettings::isSchemaInstalled(DPI_SCHEMAS)) {
        dpiSettings = new QGSettings(id);
        cursorSettings = new QGSettings(iid);
        fontSettings = new QGSettings(iiid);

        if (1 == scale)  {
            cursize = 24;
        } else if (2 == scale) {
            cursize = 48;
        } else if (3 == scale) {
            cursize = 96;
        } else {
            scale = 1;
            cursize = 24;
        }

        QStringList keys = dpiSettings->keys();
        if (keys.contains("scalingFactor")) {
            fontSettings->set(DPI_KEY, 96);
            dpiSettings->set(SCALE_KEY, scale);
        }
        cursorSettings->set(CURSOR_SIZE_KEY, cursize);

        delete dpiSettings;
        delete cursorSettings;
    }
}


void DisplayWidget::initGSettings() {
    QByteArray id(UKUI_CONTORLCENTER_PANEL_SCHEMAS);
    if(QGSettings::isSchemaInstalled(id)) {
        m_gsettings = new QGSettings(id)        ;
    }

    m_statusSessionDbus = new QDBusInterface("com.kylin.statusmanager.interface",
                                               "/",
                                               "com.kylin.statusmanager.interface",
                                               QDBusConnection::sessionBus(), this);

    m_ScreenModeDbus = new QDBusInterface("org.ukui.SettingsDaemon",
                                           "/org/ukui/SettingsDaemon/xrandr",
                                           "org.ukui.SettingsDaemon.xrandr",
                                           QDBusConnection::sessionBus(),this);
}
void DisplayWidget::slotChangeAutoBrightness(QString key)
{
    if (key == "autoBrightness") {
//        bool autobristatus = m_brilight->get(AUTO_BRIGHTINESS_KEY).toBool();
        bool autobristatus = false;
        if (m_brilight && m_brilight->keys().contains("autoBrightness")) {
            autobristatus = m_brilight->get(AUTO_BRIGHTINESS_KEY).toBool();
        }
        lightButton->setChecked(autobristatus);
    }
}
void DisplayWidget::showlightWidget(bool judge) {
    if (m_brilight && m_brilight->keys().contains("autoBrightness")) {
        m_brilight->set(AUTO_BRIGHTINESS_KEY,judge);
    }
}

void DisplayWidget::showNightWidget(bool judge) {

    if (judge) {
        ui->widget->hide();
        ui->sunframe->setVisible(true);
        ui->alldayframe->setVisible(true);
      //  ui->temptwidget->setVisible(true);
        ui->temptframe->setVisible(true);
        ui->line_5->setVisible(true);
        ui->line_6->setVisible(true);
        ui->line_9->setVisible(true);
        ui->line_10->setVisible(true);
        ui->widget->show();
    } else {
        ui->widget->hide();
        ui->sunframe->setVisible(false);
        ui->alldayframe->setVisible(false);

       // ui->temptwidget->setVisible(false);
        ui->temptframe->setVisible(false);
        ui->line_5->setVisible(false);
        ui->line_6->setVisible(false);
        ui->line_9->setVisible(false);
        ui->line_10->setVisible(false);
        ui->widget->show();
    }

   if (judge) {
       showCustomWiget(SUN);
   }
   nightModeApply();

}

void DisplayWidget::showCustomWiget(int index) {
    if (SUN == index || ALL == index) {
        ui->widget->hide();
        ui->line_7->setVisible(false);
        ui->widget->show();
    }
}

void DisplayWidget::clearOutputIdentifiers()
{
    mOutputTimer->stop();
    qDeleteAll(mOutputIdentifiers);
    mOutputIdentifiers.clear();
}

void DisplayWidget::outputAdded(const KScreen::OutputPtr &output)
{
    connect(output.data(), &KScreen::Output::isConnectedChanged,
            this, &DisplayWidget::slotOutputConnectedChanged);
    connect(output.data(), &KScreen::Output::isEnabledChanged,
            this, &DisplayWidget::slotOutputEnabledChanged);
    connect(output.data(), &KScreen::Output::posChanged,
            this, &DisplayWidget::changed);

    addOutputToPrimaryCombo(output);
}

void DisplayWidget::outputRemoved(int outputId)
{
    KScreen::OutputPtr output = mConfig->output(outputId);
    if (!output.isNull()) {
        output->disconnect(this);
    }

    const int index = ui->primaryCombo->findData(outputId);
    if (index == -1) {
        return;
    }

    if (index == ui->primaryCombo->currentIndex()) {
        // We'll get the actual primary update signal eventually
        // Don't emit currentIndexChanged
        const bool blocked = ui->primaryCombo->blockSignals(true);
        ui->primaryCombo->setCurrentIndex(0);
        ui->primaryCombo->blockSignals(blocked);
    }
    ui->primaryCombo->removeItem(index);
}

void DisplayWidget::primaryOutputSelected(int index) {
    //qDebug()<<"选中主显示器--->"<<index<<endl;
    if (!mConfig) {
        return;
    }

    const KScreen::OutputPtr newPrimary = index == 0 ? KScreen::OutputPtr() : mConfig->output(ui->primaryCombo->itemData(index).toInt());
    if (newPrimary == mConfig->primaryOutput()) {
        return;
    }

    mConfig->setPrimaryOutput(newPrimary);
    Q_EMIT changed();
}

//主输出
void DisplayWidget::primaryOutputChanged(const KScreen::OutputPtr &output) {
    Q_ASSERT(mConfig);
    int index = output.isNull() ? 0 : ui->primaryCombo->findData(output->id());
    if (index == -1 || index == ui->primaryCombo->currentIndex()) {
        return;
    }
    ui->primaryCombo->setCurrentIndex(index);
}

void DisplayWidget::slotIdentifyButtonClicked(bool checked)
{
    Q_UNUSED(checked);
    connect(new KScreen::GetConfigOperation(), &KScreen::GetConfigOperation::finished,
            this, &DisplayWidget::slotIdentifyOutputs);
}

void DisplayWidget::slotIdentifyOutputs(KScreen::ConfigOperation *op)
{
    if (op->hasError()) {
        return;
    }

    const KScreen::ConfigPtr config = qobject_cast<KScreen::GetConfigOperation*>(op)->config();

    const QString qmlPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral(QML_PATH "OutputIdentifier.qml"));

    mOutputTimer->stop();
    clearOutputIdentifiers();

    /* Obtain the current active configuration from KScreen */
    Q_FOREACH (const KScreen::OutputPtr &output, config->outputs()) {
        if (!output->isConnected() || !output->currentMode()) {
            continue;
        }

        const KScreen::ModePtr mode = output->currentMode();

        QQuickView *view = new QQuickView();

        view->setFlags(Qt::X11BypassWindowManagerHint | Qt::FramelessWindowHint);
        view->setResizeMode(QQuickView::SizeViewToRootObject);
        view->setSource(QUrl::fromLocalFile(qmlPath));
        view->installEventFilter(this);

        QQuickItem *rootObj = view->rootObject();
        if (!rootObj) {
            qWarning() << "Failed to obtain root item";
            continue;
        }

        QSize deviceSize, logicalSize;
        if (output->isHorizontal()) {
            deviceSize = mode->size();
        } else {
            deviceSize = QSize(mode->size().height(), mode->size().width());
        }

#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)
#else
        if (config->supportedFeatures() & KScreen::Config::Feature::PerOutputScaling) {
            // no scale adjustment needed on Wayland
            logicalSize = deviceSize;
        } else {
            logicalSize = deviceSize / devicePixelRatioF();
        }
#endif

//        rootObj->setProperty("outputName", Utils::outputName(output));
        rootObj->setProperty("modeName", Utils::sizeToString(deviceSize));

#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)
        view->setProperty("screenSize", QRect(output->pos(), deviceSize));
#else
        view->setProperty("screenSize", QRect(output->pos(), logicalSize));
#endif

        mOutputIdentifiers << view;
    }

    for (QQuickView *view: mOutputIdentifiers) {
        view->show();
    }

    mOutputTimer->start(2500);
}


void DisplayWidget::save()
{
    if (!this) {
        return;
    }

    const KScreen::ConfigPtr &config = this->currentConfig();
    const int countOutput = config->connectedOutputs().count();

    bool atLeastOneEnabledOutput = false;
    int connectedScreen = 0;
    Q_FOREACH(const KScreen::OutputPtr &output, config->outputs()) {
        if (output->isEnabled()) {
            atLeastOneEnabledOutput = true;
        }
        if (!output->isConnected())
            continue;

        QMLOutput *base = mScreen->primaryOutput();
        if (!base) {
            for (QMLOutput *output: mScreen->outputs()) {
                if (output->output()->isConnected() && output->output()->isEnabled()) {
                    base = output;
                    break;
                }
            }

            if (!base) {
                // WTF?
                return;
            }
        }
    }
    if (!atLeastOneEnabledOutput ) {
        qDebug()<<"atLeastOneEnabledOutput---->"<<connectedScreen<<endl;
//        KMessageBox::error(this,tr("please insure at least one output!"),
//                           tr("Warning"),KMessageBox::Notify);
        WarningDialog * warningDialog = new WarningDialog(tr("please insure at least one output!"));
        warningDialog->exec();
//        closeScreenButton->setChecked(true);
        return ;
    }
//    int scale = static_cast<int>(this->scaleRet());
//    initScreenXml(countOutput);
#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
#else
    writeScale(static_cast<float>(this->screenScale));
#endif

    if (!KScreen::Config::canBeApplied(config)) {
        KMessageBox::information(this,
                                      tr("Sorry, your configuration could not be applied.\nCommon reasons are that the overall screen size is too big, or you enabled more displays than supported by your GPU."),
                                      tr("@title:window", "Unsupported Configuration"));
        return;
    }
    m_blockChanges = true;
    /* Store the current config, apply settings */
    auto *op = new KScreen::SetConfigOperation(config);
    /* Block until the operation is completed, otherwise KCMShell will terminate
     * before we get to execute the Operation */
    op->exec();

    // The 1000ms is a bit "random" here, it's what works on the systems I've tested, but ultimately, this is a hack
    // due to the fact that we just can't be sure when xrandr is done changing things, 1000 doesn't seem to get in the way
    QTimer::singleShot(1000, this,
        [this] () {
            m_blockChanges = false;
        }
    );
    mPrevConfig = config->clone();
    writeScreenXml();
}
void DisplayWidget::nightModeApply() {
    bool nightmode = m_colorSettings->get(NIGHT_ENABLE_KEY).toBool();
    bool sunjudge = m_colorSettings->get(AUTO_KEY).toBool();
    bool dayjudge = false;
    if (m_colorSettings->keys().contains("nightLightAllday")) {
        dayjudge = m_colorSettings->get(AllDAY_KEY).toBool();
    }
    if(nightmode)
    {
        if(sunjudge)
        {
            ui->sunradioBtn->setChecked(true);
            ui->alldayradioBtn->setChecked(false);
            qDebug()<<"跟随日出日落";
        } else if(dayjudge) {
            ui->alldayradioBtn->setChecked(true);
            ui->sunradioBtn->setChecked(false);
        }
    }
    if (m_gsettings) {
        m_gsettings->set(NIGHT_MODE_KEY, nightButton->isChecked());
    }
    if (m_colorSettings) {
        m_colorSettings->set(NIGHT_ENABLE_KEY, nightButton->isChecked());
    }
}

void DisplayWidget::scaleChangedSlot(int index) {
    qDebug()<<"scale changed----------->"<<index<<endl;
    switch (index) {
    case 0:
        this->screenScale = 1;
        break;
    case 1:
        this->screenScale = 2;
        break;
    case 2:
        this->screenScale = 3;
        break;
    default:
        this->screenScale = 1;
        break;
    }
    isScaleChanged = true;
    apply();
}


//是否禁用主屏按钮
void DisplayWidget::mainScreenButtonSelect(int index){
    //qDebug()<<"index is----->"<<index<<" "<<mConfig->primaryOutput()<<endl;
    if (!mConfig) {
        return;
    }

    const KScreen::OutputPtr newPrimary = mConfig->output(ui->primaryCombo->itemData(index).toInt());
    //qDebug()<<"newPrimary----->"<<newPrimary<<" "<< index<<endl;
    if (newPrimary == mConfig->primaryOutput()) {
        ui->mainScreenButton->setEnabled(false);
    }else{
        ui->mainScreenButton->setEnabled(true);
    }
    // 设置是否勾选
//    closeScreenButton->setEnabled(true);
//   ui->checkBox->setChecked(newPrimary->isEnabled());
//    closeScreenButton->setChecked(newPrimary->isEnabled());
   mControlPanel->activateOutput(newPrimary);
}


//设置主屏按钮
void DisplayWidget::primaryButtonEnable(){
    if (!mConfig) {
        return;
    }
    int index  = ui->primaryCombo->currentIndex();
    ui->mainScreenButton->setEnabled(false);
    const KScreen::OutputPtr newPrimary = mConfig->output(ui->primaryCombo->itemData(index).toInt());
   // qDebug()<<"按下主屏按钮---->"<<newPrimary<<"index ----"<<index<<endl;
    mConfig->setPrimaryOutput(newPrimary);

    Q_EMIT changed();
    Q_EMIT primaryChange();
    apply();
}

void DisplayWidget::checkOutputScreen(bool judge){
   int index  = ui->primaryCombo->currentIndex();
   const KScreen::OutputPtr newPrimary = mConfig->output(ui->primaryCombo->itemData(index).toInt());

   KScreen::OutputPtr  mainScreen=  mConfig->primaryOutput();
   if (!mainScreen) {
       mConfig->setPrimaryOutput(newPrimary);
   }
   newPrimary->setEnabled(judge);

   int enabledOutput = 0;
   Q_FOREACH(KScreen::OutputPtr outptr , mConfig->outputs()) {
       if (outptr->isEnabled()) {
           enabledOutput++;
       }

       if (enabledOutput >= 2) {
           // 设置副屏在主屏右边
           if (!mainScreen){
               newPrimary->setPos(QPoint(mainScreen->pos().x() + mainScreen->size().width(),
                                         mainScreen->pos().y()));
           } else{
               continue;
           }
       }
   }

   ui->primaryCombo->setCurrentIndex(index);
   Q_EMIT changed();
   apply();
}

//亮度调节UI
void DisplayWidget::initBrightnessUI(){
    //亮度调节
//    ui->brightnesswidget->setStyleSheet("background-color:#F4F4F4;border-radius:6px");

    ui->brightnessSlider->setRange(10,100);
    ui->brightnessSlider->setTracking(true);

    QString screenName = getScreenName();

    setBrightnesSldierValue(screenName);

    connect(ui->brightnessSlider,&QSlider::valueChanged,
            this,&DisplayWidget::setBrightnessScreen);

    connect(ui->primaryCombo, &QComboBox::currentTextChanged,
            this, &DisplayWidget::setBrightnesSldierValue);

    if (QGSettings::isSchemaInstalled(POWER_SCHMES)) {
        connect(powerSettings, &QGSettings::changed, this, [ = ] {
            int value = powerSettings->get(POWER_KEY).toInt();
            ui->brightnessSlider->setValue(value);
        });
    }
}


QString DisplayWidget::getScreenName(QString screenname){
    if("" == screenname )
        screenname = ui->primaryCombo->currentText();
    int startPos = screenname.indexOf('(');
    int endPos = screenname.indexOf(')');
    return screenname.mid(startPos+1,endPos-startPos-1);
}

void DisplayWidget::setBrightnessScreen(int index){
    qDebug()<<index;
//    QGSettings *powerSettings;
    if (QGSettings::isSchemaInstalled(POWER_SCHMES)) {
//        QByteArray id(POWER_SCHMES);
//        powerSettings = new QGSettings(id);
        QStringList keys = powerSettings->keys();
        if (keys.contains("brightnessAc")) {
            powerSettings->set(POWER_KEY, index);
        }
    }
//    if (!powerSettings) {
//        delete powerSettings;
//    }
}

//滑块改变
void DisplayWidget::setBrightnesSldierValue(QString name){
    Q_UNUSED(name)
    int value = 99;
    if (QGSettings::isSchemaInstalled(POWER_SCHMES)) {
//        QByteArray id(POWER_SCHMES);
//        powerSettings = new QGSettings(id);
        QStringList keys = powerSettings->keys();
        if (keys.contains("brightnessAc")) {
            value = powerSettings->get(POWER_KEY).toInt();
        }
    }
//    if (!powerSettings) {
//        delete powerSettings;
//    }
    ui->brightnessSlider->setValue(value);
}

void DisplayWidget::initTemptSlider() {
    ui->temptSlider->setRange(4500,6500);
    ui->temptSlider->setTracking(true);
}

void DisplayWidget::initConfigFile() {
    QByteArray id(SETTINGS_DAEMON_COLOR_SCHEMAS);
    if(QGSettings::isSchemaInstalled(id)) {
        m_colorSettings = new QGSettings(id)        ;
    } else {
        return ;
    }
    double optime = m_colorSettings->get(NIGHT_FROM_KEY).toDouble();
    double cltime = m_colorSettings->get(NIGHT_TO_KEY).toDouble();
    double autooptime = 0;
    double autocltime = 0;
    if (m_colorSettings->keys().contains("nightLightScheduleAutomaticFrom")) {
        autooptime = m_colorSettings->get(AUTO_NIGHT_FROM_KEY).toDouble();
        autocltime = m_colorSettings->get(AUTO_NIGHT_TO_KEY).toDouble();
    }

    int ophour, opmin, clhour, clmin;
    value_to_hour_minute(optime, &ophour, &opmin);
    value_to_hour_minute(cltime, &clhour, &clmin);
    opHour = ophour;
    opMin = opmin;
    clsHour = clhour;
    clsMin = clmin;

    int autoophour, autoopmin, autoclhour, autoclmin;
    value_to_hour_minute(autooptime, &autoophour, &autoopmin);
    value_to_hour_minute(autocltime, &autoclhour, &autoclmin);
    autoopHour = autoophour;
    autoopMin = autoopmin;
    autoclsHour = autoclhour;
    autoclsMin = autoclmin;

    QString atophour,atopmin,atclhour,atclmin;
    if(autoopHour < 10){
        atophour = QString("0%1").arg(QString::number(autoopHour));
    } else {
        atophour = QString::number(autoopHour);
    }
    if(autoopMin < 10){
        atopmin = QString("0%1").arg(QString::number(autoopMin));
    } else {
        atopmin = QString::number(autoopMin);
    }
    if(autoclsHour < 10){
        atclhour = QString("0%1").arg(QString::number(autoclsHour));
    } else {
        atclhour = QString::number(autoclsHour);
    }
    if(autoclsMin < 10){
        atclmin = QString("0%1").arg(QString::number(autoclsMin));
    } else {
        atclmin = QString::number(autoclsMin);
    }
    qDebug()<<atophour<<atopmin<<atclhour<<atclmin;
    ////~ contents_path /display/follow the sunrise and sunset
    QString autotime = tr("follow the sunrise and sunset");
    autotime = autotime + "("+atophour +":"+atopmin + "--" + atclhour + ":" + atclmin + ")";
    //~ contents_path /display/Auto Switch
    ui->label_6->setText(tr("Auto Switch"));


    int value = m_colorSettings->get(NIGHT_TEMPERATURE_KEY).toInt();
    ui->temptSlider->setValue(4500 + 6500 - value);

    bool nightmode = m_colorSettings->get(NIGHT_ENABLE_KEY).toBool();
    bool sunjudge = m_colorSettings->get(AUTO_KEY).toBool();
//    bool dayjudge = m_colorSettings->get(AllDAY_KEY).toBool();
    bool dayjudge = false;
    if (m_colorSettings->keys().contains("nightLightAllday")) {
        dayjudge = m_colorSettings->get(AllDAY_KEY).toBool();
    }
    qDebug()<<"dayjudge"<<dayjudge;
    nightButton->setChecked(nightmode);
    if(nightmode)
    {
        if(sunjudge)
        {
            ui->sunradioBtn->setChecked(true);
            ui->alldayradioBtn->setChecked(false);
        } else if(dayjudge) {
            ui->alldayradioBtn->setChecked(true);
            ui->sunradioBtn->setChecked(false);
        }
    }

    connect(ui->temptSlider, &QSlider::valueChanged, [=](){
        int color_temperature = ui->temptSlider->value();
        m_colorSettings->set(NIGHT_TEMPERATURE_KEY, 4500 + 6500 - color_temperature);
    });

    connect(m_gsettings, &QGSettings::changed, [=](const QString &key){
        if(key == NIGHT_MODE_KEY)
        {
            bool nightmode = m_gsettings->get(NIGHT_MODE_KEY).toBool();
            bool sunjudge = m_colorSettings->get(AUTO_KEY).toBool();
//            bool dayjudge = m_colorSettings->get(AllDAY_KEY).toBool();
            bool dayjudge = false;
            if (m_colorSettings->keys().contains("nightLightAllday")) {
                dayjudge = m_colorSettings->get(AllDAY_KEY).toBool();
            }
            m_colorSettings->set(NIGHT_ENABLE_KEY, nightmode);

            nightButton->setChecked(nightmode);
            if(nightmode)
            {
                if(sunjudge)
                {
                    ui->sunradioBtn->setChecked(sunjudge);
                    ui->alldayradioBtn->setChecked(false);
                    showCustomWiget(SUN);
                } else if(dayjudge) {
                    ui->alldayradioBtn->setChecked(dayjudge);
                    ui->sunradioBtn->setChecked(false);
                    showCustomWiget(SUN);
                }
            }
        }
    });

    connect(m_colorSettings, &QGSettings::changed, [=](const QString &key){
        if(key == "nightLightTemperature")
        {
            int value = m_colorSettings->get(NIGHT_TEMPERATURE_KEY).toInt();
            ui->temptSlider->setValue(4500 + 6500 - value);
        }
        else if(key == "nightLightScheduleAutomatic" || key == "nightLightEnabled" || key == "nightLightAllday")
        {
            bool nightmode = m_colorSettings->get(NIGHT_ENABLE_KEY).toBool();
            bool sunjudge = m_colorSettings->get(AUTO_KEY).toBool();
            bool dayjudge = m_colorSettings->get(AllDAY_KEY).toBool();
//            bool dayjudge = false;
//            if (m_colorSettings->keys().contains(AllDAY_KEY)) {
//                dayjudge = m_colorSettings->get(AllDAY_KEY).toBool();
//            }

            nightButton->setChecked(nightmode);
            if(nightmode)
            {
                if(sunjudge)
                {
                    ui->sunradioBtn->setChecked(sunjudge);
                    ui->alldayradioBtn->setChecked(false);
                    showCustomWiget(SUN);
                } else if(dayjudge) {
                    ui->alldayradioBtn->setChecked(dayjudge);
                    ui->sunradioBtn->setChecked(false);
                    showCustomWiget(SUN);
                }
            }
        }
    });
}

void DisplayWidget::writeScreenXml(){
    MateRRScreen *rr_screen;
    MateRRConfig *rr_config;

    /* Normally, mate_rr_config_save() creates a backup file based on the
     * old monitors.xml.  However, if *that* file didn't exist, there is
     * nothing from which to create a backup.  So, here we'll save the
     * current/unchanged configuration and then let our caller call
     * mate_rr_config_save() again with the new/changed configuration, so
     * that there *will* be a backup file in the end.
     */

    rr_screen = mate_rr_screen_new (gdk_screen_get_default (), NULL); /* NULL-GError */
    if (!rr_screen)
            return;

    rr_config = mate_rr_config_new_current (rr_screen, NULL);
    mate_rr_config_save (rr_config, NULL); /* NULL-GError */

    char *backup_filename = mate_rr_config_get_backup_filename();
    unlink(backup_filename);

    g_object_unref (rr_config);
    g_object_unref (rr_screen);
}

void DisplayWidget::initUiComponent() {
    singleButton = new QButtonGroup();
    singleButton->addButton(ui->sunradioBtn);
    singleButton->addButton(ui->alldayradioBtn);

    singleButton->setId(ui->sunradioBtn, SUN);
    singleButton->setId(ui->alldayradioBtn, ALL);

    connect(ui->sunradioBtn, SIGNAL(clicked()), this, SLOT(onRadioClickButton()));
    connect(ui->alldayradioBtn, SIGNAL(clicked()), this, SLOT(onRadioClickButton()));

    showNightWidget(nightButton->isChecked());
    //ubuntukylin youker DBus interface
    QDBusInterface *brightnessInterface = new QDBusInterface("org.freedesktop.UPower",
                                     "/org/freedesktop/UPower/devices/DisplayDevice",
                                     "org.freedesktop.DBus.Properties",
                                     QDBusConnection::systemBus());
    if (!brightnessInterface->isValid()) {
        qDebug() << "Create UPower Interface Failed : " << QDBusConnection::systemBus().lastError();
        return;
    }

    QDBusReply<QVariant> briginfo;
    briginfo  = brightnessInterface ->call("Get", "org.freedesktop.UPower.Device", "PowerSupply");
    if (!briginfo.isValid()) {
        qDebug()<<"brightness info is invalid"<<endl;
        ui->brightnessframe->setVisible(false);
    } else {
        bool status = briginfo.value().toBool();
        ui->brightnessframe->setVisible(status);
    }
}

void DisplayWidget::onRadioClickButton()
{
    if (ui->sunradioBtn->isChecked()) {
        m_colorSettings->set(AUTO_KEY,true);
//        m_colorSettings->set(AllDAY_KEY,false);
        if (m_colorSettings->keys().contains("nightLightAllday")) {
            m_colorSettings->set(AllDAY_KEY,false);
        }
    }
    if (ui->alldayradioBtn->isChecked()) {
        m_colorSettings->set(AUTO_KEY,false);
//        m_colorSettings->set(AllDAY_KEY,true);
        if (m_colorSettings->keys().contains("nightLightAllday")) {
            m_colorSettings->set(AllDAY_KEY,true);
        }
    }
}
