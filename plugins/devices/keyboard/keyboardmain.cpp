#include "keyboardmain.h"

KeyboardMain::KeyboardMain(QWidget *parent)
    : QWidget(parent)
{
    mVlayout = new QVBoxLayout(this);
    mVlayout->setContentsMargins(0, 0, 0, 0);;
    initUI();
    initConnection();
}

KeyboardMain::~KeyboardMain()
{
}

QFrame* KeyboardMain::myLine()
{
    QFrame *line = new QFrame(this);
    line->setMinimumSize(QSize(0, 1));
    line->setMaximumSize(QSize(16777215, 1));
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    return line;
}

void KeyboardMain::initUI()
{
    QFrame *keyboardFrame = new QFrame(this);
    keyboardFrame->setMinimumSize(550, 0);
    keyboardFrame->setMaximumSize(16777215, 16777215);
    keyboardFrame->setFrameShape(QFrame::Box);

    QVBoxLayout *keyboardLyt = new QVBoxLayout(keyboardFrame);
    keyboardLyt->setContentsMargins(0, 0, 0, 0);

    mKeyboardSetTitleLabel = new TitleLabel(this);
    mKeyboardSetTitleLabel->setText(tr("Key board settings"));
    mKeyboardSetTitleLabel->setContentsMargins(14, 0, 0, 0);

    setKeyRepeatFrame();
    setDelayFrame();
    setSpeedFrame();
    setInputTestFrame();
    setKeyTipsFrame();

    /* Input method */
    mInputMethodSetBtn = new QPushButton(this);
    //~ contents_path /Keyboard/Input settings
    mInputMethodSetBtn->setText(tr("Input settings"));
    mInputMethodSetBtn->setFixedSize(QSize(160, 36));

    keyRepeatAndDelayLine = myLine();
    delayAndSpeedLine = myLine();
    speedAndInputTestLine = myLine();
    inputTestAndKeyTipsLine = myLine();

    /* add widget */
    keyboardLyt->addWidget(mKeyRepeatFrame);
    keyboardLyt->addWidget(keyRepeatAndDelayLine);
    keyboardLyt->addWidget(mDelayFrame);
    keyboardLyt->addWidget(delayAndSpeedLine);
    keyboardLyt->addWidget(mSpeedFrame);
    keyboardLyt->addWidget(speedAndInputTestLine);
    keyboardLyt->addWidget(mInputTestFrame);
    keyboardLyt->addWidget(inputTestAndKeyTipsLine);
    keyboardLyt->addWidget(mKeyTipsFrame);
    keyboardLyt->setSpacing(0);

    mVlayout->addWidget(mKeyboardSetTitleLabel);
    mVlayout->setSpacing(8);
    mVlayout->addWidget(keyboardFrame);
    mVlayout->addWidget(mInputMethodSetBtn);
    mVlayout->addStretch();
}

void KeyboardMain::setKeyRepeatFrame()
{
    /* Key Repeat */
    mKeyRepeatFrame = new QFrame(this);
    mKeyRepeatFrame->setFrameShape(QFrame::Shape::NoFrame);
    mKeyRepeatFrame->setMinimumSize(550, 60);
    mKeyRepeatFrame->setMaximumSize(16777215, 60);

    QHBoxLayout *KeyRepeatHLayout = new QHBoxLayout();

    //~ contents_path /Keyboard/Key repeat
    mKeyRepeatLabel = new QLabel(tr("Key repeat"), this);
    mKeyRepeatLabel->setMinimumWidth(140);
    mKeyRepeatBtn = new SwitchButton(this);
    KeyRepeatHLayout->addWidget(mKeyRepeatLabel);
    KeyRepeatHLayout->addStretch();
    KeyRepeatHLayout->addWidget(mKeyRepeatBtn);
    KeyRepeatHLayout->setContentsMargins(12, 0, 14, 0);

    mKeyRepeatFrame->setLayout(KeyRepeatHLayout);
}

void KeyboardMain::setDelayFrame()
{
    /* delay */
    mDelayFrame = new QFrame(this);
    mDelayFrame->setFrameShape(QFrame::Shape::NoFrame);
    mDelayFrame->setMinimumSize(550, 60);
    mDelayFrame->setMaximumSize(16777215, 60);

    QHBoxLayout *DelayHLayout = new QHBoxLayout();

    //~ contents_path /Keyboard/Delay
    mDelayLabel = new QLabel(tr("Delay"), this);
    mDelayLabel->setMinimumWidth(140);
    mDelayShortLabel =new QLabel(tr("Short"), this);
    mDelayShortLabel->setMinimumWidth(40);
    mDelayLongLabel = new QLabel(tr("Long"), this);
    mDelayLongLabel->setContentsMargins(8, 0, 13, 0);
    mDelayLongLabel->setMinimumWidth(40);
    mDelaySlider = new QSlider(Qt::Horizontal);
    mDelaySlider->setMinimum(200);
    mDelaySlider->setMaximum(2100);
    mDelaySlider->setPageStep(1);
    DelayHLayout->addWidget(mDelayLabel);
    DelayHLayout->addWidget(mDelayShortLabel);
    DelayHLayout->addWidget(mDelaySlider);
    DelayHLayout->addWidget(mDelayLongLabel);
    DelayHLayout->setContentsMargins(12, 0, 0, 0);

    mDelayFrame->setLayout(DelayHLayout);
}

void KeyboardMain::setSpeedFrame()
{
    /* Speed */
    mSpeedFrame = new QFrame(this);
    mSpeedFrame->setFrameShape(QFrame::Shape::NoFrame);
    mSpeedFrame->setMinimumSize(550, 60);
    mSpeedFrame->setMaximumSize(16777215, 60);

    QHBoxLayout *SpeedHLayout = new QHBoxLayout();

    //~ contents_path /Keyboard/Speed
    mSpeedLabel = new QLabel(tr("Speed"), this);
    mSpeedLabel->setMinimumWidth(140);
    mSpeedSlowLabel = new QLabel(tr("Slow"), this);
    mSpeedSlowLabel->setMinimumWidth(40);
    mSpeedFastLabel = new QLabel(tr("Fast"), this);
    mSpeedFastLabel->setContentsMargins(8, 0, 13, 0);
    mSpeedFastLabel->setMinimumWidth(40);
    mSpeedSlider = new QSlider(Qt::Horizontal);
    mSpeedSlider->setMinimum(10);
    mSpeedSlider->setMaximum(110);
    mSpeedSlider->setPageStep(1);
    SpeedHLayout->addWidget(mSpeedLabel);
    SpeedHLayout->addWidget(mSpeedSlowLabel);
    SpeedHLayout->addWidget(mSpeedSlider);
    SpeedHLayout->addWidget(mSpeedFastLabel);
    SpeedHLayout->setContentsMargins(12, 0, 0, 0);

    mSpeedFrame->setLayout(SpeedHLayout);
}

void KeyboardMain::setInputTestFrame()
{
    /* Input test */
    mInputTestFrame = new QFrame(this);
    mInputTestFrame->setFrameShape(QFrame::Shape::NoFrame);
    mInputTestFrame->setMinimumSize(550, 60);
    mInputTestFrame->setMaximumSize(16777215, 60);

    QHBoxLayout *InputTestHLayout = new QHBoxLayout();

     //~ contents_path /Keyboard/Input test
    mInputTestLabel = new QLabel(tr("Input test"), this);
    mInputTestLabel->setMinimumWidth(140);
    mInputTestLine = new QLineEdit(this);
    InputTestHLayout->addWidget(mInputTestLabel);
    InputTestHLayout->addWidget(mInputTestLine);
    InputTestHLayout->setContentsMargins(12, 0, 14, 0);

    mInputTestFrame->setLayout(InputTestHLayout);
}

void KeyboardMain::setKeyTipsFrame()
{
    /* key tips */
    mKeyTipsFrame = new QFrame(this);
    mKeyTipsFrame->setFrameShape(QFrame::Shape::NoFrame);
    mKeyTipsFrame->setMinimumSize(550, 60);
    mKeyTipsFrame->setMaximumSize(16777215, 60);

    QHBoxLayout *KeyTipsHLayout = new QHBoxLayout;

     //~ contents_path /Keyboard/Key tips
    mKeyTipsLabel = new QLabel(tr("Key tips"), this);
    mKeyTipsLabel->setMinimumWidth(140);
    mKeyTipsBtn = new SwitchButton(this);
    KeyTipsHLayout->addWidget(mKeyTipsLabel);
    KeyTipsHLayout->addStretch();
    KeyTipsHLayout->addWidget(mKeyTipsBtn);
    KeyTipsHLayout->setContentsMargins(12, 0, 14, 0);

    mKeyTipsFrame->setLayout(KeyTipsHLayout);
}

void KeyboardMain::initConnection()
{
    // 初始化键盘通用设置GSettings
    QByteArray keyBoardId(kKeyboardSchamas);
    // 初始化键盘按键提示GSettings 控制面板自带 不再判断是否安装
    QByteArray keyBoardOsdId(kKeyboardOsdSchemas);
    if (QGSettings::isSchemaInstalled(keyBoardId)) {
        mKeyboardGsettings = new QGSettings(kKeyboardSchamas, QByteArray(), this);
        mKeyboardOsdGsetting = new QGSettings(keyBoardOsdId);

        initKeyboardStatus();

        //将界面设置改动的key值写入GSettings
        connect(mKeyRepeatBtn, &SwitchButton::checkedChanged, this, &KeyboardMain::keyRepeatSlot);

        connect(mDelaySlider, &QSlider::valueChanged, this, &KeyboardMain::keyDelaySlot);

        connect(mSpeedSlider, &QSlider::valueChanged, this, &KeyboardMain::keySpeedSlot);

        connect(mKeyTipsBtn, &SwitchButton::checkedChanged, this, &KeyboardMain::keyTipsSlot);

        connect(mInputMethodSetBtn, &QPushButton::clicked, this, &KeyboardMain::inputMethodSetSlot);

        gsettingConnectUi();
    }
}

void KeyboardMain::initKeyboardStatus()
{
    //设置按键重复状态
    mKeyRepeatBtn->setChecked(mKeyboardGsettings->get(kKeyRepeatKey).toBool());
    setKeyboardVisible(mKeyRepeatBtn->isChecked());

    //设置按键重复的延时
    mDelaySlider->setValue(mKeyboardGsettings->get(kDelayKey).toInt());

    //设置按键重复的速度
    mSpeedSlider->setValue(mKeyboardGsettings->get(kSpeedKey).toInt());

    //设置按键提示状态
    mKeyTipsBtn->blockSignals(true);
    mKeyTipsBtn->setChecked(mKeyboardOsdGsetting->get(kKeyTipsKey).toBool());
    mKeyTipsBtn->blockSignals(false);

}

void KeyboardMain::setKeyboardVisible(bool checked)
{
    mDelayFrame->setVisible(checked);
    mSpeedFrame->setVisible(checked);
    mInputTestFrame->setVisible(checked);
    delayAndSpeedLine->setVisible(checked);
    speedAndInputTestLine->setVisible(checked);
    inputTestAndKeyTipsLine->setVisible(checked);
}

void KeyboardMain::gsettingConnectUi()
{
    //命令行 set key 值，界面设置做出相应的变化
    connect(mKeyboardGsettings, &QGSettings::changed, this, [=](const QString &key) {
       if(key == "repeat") {
           mKeyRepeatBtn->setChecked(mKeyboardGsettings->get(kKeyRepeatKey).toBool());
           setKeyboardVisible(mKeyRepeatBtn->isChecked());
       } else if(key == "delay") {
           mDelaySlider->setValue(mKeyboardGsettings->get(kDelayKey).toInt());
       } else if(key == "rate") {
           mSpeedSlider->setValue(mKeyboardGsettings->get(kSpeedKey).toInt());
       }
    });

    connect(mKeyboardOsdGsetting, &QGSettings::changed,this, [=](const QString &key) {
       if(key == "showLockTip") {
           mKeyTipsBtn->blockSignals(true);
           mKeyTipsBtn->setChecked(mKeyboardOsdGsetting->get(kKeyTipsKey).toBool());
           mKeyTipsBtn->blockSignals(false);
       }
    });
}

/* Slot */
void KeyboardMain::keyRepeatSlot(bool checked)
{
    setKeyboardVisible(checked);
    mKeyboardGsettings->set(kKeyRepeatKey, checked);
}

void KeyboardMain::keyDelaySlot(int value)
{
    mKeyboardGsettings->set(kDelayKey, value);
}

void KeyboardMain::keySpeedSlot(int value)
{
    mKeyboardGsettings->set(kSpeedKey, value);
}

void KeyboardMain::keyTipsSlot(bool checked)
{
    mKeyboardOsdGsetting->set(kKeyTipsKey, checked);
}

void KeyboardMain::inputMethodSetSlot()
{
    QProcess process;
    process.startDetached("fcitx-config-gtk3");
}
