#ifndef OUTPUTCONFIG_H
#define OUTPUTCONFIG_H

#include <QGroupBox>
#include <QComboBox>
#include <QWidget>

#include <KF5/KScreen/kscreen/output.h>

#include <QGSettings>

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

    QFrame *setLine(QFrame *frame);
    virtual void setOutput(const KScreen::OutputPtr &output);
    KScreen::OutputPtr output() const;

    void setTitle(const QString &title);
    void setShowScaleOption(bool showScaleOption);
    bool showScaleOption() const;

    void initConfig(const KScreen::ConfigPtr &config);

protected Q_SLOTS:
    void slotResolutionChanged(const QSize &size, bool emitFlag);
    void slotRotationChanged(int index);
    void slotRefreshRateChanged(int index);
    void slotScaleChanged(int index);
    void slotDPIChanged(QString key);
    void slotEnableWidget();

Q_SIGNALS:
    void changed();
    void scaleChanged(double scale);
    void toSetScreenPos();
protected:
    virtual void initUi();
    double getScreenScale();
    QString refreshRateToText(float refreshRate);

private:
    void initConnection();
    void initDpiConnection();
    QString scaleToString(double scale);
    void initRadeon();

protected:
    KScreen::OutputPtr mOutput;
    QLabel *mTitle = nullptr;
    QCheckBox *mEnabled = nullptr;
    ResolutionSlider *mResolution = nullptr;

    QComboBox *mRotation = nullptr;
    QComboBox *mScale = nullptr;
    QComboBox *mRefreshRate = nullptr;
    QComboBox *mMonitor = nullptr;
    QComboBox *mScaleCombox = nullptr;

    bool mShowScaleOption = false;
    bool mIsWayland = false;
    bool mIsFirstLoad = true;
    bool mIsRestore = true;  //非restore时不再去修改刷新率/分辨率，避免修改多次导致显示重复
    bool mIsRadeon;

    KScreen::ConfigPtr mConfig = nullptr;

    QGSettings *mDpiSettings = nullptr;
};

#endif // OUTPUTCONFIG_H
