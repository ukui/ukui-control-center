#ifndef MOUSEUI_H
#define MOUSEUI_H

#include <QObject>
#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QRadioButton>
#include <QSlider>
#include <QByteArray>
#include <QGSettings/QGSettings>
#include <QVariant>
#include <QApplication>
#include <QButtonGroup>
#include <QDebug>

#include <ukcc/widgets/switchbutton.h>
#include <ukcc/widgets/titlelabel.h>
#include <ukcc/widgets/uslider.h>

const QByteArray kMouseSchemas = "org.ukui.peripherals-mouse";
const QString kDominantHandKey = "left-handed";
const QString kWheelSpeedKey = "wheel-speed";
const QString kDoubleClickIntervalKey = "double-click";
const QString kPointerSpeedKey = "motion-acceleration";
const QString kMouseAccelerationKey = "mouse-accel";
const QString kPointerPositionKey = "locate-pointer";
const QString kPointerSizeKey = "cursor-size";

const QByteArray kDesktopSchemas = "org.mate.interface";
const QString kBlinkCursorOnTextKey = "cursor-blink";
const QString kCursorSpeedKey = "cursor-blink-time";

const QByteArray kThemeSchemas = "org.ukui.style";


class MyLabel : public QLabel
{
    Q_OBJECT

public:
    MyLabel();
    ~MyLabel();

public:
    QGSettings * mSettings;

protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
};

class MouseUI : public QWidget
{
    Q_OBJECT

public:
    MouseUI(QWidget *parent = nullptr);
    ~MouseUI();

private:
    QFrame *mDominantHandFrame;         // 惯用手
    QFrame *mWheelSpeedFrame;           // 滚轮速度
    QFrame *mDoubleClickIntervalFrame;  // 双击间隔时长
    QFrame *mPointerSpeedFrame;         // 指针速度
    QFrame *mMouseAccelerationFrame;    // 鼠标加速
    QFrame *mPointerPositionFrame;      // 按 ctrl 键时显示指针位置
    QFrame *mPointerSizeFrame;          // 指针大小
    QFrame *mBlinkingCursorOnTextFrame; // 文本区域闪烁光标
    QFrame *mCursorSpeedFrame;          // 光标速度

    TitleLabel *mMouseTitleLabel;
    QLabel *mDominantHandLabel;
    QLabel *mWheelSpeedLabel;
    QLabel *mWheelSpeedSlowLabel;
    QLabel *mWheelSpeedFastLabel;
    QLabel *mDoubleClickIntervalLabel;
    QLabel *mDoubleClickIntervalShortLabel;
    QLabel *mDoubleClickIntervalLongLabel;

    TitleLabel *mPointerTitleLabel;
    QLabel *mPointerSpeedLabel;
    QLabel *mPointerSpeedSlowLabel;
    QLabel *mPointerSpeedFastLabel;
    QLabel *mMouseAccelerationLabel;
    QLabel *mPointerPositionLabel;
    QLabel *mPointerSizeLabel;

    TitleLabel *mCursorTitleLabel;
    QLabel *mBlinkingCursorOnTextLabel;
    QLabel *mCursorSpeedlabel;
    QLabel *mCursorSpeedSlowlabel;
    QLabel *mCursorSpeedFastlabel;

    QRadioButton *mDominantHandLeftRadio;
    QRadioButton *mDominantHandRightRadio;
    QRadioButton *mPointerSizeSmallRadio;
    QRadioButton *mPointerSizeMediumRadio;
    QRadioButton *mPointerSizeLargeRadio;

    QButtonGroup *mDominantRadioGroup;
    QButtonGroup *mPointerSizeRadioGroup;

    SwitchButton *mMouseAccelerationBtn;
    SwitchButton *mPointerPositionBtn;
    SwitchButton *mBlinkingCursorOnTextBtn;

    Uslider *mWheelSpeedSlider;
    QSlider *mDoubleClickIntervalSlider;
    QSlider *mPointerSpeedSlider;
    QSlider *mCursorSpeedSlider;

    QVBoxLayout *mVlayout;

    QGSettings *mMouseGsetting;
    QGSettings *mDesktopGsetting;
    QGSettings *mThemeSettings;

private:
    QFrame* myLine();
    void initUI();
    void setDominantHandFrame();
    void setWheelSpeedFrame();
    void setDoubleClickFrame();
    void setPointerSpeedFrame();
    void setMouseAccelerationFrame();
    void setPointerPositionFrame();
    void setPointerSizeFrame();
    void setBlinkCursorFrame();
    void setCursorSpeedFrame();
    void initConnection();
    void gsettingConnection();

    void initEnableStatus();

    void dominantHandSlot(QAbstractButton *button);
    void wheelSpeedSlot();
    void doubleClickIntervalSlot();
    void pointerSpeedSlot(int value);
    void mouseAccelerationSlot(bool checked);
    void pointerPositionSlot(bool checked);
    void pointerSizeSlot(QAbstractButton *button);
    void blinkCursorOnTextSlot(bool checked);
    void cursorSpeedSlot();

};
#endif // MOUSEUI_H
