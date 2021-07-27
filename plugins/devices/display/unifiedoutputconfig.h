#ifndef UNIFIEDOUTPUTCONFIG_H
#define UNIFIEDOUTPUTCONFIG_H

#include "outputconfig.h"

namespace KScreen
{
class Output;
class Config;
}

class UnifiedOutputConfig : public OutputConfig
{
    Q_OBJECT
  public:
    explicit UnifiedOutputConfig(const KScreen::ConfigPtr &config, QWidget *parent);
    ~UnifiedOutputConfig() override;

    void setOutput(const KScreen::OutputPtr &output) override;
    bool is_unifiedoutput = false;

  private Q_SLOTS:
    void slotResolutionChanged();

    //统一输出后调整屏幕方向统一代码
    void slotRotationChangedDerived();

    void whetherApplyResolution();
    void whetherApplyRotation();
    void slotResolutionNotChange();
    void slotRotationNotChange();

private:
    int resolutionIndex = 0;
    int rotationIndex = 0;

private:
    void initUi() override;
    KScreen::OutputPtr createFakeOutput();
    QString findBestMode(const KScreen::OutputPtr &output, const QSize &size);
private:
    KScreen::ConfigPtr mConfig;
    QList<KScreen::OutputPtr> mClones;
    QGSettings *m_tgsettings = nullptr;
};

#endif // UNIFIEDOUTPUTCONFIG_H
