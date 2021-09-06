#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QVariantMap>
#include <QGSettings>
#include <QSettings>
#include <QButtonGroup>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusReply>
#include <QGSettings>

#include <KF5/KScreen/kscreen/config.h>

#include "outputconfig.h"
#include "slider.h"
#include "SwitchButton/switchbutton.h"
#include "warningdialog.h"
#include "applydialog.h"

#define SETTINGS_DAEMON_AUTOBRIGHTNESS  "org.ukui.SettingsDaemon.plugins.auto-brightness"
#define AUTO_BRIGHTINESS_KEY "auto-brightness"

const QString tempDayBrig  = "6500";

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

typedef struct xml{
    QString isClone;
    QString outputName;
    QString vendorName;
    QString productName;
    QString serialNum;
    QString widthValue;
    QString heightValue;
    QString rateValue;
    QString posxValue;
    QString posyValue;
    QString rotationValue;
    QString isPrimary;
    bool isEnable;
}xmlFile;

typedef enum {
    SUN,
    ALL,
    CUSTOM,
}MODE;


namespace KScreen
{
class ConfigOperation;
}

namespace Ui {
class DisplayWindow;
}

class DisplayWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit DisplayWidget(QWidget *parent = nullptr);
    ~DisplayWidget() override;

    void setConfig(const KScreen::ConfigPtr &config);
    KScreen::ConfigPtr currentConfig() const;

    void slotFocusedOutputChangedNoParam();
    // 亮度调节UI
    void initBrightnessUI();
    QString getScreenName(QString name = "");
    void initTemptSlider();
    void initConfigFile();
    void writeScreenXml();

    void setIsNightMode(bool isNightMode);

    float converToScale(const int value);
    int scaleToSlider(const float value);
    void size();
    void initUiComponent();
    void OnRandrEvent();
    void initUI();
  protected:
    bool eventFilter(QObject *object, QEvent *event) override;

  Q_SIGNALS:
    void changed();
    void primaryChange();
    void nightModeChanged(const bool nightMode) const;
    void redShiftValidChanged(const bool isValid) const;

  private Q_SLOTS:
    void slotFocusedOutputChanged(QMLOutput *output);
    void startUnifyButton();
    void m_unifybuttonChanged();
    void slotOutputEnabledChanged();
    void slotOutputConnectedChanged();

    void slotUnifyOutputs();

    void slotChangeAutoBrightness(QString key);

    void slotIdentifyButtonClicked(bool checked = true);
    void slotIdentifyOutputs(KScreen::ConfigOperation *op);
    void clearOutputIdentifiers();
    void screenAddedProcess();
    void screenAddedTimer();
    void screenRemovedProcess();
    void screenRemovedTimer();
    void outputAdded(const KScreen::OutputPtr &output);
    void outputRemoved(int outputId);
    void primaryOutputSelected(int index);
    void primaryOutputChanged(const KScreen::OutputPtr &output);

    void showNightWidget(bool judge);
    void showlightWidget(bool judge);
    void showCustomWiget(int index);
    //判断系统是否支持自动亮度
    void hasAutoLight();
    // 按钮选择主屏确认按钮
    void primaryButtonEnable();
    // 是否禁用设置主屏按钮
    void mainScreenButtonSelect(int index);
    // 是否禁用屏幕
    void checkOutputScreen(bool judge);
    // 设置屏幕亮度
    void setBrightnessScreen(int brightnessValue);
    // 设置亮度滑块数值
    void setBrightnesSldierValue(QString screeName);
    // 设置跟随日落日出或自定义时间
    void onRadioClickButton();
  public:
    SwitchButton *m_unifybutton = nullptr;
  public Q_SLOTS:
    void save();
    void scaleChangedSlot(int index);
    void widget_DbusSlot(bool tablet_mode);
  private:

    void loadQml();
    void resetPrimaryCombo();
    void addOutputToPrimaryCombo(const KScreen::OutputPtr &output);
    bool writeInit(QString group, QString key, bool value);//写入路径存储
    KScreen::OutputPtr findOutput(const KScreen::ConfigPtr &config, const QVariantMap &info);

    void writeScale(int scale);
    void initGSettings();
    void initUnifybuttonStatus();
    void apply();
    void applyConifg();
    void nightModeApply();
    bool isCloneMode();
    int opHour;
    int opMin;
    int clsHour;
    int clsMin;
    int autoopHour;
    int autoopMin;
    int autoclsHour;
    int autoclsMin;
    QGSettings *m_brilight = nullptr;
  private:
    Ui::DisplayWindow *ui;
    QMLScreen *mScreen = nullptr;
#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)
    KScreen::ConfigPtr mConfig ;
    KScreen::ConfigPtr mPrevConfig ;
    //这是outPutptr结果
    KScreen::OutputPtr res ;
#else
    KScreen::ConfigPtr mConfig = nullptr;
    KScreen::ConfigPtr mPrevConfig = nullptr;
    // outPutptr结果
    KScreen::OutputPtr res = nullptr;
#endif

    ControlPanel *mControlPanel = nullptr;
    // 设置主显示器相关控件
    OutputConfig *mOutputConfig = nullptr;

    QList<QQuickView*> mOutputIdentifiers;
    QTimer * mOutputTimer = nullptr;
    QTimer * mScreenAddTimer = nullptr;
    QTimer * mScreenRemoveTimer = nullptr;

    bool m_blockChanges = false;
    // 亮度配置文件位置
    QString brightnessFile = nullptr;
    // xml文件
    xmlFile inputXml[100];

    SwitchButton *nightButton = nullptr;
    SwitchButton *lightButton = nullptr;
    SwitchButton *closeScreenButton = nullptr;
    QLabel *nightLabel = nullptr;
    QLabel *lightLabel = nullptr;

    // 是否为夜间模式
    bool m_isNightMode = false;
    // profile文件内容
    QStringList proRes;

    Slider *slider;
    QGSettings *m_gsettings = nullptr;
    QGSettings *scaleGSettings = nullptr;
    QGSettings * m_colorSettings = nullptr;
    QGSettings * m_settings = nullptr;
    QSettings *m_qsettings = nullptr;
    QDBusInterface *m_statusSessionDbus = nullptr;
    QButtonGroup *singleButton;
    QGSettings * powerSettings;
    QGSettings * xrandrSettings;
    QDBusInterface *m_widgetInterface;
    QString m_initPath;

    QTimer* m_unifyTimer;
    int count;
    int screenScale = 1;
    bool isScaleChanged = false;
    bool oriApply;
    bool unifyChecked = false;
};

#endif // WIDGET_H

