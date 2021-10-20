#include "touchpadui.h"

TouchpadUI::TouchpadUI(QWidget *parent)
    : QWidget(parent)
{
    mVlayout = new QVBoxLayout(this);
    mVlayout->setContentsMargins(0, 0, 40, 0);
    initUI();
    initConnection();
}

TouchpadUI::~TouchpadUI()
{
}

QFrame* TouchpadUI::myLine()
{
    QFrame *line = new QFrame(this);
    line->setMinimumSize(QSize(0, 1));
    line->setMaximumSize(QSize(16777215, 1));
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    return line;
}

void TouchpadUI::initUI()
{
    QFrame *touchpadFrame = new QFrame(this);
    touchpadFrame->setMinimumSize(550, 0);
    touchpadFrame->setMaximumSize(16777215, 16777215);
    touchpadFrame->setFrameShape(QFrame::Box);

    QVBoxLayout *touchpadLyt = new QVBoxLayout(touchpadFrame);
    touchpadLyt->setContentsMargins(0, 0, 0, 0);

    mTouchpadSetTitleLabel = new TitleLabel(this);
    mTouchpadSetTitleLabel->setText(tr("Touchpad Setting"));

    /* Tip no touchpad found */
    mTipHLayout = new QHBoxLayout();
    mTipLabel = new QLabel(tr("No touchpad found"));
    mTipHLayout->addStretch();
    mTipHLayout->addWidget(mTipLabel);
    mTipHLayout->addStretch();

    /* MouseDisable */
    mMouseDisableFrame = new QFrame(this);
    mMouseDisableFrame->setFrameShape(QFrame::Shape::Box);
    mMouseDisableFrame->setMinimumSize(550, 60);
    mMouseDisableFrame->setMaximumSize(16777215, 60);

    QHBoxLayout *MouseDisableHLayout = new QHBoxLayout();

    mMouseDisableBtn = new SwitchButton(this);
    //~ contents_path /touchpad/Disable touchpad when using the mouse
    mMouseDisableLabel = new QLabel(tr("Disable touchpad when using the mouse"), this);
    MouseDisableHLayout->addSpacing(7);
    MouseDisableHLayout->addWidget(mMouseDisableLabel);
    MouseDisableHLayout->addStretch();
    MouseDisableHLayout->addWidget(mMouseDisableBtn);

    mMouseDisableFrame->setLayout(MouseDisableHLayout);

    /* CursorSpeed */
    mCursorSpeedFrame = new QFrame(this);
    mCursorSpeedFrame->setFrameShape(QFrame::Shape::Box);
    mCursorSpeedFrame->setMinimumSize(550, 60);
    mCursorSpeedFrame->setMaximumSize(16777215, 60);

    QHBoxLayout *CursorSpeedHLayout = new QHBoxLayout();

     //~ contents_path /touchpad/Cursor Speed
    mCursorSpeedLabel = new QLabel(tr("Cursor Speed"), this);
    mCursorSpeedLabel->setMinimumWidth(200);
    mCursorSpeedSlowLabel = new QLabel(tr("Slow"), this);
    mCursorSpeedFastLabel = new QLabel(tr("Fast"), this);
    mCursorSpeedSlider = new QSlider(Qt::Horizontal);
    mCursorSpeedSlider->setMinimum(100);
    mCursorSpeedSlider->setMaximum(1000);
    mCursorSpeedSlider->setSingleStep(50);
    mCursorSpeedSlider->setPageStep(50);
    CursorSpeedHLayout->addSpacing(7);
    CursorSpeedHLayout->addWidget(mCursorSpeedLabel);
    CursorSpeedHLayout->addWidget(mCursorSpeedSlowLabel);
    CursorSpeedHLayout->addWidget(mCursorSpeedSlider);
    CursorSpeedHLayout->addWidget(mCursorSpeedFastLabel);

    mCursorSpeedFrame->setLayout(CursorSpeedHLayout);

    /* TypingDisableFrame */
    mTypingDisableFrame = new QFrame(this);
    mTypingDisableFrame->setFrameShape(QFrame::Shape::Box);
    mTypingDisableFrame->setMinimumSize(550, 60);
    mTypingDisableFrame->setMaximumSize(16777215, 60);

    QHBoxLayout *TypingDisableHLayout = new QHBoxLayout();

    mTypingDisableBtn = new SwitchButton(this);
     //~ contents_path /touchpad/Disable touchpad when typing
    mTypingDisableLabel = new QLabel(tr("Disable touchpad when typing"), this);
    TypingDisableHLayout->addSpacing(7);
    TypingDisableHLayout->addWidget(mTypingDisableLabel);
    TypingDisableHLayout->addStretch();
    TypingDisableHLayout->addWidget(mTypingDisableBtn);

    mTypingDisableFrame->setLayout(TypingDisableHLayout);

    /* click */
    mClickFrame = new QFrame(this);
    mClickFrame->setFrameShape(QFrame::Shape::Box);
    mClickFrame->setMinimumSize(550, 60);
    mClickFrame->setMaximumSize(16777215, 60);

    QHBoxLayout *ClickHLayout = new QHBoxLayout();

    mClickBtn = new SwitchButton(this);
    //~ contents_path /touchpad/Touch and click on the touchpad
    mClickLabel = new QLabel(tr("Touch and click on the touchpad"), this);
    ClickHLayout->addSpacing(7);
    ClickHLayout->addWidget(mClickLabel);
    ClickHLayout->addStretch();
    ClickHLayout->addWidget(mClickBtn);

    mClickFrame->setLayout(ClickHLayout);

    /* ScrollSlideFrame */
    mScrollSlideFrame = new QFrame(this);
    mScrollSlideFrame->setFrameShape(QFrame::Shape::Box);
    mScrollSlideFrame->setMinimumSize(550, 60);
    mScrollSlideFrame->setMaximumSize(16777215, 60);

    QHBoxLayout *ScrollSlideHLayout = new QHBoxLayout();

    mScrollSlideBtn = new SwitchButton(this);
     //~ contents_path /touchpad/Scroll bar slides with finger
    mScrollSlideLabel = new QLabel(tr("Scroll bar slides with finger"), this);
    ScrollSlideHLayout->addSpacing(7);
    ScrollSlideHLayout->addWidget(mScrollSlideLabel);
    ScrollSlideHLayout->addStretch();
    ScrollSlideHLayout->addWidget(mScrollSlideBtn);

    mScrollSlideFrame->setLayout(ScrollSlideHLayout);

    /* ScrollAreaFrame */
    mScrollAreaFrame = new QFrame(this);
    mScrollAreaFrame->setFrameShape(QFrame::Shape::Box);
    mScrollAreaFrame->setMinimumSize(550, 60);
    mScrollAreaFrame->setMaximumSize(16777215, 60);

    QHBoxLayout *ScrollAreaHLayout = new QHBoxLayout();

     //~ contents_path /touchpad/Scrolling area
    mScrollAreaLabel = new QLabel(tr("Scrolling area"), this);
    mScrollTypeComBox = new QComboBox;
    mScrollTypeComBox->addItem(tr("Disable scrolling"), N_SCROLLING);
    mScrollTypeComBox->addItem(tr("Edge scrolling"), V_EDGE_KEY);
    mScrollTypeComBox->addItem(tr("Two-finger scrolling in the middle area"), V_FINGER_KEY);
    ScrollAreaHLayout->addSpacing(7);
    ScrollAreaHLayout->addWidget(mScrollAreaLabel);
    ScrollAreaHLayout->addWidget(mScrollTypeComBox);

    mScrollAreaFrame->setLayout(ScrollAreaHLayout);

    /* addwidget */
    QFrame *mouseAndSpeedLine = myLine();
    QFrame *speedAndTypingLine = myLine();
    QFrame *typingAndClickLine = myLine();
    QFrame *clickAndSlideLine = myLine();
    QFrame *slideAndAreaLine = myLine();

    touchpadLyt->addWidget(mMouseDisableFrame);
    touchpadLyt->addWidget(mouseAndSpeedLine);
    touchpadLyt->addWidget(mCursorSpeedFrame);
    touchpadLyt->addWidget(speedAndTypingLine);
    touchpadLyt->addWidget(mTypingDisableFrame);
    touchpadLyt->addWidget(typingAndClickLine);
    touchpadLyt->addWidget(mClickFrame);
    touchpadLyt->addWidget(clickAndSlideLine);
    touchpadLyt->addWidget(mScrollSlideFrame);
    touchpadLyt->addWidget(slideAndAreaLine);
    touchpadLyt->addWidget(mScrollAreaFrame);
    touchpadLyt->setSpacing(0);

    mVlayout->addWidget(mTouchpadSetTitleLabel);
    mVlayout->setSpacing(8);
    mVlayout->addWidget(touchpadFrame);
    mVlayout->addLayout(mTipHLayout);
    mVlayout->addStretch();
}

void TouchpadUI::initConnection()
{
    QByteArray touchpadId(kTouchpadSchemas);
    QByteArray mouseId(kMouseSchemas);

    if (QGSettings::isSchemaInstalled(touchpadId) && QGSettings::isSchemaInstalled(mouseId)) {
        mTouchpadGsetting = new QGSettings(kTouchpadSchemas, QByteArray(), this);
        mMouseGsetting = new QGSettings(kMouseSchemas, QByteArray(), this);

        isTouchpadExist();

        connect(mMouseDisableBtn, &SwitchButton::checkedChanged, this, &TouchpadUI::mouseDisableSlot);

        connect(mCursorSpeedSlider, &QSlider::valueChanged, this, &TouchpadUI::cursorSpeedSlot);

        connect(mTypingDisableBtn, &SwitchButton::checkedChanged, this, &TouchpadUI::typingDisableSlot);

        connect(mClickBtn, &SwitchButton::checkedChanged, this, &TouchpadUI::clickSlot);

        connect(mScrollSlideBtn, &SwitchButton::checkedChanged, this, &TouchpadUI::scrollSlideSlot);

        connect(mScrollTypeComBox, QOverload<const QString &>::of(&QComboBox::currentIndexChanged),
                this, &TouchpadUI::scrolltypeSlot);

        gsettingConnectUi();
    }
}

void TouchpadUI::gsettingConnectUi()
{
    //命令行或鼠标设置指针速度时（共用一个key）改变 key 值，界面做出相应的改变
    connect(mMouseGsetting, &QGSettings::changed, this, [=](const QString &key) {
       if (key == "motionAcceleration") {
           mCursorSpeedSlider->blockSignals(true);
           mCursorSpeedSlider->setValue(static_cast<int>(mMouseGsetting->get(kCursorSpeedKey).toDouble() * 100));
           mCursorSpeedSlider->blockSignals(false);
       }
    });
}

void TouchpadUI::initEnableStatus()
{
    // 插入鼠标时候禁用触摸板
    mMouseDisableBtn->blockSignals(true);
    mMouseDisableBtn->setChecked(mTouchpadGsetting->get(kMouseDisableKey).toBool());
    mMouseDisableBtn->blockSignals(false);

    // 初始化光标速度
    mCursorSpeedSlider->blockSignals(true);
    mCursorSpeedSlider->setValue(static_cast<int>(mMouseGsetting->get(kCursorSpeedKey).toDouble() * 100));
    mCursorSpeedSlider->blockSignals(false);

    // 初始化打字时禁用触摸板
    mTypingDisableBtn->blockSignals(true);
    mTypingDisableBtn->setChecked(mTouchpadGsetting->get(kTypingDisableKey).toBool());
    mTypingDisableBtn->blockSignals(false);

    // 初始化触摸板轻触点击
    mClickBtn->blockSignals(true);
    mClickBtn->setChecked(mTouchpadGsetting->get(kClickKey).toBool());
    mClickBtn->blockSignals(false);

    // 初始化滚动条跟随手指滑动
    mScrollSlideBtn->blockSignals(true);
    mScrollSlideBtn->setChecked(!mTouchpadGsetting->get(kScrollSlideKey).toBool());
    mScrollSlideBtn->blockSignals(false);

    //初始化滚动
    mScrollTypeComBox->blockSignals(true);
    mScrollTypeComBox->setCurrentIndex(mScrollTypeComBox->findData(_findKeyScrollingType()));
    mScrollTypeComBox->blockSignals(false);
}

void TouchpadUI::isTouchpadExist()
{
    if (findSynaptics()) {
        qDebug() << "Touch Devices Available";
        setFrameVisible(true);
        mTipLabel->hide();
        initEnableStatus();

        // 默认水平双指滚动有效
        mTouchpadGsetting->set(H_FINGER_KEY, true);
    } else {
        setFrameVisible(false);
    }
}

QString TouchpadUI::_findKeyScrollingType()
{
    if (mTouchpadGsetting->get(V_EDGE_KEY).toBool()) {
        return V_EDGE_KEY;
    }

    if (mTouchpadGsetting->get(V_FINGER_KEY).toBool()) {
        return V_FINGER_KEY;
    }

    return N_SCROLLING;
}

void TouchpadUI::setFrameVisible(bool visible)
{
    mMouseDisableFrame->setVisible(visible);
    mCursorSpeedFrame->setVisible(visible);
    mTypingDisableFrame->setVisible(visible);
    mClickFrame->setVisible(visible);
    mScrollSlideFrame->setVisible(visible);
    mScrollAreaFrame->setVisible(visible);
}

bool TouchpadUI::findSynaptics()
{
    XDeviceInfo *device_info;
    int n_devices;
    bool retval;

    if (_supportsXinputDevices() == false) {
        return true;
    }

    device_info = XListInputDevices (QX11Info::display(), &n_devices);
    if (device_info == nullptr) {
        return false;
    }

    retval = false;
    for (int i = 0; i < n_devices; i++) {
        XDevice *device;

        device = _deviceIsTouchpad (&device_info[i]);
        if (device != nullptr) {
            retval = true;
            break;
        }
    }
    if (device_info != nullptr) {
        XFreeDeviceList (device_info);
    }

    return retval;
}

bool TouchpadUI::_supportsXinputDevices()
{
    int op_code, event, error;

    return XQueryExtension (QX11Info::display(),
                            "XInputExtension",
                            &op_code,
                            &event,
                            &error);
}

XDevice* TouchpadUI::_deviceIsTouchpad (XDeviceInfo *deviceinfo)
{
    XDevice *device;
    if (deviceinfo->type != XInternAtom (QX11Info::display(), XI_TOUCHPAD, true)) {
        return nullptr;
    }

    device = XOpenDevice (QX11Info::display(), deviceinfo->id);
    if(device == nullptr) {
        qDebug()<<"device== null";
        return nullptr;
    }

    if (_deviceHasProperty(device, "libinput Tapping Enabled") ||
            _deviceHasProperty(device, "Synaptics Off")) {
        return device;
    }
    XCloseDevice (QX11Info::display(), device);
    return nullptr;
}

bool TouchpadUI::_deviceHasProperty(XDevice *device, const char *property_name)
{
    Atom realtype, prop;
    int realformat;
    unsigned long nitems, bytes_after;
    unsigned char *data;

    prop = XInternAtom (QX11Info::display(), property_name, True);
    if (!prop) {
        return false;
    }

    if ((XGetDeviceProperty (QX11Info::display(), device, prop, 0, 1, False,
                             XA_INTEGER, &realtype, &realformat, &nitems,
                             &bytes_after, &data) == Success) && (realtype != None))
    {
        XFree (data);
        return true;
    }
    return false;
}

/* slot functions */

void TouchpadUI::mouseDisableSlot(bool status)
{
    mTouchpadGsetting->set(kMouseDisableKey, status);
}

void TouchpadUI::cursorSpeedSlot(int value)
{
    mMouseGsetting->set(kCursorSpeedKey, static_cast<double>(value) / mCursorSpeedSlider->maximum() * 10);
}

void TouchpadUI::typingDisableSlot(bool status)
{
    mTouchpadGsetting->set(kTypingDisableKey, status);
}

void TouchpadUI::clickSlot(bool status)
{
    mTouchpadGsetting->set(kClickKey, status);
}

void TouchpadUI::scrollSlideSlot(bool status)
{
    mTouchpadGsetting->set(kScrollSlideKey, !status);
}

void TouchpadUI::scrolltypeSlot()
{
    //旧滚动类型设置为false,跳过N_SCROLLING
    QString oldType = _findKeyScrollingType();
    if (QString::compare(oldType, N_SCROLLING) != 0) {
        mTouchpadGsetting->set(oldType, false);
    }

    //新滚动类型设置为true,跳过N_SCROLLING
    QString data = mScrollTypeComBox->currentData().toString();
    if (QString::compare(data, N_SCROLLING) != 0) {
        mTouchpadGsetting->set(data, true);
    }
}


