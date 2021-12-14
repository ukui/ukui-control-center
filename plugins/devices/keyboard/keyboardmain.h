#ifndef KEYBOARDMAIN_H
#define KEYBOARDMAIN_H

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>
#include <QSlider>
#include <QLineEdit>
#include <QGSettings/QGSettings>
#include <QPushButton>
#include <QProcess>
#include <QDebug>

#include <ukcc/widgets/switchbutton.h>
#include <ukcc/widgets/titlelabel.h>


const QByteArray kKeyboardSchamas = "org.ukui.peripherals-keyboard";
const QString kKeyRepeatKey = "repeat";
const QString kDelayKey = "delay";
const QString kSpeedKey = "rate";

const QByteArray kKeyboardOsdSchemas = "org.ukui.control-center.osd";
const QString kKeyTipsKey = "show-lock-tip";

class KeyboardMain : public QWidget
{
    Q_OBJECT

public:
    KeyboardMain(QWidget *parent = nullptr);
    ~KeyboardMain();

private:
    QFrame *mKeyRepeatFrame;        //按键重复
    QFrame *mDelayFrame;            //延迟
    QFrame *mSpeedFrame;            //速度
    QFrame *mInputTestFrame;        //输入测试
    QFrame *mKeyTipsFrame;          //按键提示

    SwitchButton *mKeyTipsBtn;
    SwitchButton *mKeyRepeatBtn;

    TitleLabel *mKeyboardSetTitleLabel;
    QLabel *mKeyRepeatLabel;
    QLabel *mDelayLabel;
    QLabel *mDelayShortLabel;
    QLabel *mDelayLongLabel;
    QLabel *mSpeedLabel;
    QLabel *mSpeedFastLabel;
    QLabel *mSpeedSlowLabel;
    QLabel *mInputTestLabel;
    QLabel *mKeyTipsLabel;

    QFrame *keyRepeatAndDelayLine;
    QFrame *delayAndSpeedLine;
    QFrame *speedAndInputTestLine;
    QFrame *inputTestAndKeyTipsLine;

    QPushButton *mInputMethodSetBtn;    //输入法设置

    QVBoxLayout *mVlayout;

    QSlider *mDelaySlider;
    QSlider *mSpeedSlider;

    QLineEdit *mInputTestLine;

    QGSettings *mKeyboardGsettings;
    QGSettings *mKeyboardOsdGsetting;

private:
    QFrame* myLine();
    void initUI();
    void setKeyRepeatFrame();
    void setDelayFrame();
    void setSpeedFrame();
    void setInputTestFrame();
    void setKeyTipsFrame();
    void initConnection();
    void initKeyboardStatus();
    void setKeyboardVisible(bool checked);
    void gsettingConnectUi();

    void keyRepeatSlot(bool checked);
    void keyDelaySlot(int value);
    void keySpeedSlot(int value);
    void inputTestSlot();
    void keyTipsSlot(bool checked);
    void inputMethodSetSlot();

};
#endif // KEYBOARDMAIN_H
