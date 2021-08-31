#ifndef OUTPUTCONFIG_H
#define OUTPUTCONFIG_H

#include <QGroupBox>
#include <QComboBox>
#include <QWidget>
#include <QRadioButton>
#include <KF5/KScreen/kscreen/output.h>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnection>
#include <QGSettings>
#include "applydialog.h"
#include "SwitchButton/switchbutton.h"
class QCheckBox;
class ResolutionSlider;
class QLabel;
class QStyledItemDelegate;


namespace Ui {
class KScreenWidget;
}

class OutputConfig : public QWidget
{
    Q_OBJECT

  public:
    explicit OutputConfig(QWidget *parent);
    explicit OutputConfig(const KScreen::OutputPtr &output, QWidget *parent = nullptr);
    ~OutputConfig() override;

    virtual void setOutput(const KScreen::OutputPtr &output);
    KScreen::OutputPtr output() const;

    void setTitle(const QString &title);
    void setShowScaleOption(bool showScaleOption);
    bool showScaleOption() const;

    //拿取配置
    void initConfig(const KScreen::ConfigPtr &config);
    QStringList readFile(const QString& filepath);
public Q_SLOTS:
    void slotScaleChanged();
    void slotRotationChanged();
protected Q_SLOTS:
    void slotResolutionChanged();
//    void slotRotationChanged(int index);
    void slotRefreshRateChanged(int index);
//    void slotScaleChanged(int index);
    void whetherApplyResolution();
    void whetherApplyScale();
    void whetherApplyRotation();
    void slotResolutionNotChange();
    void slotScaleNotChange();
    void slotRotationNotChange();
    void rotationDbusSlot(bool auto_rotation);
    void tabletModeDbusSlot(bool tablet_mode);
    void mode_rotationDbusSlot(bool tablet_mode);
    void mrotationDbusSlot(bool auto_rotation);

private:
    int resolutionIndex = 0;
    int rotationIndex = 0;
    int scaleIndex = 0;

  Q_SIGNALS:
    void changed();
    void scaleChanged(int index);

  protected:
    virtual void initUi();
    int getScreenScale();

  protected:
    QLabel *mTitle = nullptr;
    KScreen::OutputPtr mOutput;
    QCheckBox *mEnabled = nullptr;
    ResolutionSlider *mResolution = nullptr;
    QComboBox *mRotation = nullptr;
    QComboBox *mScale = nullptr;
    QComboBox *mRefreshRate = nullptr;
    QComboBox *mMonitor = nullptr;
    QComboBox *tmpResolution = nullptr;
    QComboBox *scaleCombox = nullptr;
    QLabel *rotationLabel = nullptr;
    SwitchButton *rotationRadioBtn;
    bool mShowScaleOption  = false;
#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)
    KScreen::ConfigPtr mConfig ;
#else
    KScreen::ConfigPtr mConfig = nullptr;
#endif

    QString qss;
    QStringList proRes;        //profile文件内容

    QGSettings *m_gsettings = nullptr;
    QDBusInterface *m_outputSessionDbus = nullptr;

};

#endif // OUTPUTCONFIG_H
