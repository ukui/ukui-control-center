/*
 * Copyright 2021 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef WIDGET_H
#define WIDGET_H

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

#include <KF5/KScreen/kscreen/config.h>

#include "outputconfig.h"
#include "SwitchButton/switchbutton.h"
#include "brightnessFrame.h"
#include "screenConfig.h"

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

    void setConfig(const KScreen::ConfigPtr &config, bool showBrightnessFrameFlag = false);
    KScreen::ConfigPtr currentConfig() const;

    void initConnection();
    QString getScreenName(QString name = "");
    void initTemptSlider();

    bool writeFile(const QString &filePath);
    void writeGlobal(const KScreen::OutputPtr &output);
    bool writeGlobalPart(const KScreen::OutputPtr &output, QVariantMap &info,
                         const KScreen::OutputPtr &fallback);
    QString globalFileName(const QString &hash);
    QVariantMap getGlobalData(KScreen::OutputPtr output);

    float converToScale(const int value);
    int scaleToSlider(const float value);

    void initUiComponent();
    void setScreenKDS(QString kdsConfig);
    void setActiveScreen(QString status = "");
    void addBrightnessFrame(QString name, bool openFlag, QString edidHash);
    void showBrightnessFrame(const int flag = 0);

    QList<ScreenConfig> getPreScreenCfg();
    void setPreScreenCfg(KScreen::OutputList screens);

    void changescale();
    QDBusInterface *dbusEdid = nullptr;
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

    void slotIdentifyButtonClicked(bool checked = true);
    void slotIdentifyOutputs(KScreen::ConfigOperation *op);
    void clearOutputIdentifiers();

    void outputAdded(const KScreen::OutputPtr &output, bool connectChanged);
    void outputRemoved(int outputId, bool connectChanged);
    void primaryOutputSelected(int index);
    void primaryOutputChanged(const KScreen::OutputPtr &output);

    void showNightWidget(bool judge);
    void showCustomWiget(int index);

    void slotThemeChanged(bool judge);

    void primaryButtonEnable(bool);             // 按钮选择主屏确认按钮
    void mainScreenButtonSelect(int index);     // 是否禁用设置主屏按钮
    void checkOutputScreen(bool judge);         // 是否禁用屏幕

    void setNightMode(const bool nightMode);    // 设置夜间模式

    void initNightStatus();                     // 初始化夜间模式
    void nightChangedSlot(QHash<QString, QVariant> nightArg);

    void callMethod(QRect geometry, QString name);// 设置wayland主屏幕
    QString getPrimaryWaylandScreen();
    void isWayland();
    void kdsScreenchangeSlot(QString status);

    void applyNightModeSlot();

    void delayApply();

public Q_SLOTS:
    void save();
    void scaleChangedSlot(double scale);
    void changedSlot();
    void propertiesChangedSlot(QString, QMap<QString, QVariant>, QStringList);

private:
    void loadQml();
    void resetPrimaryCombo();
    void addOutputToPrimaryCombo(const KScreen::OutputPtr &output);
    KScreen::OutputPtr findOutput(const KScreen::ConfigPtr &config, const QVariantMap &info);

    void setHideModuleInfo();
    void setTitleLabel();
    void writeScale(double scale);
    void initGSettings();
    void setcomBoxScale();
    void initNightUI();

    bool isRestoreConfig();   // 是否恢复应用之前的配置
    bool isCloneMode();
    bool isBacklight();
    bool isLaptopScreen();
    bool isVisibleBrightness();

    QString getCpuInfo();
    QString getMonitorType();

    int getPrimaryScreenID();

    void setScreenIsApply(bool isApply);

private:
    Ui::DisplayWindow *ui;
    QMLScreen *mScreen = nullptr;

    KScreen::ConfigPtr mConfig = nullptr;
    KScreen::ConfigPtr mPrevConfig = nullptr;

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

    QLabel *nightLabel = nullptr;

    QGSettings *mGsettings = nullptr;
    QGSettings *scaleGSettings = nullptr;
    QGSettings *mPowerGSettings = nullptr;

    QSettings *mQsettings = nullptr;

    QButtonGroup *singleButton;

    QSharedPointer<QDBusInterface> mUPowerInterface;
    QSharedPointer<QDBusInterface> mUkccInterface;

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

    QString firstAddOutputName;


    QShortcut *mApplyShortcut;
    QVector<BrightnessFrame*> BrightnessFrameV;
    //BrightnessFrame *currentBrightnessFrame;
    bool exitFlag = false;
    QString     mKDSCfg;
    bool unifySetconfig = false;
};

#endif // WIDGET_H
