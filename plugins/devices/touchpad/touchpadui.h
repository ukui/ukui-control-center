#ifndef TOUCHPADUI_H
#define TOUCHPADUI_H

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QComboBox>
#include <QSlider>
#include <QVBoxLayout>
#include <QGSettings/QGSettings>
#include <QByteArray>
#include <QDebug>

#include "SwitchButton/switchbutton.h"
#include "Label/titlelabel.h"

#include <QX11Info>
extern "C" {
#include <X11/extensions/XInput.h>
#include <X11/Xatom.h>
}
//垂直边界
#define V_EDGE_KEY               "vertical-edge-scrolling"
//垂直中间
#define V_FINGER_KEY             "vertical-two-finger-scrolling"
#define N_SCROLLING              "none"
//水平中间
#define H_FINGER_KEY             "horizontal-two-finger-scrolling"
//水平边界
#define H_EDGE_KEY               "horizontal-edge-scrolling"

const QByteArray kTouchpadSchemas = "org.ukui.peripherals-touchpad";
const QString kMouseDisableKey = "disable-on-external-mouse";
const QString kTypingDisableKey = "disable-while-typing";
const QString kClickKey = "tap-to-click";
const QString kScrollSlideKey = "natural-scroll";
const QString kPointerSpeedKey = "motion-acceleration";

/*
*motion-acceleration
* 0.1 --- 10
*  快 --- 慢
*/
const double kPointerSpeedMin = 0.1;
const double kPointerSpeedMax = 10;

class TouchpadUI : public QWidget
{
    Q_OBJECT

public:
    TouchpadUI(QWidget *parent = nullptr);
    ~TouchpadUI();

private:
    QFrame *mMouseDisableFrame;
    QFrame *mPointerSpeedFrame;
    QFrame *mTypingDisableFrame;
    QFrame *mClickFrame;
    QFrame *mScrollSlideFrame;
    QFrame *mScrollAreaFrame;

    SwitchButton *mMouseDisableBtn;     // 插入鼠标时禁用触摸板
    SwitchButton *mTypingDisableBtn;    // 打字时禁用触摸板
    SwitchButton *mClickBtn;            // 触摸板轻触点击
    SwitchButton *mScrollSlideBtn;      // 滚动条跟随手指滑动
    QSlider *mPointerSpeedSlider;       // 指针速度
    QComboBox *mScrollTypeComBox;       // 滚动区域

    TitleLabel *mTouchpadSetTitleLabel; //触摸板设置

    QLabel *mMouseDisableLabel;
    QLabel *mPointerSpeedLabel;
    QLabel *mPointerSpeedSlowLabel;
    QLabel *mPointerSpeedFastLabel;
    QLabel *mTypingDisableLabel;
    QLabel *mClickLabel;
    QLabel *mScrollSlideLabel;
    QLabel *mScrollAreaLabel;

    QVBoxLayout *mVlayout;

    QGSettings *mTouchpadGsetting;

private:
    QFrame* myLine();
    void initUI();
    void initConnection();
    void gsettingConnectUi();
    void initEnableStatus();
    QString _findKeyScrollingType();

    void mouseDisableSlot(bool status);
    void pointerSpeedSlot(int value);
    void typingDisableSlot(bool status);
    void clickSlot(bool status);
    void scrollSlideSlot(bool status);
    void scrolltypeSlot();
};
#endif // TOUCHPAD_H
