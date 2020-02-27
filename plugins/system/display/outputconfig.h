#ifndef OUTPUTCONFIG_H
#define OUTPUTCONFIG_H

#include <QGroupBox>
#include <QComboBox>
#include <QWidget>

#include <KF5/KScreen/kscreen/output.h>

class QCheckBox;
class ResolutionSlider;
class QLabel;


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



  protected Q_SLOTS:
    void slotResolutionChanged(const QSize &size);
    void slotRotationChanged(int index);
    void slotRefreshRateChanged(int index);
    void slotScaleChanged(int index);

  Q_SIGNALS:
    void changed();

  protected:
    virtual void initUi();

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
    bool mShowScaleOption  = false;

    KScreen::ConfigPtr mConfig = nullptr;


};

#endif // OUTPUTCONFIG_H
