#ifndef BRIGHTNESSFRAME_H
#define BRIGHTNESSFRAME_H

#include <QFrame>
#include <QLabel>
#include <QWidget>
#include <QLabel>
//#include <ukcc/widgets/uslider.h>
#include "commonComponent/Uslider/uslider.h"
#include <QMutex>
#include <QFuture>
//#include <ukcc/widgets/fixlabel.h>


class BrightnessFrame : public QFrame
{
    Q_OBJECT
public:
    BrightnessFrame(const QString &name, const bool &isBattery, const QString &edidHash = "", QWidget *parent = nullptr);
    ~BrightnessFrame();
    void setTextLabelName(QString text);
    void setTextLabelValue(QString text);
    void setOutputEnable(const bool &enable);
    bool getOutputEnable();
    void runConnectThread(const bool &openFlag);
    int  getDDCBrighthess();
    bool getSliderEnable();
    void setSliderEnable(const bool &enable);
    void setDDCBrightness(const int &value);
    void updateEdidHash(const QString &edid);
    QString getEdidHash();
    QString getOutputName();

Q_SIGNALS:
    void sliderEnableChanged();

private:
    QLabel *labelName = nullptr;
    QLabel *labelValue = nullptr;
    Uslider *slider = nullptr;
    QString outputName;          //屏幕名
    bool    outputEnable;     //该屏幕是否打开
    bool    connectFlag;  //该屏幕是否连接
    QString edidHash;
    QMutex      mLock;
    bool exitFlag;
    bool isBattery;
    QFuture<void> threadRun;
    volatile bool threadRunFlag;
    QLabel *labelMsg = nullptr;
};

#endif // BRIGHTNESSFRAME_H
