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

#define V_EDGE_KEY               "vertical-edge-scrolling"
#define V_FINGER_KEY             "vertical-two-finger-scrolling"
#define N_SCROLLING              "none"
#define H_FINGER_KEY             "horizontal-two-finger-scrolling"

const QByteArray kTouchpadSchemas = "org.ukui.peripherals-touchpad";
const QString kMouseDisableKey = "disable-on-external-mouse";
const QString kTypingDisableKey = "disable-while-typing";
const QString kClickKey = "tap-to-click";
const QString kScrollSlideKey = "natural-scroll";

const QByteArray kMouseSchemas = "org.ukui.peripherals-mouse";
const QString kCursorSpeedKey = "motion-acceleration";

class TouchpadUI : public QWidget
{
    Q_OBJECT

public:
    TouchpadUI(QWidget *parent = nullptr);
    ~TouchpadUI();

private:
    QFrame *mMouseDisableFrame;
    QFrame *mCursorSpeedFrame;
    QFrame *mTypingDisableFrame;
    QFrame *mClickFrame;
    QFrame *mScrollSlideFrame;
    QFrame *mScrollAreaFrame;

    SwitchButton *mMouseDisableBtn;     // 插入鼠标时禁用触摸板
    SwitchButton *mTypingDisableBtn;    // 打字时禁用触摸板
    SwitchButton *mClickBtn;            // 触摸板轻触点击
    SwitchButton *mScrollSlideBtn;      // 滚动条跟随手指滑动
    QSlider *mCursorSpeedSlider;        // 光标速度
    QComboBox *mScrollTypeComBox;       // 滚动区域

    TitleLabel *mTouchpadSetTitleLabel; //触摸板设置

    QLabel *mMouseDisableLabel;
    QLabel *mCursorSpeedLabel;
    QLabel *mCursorSpeedSlowLabel;
    QLabel *mCursorSpeedFastLabel;
    QLabel *mTypingDisableLabel;
    QLabel *mClickLabel;
    QLabel *mScrollSlideLabel;
    QLabel *mScrollAreaLabel;
    QLabel *mTipLabel;

    QHBoxLayout *mTipHLayout;

    QVBoxLayout *mVlayout;

    QGSettings *mTouchpadGsetting;
    QGSettings *mMouseGsetting;

private:
    void initUI();
    void initConnection();
    void gsettingConnectUi();
    void initEnableStatus();
    void setFrameVisible(bool visible);
    void isTouchpadExist();
    QString _findKeyScrollingType();
    bool findSynaptics();
    bool _supportsXinputDevices();
    XDevice* _deviceIsTouchpad (XDeviceInfo * deviceinfo);
    bool _deviceHasProperty (XDevice * device, const char * property_name);

    void mouseDisableSlot(bool status);
    void cursorSpeedSlot(int value);
    void typingDisableSlot(bool status);
    void clickSlot(bool status);
    void scrollSlideSlot(bool status);
    void scrolltypeSlot();
};
#endif // TOUCHPAD_H
