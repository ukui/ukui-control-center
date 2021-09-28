#include "widget.h"
#include "controlpanel.h"
#include "declarative/qmloutput.h"
#include "declarative/qmlscreen.h"
#include "utils.h"
#include "ui_display.h"
#include "displayperformancedialog.h"
#include "colorinfo.h"
#include "scalesize.h"
#include "../../../shell/utils/utils.h"
#include "../../../shell/mainwindow.h"

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
#include <QMessageBox>
#include <QDBusConnection>
#include <QJsonDocument>
#include <QtConcurrent>
#include <QVariantMap>
#include <QDBusMetaType>

#include <KF5/KScreen/kscreen/output.h>
#include <KF5/KScreen/kscreen/edid.h>
#include <KF5/KScreen/kscreen/mode.h>
#include <KF5/KScreen/kscreen/config.h>
#include <KF5/KScreen/kscreen/getconfigoperation.h>
#include <KF5/KScreen/kscreen/configmonitor.h>
#include <KF5/KScreen/kscreen/setconfigoperation.h>
#include <KF5/KScreen/kscreen/edid.h>
#include <KF5/KScreen/kscreen/types.h>


extern "C" {
#include <math.h>
}

#ifdef signals
#undef signals
#endif

#define QML_PATH "kcm_kscreen/qml/"

#define UKUI_CONTORLCENTER_PANEL_SCHEMAS "org.ukui.control-center.panel.plugins"
#define THEME_NIGHT_KEY                  "themebynight"

#define FONT_RENDERING_DPI               "org.ukui.SettingsDaemon.plugins.xsettings"
#define SCALE_KEY                        "scaling-factor"

#define SETTINGS_DAEMON_COLOR_SCHEMAS    "org.ukui.SettingsDaemon.plugins.color"
#define AUTO_KEY                         "night-light-schedule-automatic"
#define AllDAY_KEY                       "night-light-allday"
#define NIGHT_ENABLE_KEY                 "night-light-enabled"
#define NIGHT_FROM_KEY                   "night-light-schedule-from"
#define NIGHT_TO_KEY                     "night-light-schedule-to"
#define AUTO_NIGHT_FROM_KEY              "night-light-schedule-automatic-from"
#define AUTO_NIGHT_TO_KEY                "night-light-schedule-automatic-to"
#define NIGHT_TEMPERATURE_KEY            "night-light-temperature"

#define MOUSE_SIZE_SCHEMAS               "org.ukui.peripherals-mouse"
#define CURSOR_SIZE_KEY                  "cursor-size"

#define POWER_SCHMES                     "org.ukui.power-manager"
#define POWER_KEY                        "brightness-ac"

#define ADVANCED_SCHEMAS                 "org.ukui.session.required-components"
#define ADVANCED_KEY                     "windowmanager"

const QString kCpu = "ZHAOXIN";
const QString kLoong = "Loongson";
const QString tempDayBrig = "6500";

void value_to_hour_minute(double value, int *hour, int *minute)
{
    double hours;
    double mins = 0.f;
    /* display the right thing for AM/PM */
    mins = modf (value, &hours) * 60.f;
    *hour = int(hours);
    *minute = int(mins);
}

double hour_minute_to_value(int hour, int minute) {
    double value = (double)minute/60;
    return (double)hour + value;
}

Q_DECLARE_METATYPE(KScreen::OutputPtr)

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DisplayWindow())
{
    qRegisterMetaType<QQuickView *>();

    ui->setupUi(this);
    initNightModeUi();
    ui->quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    ui->quickWidget->setContentsMargins(0, 0, 0, 9);

    qDBusRegisterMetaType<ScreenConfig>();
    firstAddOutputName = "";
    initComponent();
    setHideModuleInfo();
    initNightUI();

    initAdvanceScreen();

    setTitleLabel();
    initGSettings();
    setNightComponent();
    initUiComponent();
    initNightStatus();

#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)
    mNightModeLabel->hide();
    mNightModeFrame->setVisible(false);
#else
    mNightModeFrame->setVisible(this->mRedshiftIsValid);
    mNightModeLabel->setVisible(this->mRedshiftIsValid);
#endif

    initConnection();
    loadQml();

    mScreenScale = scaleGSettings->get(SCALE_KEY).toDouble();
}

Widget::~Widget()
{
    clearOutputIdentifiers();
    delete ui;
    ui = nullptr;
}

bool Widget::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::Resize) {
        if (mOutputIdentifiers.contains(qobject_cast<QQuickView *>(object))) {
            QResizeEvent *e = static_cast<QResizeEvent *>(event);
            const QRect screenSize = object->property("screenSize").toRect();
            QRect geometry(QPoint(0, 0), e->size());
            geometry.moveCenter(screenSize.center());
            static_cast<QQuickView *>(object)->setGeometry(geometry);
            // Pass the event further
        }
    }
    return QObject::eventFilter(object, event);
}

void Widget::setConfig(const KScreen::ConfigPtr &config, bool showBrightnessFrameFlag)
{
    if (mConfig) {
        KScreen::ConfigMonitor::instance()->removeConfig(mConfig);
        for (const KScreen::OutputPtr &output : mConfig->outputs()) {
            output->disconnect(this);
        }
        mConfig->disconnect(this);
    }
    mConfig = config;
    mPrevConfig = config->clone();

    KScreen::ConfigMonitor::instance()->addConfig(mConfig);
    resetPrimaryCombo();
    changescale();
    connect(mConfig.data(), &KScreen::Config::outputAdded,
            this, [=](const KScreen::OutputPtr &output){
        outputAdded(output, false);
    });
    connect(mConfig.data(), &KScreen::Config::outputRemoved,
            this, [=](int outputId){
        outputRemoved(outputId, false);
    });

    for (const KScreen::OutputPtr &output : mConfig->outputs()) {
        if (output->isConnected()) {
            connect(output.data(), &KScreen::Output::currentModeIdChanged,
                    this, [=]() {
                if (output->currentMode()) {
                    if (ui->scaleCombo) {
                        changescale();
                    }
                }
            });
        }

    }

    connect(mConfig.data(), &KScreen::Config::primaryOutputChanged,
            this, &Widget::primaryOutputChanged);

    // 上面屏幕拿取配置
    mScreen->setConfig(mConfig);
    mControlPanel->setConfig(mConfig);
    mUnifyButton->setEnabled(mConfig->connectedOutputs().count() > 1);
    ui->unionframe->setVisible(false);

    for (const KScreen::OutputPtr &output : mConfig->outputs()) {
        if (false == unifySetconfig) {
            outputAdded(output, false);
        } else { //解决统一输出之后connect信号不再触发的问题
            connect(output.data(), &KScreen::Output::isConnectedChanged,
                this, &Widget::slotOutputConnectedChanged);
            connect(output.data(), &KScreen::Output::isEnabledChanged,
                this, &Widget::slotOutputEnabledChanged);
        }
    }
   unifySetconfig = false;
    // 择主屏幕输出
    QMLOutput *qmlOutput = mScreen->primaryOutput();

    if (qmlOutput) {
        mScreen->setActiveOutput(qmlOutput);
    } else {
        if (!mScreen->outputs().isEmpty()) {
            mScreen->setActiveOutput(mScreen->outputs().at(0));
            // 择一个主屏幕，避免闪退现象
            primaryButtonEnable(true);
        }
    }
    slotOutputEnabledChanged();

    if (mFirstLoad) {
        if (isCloneMode()) {
            mUnifyButton->blockSignals(true);
            mUnifyButton->setChecked(true);
            mUnifyButton->blockSignals(false);
            slotUnifyOutputs();
        }
        setMulScreenVisiable();
    }
    mFirstLoad = false;

    if (showBrightnessFrameFlag == true) {
        showBrightnessFrame();   //初始化的时候，显示
    }
}

KScreen::ConfigPtr Widget::currentConfig() const
{
    return mConfig;
}

void Widget::initNightModeUi()
{
    ui->verticalLayout->setSpacing(8);
    mNightModeLabel = new TitleLabel(this);
    mNightModeLabel->setText(tr("Night Mode"));

    mNightModeFrame = new QFrame(this);
    mNightModeFrame->setMinimumSize(550, 0);
    mNightModeFrame->setMaximumSize(16777215, 16777215);
    mNightModeFrame->setFrameShape(QFrame::Box);

    QVBoxLayout *mNightModeLyt = new QVBoxLayout(mNightModeFrame);
    mNightModeLyt->setSpacing(0);
    mNightModeLyt->setContentsMargins(0, 0, 0, 0);

    /* Open */
    mOpenFrame = new QFrame(mNightModeFrame);
    mOpenFrame->setMinimumSize(550, 60);
    mOpenFrame->setMaximumSize(16777215, 60);
    mOpenFrame->setFrameShape(QFrame::NoFrame);
    QHBoxLayout *mOpenLyt = new QHBoxLayout(mOpenFrame);
    mOpenLyt->setSpacing(0);
    mOpenLabel = new QLabel(mOpenFrame);
    mOpenLabel->setFixedWidth(200);
    mOpenLabel->setText(tr("Open"));
    mNightModeBtn = new SwitchButton(mOpenFrame);
    mOpenLyt->addWidget(mOpenLabel);
    mOpenLyt->addStretch();
    mOpenLyt->addWidget(mNightModeBtn);

    /* Time Mode */
    mTimeModeFrame = new QFrame(mNightModeFrame);
    mTimeModeFrame->setMinimumSize(550, 60);
    mTimeModeFrame->setMaximumSize(16777215, 60);
    mTimeModeFrame->setFrameShape(QFrame::NoFrame);
    QHBoxLayout *mTimeModeLyt = new QHBoxLayout(mTimeModeFrame);
    mTimeModeLyt->setSpacing(8);
    mTimeModeLabel = new QLabel(mTimeModeFrame);
    mTimeModeLabel->setFixedWidth(200);
    mTimeModeLabel->setText(tr("Time"));
    mTimeModeCombox = new QComboBox(mTimeModeFrame);
    mTimeModeCombox->setMinimumWidth(0);
    mTimeModeCombox->setMaximumWidth(16777215);
    mTimeModeLyt->addWidget(mTimeModeLabel);
    mTimeModeLyt->addWidget(mTimeModeCombox);

    /* Custom Time */
    mCustomTimeFrame = new QFrame(mNightModeFrame);
    mCustomTimeFrame->setMinimumSize(550, 60);
    mCustomTimeFrame->setMaximumSize(16777215, 60);
    mCustomTimeFrame->setFrameShape(QFrame::NoFrame);
    QHBoxLayout *mCustomTimeLyt = new QHBoxLayout(mCustomTimeFrame);
    mCustomTimeLyt->setSpacing(8);
    mCustomTimeLabel = new QLabel(mCustomTimeFrame);
    mCustomTimeLabel->setFixedWidth(350);
    mCustomTimeLabel->setText(tr("Custom Time"));
    mOpenTimeHCombox = new QComboBox(mCustomTimeFrame);
    mOpenTimeHCombox->setFixedWidth(64);
    mQpenTimeMCombox = new QComboBox(mCustomTimeFrame);
    mQpenTimeMCombox->setFixedWidth(64);
    mCloseTimeHCombox = new QComboBox(mCustomTimeFrame);
    mCloseTimeHCombox->setFixedWidth(64);
    mCloseTimeMCombox = new QComboBox(mCustomTimeFrame);
    mCloseTimeMCombox->setFixedWidth(64);
    mLabel_1 = new QLabel(mCustomTimeFrame);
    mLabel_1->setFixedWidth(20);
    mLabel_1->setText(tr("to"));
    QLabel *mLabel_2 = new QLabel(mCustomTimeFrame);
    mLabel_2->setFixedWidth(4);
    mLabel_2->setText(":");
    QLabel *mLabel_3 = new QLabel(mCustomTimeFrame);
    mLabel_3->setFixedWidth(4);
    mLabel_3->setText(":");
    mCustomTimeLyt->addWidget(mCustomTimeLabel);
    mCustomTimeLyt->addStretch();
    mCustomTimeLyt->addWidget(mOpenTimeHCombox);
    mCustomTimeLyt->addWidget(mLabel_2);
    mCustomTimeLyt->addWidget(mQpenTimeMCombox);
    mCustomTimeLyt->addWidget(mLabel_1);
    mCustomTimeLyt->addWidget(mCloseTimeHCombox);
    mCustomTimeLyt->addWidget(mLabel_3);
    mCustomTimeLyt->addWidget(mCloseTimeMCombox);

    /* Color Temperature */
    mTemptFrame = new QFrame(mNightModeFrame);
    mTemptFrame->setMinimumSize(550, 60);
    mTemptFrame->setMaximumSize(16777215, 60);
    mTemptFrame->setFrameShape(QFrame::NoFrame);
    QHBoxLayout *mTemptLyt = new QHBoxLayout(mTemptFrame);
    mTemptLyt->setSpacing(8);
    mTemptLabel = new FixLabel(mTemptFrame);
    mTemptLabel->setFixedWidth(200);
    mTemptLabel->setText(tr("Color Temperature"));
    mWarmLabel = new QLabel(mTemptFrame);
    mWarmLabel->setFixedWidth(64);
    mWarmLabel->setText(tr("Warmer"));
    mColdLabel = new QLabel(mTemptFrame);
    mColdLabel->setFixedWidth(64);
    mColdLabel->setText(tr("Colder"));
    mColdLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    mTemptSlider = new Uslider(mTemptFrame);
    mTemptSlider->setMinimumWidth(0);
    mTemptSlider->setMinimumWidth(16777215);
    mTemptSlider->setOrientation(Qt::Orientation::Horizontal);
    mTemptLyt->addWidget(mTemptLabel);
    mTemptLyt->addWidget(mWarmLabel);
    mTemptLyt->addWidget(mTemptSlider);
    mTemptLyt->addWidget(mColdLabel);

    line_1 = setLine(mNightModeFrame);
    line_2 = setLine(mNightModeFrame);
    line_3 = setLine(mNightModeFrame);

    mNightModeLyt->addWidget(mOpenFrame);
    mNightModeLyt->addWidget(line_1);
    mNightModeLyt->addWidget(mTimeModeFrame);
    mNightModeLyt->addWidget(line_2);
    mNightModeLyt->addWidget(mCustomTimeFrame);
    mNightModeLyt->addWidget(line_3);
    mNightModeLyt->addWidget(mTemptFrame);

    ui->verticalLayout->addWidget(mNightModeLabel);
    ui->verticalLayout->addWidget(mNightModeFrame);
    ui->sunframe->setVisible(false);
    ui->customframe->setVisible(false);
    ui->temptframe->setVisible(false);
    ui->themeFrame->setVisible(false);
    ui->nightframe->setVisible(false);
    ui->opframe->setVisible(false);
    ui->clsframe->setVisible(false);
}

void Widget::loadQml()
{
    qmlRegisterType<QMLOutput>("org.kde.kscreen", 1, 0, "QMLOutput");
    qmlRegisterType<QMLScreen>("org.kde.kscreen", 1, 0, "QMLScreen");

    qmlRegisterType<KScreen::Output>("org.kde.kscreen", 1, 0, "KScreenOutput");
    qmlRegisterType<KScreen::Edid>("org.kde.kscreen", 1, 0, "KScreenEdid");
    qmlRegisterType<KScreen::Mode>("org.kde.kscreen", 1, 0, "KScreenMode");

    ui->quickWidget->setSource(QUrl("qrc:/qml/main.qml"));

    QQuickItem *rootObject = ui->quickWidget->rootObject();
    mScreen = rootObject->findChild<QMLScreen *>(QStringLiteral("outputView"));

    connect(mScreen, &QMLScreen::released, this, [=]() {
        delayApply();
    });

    if (!mScreen) {
        return;
    }
    connect(mScreen, &QMLScreen::focusedOutputChanged,
            this, &Widget::slotFocusedOutputChanged);
}

void Widget::resetPrimaryCombo()
{
    // Don't emit currentIndexChanged when resetting
    bool blocked = ui->primaryCombo->blockSignals(true);
    ui->primaryCombo->clear();
    ui->primaryCombo->blockSignals(blocked);

    if (!mConfig) {
        return;
    }

    for (auto &output: mConfig->outputs()) {
        addOutputToPrimaryCombo(output);
    }
}

void Widget::addOutputToPrimaryCombo(const KScreen::OutputPtr &output)
{
    // 注释后让他显示全部屏幕下拉框
    if (!output->isConnected()) {
        return;
    }

    ui->primaryCombo->addItem(Utils::outputName(output), output->id());
    if (output->isPrimary() && !mIsWayland) {
        Q_ASSERT(mConfig);
        int lastIndex = ui->primaryCombo->count() - 1;
        ui->primaryCombo->setCurrentIndex(lastIndex);
    }
}

// 这里从屏幕点击来读取输出
void Widget::slotFocusedOutputChanged(QMLOutput *output)
{
    mControlPanel->activateOutput(output->outputPtr());

    // 读取屏幕点击选择下拉框
    Q_ASSERT(mConfig);
    int index = output->outputPtr().isNull() ? 0 : ui->primaryCombo->findData(output->outputPtr()->id());
    if (index == -1 || index == ui->primaryCombo->currentIndex()) {
        return;
    }
    ui->primaryCombo->setCurrentIndex(index);
}

void Widget::slotFocusedOutputChangedNoParam()
{
    mControlPanel->activateOutput(res);
}

void Widget::slotOutputEnabledChanged()
{
    // 点击禁用屏幕输出后的改变
    resetPrimaryCombo();
    setActiveScreen(mKDSCfg);
    int enabledOutputsCount = 0;
    Q_FOREACH (const KScreen::OutputPtr &output, mConfig->outputs()) {
        if (output->isEnabled()) {
            ++enabledOutputsCount;
            for (int i = 0; i < BrightnessFrameV.size(); ++i) {
                if (BrightnessFrameV[i]->getOutputName() == Utils::outputName(output) && !BrightnessFrameV[i]->getSliderEnable()) {
                    BrightnessFrameV[i]->runConnectThread(true);
                }
            }
        }
        if (enabledOutputsCount > 1) {
            break;
        }
    }
    mUnifyButton->setEnabled(enabledOutputsCount > 1);
    ui->unionframe->setVisible(false);
}

void Widget::slotOutputConnectedChanged()
{
    const KScreen::OutputPtr output(qobject_cast<KScreen::Output *>(sender()), [](void *){});

    if (output->isConnected()) {
        outputAdded(output, true);
    } else {
        outputRemoved(output->id(), true);
    }

    resetPrimaryCombo();

    setMulScreenVisiable();
}

void Widget::slotUnifyOutputs()
{
    QMLOutput *base = mScreen->primaryOutput();

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
    for (QMLOutput *output: mScreen->outputs()) {
        if (mUnifyButton->isChecked() && output == base) {
            output->setIsCloneMode(true, true);
        } else {
            output->setIsCloneMode(mUnifyButton->isChecked(), false);
        }
    }
    // 取消统一输出
    if (!mUnifyButton->isChecked()) {
        bool isExistCfg = QFile::exists((QDir::homePath() + "/.config/ukui/ukcc-screenPreCfg.json"));
        if (mKDSCfg.isEmpty() && isExistCfg) {
            KScreen::OutputList screens = mPrevConfig->connectedOutputs();
            QList<ScreenConfig> preScreenCfg = getPreScreenCfg();
            int posX = preScreenCfg.at(0).screenPosX;
            bool isOverlap = false;
            for (int i = 1; i< preScreenCfg.count(); i++) {
                if (posX == preScreenCfg.at(i).screenPosX) {
                    isOverlap = true;
                    setScreenKDS("expand");
                    break;
                }
            }

            Q_FOREACH(ScreenConfig cfg, preScreenCfg) {
                Q_FOREACH(KScreen::OutputPtr output, screens) {
                    if (!cfg.screenId.compare(output->name()) && !isOverlap) {
                        output->setCurrentModeId(cfg.screenModeId);
                        output->setPos(QPoint(cfg.screenPosX, cfg.screenPosY));
                    }
                }
            }
        } else {
            setScreenKDS("expand");
        }
        unifySetconfig = true;
        setConfig(mPrevConfig);

        ui->primaryCombo->setEnabled(true);
        mCloseScreenButton->setEnabled(true);
        ui->showMonitorframe->setVisible(true);
        ui->primaryCombo->setEnabled(true);
    } else if (mUnifyButton->isChecked()) {
        // Clone the current config, so that we can restore it in case user
        // breaks the cloning
        mPrevConfig = mConfig->clone();

        if (!mFirstLoad) {
            setPreScreenCfg(mPrevConfig->connectedOutputs());
        }

        for (QMLOutput *output: mScreen->outputs()) {
            if (output != mScreen->primaryOutput()) {
                output->output()->setRotation(mScreen->primaryOutput()->output()->rotation());
            }

            if (!output->output()->isConnected()) {
                continue;
            }

            if (!output->output()->isEnabled()) {
                continue;
            }

            if (!base) {
                base = output;
            }

            output->setOutputX(0);
            output->setOutputY(0);
            output->output()->setPos(QPoint(0, 0));
            output->output()->setClones(QList<int>());

            if (base != output) {
                clones << output->output()->id();
                output->setCloneOf(base);
            }
        }

        base->output()->setClones(clones);
        mScreen->updateOutputsPlacement();

        // 关闭开关
        mCloseScreenButton->setEnabled(false);
        ui->showMonitorframe->setVisible(false);
        ui->primaryCombo->setEnabled(false);
        ui->mainScreenButton->setVisible(false);
        mControlPanel->setUnifiedOutput(base->outputPtr());
    }
}

// FIXME: Copy-pasted from KDED's Serializer::findOutput()
KScreen::OutputPtr Widget::findOutput(const KScreen::ConfigPtr &config, const QVariantMap &info)
{
    KScreen::OutputList outputs = config->outputs();
    Q_FOREACH (const KScreen::OutputPtr &output, outputs) {
        if (!output->isConnected()) {
            continue;
        }

        const QString outputId
            = (output->edid()
               && output->edid()->isValid()) ? output->edid()->hash() : output->name();
        if (outputId != info[QStringLiteral("id")].toString()) {
            continue;
        }

        QVariantMap posInfo = info[QStringLiteral("pos")].toMap();
        QPoint point(posInfo[QStringLiteral("x")].toInt(), posInfo[QStringLiteral("y")].toInt());
        output->setPos(point);
        output->setPrimary(info[QStringLiteral("primary")].toBool());
        output->setEnabled(info[QStringLiteral("enabled")].toBool());
        output->setRotation(static_cast<KScreen::Output::Rotation>(info[QStringLiteral("rotation")].
                                                                   toInt()));

        QVariantMap modeInfo = info[QStringLiteral("mode")].toMap();
        QVariantMap modeSize = modeInfo[QStringLiteral("size")].toMap();
        QSize size(modeSize[QStringLiteral("width")].toInt(),
                   modeSize[QStringLiteral("height")].toInt());

        const KScreen::ModeList modes = output->modes();
        Q_FOREACH (const KScreen::ModePtr &mode, modes) {
            if (mode->size() != size) {
                continue;
            }
            if (QString::number(mode->refreshRate())
                != modeInfo[QStringLiteral("refresh")].toString()) {
                continue;
            }

            output->setCurrentModeId(mode->id());
            break;
        }
        return output;
    }

    return KScreen::OutputPtr();
}

void Widget::initComponent()
{
    mCloseScreenButton = new SwitchButton(this);
    ui->showScreenLayout->addWidget(mCloseScreenButton);

    mUnifyButton = new SwitchButton(this);
    ui->unionLayout->addWidget(mUnifyButton);

    mMultiScreenFrame = new QFrame(this);
    mMultiScreenFrame->setFrameShape(QFrame::Shape::Box);

    QHBoxLayout *multiScreenlay = new QHBoxLayout();
    mMultiScreenLabel = new QLabel(tr("Multi-screen"), this);
    mMultiScreenLabel->setFixedSize(118, 30);

    mMultiScreenCombox = new QComboBox(this);

    mMultiScreenCombox->addItem(tr("First Screen"));
    mMultiScreenCombox->addItem(tr("Clone Screen"));
    mMultiScreenCombox->addItem(tr("Extend Screen"));
    mMultiScreenCombox->addItem(tr("Vice Screen"));

    multiScreenlay->addWidget(mMultiScreenLabel);
    multiScreenlay->addWidget(mMultiScreenCombox);

    mMultiScreenFrame->setLayout(multiScreenlay);
    ui->multiscreenLyt->addWidget(mMultiScreenFrame);
}

void Widget::setHideModuleInfo()
{
    mCPU = getCpuInfo();
    if (!mCPU.startsWith(kCpu, Qt::CaseInsensitive)) {
        ui->quickWidget->setAttribute(Qt::WA_AlwaysStackOnTop);
        ui->quickWidget->setClearColor(Qt::transparent);
    }
}

void Widget::setTitleLabel()
{
    ui->primaryLabel->setText(tr("monitor"));
}

void Widget::writeScale(double scale)
{
    if (scale != scaleGSettings->get(SCALE_KEY).toDouble()) {
        mIsScaleChanged = true;
    }

    if (mIsScaleChanged) {
        if (!mIsChange) {
            QMessageBox::information(this, tr("Information"),
                                     tr("Some applications need to be logouted to take effect"));
        } else {
            // 非主动切换缩放率，则不弹提示弹窗
            mIsChange = false;
        }
    } else {
        return;
    }


    mIsScaleChanged = false;
    int cursize;
    QByteArray iid(MOUSE_SIZE_SCHEMAS);
    if (QGSettings::isSchemaInstalled(MOUSE_SIZE_SCHEMAS)) {
        QGSettings cursorSettings(iid);

        if (1.0 == scale) {
            cursize = 24;
        } else if (2.0 == scale) {
            cursize = 48;
        } else if (3.0 == scale) {
            cursize = 96;
        } else {
            cursize = 24;
        }

        QStringList keys = scaleGSettings->keys();
        if (keys.contains("scalingFactor")) {

            scaleGSettings->set(SCALE_KEY, scale);
        }
        cursorSettings.set(CURSOR_SIZE_KEY, cursize);
        Utils::setKwinMouseSize(cursize);
    }
}

void Widget::initGSettings()
{
    QByteArray id(UKUI_CONTORLCENTER_PANEL_SCHEMAS);
    if (QGSettings::isSchemaInstalled(id)) {
        mGsettings = new QGSettings(id, QByteArray(), this);
        if (mGsettings->keys().contains(THEME_NIGHT_KEY)) {
            mThemeButton->setChecked(mGsettings->get(THEME_NIGHT_KEY).toBool());
        }
    } else {
        qDebug() << Q_FUNC_INFO << "org.ukui.control-center.panel.plugins not install";
    }

    QByteArray nightId(SETTINGS_DAEMON_COLOR_SCHEMAS);
    if(QGSettings::isSchemaInstalled(nightId)) {
        m_colorSettings = new QGSettings(nightId);
        // 暂时解决点击夜间模式闪退问题
       //m_colorSettings = nullptr;

        if (m_colorSettings) {
            connect(m_colorSettings, &QGSettings::changed, [=](const QString &key){
                if(key == "nightLightTemperature")
                {
                    int value = m_colorSettings->get(NIGHT_TEMPERATURE_KEY).toInt();
                    mTemptSlider->setValue(value);
                }
                else if(key == "nightLightScheduleAutomatic" || key == "nightLightEnabled" || key == "nightLightAllday") {
                    setNightModeSetting();
                }
            });
        }

    } else {
        qDebug() << Q_FUNC_INFO << "org.ukui.SettingsDaemon.plugins.color not install";
    }

    QByteArray scaleId(FONT_RENDERING_DPI);
    if (QGSettings::isSchemaInstalled(scaleId)) {
        scaleGSettings = new QGSettings(scaleId, QByteArray(), this);
    }
}

void Widget::setcomBoxScale()
{
    int scale = 1;
    QComboBox *scaleCombox = findChild<QComboBox *>(QString("scaleCombox"));
    if (scaleCombox) {
        scale = ("100%" == scaleCombox->currentText() ? 1 : 2);
    }
    writeScale(scale);
}

void Widget::initNightUI()
{
    ui->unifyLabel->setText(tr("unify output"));

    QHBoxLayout *nightLayout = new QHBoxLayout(ui->nightframe);
    //~ contents_path /display/night mode
    nightLabel = new QLabel(tr("night mode"), this);
    mNightButton = new SwitchButton(this);
    nightLayout->addWidget(nightLabel);
    nightLayout->addStretch();
    nightLayout->addWidget(mNightButton);

    QHBoxLayout *themeLayout = new QHBoxLayout(ui->themeFrame);
    mThemeButton = new SwitchButton(this);
    themeLayout->addWidget(new QLabel(tr("Theme follow night mode")));
    themeLayout->addStretch();
    themeLayout->addWidget(mThemeButton);
}

QFrame *Widget::setLine(QFrame *frame)
{
    QFrame *line = new QFrame(frame);
    line->setMinimumSize(QSize(0, 1));
    line->setMaximumSize(QSize(16777215, 1));
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    return line;
}

void Widget::initAdvanceScreen()
{
    QProcess *process = new QProcess;
    process->start("lsb_release -r");
    process->waitForFinished();

    QByteArray ba = process->readAllStandardOutput();
    QString osReleaseCrude = QString(ba.data());
    QStringList res = osReleaseCrude.split(":");
    QString osRelease = res.length() >= 2 ? res.at(1) : "";
    osRelease = osRelease.simplified();

    const QByteArray idd(ADVANCED_SCHEMAS);
    if (QGSettings::isSchemaInstalled(idd) && osRelease == "V10") {
        ui->advancedBtn->show();
        ui->advancedHorLayout->setContentsMargins(9, 8, 9, 32);
    } else {
        ui->advancedBtn->hide();
        ui->advancedHorLayout->setContentsMargins(9, 0, 9, 0);
    }
}

bool Widget::isRestoreConfig()
{
    int cnt = 15;
    int ret = -100;
    MainWindow *mainWindow = static_cast<MainWindow*>(this->topLevelWidget());
    QMessageBox msg;
    connect(mainWindow, &MainWindow::posChanged, this, [=,&msg]() {
        QTimer::singleShot(8, this, [=,&msg]() { //窗管会移动窗口，等待8ms,确保在窗管移动之后再move，时间不能太长，否则会看到移动的路径
            QRect rect = this->topLevelWidget()->geometry();
            int msgX = 0, msgY = 0;
            msgX = rect.x() + rect.width()/2 - 380/2;
            msgY = rect.y() + rect.height()/2 - 130/2;
            msg.move(msgX, msgY);
        });
    });
    if (mConfigChanged) {
        msg.setWindowTitle(tr("Hint"));
        msg.setText(tr("After modifying the resolution or refresh rate, "
                       "due to compatibility issues between the display device and the graphics card, "
                       "the display may be abnormal or unable to display\n"
                       "the settings will be saved after 14 seconds"));
        msg.addButton(tr("Save"), QMessageBox::RejectRole);
        msg.addButton(tr("Not Save"), QMessageBox::AcceptRole);

        QTimer cntDown;
        QObject::connect(&cntDown, &QTimer::timeout, [&msg, &cnt, &cntDown, &ret]()->void {
            if (--cnt < 0) {
                cntDown.stop();
                msg.hide();
                msg.close();
            } else {
                msg.setText(QString(tr("After modifying the resolution or refresh rate, "
                                       "due to compatibility issues between the display device and the graphics card, "
                                       "the display may be abnormal or unable to display \n"
                                       "the settings will be saved after %1 seconds")).arg(cnt));
                msg.show();
            }
        });
        cntDown.start(1000);
        QRect rect = this->topLevelWidget()->geometry();
        int msgX = 0, msgY = 0;
        msgX = rect.x() + rect.width()/2 - 380/2;
        msgY = rect.y() + rect.height()/2 - 130/2;
        msg.move(msgX, msgY);
        ret = msg.exec();
    }
    disconnect(mainWindow, &MainWindow::posChanged, 0, 0);
    bool res = false;
    switch (ret) {
    case QMessageBox::AcceptRole:
        res = false;
        break;
    case QMessageBox::RejectRole:
        if (mIsSCaleRes) {
            QStringList keys = scaleGSettings->keys();
            if (keys.contains("scalingFactor")) {
                scaleGSettings->set(SCALE_KEY,scaleres);
            }
            mIsSCaleRes = false;
        }
        res = true;
        break;
    }
    return res;
}

QString Widget::getCpuInfo()
{
    return Utils::getCpuInfo();
}

bool Widget::isCloneMode()
{
    KScreen::OutputPtr output = mConfig->primaryOutput();
    if (!output) {
        return false;
    }
    if (mConfig->connectedOutputs().count() >= 2) {
        foreach (KScreen::OutputPtr secOutput, mConfig->connectedOutputs()) {
            if (secOutput->pos() != output->pos() || !secOutput->isEnabled()) {
                return false;
            }
        }
    } else {
        return false;
    }
    return true;
}

bool Widget::isBacklight()
{
    QString cmd = "ukui-power-backlight-helper --get-max-brightness";
    QProcess process;
    process.start(cmd);
    process.waitForFinished();
    QString result = process.readAllStandardOutput().trimmed();

    QString pattern("^[0-9]*$");
    QRegExp reg(pattern);

    return reg.exactMatch(result);
}

QString Widget::getMonitorType()
{
    QString monitor = ui->primaryCombo->currentText();
    QString type;
    if (monitor.contains("VGA", Qt::CaseInsensitive)) {
        type = "4";
    } else {
        type = "8";
    }
    return type;
}

bool Widget::isLaptopScreen()
{
    int index = ui->primaryCombo->currentIndex();
    KScreen::OutputPtr output = mConfig->output(ui->primaryCombo->itemData(index).toInt());
    if (output->type() == KScreen::Output::Type::Panel) {
        return true;
    }
    return false;
}

bool Widget::isVisibleBrightness()
{
    if ((mIsBattery && isLaptopScreen())
            || (mIsWayland && !mIsBattery)
            || (!mIsWayland && mIsBattery)) {
        return true;
    }
    return false;
}


int Widget::getPrimaryScreenID()
{
    QString primaryScreen = getPrimaryWaylandScreen();
    int screenId;
    for (const KScreen::OutputPtr &output : mConfig->outputs()) {
        if (!output->name().compare(primaryScreen, Qt::CaseInsensitive)) {
            screenId = output->id();
        }
    }
    return screenId;
}

void Widget::setScreenIsApply(bool isApply)
{
    mIsScreenAdd = !isApply;
}

void Widget::setMulScreenVisiable()
{
    bool isMult = mConfig->connectedOutputs().count() >= 2 ? true : false;
    mMultiScreenFrame->setVisible(isMult);
    initMultScreenStatus();
}

void Widget::initMultScreenStatus()
{
    mMultiScreenCombox->blockSignals(true);
    if (isCloneMode()) {
        mMultiScreenCombox->setCurrentIndex(CLONE);
    } else {
        int enableCount = 0;
        KScreen::OutputList screens = mConfig->connectedOutputs();
        Q_FOREACH(KScreen::OutputPtr output, screens) {
            if (output->isEnabled()) {
                enableCount++;
            }
        }
        if (enableCount >= 2) {
            mMultiScreenCombox->setCurrentIndex(EXTEND);
        } else if (screens.begin().value()->isEnabled()) {
            mMultiScreenCombox->setCurrentIndex(FIRST);
        } else {
            mMultiScreenCombox->setCurrentIndex(VICE);
        }
    }
    mMultiScreenCombox->blockSignals(false);
}

void Widget::showNightWidget(bool judge)
{
    mTimeModeFrame->setVisible(judge);
    if (mTimeModeCombox->currentIndex() == 2) {
        mCustomTimeFrame->setVisible(judge);
        line_2->setVisible(judge);
    } else {
        mCustomTimeFrame->setVisible(false);
        line_2->setVisible(false);
    }

    mTemptFrame->setVisible(judge);
    line_1->setVisible(judge);
    line_3->setVisible(judge);
}

void Widget::showCustomWiget(int index)
{
    if (SUN == index) {
        ui->opframe->setVisible(false);
        ui->clsframe->setVisible(false);
    } else if (CUSTOM == index) {
        ui->opframe->setVisible(true);
        ui->clsframe->setVisible(true);
    }
}

void Widget::clearOutputIdentifiers()
{
    mOutputTimer->stop();
    qDeleteAll(mOutputIdentifiers);
    mOutputIdentifiers.clear();
}

void Widget::addBrightnessFrame(QString name, bool openFlag, QString serialNum)
{
    if (mIsBattery && name != firstAddOutputName)  //笔记本非内置
        return;
    for (int i = 0; i < BrightnessFrameV.size(); ++i) {  //已经有了
        if (name == BrightnessFrameV[i]->getOutputName())
            return;
    }
    BrightnessFrame *frame = nullptr;
    if (mIsBattery && name == firstAddOutputName) {
        frame = new BrightnessFrame(name, true, serialNum);
    } else if(!mIsBattery) {
        frame = new BrightnessFrame(name, false, serialNum);
    }
    if (frame != nullptr) {
        BrightnessFrameV.push_back(frame);
        ui->unifyBrightLayout->addWidget(frame);
        frame->runConnectThread(openFlag);
    }

}


void Widget::outputAdded(const KScreen::OutputPtr &output, bool connectChanged)
{
    if (firstAddOutputName == "" && output->isConnected()) {
        firstAddOutputName = Utils::outputName(output);
    }

    if (output->isConnected()) {
        QString name = Utils::outputName(output);
        addBrightnessFrame(name, output->isEnabled(), output->edid()->serial());
    }
    // 刷新缩放选项，监听新增显示屏的mode变化
    changescale();
    if (output->isConnected()) {
        connect(output.data(), &KScreen::Output::currentModeIdChanged,
                this, [=]() {
            if (output->currentMode()) {
                if (ui->scaleCombo) {
                    ui->scaleCombo->blockSignals(true);
                    changescale();
                    ui->scaleCombo->blockSignals(false);
                }
            }
        });
    }
    if (!connectChanged) {
        connect(output.data(), &KScreen::Output::isConnectedChanged,
                this, &Widget::slotOutputConnectedChanged);
        connect(output.data(), &KScreen::Output::isEnabledChanged,
                this, &Widget::slotOutputEnabledChanged);
    }

    addOutputToPrimaryCombo(output);

    if (!mFirstLoad) {
        bool m_isCloneMode = isCloneMode();
        if (m_isCloneMode != mUnifyButton->isChecked())
            mIsScreenAdd = true;
        mUnifyButton->setChecked(m_isCloneMode);
        QTimer::singleShot(2000, this, [=] {
            mainScreenButtonSelect(ui->primaryCombo->currentIndex());
        });
    }

    ui->unionframe->setVisible(false);
    mUnifyButton->setEnabled(mConfig->connectedOutputs().count() > 1);

    showBrightnessFrame();
}

void Widget::outputRemoved(int outputId, bool connectChanged)
{
    // 刷新缩放选项
    changescale();
    if (!connectChanged) {
        KScreen::OutputPtr output = mConfig->output(outputId);
        if (!output.isNull()) {
            output->disconnect(this);
        }
    }
    const int index = ui->primaryCombo->findData(outputId);
    if (index != -1) {
        if (index == ui->primaryCombo->currentIndex()) {
            // We'll get the actual primary update signal eventually
            // Don't emit currentIndexChanged
            const bool blocked = ui->primaryCombo->blockSignals(true);
            ui->primaryCombo->setCurrentIndex(0);
            ui->primaryCombo->blockSignals(blocked);
        }

        ui->primaryCombo->removeItem(index);
    }

    // 检查统一输出-防止移除后没有屏幕可显示
    if (mUnifyButton->isChecked()) {
        for (QMLOutput *qmlOutput: mScreen->outputs()) {
            if (!qmlOutput->output()->isConnected()) {
                continue;
            }
            qmlOutput->setIsCloneMode(false, false);
        }
    }
    ui->unionframe->setVisible(false);
    mUnifyButton->blockSignals(true);
    mUnifyButton->setChecked(mConfig->connectedOutputs().count() > 1);
    mUnifyButton->blockSignals(false);
    mainScreenButtonSelect(ui->primaryCombo->currentIndex());
    showBrightnessFrame();
}

void Widget::primaryOutputSelected(int index)
{
    if (!mConfig) {
        return;
    }

    const KScreen::OutputPtr newPrimary = index == 0 ? KScreen::OutputPtr() : mConfig->output(ui->primaryCombo->itemData(index).toInt());
    if (newPrimary == mConfig->primaryOutput()) {
        return;
    }

    mConfig->setPrimaryOutput(newPrimary);
}

// 主输出
void Widget::primaryOutputChanged(const KScreen::OutputPtr &output)
{
    Q_ASSERT(mConfig);
    int index = output.isNull() ? 0 : ui->primaryCombo->findData(output->id());
    if (index == -1 || index == ui->primaryCombo->currentIndex()) {
        return;
    }
    ui->primaryCombo->setCurrentIndex(index);
}

void Widget::slotIdentifyButtonClicked(bool checked)
{
    Q_UNUSED(checked);
    connect(new KScreen::GetConfigOperation(), &KScreen::GetConfigOperation::finished,
            this, &Widget::slotIdentifyOutputs);
}

void Widget::slotIdentifyOutputs(KScreen::ConfigOperation *op)
{
    if (op->hasError()) {
        return;
    }

    const KScreen::ConfigPtr config = qobject_cast<KScreen::GetConfigOperation *>(op)->config();

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

        rootObj->setProperty("outputName", Utils::outputName(output));
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

void Widget::callMethod(QRect geometry, QString name)
{
    auto scale = 1;
    QDBusInterface waylandIfc("org.ukui.SettingsDaemon",
                              "/org/ukui/SettingsDaemon/wayland",
                              "org.ukui.SettingsDaemon.wayland",
                              QDBusConnection::sessionBus());

    QDBusReply<int> reply = waylandIfc.call("scale");
    if (reply.isValid()) {
        scale = reply.value();
    }

    QDBusMessage message = QDBusMessage::createMethodCall("org.ukui.SettingsDaemon",
                                                          "/org/ukui/SettingsDaemon/wayland",
                                                          "org.ukui.SettingsDaemon.wayland",
                                                          "priScreenChanged");
    message << geometry.x() / scale << geometry.y() / scale << geometry.width() / scale <<
        geometry.height() / scale << name;
    QDBusConnection::sessionBus().send(message);
}

QString Widget::getPrimaryWaylandScreen()
{
    QDBusInterface screenIfc("org.ukui.SettingsDaemon",
                             "/org/ukui/SettingsDaemon/wayland",
                             "org.ukui.SettingsDaemon.wayland",
                             QDBusConnection::sessionBus());
    QDBusReply<QString> screenReply = screenIfc.call("priScreenName");
    if (screenReply.isValid()) {
        return screenReply.value();
    }
    return QString();
}

void Widget::applyNightModeSlot()
{
    if (((mOpenTimeHCombox->currentIndex() < mCloseTimeHCombox->currentIndex())
         || (mOpenTimeHCombox->currentIndex() == mCloseTimeHCombox->currentIndex()
             && mQpenTimeMCombox->currentIndex() <= mCloseTimeMCombox->currentIndex()))
            && mTimeModeCombox->currentIndex() == 2 && mNightModeBtn->isChecked()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("Open time should be earlier than close time!"));
        return;
    }

    setNightMode(mNightModeBtn->isChecked());
}


void Widget::setScreenKDS(QString kdsConfig)
{
    KScreen::OutputList screens = mConfig->connectedOutputs();
    if (kdsConfig == "expand") {
        Q_FOREACH(KScreen::OutputPtr output, screens) {
            if (!output.isNull() && !mUnifyButton->isChecked()) {
                output->setEnabled(true);
                output->setCurrentModeId("0");
            }
        }

        KScreen::OutputList screensPre = mConfig->connectedOutputs();

        KScreen::OutputPtr mainScreen = mConfig->primaryOutput();
        if (!mainScreen.isNull()) {
            mainScreen->setPos(QPoint(0, 0));
        }

        KScreen::OutputPtr preIt = mainScreen;
        QMap<int, KScreen::OutputPtr>::iterator nowIt = screensPre.begin();

        while (nowIt != screensPre.end()) {
            if (nowIt.value() != mainScreen && !nowIt.value().isNull()) {
                nowIt.value()->setPos(QPoint(preIt->pos().x() + preIt->size().width(), 0));
                KScreen::ModeList modes = preIt->modes();
                Q_FOREACH (const KScreen::ModePtr &mode, modes) {
                    if (preIt->currentModeId() == mode->id()) {
                        if (preIt->rotation() != KScreen::Output::Rotation::Left && preIt->rotation() != KScreen::Output::Rotation::Right) {
                            nowIt.value()->setPos(QPoint(preIt->pos().x() + mode->size().width(), 0));
                        } else {
                            nowIt.value()->setPos(QPoint(preIt->pos().x() + mode->size().height(), 0));
                        }
                    }
                }
                preIt = nowIt.value();
            }
            nowIt++;
        }
    } else if (kdsConfig == "first") {
        QMap<int, KScreen::OutputPtr>::iterator nowIt = screens.begin();
        while (nowIt != screens.end()) {
            if (!nowIt.value().isNull()) {
                bool flag = (nowIt.key() == screens.begin().key());
                nowIt.value()->setEnabled(flag);
                nowIt.value()->setPrimary(flag);
            }
            nowIt++;
        }
        delayApply();
    } else if (kdsConfig == "second") {
        QMap<int, KScreen::OutputPtr>::iterator nowIt = screens.begin();
        while (nowIt != screens.end()) {
            if (!nowIt.value().isNull()) {
                bool flag = (nowIt.key() != screens.begin().key());
                nowIt.value()->setEnabled(flag);
                nowIt.value()->setPrimary(flag);
            }
            nowIt++;
        }
        delayApply();
    } else {
        Q_FOREACH(KScreen::OutputPtr output, screens) {
            if (!output.isNull()) {
                output->setEnabled(true);
            }
        }
    }
}

void Widget::setActiveScreen(QString status)
{
    int activeScreenId = 1;
    int enableCount = 0;
    int connectCount = 0;
    Q_FOREACH(const KScreen::OutputPtr &output, mConfig->connectedOutputs()) {
        connectCount++;
        enableCount = (output->isEnabled() ? (++enableCount) : enableCount);
    }

    if (status == "second") {
        activeScreenId = connectCount;
    }

    for (int index = 0; index <= ui->primaryCombo->count(); index++) {
        KScreen::OutputPtr output = mConfig->output(ui->primaryCombo->itemData(index).toInt());
        if (status.isEmpty() && connectCount > enableCount && !output.isNull() && output->isEnabled()) {
            ui->primaryCombo->setCurrentIndex(index);
        }

        if (!status.isEmpty() && !output.isNull() && activeScreenId == output->id()) {
            ui->primaryCombo->setCurrentIndex(index);
        }
    }

}

// 等相关包上传
//通过win+p修改，不存在按钮影响亮度显示的情况，直接就应用了，此时每个屏幕的openFlag是没有修改的，需要单独处理(setScreenKDS)
void Widget::kdsScreenchangeSlot(QString status)
{
    bool isCheck = (status == "copy") ? true : false;
    mKDSCfg = status;
    setScreenKDS(mKDSCfg);
    if (mConfig->connectedOutputs().count() >= 2) {
        mUnifyButton->setChecked(isCheck);
    }
    QTimer::singleShot(1500, this, [=]{ //需要延时
        Q_FOREACH(KScreen::OutputPtr output, mConfig->connectedOutputs()) {
            if (output.isNull())
                continue;
            for (int i = 0; i < BrightnessFrameV.size(); ++i) {
                if (BrightnessFrameV[i]->getOutputName() == Utils::outputName(output)) {
                    BrightnessFrameV[i]->setOutputEnable(output->isEnabled());
                }
            }
        }
        if (true == isCheck ) {
            showBrightnessFrame(1);
        } else {
            showBrightnessFrame(2);
        }
        initMultScreenStatus();
    });
}

void Widget::setMultiScreenSlot(int index)
{
    if (FIRST == index) {
        if (!mUnifyButton->isChecked()) {
            setPreScreenCfg(mConfig->connectedOutputs());
        }
        mUnifyButton->setChecked(false);
        setScreenKDS("first");
    } else if (CLONE == index) {
        setScreenKDS("clone");
        mUnifyButton->setChecked(true);
    } else if (EXTEND == index) {
        setScreenKDS("expand");
        mUnifyButton->setChecked(false);
        delayApply();
    } else {
        if (!mUnifyButton->isChecked()) {
            setPreScreenCfg(mConfig->connectedOutputs());
        }
        mUnifyButton->setChecked(false);
        setScreenKDS("second");
    }
}

void Widget::delayApply()
{
    QTimer::singleShot(200, this, [=]() {
        // kds与插拔不触发应用操作
        if (mKDSCfg.isEmpty() && !mIsScreenAdd) {
            save();
        }
        mKDSCfg.clear();
        mIsScreenAdd = false;
    });
}

void Widget::save()
{
    if (!this) {
        return;
    }

    const KScreen::ConfigPtr &config = this->currentConfig();

    bool atLeastOneEnabledOutput = false;
    Q_FOREACH (const KScreen::OutputPtr &output, config->outputs()) {
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

    if (!atLeastOneEnabledOutput) {
        QMessageBox::warning(this, tr("Warning"), tr("please insure at least one output!"));
        mCloseScreenButton->setChecked(true);
        return;
    }

    if (!KScreen::Config::canBeApplied(config)) {
        QMessageBox::information(this,
                                 tr("Warning"),
                                 tr("Sorry, your configuration could not be applied.\nCommon reasons are that the overall screen size is too big, or you enabled more displays than supported by your GPU."));
        return;
    }
    mBlockChanges = true;
    /* Store the current config, apply settings */
    auto *op = new KScreen::SetConfigOperation(config);

    /* Block until the operation is completed, otherwise KCMShell will terminate
     * before we get to execute the Operation */
    op->exec();

    // The 1000ms is a bit "random" here, it's what works on the systems I've tested, but ultimately, this is a hack
    // due to the fact that we just can't be sure when xrandr is done changing things, 1000 doesn't seem to get in the way
    QTimer::singleShot(1000, this,
                       [=]() {

        QString hash = config->connectedOutputsHash();
        writeFile(mDir % hash);

        mBlockChanges = false;
        mConfigChanged = false;
        setMulScreenVisiable();
    });

    int enableScreenCount = 0;
    KScreen::OutputPtr enableOutput;
    for (const KScreen::OutputPtr &output : mConfig->outputs()) {
        if (output->isEnabled()) {
            enableOutput = output;
            enableScreenCount++;
        }
    }

    if (isRestoreConfig()) {
        auto *op = new KScreen::SetConfigOperation(mPrevConfig);
        op->exec();

    } else {
        mPrevConfig = mConfig->clone();
    }

	for (int i = 0; i < BrightnessFrameV.size(); ++i) {   //应用成功再更新屏幕是否开启的状态，判断亮度条是否打开
        for (KScreen::OutputPtr output : mConfig->outputs()) {
            if (BrightnessFrameV[i]->getOutputName() == Utils::outputName(output)) {
                BrightnessFrameV[i]->setOutputEnable(output->isEnabled());
            }
        }
    }
    int flag = mUnifyButton->isChecked() ? 1 : 2;
    showBrightnessFrame(flag);  //成功应用之后，重新显示亮度条,传入是否统一输出,1表示打开，2表示关闭

}

QVariantMap metadata(const KScreen::OutputPtr &output)
{
    QVariantMap metadata;
    metadata[QStringLiteral("name")] = output->name();
    if (!output->edid() || !output->edid()->isValid()) {
        return metadata;
    }

    metadata[QStringLiteral("fullname")] = output->edid()->deviceId();
    return metadata;
}

QString Widget::globalFileName(const QString &hash)
{
    QString s_dirPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                        %QStringLiteral("/kscreen/");
    QString dir = s_dirPath  % QStringLiteral("outputs/");
    if (!QDir().mkpath(dir)) {
        return QString();
    }
    return QString();
}

QVariantMap Widget::getGlobalData(KScreen::OutputPtr output)
{
    QFile file(globalFileName(output->hashMd5()));
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open file" << file.fileName();
        return QVariantMap();
    }
    QJsonDocument parser;
    return parser.fromJson(file.readAll()).toVariant().toMap();
}

void Widget::writeGlobal(const KScreen::OutputPtr &output)
{
    // get old values and subsequently override
    QVariantMap info = getGlobalData(output);
    if (!writeGlobalPart(output, info, nullptr)) {
        return;
    }
    QFile file(globalFileName(output->hashMd5()));
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open global output file for writing! " << file.errorString();
        return;
    }

    file.write(QJsonDocument::fromVariant(info).toJson());
    return;
}

bool Widget::writeGlobalPart(const KScreen::OutputPtr &output, QVariantMap &info,
                             const KScreen::OutputPtr &fallback)
{
    info[QStringLiteral("id")] = output->hash();
    info[QStringLiteral("metadata")] = metadata(output);
    info[QStringLiteral("rotation")] = output->rotation();

    // Round scale to four digits
    info[QStringLiteral("scale")] = int(output->scale() * 10000 + 0.5) / 10000.;

    QVariantMap modeInfo;
    float refreshRate = -1.;
    QSize modeSize;
    if (output->currentMode() && output->isEnabled()) {
        refreshRate = output->currentMode()->refreshRate();
        modeSize = output->currentMode()->size();
    } else if (fallback && fallback->currentMode()) {
        refreshRate = fallback->currentMode()->refreshRate();
        modeSize = fallback->currentMode()->size();
    }

    if (refreshRate < 0 || !modeSize.isValid()) {
        return false;
    }

    modeInfo[QStringLiteral("refresh")] = refreshRate;

    QVariantMap modeSizeMap;
    modeSizeMap[QStringLiteral("width")] = modeSize.width();
    modeSizeMap[QStringLiteral("height")] = modeSize.height();
    modeInfo[QStringLiteral("size")] = modeSizeMap;

    info[QStringLiteral("mode")] = modeInfo;

    return true;
}

bool Widget::writeFile(const QString &filePath)
{
    const KScreen::OutputList outputs = mConfig->outputs();
    const auto oldConfig = mPrevConfig;
    KScreen::OutputList oldOutputs;
    if (oldConfig) {
        oldOutputs = oldConfig->outputs();
    }
    QVariantList outputList;
    for (const KScreen::OutputPtr &output : outputs) {
        QVariantMap info;
        const auto oldOutputIt = std::find_if(oldOutputs.constBegin(), oldOutputs.constEnd(),
                                              [output](const KScreen::OutputPtr &out) {
            return out->hashMd5() == output->hashMd5();
        });
        const KScreen::OutputPtr oldOutput = oldOutputIt != oldOutputs.constEnd() ? *oldOutputIt
                                             : nullptr;
        if (!output->isConnected()) {
            continue;
        }

        writeGlobalPart(output, info, oldOutput);
        info[QStringLiteral("primary")] = output->isPrimary();
        info[QStringLiteral("enabled")] = output->isEnabled();

        auto setOutputConfigInfo = [&info](const KScreen::OutputPtr &out) {
                                       if (!out) {
                                           return;
                                       }

                                       QVariantMap pos;
                                       pos[QStringLiteral("x")] = out->pos().x();
                                       pos[QStringLiteral("y")] = out->pos().y();
                                       info[QStringLiteral("pos")] = pos;
                                   };
        setOutputConfigInfo(output->isEnabled() ? output : oldOutput);

        if (output->isEnabled()) {
            // try to update global output data
            writeGlobal(output);
        }
        outputList.append(info);
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open config file for writing! " << file.errorString();
        return false;
    }
    file.write(QJsonDocument::fromVariant(outputList).toJson());
    qDebug() << "Config saved on: " << file.fileName();

    return true;
}

void Widget::scaleChangedSlot(double scale)
{
    if (scaleGSettings->get(SCALE_KEY).toDouble() != scale) {
        mIsScaleChanged = true;
    } else {
        mIsScaleChanged = false;
    }

    writeScale(scale);
}

void Widget::changedSlot()
{
    mConfigChanged = true;
}

void Widget::propertiesChangedSlot(QString property, QMap<QString, QVariant> propertyMap,
                                   QStringList propertyList)
{
    Q_UNUSED(property);
    Q_UNUSED(propertyList);
    if (propertyMap.keys().contains("OnBattery")) {
        mOnBattery = propertyMap.value("OnBattery").toBool();
    }
}

// 是否禁用主屏按钮
void Widget::mainScreenButtonSelect(int index)
{
    if (!mConfig || ui->primaryCombo->count() <= 0) {
        return;
    }

    const KScreen::OutputPtr newPrimary = mConfig->output(ui->primaryCombo->itemData(index).toInt());
    int connectCount = mConfig->connectedOutputs().count();

    if (mIsWayland) {
        if (!getPrimaryWaylandScreen().compare(newPrimary->name(), Qt::CaseInsensitive)) {
            ui->mainScreenButton->setEnabled(false);
        } else {
            ui->mainScreenButton->setEnabled(true);
        }
    } else {
        if (newPrimary == mConfig->primaryOutput()) {
            ui->mainScreenButton->setVisible(false);
        } else {
            ui->mainScreenButton->setVisible(true);
        }
    }

    if (!newPrimary->isEnabled()) {
        ui->scaleCombo->setEnabled(false);
    } else {
        ui->scaleCombo->setEnabled(true);
    }

    // 设置是否勾选
    mCloseScreenButton->setEnabled(true);
    ui->showMonitorframe->setVisible(connectCount > 1 && !mUnifyButton->isChecked());

    // 初始化时不要发射信号
    mCloseScreenButton->blockSignals(true);
    mCloseScreenButton->setChecked(newPrimary->isEnabled());
    mCloseScreenButton->blockSignals(false);

    mControlPanel->activateOutput(newPrimary);

    mScreen->setActiveOutputByCombox(newPrimary->id());
}

// 设置主屏按钮
void Widget::primaryButtonEnable(bool status)
{
    Q_UNUSED(status);
    if (!mConfig) {
        return;
    }
    int index = ui->primaryCombo->currentIndex();
    ui->mainScreenButton->setVisible(false);
    const KScreen::OutputPtr newPrimary = mConfig->output(ui->primaryCombo->itemData(index).toInt());
    mConfig->setPrimaryOutput(newPrimary);
}

void Widget::checkOutputScreen(bool judge)
{
    ui->primaryCombo->blockSignals(true);
    int index = ui->primaryCombo->currentIndex();
    KScreen::OutputPtr newPrimary = mConfig->output(ui->primaryCombo->itemData(index).toInt());

    KScreen::OutputPtr mainScreen = mConfig->primaryOutput();

    if (!mainScreen) {
        mConfig->setPrimaryOutput(newPrimary);
    }
    mainScreen = mConfig->primaryOutput();

    newPrimary->setEnabled(judge);

    int enabledOutput = 0;
    Q_FOREACH (KScreen::OutputPtr outptr, mConfig->outputs()) {
        if (outptr->isEnabled()) {
            enabledOutput++;
        }
        if (mainScreen != outptr && outptr->isConnected()) {
            newPrimary = outptr;
        }

        if (enabledOutput >= 2) {
            // 设置副屏在主屏右边
            newPrimary->setPos(QPoint(mainScreen->pos().x() + mainScreen->geometry().width(),
                                      mainScreen->pos().y()));
        }
    }

    ui->primaryCombo->setCurrentIndex(index);
	ui->primaryCombo->blockSignals(false);
}


void Widget::initConnection()
{
    connect(ui->primaryCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &Widget::mainScreenButtonSelect);

    connect(ui->mainScreenButton, &QPushButton::clicked, this, [=](bool status){
        primaryButtonEnable(status);
        delayApply();
    });

    mControlPanel = new ControlPanel(this);

    // Intel隐藏分辨率等调整选项
    if (Utils::isTablet()) {
        mControlPanel->setVisible(false);
    }

    connect(mControlPanel, &ControlPanel::changed, this, &Widget::changed);
    connect(this, &Widget::changed, this, [=]() {
            changedSlot();
            delayApply();
    });
    connect(mControlPanel, &ControlPanel::scaleChanged, this, &Widget::scaleChangedSlot);

    ui->controlPanelLayout->addWidget(mControlPanel);

    connect(ui->advancedBtn, &QPushButton::clicked, this, [=] {
        DisplayPerformanceDialog *dialog = new DisplayPerformanceDialog(this);
        dialog->exec();
    });

    connect(mUnifyButton, &SwitchButton::checkedChanged,
            [this] {
        slotUnifyOutputs();
        /*  bug#54275 避免拔插触发save()
            setScreenIsApply(true);
        */
        delayApply();
		showBrightnessFrame();
    });

    connect(mCloseScreenButton, &SwitchButton::checkedChanged,
            this, [=](bool checked) {
        checkOutputScreen(checked);
        delayApply();
        changescale();
    });

    connect(mOpenTimeHCombox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]{
        if (m_colorSettings) {           
            m_colorSettings->set(NIGHT_FROM_KEY,QString::number(hour_minute_to_value((mOpenTimeHCombox->currentText()).toInt(),(mQpenTimeMCombox->currentText()).toInt()),'f', 2).toDouble());
        } else {
            applyNightModeSlot();
        }
    });

    connect(mQpenTimeMCombox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]{
        if (m_colorSettings) {
            m_colorSettings->set(NIGHT_FROM_KEY,QString::number(hour_minute_to_value((mOpenTimeHCombox->currentText()).toInt(),(mQpenTimeMCombox->currentText()).toInt()),'f', 2).toDouble());
        } else {
            applyNightModeSlot();
        }
    });

    connect(mCloseTimeHCombox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]{
        if (m_colorSettings) {
             m_colorSettings->set(NIGHT_TO_KEY,QString::number(hour_minute_to_value((mCloseTimeHCombox->currentText()).toInt(),(mCloseTimeMCombox->currentText()).toInt()),'f', 2).toDouble());
        } else {
            applyNightModeSlot();
        }
    });

    connect(mCloseTimeMCombox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]{
        if (m_colorSettings) {
            m_colorSettings->set(NIGHT_TO_KEY,QString::number(hour_minute_to_value((mCloseTimeHCombox->currentText()).toInt(),(mCloseTimeMCombox->currentText()).toInt()),'f', 2).toDouble());
        } else {
            applyNightModeSlot();
        }
    });

    connect(mTemptSlider, &QSlider::valueChanged, this, [=]{
        if (m_colorSettings) {
            m_colorSettings->set(NIGHT_TEMPERATURE_KEY,mTemptSlider->value());
        } else {
            applyNightModeSlot();
        }

    });

    connect(mMultiScreenCombox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index){
        setMultiScreenSlot(index);
    });

    QDBusConnection::sessionBus().connect(QString(),
                                              QString("/"),
                                              "org.ukui.ukcc.session.interface",
                                              "screenChanged",
                                              this,
                                              SLOT(kdsScreenchangeSlot(QString)));


    QDBusConnection::sessionBus().connect(QString(),
                                          QString("/ColorCorrect"),
                                          "org.ukui.kwin.ColorCorrect",
                                          "nightColorConfigChanged",
                                          this,
                                          SLOT(nightChangedSlot(QHash<QString,QVariant>)));

    mOutputTimer = new QTimer(this);
    connect(mOutputTimer, &QTimer::timeout,
            this, &Widget::clearOutputIdentifiers);

    mApplyShortcut = new QShortcut(QKeySequence("Ctrl+A"), this);
    connect(mApplyShortcut, SIGNAL(activated()), this, SLOT(save()));

    connect(ui->primaryCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),////////////////
            this, [=](int index) {
       // mainScreenButtonSelect(index);
        showBrightnessFrame();  //当前屏幕框变化的时候，显示，此时不判断
    });

    connect(mTimeModeCombox, QOverload<int>::of(&QComboBox::currentIndexChanged),[=](){
        switch (mTimeModeCombox->currentIndex()) {
        case 0:
            mCustomTimeFrame->hide();
            line_2->hide();
            if (m_colorSettings) {
                m_colorSettings->set(AllDAY_KEY,true);
                m_colorSettings->set(AUTO_KEY,false);
            }
            break;
        case 1:
            mCustomTimeFrame->hide();
            line_2->hide();
            if (m_colorSettings) {
                m_colorSettings->set(AUTO_KEY,true);
                m_colorSettings->set(AllDAY_KEY,false);
            }
            break;
        case 2:
            mCustomTimeFrame->setVisible(true);
            line_2->setVisible(true);
            if (m_colorSettings) {
                m_colorSettings->set(AUTO_KEY,false);
                m_colorSettings->set(AllDAY_KEY,false);
            }
            break;
        }
    });

    connect(mNightModeBtn, &SwitchButton::checkedChanged,[=](bool checked){
        showNightWidget(checked);
        if (m_colorSettings) {
            m_colorSettings->set(NIGHT_ENABLE_KEY,checked);
        } else {
            applyNightModeSlot();
        }

    });

    connect(ui->scaleCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [=](int index){
        scaleChangedSlot(ui->scaleCombo->itemData(index).toDouble());
    });
    connect(scaleGSettings,&QGSettings::changed,this,[=](QString key){
        if (!key.compare("scalingFactor", Qt::CaseSensitive)) {
            double scale = scaleGSettings->get(key).toDouble();
            if (ui->scaleCombo->findData(scale) == -1) {
                scale = 1.0;
            }
            ui->scaleCombo->blockSignals(true);
            ui->scaleCombo->setCurrentText(QString::number(scale * 100) + "%");
            ui->scaleCombo->blockSignals(false);
        }
    });
}


void Widget::setNightComponent()
{
    /* 设置时间模式 */
    mTimeModeStringList << tr("All Day") << tr("Follow the sunrise and sunset(17:55-06:23)") << tr("Custom Time");
    mTimeModeCombox->insertItem(0, mTimeModeStringList.at(0));
    mTimeModeCombox->insertItem(1, mTimeModeStringList.at(1));
    mTimeModeCombox->insertItem(2, mTimeModeStringList.at(2));


    mTemptSlider->setRange(1.1*1000, 6500);
    mTemptSlider->setTracking(true);

    for (int i = 0; i < 24; i++) {
        mOpenTimeHCombox->addItem(QStringLiteral("%1").arg(i, 2, 10, QLatin1Char('0')));
        mCloseTimeHCombox->addItem(QStringLiteral("%1").arg(i, 2, 10, QLatin1Char('0')));
    }

    for (int i = 0; i < 60; i++) {
        mQpenTimeMCombox->addItem(QStringLiteral("%1").arg(i, 2, 10, QLatin1Char('0')));
        mCloseTimeMCombox->addItem(QStringLiteral("%1").arg(i, 2, 10, QLatin1Char('0')));
    }
}


void Widget::setNightMode(const bool nightMode)
{
    QDBusInterface colorIft("org.ukui.KWin",
                            "/ColorCorrect",
                            "org.ukui.kwin.ColorCorrect",
                            QDBusConnection::sessionBus());
    if (!colorIft.isValid()) {
        qWarning() << "create org.ukui.kwin.ColorCorrect failed";
        return;
    }

    if (!nightMode) {
        mNightConfig["Active"] = false;
    } else {
        mNightConfig["Active"] = true;
        if (mTimeModeCombox->currentIndex() == 1) {
            mNightConfig["EveningBeginFixed"] = "17:55:01";
            mNightConfig["MorningBeginFixed"] = "06:23:00";
            mNightConfig["Mode"] = 2;
        } else if (mTimeModeCombox->currentIndex() == 2) {
            mNightConfig["EveningBeginFixed"] = mOpenTimeHCombox->currentText() + ":"
                                                + mQpenTimeMCombox->currentText() + ":00";
            mNightConfig["MorningBeginFixed"] = mCloseTimeHCombox->currentText() + ":"
                                                + mCloseTimeMCombox->currentText() + ":00";
            mNightConfig["Mode"] = 2;
        } else if (mTimeModeCombox->currentIndex() == 0) {
             mNightConfig["Mode"] = 3;
        }
        mNightConfig["NightTemperature"] = mTemptSlider->value();
    }

    colorIft.call("setNightColorConfig", mNightConfig);
}

void Widget::initUiComponent()
{
    mDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
            %QStringLiteral("/kscreen/")
            %QStringLiteral("" /*"configs/"*/);

    singleButton = new QButtonGroup();
    singleButton->addButton(ui->sunradioBtn);
    singleButton->addButton(ui->customradioBtn);

    singleButton->setId(ui->sunradioBtn, SUN);
    singleButton->setId(ui->customradioBtn, CUSTOM);

    MODE value = ui->customradioBtn->isChecked() == SUN ? SUN : CUSTOM;
    showNightWidget(mNightButton->isChecked());
    if (mNightButton->isChecked()) {
        showCustomWiget(value);
    }

    QDBusInterface brightnessInterface("org.freedesktop.UPower",
                                       "/org/freedesktop/UPower/devices/DisplayDevice",
                                       "org.freedesktop.DBus.Properties",
                                       QDBusConnection::systemBus());
    if (!brightnessInterface.isValid()) {
        qDebug() << "Create UPower Interface Failed : " << QDBusConnection::systemBus().lastError();
        return;
    }

    mIsBattery = isBacklight();

    mUPowerInterface = QSharedPointer<QDBusInterface>(
        new QDBusInterface("org.freedesktop.UPower",
                           "/org/freedesktop/UPower",
                           "org.freedesktop.DBus.Properties",
                           QDBusConnection::systemBus()));

    if (!mUPowerInterface.get()->isValid()) {
        qDebug() << "Create UPower Battery Interface Failed : " <<
            QDBusConnection::systemBus().lastError();
        return;
    }

    QDBusReply<QVariant> batteryInfo;
    batteryInfo = mUPowerInterface.get()->call("Get", "org.freedesktop.UPower", "OnBattery");
    if (batteryInfo.isValid()) {
        mOnBattery = batteryInfo.value().toBool();
    }

    mUPowerInterface.get()->connection().connect("org.freedesktop.UPower",
                                                 "/org/freedesktop/UPower",
                                                 "org.freedesktop.DBus.Properties",
                                                 "PropertiesChanged",
                                                 this,
                                                 SLOT(propertiesChangedSlot(QString, QMap<QString,QVariant>, QStringList)));

    mUkccInterface = QSharedPointer<QDBusInterface>(
                new QDBusInterface("org.ukui.ukcc.session",
                                   "/",
                                   "org.ukui.ukcc.session.interface",
                                   QDBusConnection::sessionBus()));
}

void Widget::initNightStatus()
{
    QDBusInterface colorIft("org.ukui.KWin",
                            "/ColorCorrect",
                            "org.ukui.kwin.ColorCorrect",
                            QDBusConnection::sessionBus());
    if (colorIft.isValid() && Utils::isExistEffect() && !mIsWayland) {
        this->mRedshiftIsValid = true;
    } else {
        qWarning() << "create org.ukui.kwin.ColorCorrect failed";
        return;
    }
    if (m_colorSettings) {
        this->mIsNightMode = m_colorSettings->get(NIGHT_ENABLE_KEY).toBool();
        mNightModeBtn->setChecked(this->mIsNightMode);
        showNightWidget(mNightModeBtn->isChecked());
        setNightModeSetting();
        return;
    }

    QDBusMessage result = colorIft.call("nightColorInfo");

    QList<QVariant> outArgs = result.arguments();
    QVariant first = outArgs.at(0);
    QDBusArgument dbvFirst = first.value<QDBusArgument>();
    QVariant vFirst = dbvFirst.asVariant();
    const QDBusArgument &dbusArgs = vFirst.value<QDBusArgument>();

    QVector<ColorInfo> nightColor;

    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        ColorInfo color;
        dbusArgs >> color;
        nightColor.push_back(color);
    }
    dbusArgs.endArray();

    for (ColorInfo it : nightColor) {
        mNightConfig.insert(it.arg, it.out.variant());
    }

    this->mIsNightMode = mNightConfig["Active"].toBool();
    mNightModeBtn->setChecked(this->mIsNightMode);
    showNightWidget(mNightModeBtn->isChecked());
    mTemptSlider->setValue(mNightConfig["CurrentColorTemperature"].toInt());
    if (mNightConfig["EveningBeginFixed"].toString() == "17:55:01" && mNightConfig["Mode"].toInt() == 2) {
        mTimeModeCombox->setCurrentIndex(1);
        mCustomTimeFrame->hide();
        line_2->hide();
    } else if (mNightConfig["Mode"].toInt() == 3) {
        mTimeModeCombox->setCurrentIndex(0);
        mCustomTimeFrame->hide();
        line_2->hide();
    } else {
        mTimeModeCombox->setCurrentIndex(2);
        QString openTime = mNightConfig["EveningBeginFixed"].toString();
        QString ophour = openTime.split(":").at(0);
        QString opmin = openTime.split(":").at(1);

        mOpenTimeHCombox->setCurrentIndex(ophour.toInt());
        mQpenTimeMCombox->setCurrentIndex(opmin.toInt());

        QString cltime = mNightConfig["MorningBeginFixed"].toString();
        QString clhour = cltime.split(":").at(0);
        QString clmin = cltime.split(":").at(1);

        mCloseTimeHCombox->setCurrentIndex(clhour.toInt());
        mCloseTimeMCombox->setCurrentIndex(clmin.toInt());
    }

}

void Widget::setNightModeSetting()
{
    if (!m_colorSettings) {
        applyNightModeSlot();
        return;
    }
    mTimeModeCombox->blockSignals(true);
    if (m_colorSettings->get(NIGHT_ENABLE_KEY).toBool()) {
        mNightModeBtn->setChecked(true);
        if(m_colorSettings->get(AllDAY_KEY).toBool())
        {
            mTimeModeCombox->setCurrentIndex(0);
        } else if(m_colorSettings->get(AUTO_KEY).toBool()) {
            mTimeModeCombox->setCurrentIndex(1);
            double openTime = m_colorSettings->get(AUTO_NIGHT_FROM_KEY).toDouble();
            double cltime = m_colorSettings->get(AUTO_NIGHT_TO_KEY).toDouble();
//            qDebug()<<"openTime = "<<openTime;
//            qDebug()<<"cltime = "<<cltime;
            int ophour, opmin, clhour, clmin;
            value_to_hour_minute(openTime, &ophour, &opmin);
            value_to_hour_minute(cltime, &clhour, &clmin);

            if (ophour != 17 || opmin != 55 || clhour != 6 || clmin != 23) {
                m_colorSettings->set(AUTO_NIGHT_FROM_KEY,QString::number(hour_minute_to_value(17,55),'f', 2).toDouble());
                m_colorSettings->set(AUTO_NIGHT_TO_KEY, QString::number(hour_minute_to_value(6,23),'f', 2).toDouble());
//                qDebug()<<QString::number(hour_minute_to_value(17,55),'f', 2).toDouble();
            }

        } else {
            mTimeModeCombox->setCurrentIndex(2);
            double openTime = m_colorSettings->get(NIGHT_FROM_KEY).toDouble();
            double cltime = m_colorSettings->get(NIGHT_TO_KEY).toDouble();
//            qDebug()<<"openTime = "<<openTime;
//            qDebug()<<"cltime = "<<cltime;
            int ophour, opmin, clhour, clmin;
            value_to_hour_minute(openTime, &ophour, &opmin);
            value_to_hour_minute(cltime, &clhour, &clmin);

            mOpenTimeHCombox->setCurrentIndex(ophour);
            mQpenTimeMCombox->setCurrentIndex(opmin);
            mCloseTimeHCombox->setCurrentIndex(clhour);
            mCloseTimeMCombox->setCurrentIndex(clmin);
        }
        showNightWidget(true);
        int value = m_colorSettings->get(NIGHT_TEMPERATURE_KEY).toInt();
        mTemptSlider->setValue(value);
    } else {
        mNightModeBtn->setChecked(false);
        showNightWidget(false);
    }
    mTimeModeCombox->blockSignals(false);
}

void Widget::nightChangedSlot(QHash<QString, QVariant> nightArg)
{
    if (this->mRedshiftIsValid) {
        mNightModeBtn->setChecked(nightArg["Active"].toBool());
    }
}


/* 总结: 亮度条怎么显示和实际的屏幕状态有关,与按钮选择状态关系不大:
 * 实际为镜像模式，就显示所有屏幕的亮度(笔记本外显除外，笔记本外显任何情况均隐藏，这里未涉及)。
 * 实际为扩展模式，就显示当前选中的屏幕亮度，如果当前选中复制模式，则亮度条隐藏不显示，应用之后再显示所有亮度条;
 * 实际为单屏模式，即另一个屏幕关闭，则显示打开屏幕的亮度，关闭的显示器不显示亮度
 *
 *ps: by feng chao
*/

void Widget::showBrightnessFrame(const int flag)
{
    bool allShowFlag = true;
    allShowFlag = isCloneMode();

    ui->unifyBrightFrame->setFixedHeight(0);
    if (flag == 0 && allShowFlag == false && mUnifyButton->isChecked()) {  //选中了镜像模式，实际是扩展模式

    } else if ((allShowFlag == true && flag == 0) || flag == 1) { //镜像模式/即将成为镜像模式
        ui->unifyBrightFrame->setFixedHeight(BrightnessFrameV.size() * (50 + 2 + 2));
        for (int i = 0; i < BrightnessFrameV.size(); ++i) {
            BrightnessFrameV[i]->setOutputEnable(true);
            BrightnessFrameV[i]->setTextLabelName(tr("Brightness") + QString("(") + BrightnessFrameV[i]->getOutputName() + QString(")"));
            BrightnessFrameV[i]->setVisible(true);
        }
    } else {
        for (int i = 0; i < BrightnessFrameV.size(); ++i) {
            if (ui->primaryCombo->currentText() == BrightnessFrameV[i]->getOutputName() && BrightnessFrameV[i]->getOutputEnable()) {
                ui->unifyBrightFrame->setFixedHeight(52);
                //~ contents_path /display/Brightness
                BrightnessFrameV[i]->setTextLabelName(tr("Brightness"));
                BrightnessFrameV[i]->setVisible(true);
                //不能break，要把其他的frame隐藏
            } else {
                BrightnessFrameV[i]->setVisible(false);
            }
        }
    }
    if (ui->unifyBrightFrame->height() > 0) {
        ui->unifyBrightFrame->setVisible(true);
    } else {
        ui->unifyBrightFrame->setVisible(false);
    }
}

QList<ScreenConfig> Widget::getPreScreenCfg()
{
    QDBusMessage msg = mUkccInterface.get()->call("getPreScreenCfg");
    if(msg.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "get pre screen cfg failed";
    }
    QDBusArgument argument = msg.arguments().at(0).value<QDBusArgument>();
    QList<QVariant> infos;
    argument >> infos;

    QList<ScreenConfig> preScreenCfg;
    for (int i = 0; i < infos.size(); i++){
        ScreenConfig cfg;
        infos.at(i).value<QDBusArgument>() >> cfg;
        preScreenCfg.append(cfg);
    }

    return preScreenCfg;
}

void Widget::setPreScreenCfg(KScreen::OutputList screens)
{
    QMap<int, KScreen::OutputPtr>::iterator nowIt = screens.begin();

    int posCount = 0;
    QVariantList retlist;
    while (nowIt != screens.end()) {
        ScreenConfig cfg;
        cfg.screenId = nowIt.value()->name();
        cfg.screenModeId = nowIt.value()->currentModeId();
        cfg.screenPosX = nowIt.value()->pos().x();
        cfg.screenPosY = nowIt.value()->pos().y();

        QVariant variant = QVariant::fromValue(cfg);
        retlist << variant;

        if (nowIt.value()->pos() == QPoint(0, 0)) {
            posCount++;
        }
        nowIt++;
    }

    if (posCount >= 2) {
        return;
    }

    mUkccInterface.get()->call("setPreScreenCfg", retlist);

    QVariantList outputList;
    Q_FOREACH(QVariant variant, retlist) {
        ScreenConfig screenCfg = variant.value<ScreenConfig>();
        QVariantMap map;
        map["id"] = screenCfg.screenId;
        map["modeid"] = screenCfg.screenModeId;
        map["x"] = screenCfg.screenPosX;
        map["y"] = screenCfg.screenPosY;
        outputList << map;
    }

    QString filePath = QDir::homePath() + "/.config/ukui/ukcc-screenPreCfg.json";
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open config file for writing! " << file.errorString();

    }
    file.write(QJsonDocument::fromVariant(outputList).toJson());
}

void Widget::changescale()
{
    mScaleSizeRes = QSize();
    for (const KScreen::OutputPtr &output : mConfig->outputs()) {
        if (output->isEnabled()) {
            // 作判空判断，防止控制面板闪退
            if (output->currentMode()) {
                if (mScaleSizeRes == QSize()) {
                    mScaleSizeRes = output->currentMode()->size();
                } else {
                    mScaleSizeRes = mScaleSizeRes.width() < output->currentMode()->size().width()?mScaleSizeRes:output->currentMode()->size();
                }
            } else {
                return;
            }

        }
    }

    if (mScaleSizeRes != QSize(0,0)) {
        QSize scalesize = mScaleSizeRes;
        ui->scaleCombo->blockSignals(true);
        ui->scaleCombo->clear();
        ui->scaleCombo->addItem("100%", 1.0);

        if (scalesize.width() > 1024 ) {
            ui->scaleCombo->addItem("125%", 1.25);
        }
        if (scalesize.width() == 1920 ) {
            ui->scaleCombo->addItem("150%", 1.5);
        }
        if (scalesize.width() > 1920) {
            ui->scaleCombo->addItem("150%", 1.5);
            ui->scaleCombo->addItem("175%", 1.75);
        }
        if (scalesize.width() >= 2160) {
            ui->scaleCombo->addItem("200%", 2.0);
        }
        if (scalesize.width() > 2560) {
            ui->scaleCombo->addItem("225%", 2.25);
        }
        if (scalesize.width() > 3072) {
            ui->scaleCombo->addItem("250%", 2.5);
        }
        if (scalesize.width() > 3840) {
            ui->scaleCombo->addItem("275%", 2.75);
        }

        double scale;
        QStringList keys = scaleGSettings->keys();
        if (keys.contains("scalingFactor")) {
            scale = scaleGSettings->get(SCALE_KEY).toDouble();
        }
        if (ui->scaleCombo->findData(scale) == -1) {
            //记录分辨率切换时，新分辨率不存在的缩放率，在用户点击恢复设置时写入
            mIsSCaleRes = true;

            //记录是否因分辨率导致的缩放率变化
            mIsChange = true;

            scaleres = scale;
            scale = 1.0;
        }
        ui->scaleCombo->setCurrentText(QString::number(scale * 100) + "%");
        scaleChangedSlot(scale);
        ui->scaleCombo->blockSignals(false);
        mScaleSizeRes = QSize();

    }
}
