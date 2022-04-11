#include "touchpadui.h"

TouchpadUI::TouchpadUI(QWidget *parent)
    : QWidget(parent)
{
    mVlayout = new QVBoxLayout(this);
    mVlayout->setContentsMargins(0, 0, 0, 0);
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
    mTouchpadSetTitleLabel->setContentsMargins(14, 0, 0, 0);

    /* MouseDisable */
    mMouseDisableFrame = new QFrame(this);
    mMouseDisableFrame->setFrameShape(QFrame::Shape::Box);
    mMouseDisableFrame->setMinimumSize(550, 60);
    mMouseDisableFrame->setMaximumSize(16777215, 60);

    QHBoxLayout *MouseDisableHLayout = new QHBoxLayout();

    mMouseDisableBtn = new SwitchButton(this);
    //~ contents_path /Touchpad/Disable touchpad when using the mouse
    mMouseDisableLabel = new QLabel(tr("Disable touchpad when using the mouse"), this);
    MouseDisableHLayout->addWidget(mMouseDisableLabel);
    MouseDisableHLayout->addStretch();
    MouseDisableHLayout->addWidget(mMouseDisableBtn);
    MouseDisableHLayout->setContentsMargins(12, 0, 14, 0);

    mMouseDisableFrame->setLayout(MouseDisableHLayout);

    /* CursorSpeed */
    mPointerSpeedFrame = new QFrame(this);
    mPointerSpeedFrame->setFrameShape(QFrame::Shape::Box);
    mPointerSpeedFrame->setMinimumSize(550, 60);
    mPointerSpeedFrame->setMaximumSize(16777215, 60);

    QHBoxLayout *pointerSpeedHLayout = new QHBoxLayout();

     //~ contents_path /Touchpad/Pointer Speed
    mPointerSpeedLabel = new QLabel(tr("Pointer Speed"), this);
    mPointerSpeedLabel->setMinimumWidth(200);
    mPointerSpeedSlowLabel = new QLabel(tr("Slow"), this);
    mPointerSpeedFastLabel = new QLabel(tr("Fast"), this);
    mPointerSpeedFastLabel->setContentsMargins(8, 0, 13, 0);
    mPointerSpeedSlider = new QSlider(Qt::Horizontal);
    mPointerSpeedSlider->setMinimum(10);
    mPointerSpeedSlider->setMaximum(1000);
    mPointerSpeedSlider->setSingleStep(50);
    mPointerSpeedSlider->setPageStep(50);
    pointerSpeedHLayout->addWidget(mPointerSpeedLabel);
    pointerSpeedHLayout->addWidget(mPointerSpeedSlowLabel);
    pointerSpeedHLayout->addWidget(mPointerSpeedSlider);
    pointerSpeedHLayout->addWidget(mPointerSpeedFastLabel);
    pointerSpeedHLayout->setContentsMargins(12, 0, 0, 0);

    mPointerSpeedFrame->setLayout(pointerSpeedHLayout);

    /* TypingDisableFrame */
    mTypingDisableFrame = new QFrame(this);
    mTypingDisableFrame->setFrameShape(QFrame::Shape::Box);
    mTypingDisableFrame->setMinimumSize(550, 60);
    mTypingDisableFrame->setMaximumSize(16777215, 60);

    QHBoxLayout *TypingDisableHLayout = new QHBoxLayout();

    mTypingDisableBtn = new SwitchButton(this);
     //~ contents_path /Touchpad/Disable touchpad when typing
    mTypingDisableLabel = new QLabel(tr("Disable touchpad when typing"), this);
    TypingDisableHLayout->addWidget(mTypingDisableLabel);
    TypingDisableHLayout->addStretch();
    TypingDisableHLayout->addWidget(mTypingDisableBtn);
    TypingDisableHLayout->setContentsMargins(12, 0, 14, 0);

    mTypingDisableFrame->setLayout(TypingDisableHLayout);

    /* click */
    mClickFrame = new QFrame(this);
    mClickFrame->setFrameShape(QFrame::Shape::Box);
    mClickFrame->setMinimumSize(550, 60);
    mClickFrame->setMaximumSize(16777215, 60);

    QHBoxLayout *ClickHLayout = new QHBoxLayout();

    mClickBtn = new SwitchButton(this);
    //~ contents_path /Touchpad/Touch and click on the touchpad
    mClickLabel = new QLabel(tr("Touch and click on the touchpad"), this);
    ClickHLayout->addWidget(mClickLabel);
    ClickHLayout->addStretch();
    ClickHLayout->addWidget(mClickBtn);
    ClickHLayout->setContentsMargins(12, 0, 14, 0);

    mClickFrame->setLayout(ClickHLayout);

    /* ScrollSlideFrame */
    mScrollSlideFrame = new QFrame(this);
    mScrollSlideFrame->setFrameShape(QFrame::Shape::Box);
    mScrollSlideFrame->setMinimumSize(550, 60);
    mScrollSlideFrame->setMaximumSize(16777215, 60);

    QHBoxLayout *ScrollSlideHLayout = new QHBoxLayout();

    mScrollSlideBtn = new SwitchButton(this);
     //~ contents_path /Touchpad/Scroll bar slides with finger
    mScrollSlideLabel = new QLabel(tr("Scroll bar slides with finger"), this);
    ScrollSlideHLayout->addWidget(mScrollSlideLabel);
    ScrollSlideHLayout->addStretch();
    ScrollSlideHLayout->addWidget(mScrollSlideBtn);
    ScrollSlideHLayout->setContentsMargins(12, 0, 14, 0);

    mScrollSlideFrame->setLayout(ScrollSlideHLayout);

    /* ScrollAreaFrame */
    mScrollAreaFrame = new QFrame(this);
    mScrollAreaFrame->setFrameShape(QFrame::Shape::Box);
    mScrollAreaFrame->setMinimumSize(550, 60);
    mScrollAreaFrame->setMaximumSize(16777215, 60);

    QHBoxLayout *ScrollAreaHLayout = new QHBoxLayout();

     //~ contents_path /Touchpad/Scrolling area
    mScrollAreaLabel = new QLabel(tr("Scrolling area"), this);
    mScrollTypeComBox = new QComboBox;
    mScrollTypeComBox->addItem(tr("Two-finger scrolling in the middle area"), V_FINGER_KEY);
    mScrollTypeComBox->addItem(tr("Edge scrolling"), V_EDGE_KEY);
    mScrollTypeComBox->addItem(tr("Disable scrolling"), N_SCROLLING);
    ScrollAreaHLayout->addWidget(mScrollAreaLabel);
    ScrollAreaHLayout->addWidget(mScrollTypeComBox);
    ScrollAreaHLayout->setContentsMargins(12, 0, 14, 0);

    mScrollAreaFrame->setLayout(ScrollAreaHLayout);

    /* addwidget */
    QFrame *mouseAndSpeedLine = myLine();
    QFrame *speedAndTypingLine = myLine();
    QFrame *typingAndClickLine = myLine();
    QFrame *clickAndSlideLine = myLine();
    QFrame *slideAndAreaLine = myLine();

    touchpadLyt->addWidget(mMouseDisableFrame);
    touchpadLyt->addWidget(mouseAndSpeedLine);
    touchpadLyt->addWidget(mPointerSpeedFrame);
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
    mVlayout->addStretch();
}

void TouchpadUI::initConnection()
{
    QByteArray touchpadId(kTouchpadSchemas);

    if (QGSettings::isSchemaInstalled(touchpadId)) {
        mTouchpadGsetting = new QGSettings(kTouchpadSchemas, QByteArray(), this);

        initEnableStatus();

        connect(mMouseDisableBtn, &SwitchButton::checkedChanged, this, &TouchpadUI::mouseDisableSlot);

        connect(mPointerSpeedSlider, &QSlider::valueChanged, this, &TouchpadUI::pointerSpeedSlot);

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
    //命令行指针速度改变 key 值，界面做出相应的改变
    connect(mTouchpadGsetting, &QGSettings::changed, this, [=](const QString &key) {
       if (key == "motionAcceleration") {
           mPointerSpeedSlider->blockSignals(true);
           mPointerSpeedSlider->setValue((kPointerSpeedMin + kPointerSpeedMax - mTouchpadGsetting->get(kPointerSpeedKey).toDouble()) * 100);
           mPointerSpeedSlider->blockSignals(false);
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
    mPointerSpeedSlider->blockSignals(true);
    mPointerSpeedSlider->setValue((kPointerSpeedMin + kPointerSpeedMax - mTouchpadGsetting->get(kPointerSpeedKey).toDouble()) * 100);
    mPointerSpeedSlider->blockSignals(false);

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

    // 边界滚动默认水平边界有效
    if (QString::compare(V_EDGE_KEY, mScrollTypeComBox->currentData().toString()) == 0) {
        mTouchpadGsetting->set(H_EDGE_KEY, true);
        mTouchpadGsetting->set(H_FINGER_KEY, false);
    }
    if (QString::compare(V_FINGER_KEY, mScrollTypeComBox->currentData().toString()) == 0) {
        mTouchpadGsetting->set(H_EDGE_KEY, false);
        mTouchpadGsetting->set(H_FINGER_KEY, true);
    }
    if (QString::compare(N_SCROLLING, mScrollTypeComBox->currentData().toString()) == 0) {
        mTouchpadGsetting->set(H_EDGE_KEY, false);
        mTouchpadGsetting->set(H_FINGER_KEY, false);
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

/* slot functions */
void TouchpadUI::mouseDisableSlot(bool status)
{
    mTouchpadGsetting->set(kMouseDisableKey, status);
}

void TouchpadUI::pointerSpeedSlot(int value)
{
    mTouchpadGsetting->set(kPointerSpeedKey, kPointerSpeedMin + kPointerSpeedMax-static_cast<double>(value) / mPointerSpeedSlider->maximum() * 10);
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
    //旧滚动类型设置为false，新滚动类型设置为true
    QString data = mScrollTypeComBox->currentData().toString();
    // 禁用滚动
    if (QString::compare(data, N_SCROLLING) == 0) {
        mTouchpadGsetting->set(V_EDGE_KEY, false);
        mTouchpadGsetting->set(H_EDGE_KEY, false);
        mTouchpadGsetting->set(V_FINGER_KEY, false);
        mTouchpadGsetting->set(H_FINGER_KEY, false);
    }
    // 边界滚动:垂直边界+水平边界
    if (QString::compare(data, V_EDGE_KEY) == 0) {
        mTouchpadGsetting->set(V_EDGE_KEY, true);
        mTouchpadGsetting->set(H_EDGE_KEY, true);
        mTouchpadGsetting->set(V_FINGER_KEY, false);
        mTouchpadGsetting->set(H_FINGER_KEY, false);
    }
    // 中间双指:垂直中间+水平中间
    if (QString::compare(data, V_FINGER_KEY) == 0) {
        mTouchpadGsetting->set(V_EDGE_KEY, false);
        mTouchpadGsetting->set(H_EDGE_KEY, false);
        mTouchpadGsetting->set(V_FINGER_KEY, true);
        mTouchpadGsetting->set(H_FINGER_KEY, true);
    }
}
