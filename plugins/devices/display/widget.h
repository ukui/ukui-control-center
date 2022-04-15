#ifndef WIDGET_H
#define WIDGET_H

#include <limits>

#include <QMutex>
#include <QWidget>
#include <QVariantMap>
#include <QGSettings>
#include <QSettings>
#include <QButtonGroup>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusReply>
#include <QStringList>
#include <QSharedPointer>
#include <QHash>
#include <QVariant>
#include <QScreen>
#include <QMultiMap>
#include <QShortcut>
#include <QFuture>
#include <QVBoxLayout>

#include <KF5/KScreen/kscreen/config.h>

#include "outputconfig.h"
//#include <ukcc/widgets/switchbutton.h>
#include "commonComponent/SwitchButton/switchbutton.h"
#include "brightnessFrame.h"
#include "screenConfig.h"
//#include <ukcc/widgets/titlelabel.h>
//#include <ukcc/widgets/fixlabel.h>
//#include <ukcc/widgets/uslider.h>
#include "commonComponent/Uslider/uslider.h"

#define USD_CLONE_MODE 1

class QLabel;
class QMLOutput;
class QMLScreen;
class ControlPanel;
class PrimaryOutputCombo;

class QPushButton;
class QComboBox;

class QQuickView;
class QQuickWidget;
class QStyledItemDelegate;

typedef enum {
    SUN,
    CUSTOM,
}MODE;

typedef enum {
    EXTEND,
    CLONE,
}MULTISCREEN;

namespace KScreen {
class ConfigOperation;
}

namespace Ui {
class DisplayWindow;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

    void setConfig(const KScreen::ConfigPtr &config);
    KScreen::ConfigPtr currentConfig() const;

    void initNightModeUi();
    void slotFocusedOutputChangedNoParam();
    void initConnection();
    QString getScreenName(QString name = "");
    void setNightComponent();

    bool writeFile(const QString &filePath);
    void writeGlobal(const KScreen::OutputPtr &output);
    bool writeGlobalPart(const KScreen::OutputPtr &output, QVariantMap &info,
                         const KScreen::OutputPtr &fallback);
    QString globalFileName(const QString &hash);
    QVariantMap getGlobalData(KScreen::OutputPtr output);

    float converToScale(const int value);
    int scaleToSlider(const float value);

    void initUiComponent();
    void addBrightnessFrame(QString name, bool openFlag, QString edidHash);
    void showBrightnessFrame(const int flag = 0);

    QList<ScreenConfig> getPreScreenCfg();
    void setPreScreenCfg(KScreen::OutputList screens);

    void changescale();
protected:
    bool eventFilter(QObject *object, QEvent *event) override;

Q_SIGNALS:
    void changed();

    void nightModeChanged(const bool nightMode) const;
    void redShiftValidChanged(const bool isValid) const;

private Q_SLOTS:
    void slotFocusedOutputChanged(QMLOutput *output);

    void slotOutputEnabledChanged();
    void slotOutputConnectedChanged();

    void slotUnifyOutputs();
    void tabletMode_DbusSlot(bool tablet_mode);
    void rotationDbusSlot(bool autoRotation);

    void slotIdentifyButtonClicked(bool checked = true);
    void slotIdentifyOutputs(KScreen::ConfigOperation *op);
    void clearOutputIdentifiers();

    void outputAdded(const KScreen::OutputPtr &output, bool connectChanged);
    void outputRemoved(int outputId, bool connectChanged);
    void primaryOutputSelected(int index);
    void primaryOutputChanged(const KScreen::OutputPtr &output);

    void showNightWidget(bool judge);
    void showCustomWiget(int index);

    void primaryButtonEnable(bool);             // 按钮选择主屏确认按钮
    void mainScreenButtonSelect(int index);     // 是否禁用设置主屏按钮
    void checkOutputScreen(bool judge);         // 是否禁用屏幕

    void setNightMode(const bool nightMode);    // 设置夜间模式

    void initNightStatus();                     // 初始化夜间模式

    void setNightModeSetting();                 // 通过配置文件设置夜间模式
    void nightChangedSlot(QHash<QString, QVariant> nightArg);

    void callMethod(QRect geometry, QString name);// 设置wayland主屏幕
    QString getPrimaryWaylandScreen();
    void usdScreenModeChangedSlot(int status);

    void setMultiScreenSlot(int index);

    void applyNightModeSlot();

    void delayApply();

public Q_SLOTS:
    void save();
    void scaleChangedSlot(double scale);
    void changedSlot();
    void propertiesChangedSlot(QString, QMap<QString, QVariant>, QStringList);
    void mOutputClicked();

private:
    void loadQml();
    void resetPrimaryCombo();
    void addOutputToPrimaryCombo(const KScreen::OutputPtr &output);
    KScreen::OutputPtr findOutput(const KScreen::ConfigPtr &config, const QVariantMap &info);

    void initComponent();
    void initDbusComponent();
    void setHideModuleInfo();
    void setTitleLabel();
    void writeScale(double scale);
    void initGSettings();
    void setcomBoxScale();
    void initNightUI();
    QFrame *setLine(QFrame *frame);
    void initAdvanceScreen();

    bool isRestoreConfig();   // 是否恢复应用之前的配置
    bool isCloneMode();
    bool isBacklight();
    bool isLaptopScreen();
    bool isVisibleBrightness();

    QString getCpuInfo();
    QString getMonitorType();

    int getPrimaryScreenID();

    void setScreenIsApply(bool isApply);

    void setMulScreenVisiable();
    void initMultScreenStatus();
    void updateMultiScreen(); // 屏幕插拔更改状态
    void updateScreenConfig();// 获取系统最新配置

    void showZoomtips();      // 缩放注销提示框

    void paintEvent(QPaintEvent *event);

private:
    Ui::DisplayWindow *ui;
    QMLScreen *mScreen = nullptr;

#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)
    KScreen::ConfigPtr mConfig;
    KScreen::ConfigPtr mPrevConfig;
    KScreen::OutputPtr res;                       // 这是outPutptr结果
#else
    KScreen::ConfigPtr mConfig = nullptr;
    KScreen::ConfigPtr mPrevConfig = nullptr;
    KScreen::OutputPtr res = nullptr;
#endif

    ControlPanel *mControlPanel = nullptr;

    OutputConfig *mOutputConfig = nullptr;        // 设置主显示器相关控件

    QList<QQuickView *> mOutputIdentifiers;
    QTimer *mOutputTimer = nullptr;

    QString     mCPU;
    QString     mDir;
    QStringList mPowerKeys;

    SwitchButton *mNightButton = nullptr;
    SwitchButton *mCloseScreenButton = nullptr;
    SwitchButton *mUnifyButton = nullptr;
    SwitchButton *mThemeButton = nullptr;
    SwitchButton *mNightModeBtn = nullptr;
    SwitchButton *mAutoRotationBtn = nullptr;

    QLabel *mNightModeLabel = nullptr;
    QLabel *mOpenLabel = nullptr;
    QLabel *mTimeModeLabel = nullptr;
    QLabel *mCustomTimeLabel = nullptr;
    QLabel *mTemptLabel = nullptr;
    QLabel *mWarmLabel = nullptr;
    QLabel *mColdLabel = nullptr;
    QLabel *mLabel_1 = nullptr;

    QLabel *nightLabel = nullptr;
    QLabel *mMultiScreenLabel = nullptr;

    QComboBox *mTimeModeCombox = nullptr;
    QComboBox *mOpenTimeHCombox = nullptr;
    QComboBox *mQpenTimeMCombox = nullptr;
    QComboBox *mCloseTimeHCombox = nullptr;
    QComboBox *mCloseTimeMCombox = nullptr;
    QComboBox *mMultiScreenCombox = nullptr;

    Uslider *mTemptSlider = nullptr;

    QFrame *mMultiScreenFrame = nullptr;   
    QFrame *mNightModeFrame = nullptr;
    QFrame *mOpenFrame = nullptr;
    QFrame *mTimeModeFrame = nullptr;
    QFrame *mCustomTimeFrame = nullptr;
    QFrame *mTemptFrame = nullptr;
    QFrame *line_1 = nullptr;
    QFrame *line_2 = nullptr;
    QFrame *line_3 = nullptr;

    QGSettings *mGsettings = nullptr;
    QGSettings *scaleGSettings = nullptr;
    QGSettings *mPowerGSettings = nullptr;
    QGSettings *m_colorSettings = nullptr;

    QSettings *mQsettings = nullptr;

    QButtonGroup *singleButton;

    QSharedPointer<QDBusInterface> mUPowerInterface;
    QSharedPointer<QDBusInterface> mUkccInterface;
    QDBusInterface *mUsdDbus;
    QDBusInterface *m_StatusDbus = nullptr;

    QHash<QString, QVariant> mNightConfig;

    double mScreenScale = 1.0;
    double scaleres = 1.0;

    QSize mScaleSizeRes = QSize();

    bool mIsNightMode = false;
    bool mRedshiftIsValid = false;
    bool mIsScaleChanged = false;
    bool mOriApply;
    bool mConfigChanged = false;
    bool mOnBattery = false;
    bool mBlockChanges = false;
    bool mFirstLoad = true;
    bool mIsWayland = false;
    bool mIsBattery = false;
    bool mIsScreenAdd = false;
    bool mIsRestore = false;

    bool mIsSCaleRes = false;
    bool mIsChange = false;

    QStringList mTimeModeStringList;

    QString firstAddOutputName;
    QShortcut *mApplyShortcut;
    QVector<BrightnessFrame*> BrightnessFrameV;
    //BrightnessFrame *currentBrightnessFrame;
    bool exitFlag = false;
    bool unifySetconfig = false;
    QString mOutputClickedName;
    QDBusInterface *dbusEdid = nullptr;
    QString cpuArchitecture = "";
    bool mIscloneMode = false;

    int mKdsStatus = INT_MIN;
};

#endif // WIDGET_H
