#include "mouseui.h"

MyLabel::MyLabel()
{
    setAttribute(Qt::WA_DeleteOnClose);

    QSizePolicy pSizePolicy = this->sizePolicy();
    pSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    pSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    this->setSizePolicy(pSizePolicy);

    setFixedSize(QSize(54, 28));
    setScaledContents(true);

    setPixmap(QPixmap(":/img/plugins/mouse/double-click-off.png"));

    const QByteArray id(kMouseSchemas);
    if (QGSettings::isSchemaInstalled(id)){
        mSettings = new QGSettings(id, QByteArray(), this);
    }
    this->setToolTip(tr("double-click to test"));
}

MyLabel::~MyLabel()
{
}

void MyLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    int delay = mSettings->get(kDoubleClickIntervalKey).toInt();
    setPixmap(QPixmap(":/img/plugins/mouse/double-click-on.png"));
    QTimer::singleShot(delay, this, [=]{
        setPixmap(QPixmap(":/img/plugins/mouse/double-click-off.png"));
    });
}

MouseUI::MouseUI(QWidget *parent)
    : QWidget(parent)
{
    mVlayout = new QVBoxLayout(this);
    mVlayout->setContentsMargins(0, 0, 0, 0);
    initUI();
    initConnection();
}

MouseUI::~MouseUI()
{
}

QFrame* MouseUI::myLine()
{
    QFrame *line = new QFrame(this);
    line->setMinimumSize(QSize(0, 1));
    line->setMaximumSize(QSize(16777215, 1));
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    return line;
}

void MouseUI::hideOnIntel()
{
    mDominantAndWheelLine->hide();
    mWheelSpeedFrame->hide();
    mWheelAndDoubleLine->hide();
    mDoubleClickIntervalFrame->hide();
    mPointerAndMouseLine->hide();
    mMouseAccelerationFrame->hide();
}

void MouseUI::initUI()
{
    //鼠标
    QFrame *mouseFrame = new QFrame(this);
    mouseFrame->setMinimumSize(550, 0);
    mouseFrame->setMaximumSize(16777215, 16777215);
    mouseFrame->setFrameShape(QFrame::Box);

    QVBoxLayout *mouseLyt = new QVBoxLayout(mouseFrame);
    mouseLyt->setContentsMargins(0, 0, 0, 0);

    //指针
    QFrame *pointerFrame = new QFrame(this);
    pointerFrame->setMinimumSize(550, 0);
    pointerFrame->setMaximumSize(16777215, 16777215);
    pointerFrame->setFrameShape(QFrame::Box);

    QVBoxLayout *pointerLyt = new QVBoxLayout(pointerFrame);
    pointerLyt->setContentsMargins(0, 0, 0, 0);

    //光标
    QFrame *cursorFrame = new QFrame(this);
    cursorFrame->setMinimumSize(550, 0);
    cursorFrame->setMaximumSize(16777215, 16777215);
    cursorFrame->setFrameShape(QFrame::Box);

    QVBoxLayout *cursorLyt = new QVBoxLayout(cursorFrame);
    cursorLyt->setContentsMargins(0, 0, 0, 0);

    mMouseTitleLabel = new TitleLabel(this);
    mMouseTitleLabel->setText(tr("Mouse"));
    mMouseTitleLabel->setContentsMargins(14,0,0,0);
    setDominantHandFrame();
    setWheelSpeedFrame();
    setDoubleClickFrame();

    mPointerTitleLabel = new TitleLabel(this);
    mPointerTitleLabel->setText(tr("Pointer"));
    mPointerTitleLabel->setContentsMargins(14,0,0,0);
    setPointerSpeedFrame();
    setMouseAccelerationFrame();
    setPointerPositionFrame();
    setPointerSizeFrame();

    mCursorTitleLabel = new TitleLabel(this);
    mCursorTitleLabel->setText(tr("Cursor"));
    mCursorTitleLabel->setContentsMargins(14,0,0,0);
    setBlinkCursorFrame();
    setCursorSpeedFrame();

    /* add all frame to widget */
    mDominantAndWheelLine =myLine();
    mWheelAndDoubleLine = myLine();
    mPointerAndMouseLine = myLine();
    mMouseAndPointerPositionLine = myLine();
    mPositionAndSizeLine = myLine();
    mBlinkAndCursorspeedLine = myLine();

    mouseLyt->addWidget(mDominantHandFrame);
    mouseLyt->addWidget(mDominantAndWheelLine);
    mouseLyt->addWidget(mWheelSpeedFrame);
    mouseLyt->addWidget(mWheelAndDoubleLine);
    mouseLyt->addWidget(mDoubleClickIntervalFrame);
    mouseLyt->setSpacing(0);

    pointerLyt->addWidget(mPointerSpeedFrame);
    pointerLyt->addWidget(mPointerAndMouseLine);
    pointerLyt->addWidget(mMouseAccelerationFrame);
    pointerLyt->addWidget(mMouseAndPointerPositionLine);
    pointerLyt->addWidget(mPointerPositionFrame);
    pointerLyt->addWidget(mPositionAndSizeLine);
    pointerLyt->addWidget(mPointerSizeFrame);
    pointerLyt->setSpacing(0);

    cursorLyt->addWidget(mBlinkingCursorOnTextFrame);
    cursorLyt->addWidget(mBlinkAndCursorspeedLine);
    cursorLyt->addWidget(mCursorSpeedFrame);
    cursorLyt->setSpacing(0);

    mVlayout->addWidget(mMouseTitleLabel);
    mVlayout->addWidget(mouseFrame);
    mVlayout->addSpacing(32);
    mVlayout->setSpacing(8);

    mVlayout->addWidget(mPointerTitleLabel);
    mVlayout->addWidget(pointerFrame);
    mVlayout->addSpacing(32);
    mVlayout->setSpacing(8);

    mVlayout->addWidget(mCursorTitleLabel);
    mVlayout->addWidget(cursorFrame);
    mVlayout->addStretch();
}

void MouseUI::setDominantHandFrame()
{
    /* dominant hand Frame Setting */
    mDominantHandFrame = new QFrame(this);
    mDominantHandFrame->setFrameShape(QFrame::Shape::Box);
    mDominantHandFrame->setMinimumSize(550, 60);
    mDominantHandFrame->setMaximumSize(16777215, 60);

    QHBoxLayout *dominantHandHLayout = new QHBoxLayout();

     //~ contents_path /Mouse/Dominant hand
    mDominantHandLabel = new QLabel(tr("Dominant hand"), this);
    mDominantHandLabel->setMinimumWidth(140);
    mDominantHandLeftRadio = new QRadioButton(tr("Left hand"), this);
    mDominantHandRightRadio = new QRadioButton(tr("Right hand"), this);
    mDominantHandLeftRadio->setProperty("dominatHand", "left-key");
    mDominantHandRightRadio->setProperty("dominatHand", "right-key");
    mDominantRadioGroup = new QButtonGroup(this);
    mDominantRadioGroup->addButton(mDominantHandLeftRadio);
    mDominantRadioGroup->addButton(mDominantHandRightRadio);
    dominantHandHLayout->addSpacing(4);
    dominantHandHLayout->addWidget(mDominantHandLabel);
    dominantHandHLayout->addWidget(mDominantHandLeftRadio);
    dominantHandHLayout->addSpacing(80);
    dominantHandHLayout->addWidget(mDominantHandRightRadio);
    dominantHandHLayout->addStretch();

    mDominantHandFrame->setLayout(dominantHandHLayout);
}

void MouseUI::setWheelSpeedFrame()
{
    /* wheel speed Frame Setting */
    mWheelSpeedFrame = new QFrame(this);
    mWheelSpeedFrame->setFrameShape(QFrame::Shape::Box);
    mWheelSpeedFrame->setMinimumSize(550, 60);
    mWheelSpeedFrame->setMaximumSize(16777215, 60);


    QHBoxLayout *wheelSpeedHLayout = new QHBoxLayout();

    //~ contents_path /Mouse/Wheel speed
    mWheelSpeedLabel = new QLabel(tr("Wheel speed"), this);
    mWheelSpeedLabel->setMinimumWidth(140);
    mWheelSpeedSlowLabel = new QLabel(tr("Slow"), this);
    mWheelSpeedSlowLabel->setMinimumWidth(40);
    QStringList wheelSpeed;
    wheelSpeed<< " " << " " << " " << " " <<" ";
    mWheelSpeedSlider = new Uslider(wheelSpeed);
    // 1=3行，2=6行，3=9行，4=12行，5=15行
    mWheelSpeedSlider->setRange(1,5);
    mWheelSpeedSlider->setTickInterval(1);
    mWheelSpeedSlider->setPageStep(1);
    mWheelSpeedFastLabel = new QLabel(tr("Fast"), this);
    mWheelSpeedFastLabel->setMinimumWidth(40);
    mWheelSpeedFastLabel->setContentsMargins(8, 0, 13, 0);
    wheelSpeedHLayout->addWidget(mWheelSpeedLabel);
    wheelSpeedHLayout->addWidget(mWheelSpeedSlowLabel);
    wheelSpeedHLayout->addWidget(mWheelSpeedSlider);
    wheelSpeedHLayout->addWidget(mWheelSpeedFastLabel);
    wheelSpeedHLayout->setContentsMargins(12, 0, 0, 0);

    mWheelSpeedFrame->setLayout(wheelSpeedHLayout);
}

void MouseUI::setDoubleClickFrame()
{
    /* Double Click Interval time Frame Setting */
    mDoubleClickIntervalFrame = new QFrame(this);
    mDoubleClickIntervalFrame->setFrameShape(QFrame::Shape::Box);
    mDoubleClickIntervalFrame->setMinimumSize(550, 60);
    mDoubleClickIntervalFrame->setMaximumSize(16777215, 60);

    QHBoxLayout *doubleClickIntervalHLayout = new QHBoxLayout();

    //~ contents_path /Mouse/Double-click interval time
    mDoubleClickIntervalLabel = new QLabel(tr("Double-click interval time"), this);
    mDoubleClickIntervalLabel->setMinimumWidth(140);
    mDoubleClickIntervalShortLabel = new QLabel(tr("Short"), this);
    mDoubleClickIntervalShortLabel->setMinimumWidth(40);
    mDoubleClickIntervalSlider = new QSlider(Qt::Horizontal);
    mDoubleClickIntervalSlider->setMinimum(170);
    mDoubleClickIntervalSlider->setMaximum(1000);
    mDoubleClickIntervalSlider->setSingleStep(100);
    mDoubleClickIntervalSlider->setPageStep(100);
    mDoubleClickIntervalLongLabel = new QLabel(tr("Long"), this);
    mDoubleClickIntervalLongLabel->setContentsMargins(8, 0, 0, 0);
    mDoubleClickIntervalLongLabel->setMinimumWidth(40);
    doubleClickIntervalHLayout->addWidget(mDoubleClickIntervalLabel);
    doubleClickIntervalHLayout->addWidget(mDoubleClickIntervalShortLabel);
    doubleClickIntervalHLayout->addWidget(mDoubleClickIntervalSlider);
    doubleClickIntervalHLayout->addWidget(mDoubleClickIntervalLongLabel);
    doubleClickIntervalHLayout->addWidget(new MyLabel());
    doubleClickIntervalHLayout->setContentsMargins(12, 0, 12, 0);

    mDoubleClickIntervalFrame->setLayout(doubleClickIntervalHLayout);
}

void MouseUI::setPointerSpeedFrame()
{
    /* Pointer Speed Frame Setting */
    mPointerSpeedFrame = new QFrame(this);
    mPointerSpeedFrame->setFrameShape(QFrame::Shape::Box);
    mPointerSpeedFrame->setMinimumSize(550, 60);
    mPointerSpeedFrame->setMaximumSize(16777215, 60);

    QHBoxLayout *pointerSpeedHLayout = new QHBoxLayout();

    //~ contents_path /Mouse/Pointer speed
    mPointerSpeedLabel = new QLabel(tr("Pointer speed"), this);
    mPointerSpeedLabel->setMinimumWidth(140);
    mPointerSpeedSlowLabel = new QLabel(tr("Slow"), this);
    mPointerSpeedSlowLabel->setMinimumWidth(40);
    mPointerSpeedSlider = new QSlider(Qt::Horizontal);
    mPointerSpeedSlider->setMinimum(100);
    mPointerSpeedSlider->setMaximum(1000);
    mPointerSpeedSlider->setSingleStep(50);
    mPointerSpeedSlider->setPageStep(50);
    mPointerSpeedFastLabel = new QLabel(tr("Fast"), this);
    mPointerSpeedFastLabel->setContentsMargins(8, 0, 13, 0);
    mPointerSpeedFastLabel->setMinimumWidth(40);
    pointerSpeedHLayout->addWidget(mPointerSpeedLabel);
    pointerSpeedHLayout->addWidget(mPointerSpeedSlowLabel);
    pointerSpeedHLayout->addWidget(mPointerSpeedSlider);
    pointerSpeedHLayout->addWidget(mPointerSpeedFastLabel);
    pointerSpeedHLayout->setContentsMargins(12, 0, 0, 0);

    mPointerSpeedFrame->setLayout(pointerSpeedHLayout);
}

void MouseUI::setMouseAccelerationFrame()
{
    /* Mouse Acceleration Frame Setting */
    mMouseAccelerationFrame = new QFrame(this);
    mMouseAccelerationFrame->setFrameShape(QFrame::Shape::Box);
    mMouseAccelerationFrame->setMinimumSize(550, 60);
    mMouseAccelerationFrame->setMaximumSize(16777215, 60);

    QHBoxLayout *mouseAccelerationHLayout = new QHBoxLayout();

    mMouseAccelerationBtn = new SwitchButton(this);
    //~ contents_path /Mouse/Mouse acceleration
    mMouseAccelerationLabel = new QLabel(tr("Mouse acceleration"), this);
    mMouseAccelerationLabel->setMinimumWidth(140);
    mouseAccelerationHLayout->addWidget(mMouseAccelerationLabel);
    mouseAccelerationHLayout->addStretch();
    mouseAccelerationHLayout->addWidget(mMouseAccelerationBtn);
    mouseAccelerationHLayout->setContentsMargins(12, 0, 14, 0);

    mMouseAccelerationFrame->setLayout(mouseAccelerationHLayout);
}

void MouseUI::setPointerPositionFrame()
{
    /* Pointer Position Frame Setting */
    mPointerPositionFrame = new QFrame(this);
    mPointerPositionFrame->setFrameShape(QFrame::Shape::Box);
    mPointerPositionFrame->setMinimumSize(550, 60);
    mPointerPositionFrame->setMaximumSize(16777215, 60);

    QHBoxLayout *PointerPositionHLayout = new QHBoxLayout();

    //~ contents_path /Mouse/Show pointer position when pressing ctrl
    mPointerPositionLabel = new QLabel(tr("Show pointer position when pressing ctrl"), this);
    mPointerPositionLabel->setMinimumWidth(140);
    mPointerPositionBtn = new SwitchButton(this);
    PointerPositionHLayout->addWidget(mPointerPositionLabel);
    PointerPositionHLayout->addStretch();
    PointerPositionHLayout->addWidget(mPointerPositionBtn);
    PointerPositionHLayout->setContentsMargins(12, 0, 14, 0);

    mPointerPositionFrame->setLayout(PointerPositionHLayout);
}

void MouseUI::setPointerSizeFrame()
{
    /* Pointer Size Frame Setting */
    mPointerSizeFrame = new QFrame(this);
    mPointerSizeFrame->setFrameShape(QFrame::Shape::Box);
    mPointerSizeFrame->setMinimumSize(550, 60);
    mPointerSizeFrame->setMaximumSize(16777215, 60);

    QHBoxLayout *pointerSizeHLayout = new QHBoxLayout();

     //~ contents_path /Mouse/Pointer size
    mPointerSizeLabel = new QLabel(tr("Pointer size"),this);
    mPointerSizeLabel->setMinimumWidth(140);
    mPointerSizeSmallRadio = new QRadioButton(tr("Small(recommend)"), this);
    mPointerSizeMediumRadio = new QRadioButton(tr("Medium"), this);
    mPointerSizeLargeRadio = new QRadioButton(tr("Large"), this);
    mPointerSizeSmallRadio->setProperty("pointer", "Small");
    mPointerSizeMediumRadio->setProperty("pointer", "Medium");
    mPointerSizeLargeRadio->setProperty("pointer", "Large");
    mPointerSizeRadioGroup = new QButtonGroup(this);
    mPointerSizeRadioGroup->addButton(mPointerSizeSmallRadio);
    mPointerSizeRadioGroup->addButton(mPointerSizeMediumRadio);
    mPointerSizeRadioGroup->addButton(mPointerSizeLargeRadio);
    pointerSizeHLayout->addSpacing(4);
    pointerSizeHLayout->addWidget(mPointerSizeLabel);
    pointerSizeHLayout->addWidget(mPointerSizeSmallRadio);
    pointerSizeHLayout->addSpacing(50);
    pointerSizeHLayout->addWidget(mPointerSizeMediumRadio);
    pointerSizeHLayout->addSpacing(50);
    pointerSizeHLayout->addWidget(mPointerSizeLargeRadio);
    pointerSizeHLayout->addStretch();

    mPointerSizeFrame->setLayout(pointerSizeHLayout);
}

void MouseUI::setBlinkCursorFrame()
{
    /* Blinking Cursor On Text Frame Setting */
    mBlinkingCursorOnTextFrame = new QFrame(this);
    mBlinkingCursorOnTextFrame->setFrameShape(QFrame::Shape::Box);
    mBlinkingCursorOnTextFrame->setMinimumSize(550, 60);
    mBlinkingCursorOnTextFrame->setMaximumSize(16777215, 60);

    QHBoxLayout *blinkingCursorOnTextHLayout = new QHBoxLayout();

     //~ contents_path /Mouse/Blinking cursor in text area
    mBlinkingCursorOnTextLabel = new QLabel(tr("Blinking cursor in text area"), this);
    mBlinkingCursorOnTextLabel->setMinimumWidth(140);
    mBlinkingCursorOnTextBtn = new SwitchButton(this);
    blinkingCursorOnTextHLayout->addWidget(mBlinkingCursorOnTextLabel);
    blinkingCursorOnTextHLayout->addStretch();
    blinkingCursorOnTextHLayout->addWidget(mBlinkingCursorOnTextBtn);
    blinkingCursorOnTextHLayout->setContentsMargins(12, 0, 14, 0);

    mBlinkingCursorOnTextFrame->setLayout(blinkingCursorOnTextHLayout);
}

void MouseUI::setCursorSpeedFrame()
{
    /* Cursor Speed Frame Setting */
    mCursorSpeedFrame = new QFrame(this);
    mCursorSpeedFrame->setFrameShape(QFrame::Shape::Box);
    mCursorSpeedFrame->setMinimumSize(550, 60);
    mCursorSpeedFrame->setMaximumSize(16777215, 60);

    QHBoxLayout *cursorSpeedHLayout = new QHBoxLayout();

    //~ contents_path /Mouse/Cursor speed
    mCursorSpeedlabel = new QLabel(tr("Cursor speed"), this);
    mCursorSpeedlabel->setMinimumWidth(140);
    mCursorSpeedSlowlabel = new QLabel(tr("Slow"), this);
    mCursorSpeedSlowlabel->setMinimumWidth(40);
    mCursorSpeedSlider = new QSlider(Qt::Horizontal);
    mCursorSpeedSlider->setMinimum(100);
    mCursorSpeedSlider->setMaximum(2500);
    mCursorSpeedSlider->setSingleStep(200);
    mCursorSpeedSlider->setPageStep(200);
    mCursorSpeedFastlabel = new QLabel(tr("Fast"), this);
    mCursorSpeedFastlabel->setContentsMargins(8, 0, 13, 0);
    mCursorSpeedFastlabel->setMinimumWidth(40);
    cursorSpeedHLayout->addWidget(mCursorSpeedlabel);
    cursorSpeedHLayout->addWidget(mCursorSpeedSlowlabel);
    cursorSpeedHLayout->addWidget(mCursorSpeedSlider);
    cursorSpeedHLayout->addWidget(mCursorSpeedFastlabel);
    cursorSpeedHLayout->setContentsMargins(12, 0, 0, 0);

    mCursorSpeedFrame->setLayout(cursorSpeedHLayout);
}

void MouseUI::initConnection()
{
    QByteArray mouseId(kMouseSchemas);
    QByteArray desktopId(kDesktopSchemas);
    QByteArray themeId(kThemeSchemas);
    if (QGSettings::isSchemaInstalled(mouseId) && QGSettings::isSchemaInstalled(desktopId)) {
        mMouseGsetting = new QGSettings(mouseId, QByteArray(), this);
        mDesktopGsetting = new QGSettings(desktopId, QByteArray(), this);
        mThemeSettings = new QGSettings(themeId, QByteArray(), this);

        initEnableStatus();

        connect(mDominantRadioGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
                this, &MouseUI::dominantHandSlot);

        connect(mWheelSpeedSlider, &QSlider::valueChanged, this, &MouseUI::wheelSpeedSlot);

        connect(mDoubleClickIntervalSlider, &QSlider::valueChanged , this, &MouseUI::doubleClickIntervalSlot);

        connect(mPointerSpeedSlider, &QSlider::valueChanged, this, &MouseUI::pointerSpeedSlot);

        connect(mMouseAccelerationBtn, &SwitchButton::checkedChanged, this, &MouseUI::mouseAccelerationSlot);

        connect(mPointerPositionBtn, &SwitchButton::checkedChanged, this, &MouseUI::pointerPositionSlot);

        connect(mPointerSizeRadioGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
                this, &MouseUI::pointerSizeSlot);

        connect(mBlinkingCursorOnTextBtn, &SwitchButton::checkedChanged, this, &MouseUI::blinkCursorOnTextSlot);

        connect(mCursorSpeedSlider, &QSlider::valueChanged, this, &MouseUI::cursorSpeedSlot);

        gsettingConnection();
    }
}

void MouseUI::gsettingConnection()
{
    // 命令行修改 gsetting 的 key 值图形界面做出相应改变
    connect(mMouseGsetting,&QGSettings::changed,[=] (const QString &key){
        if(key == "leftHanded") {
            int handHabit = mMouseGsetting->get(kDominantHandKey).toBool();
            if (handHabit == true) {
                mDominantHandRightRadio->setChecked(true);
            } else {
                mDominantHandLeftRadio->setChecked(true);
            }
        } else if(key == "wheelSpeed") {
            mWheelSpeedSlider->setValue(mMouseGsetting->get(kWheelSpeedKey).toInt());
        } else if(key == "doubleClick") {
            int dc = mMouseGsetting->get(kDoubleClickIntervalKey).toInt();
            mDoubleClickIntervalSlider->blockSignals(true);
            mDoubleClickIntervalSlider->setValue(dc);
            mDoubleClickIntervalSlider->blockSignals(false);
        } else if(key == "motionAcceleration") {
            mPointerSpeedSlider->blockSignals(true);
            mPointerSpeedSlider->setValue(static_cast<int>(mMouseGsetting->get(kPointerSpeedKey).toDouble()*100));
            mPointerSpeedSlider->blockSignals(false);
        } else if(key == "mouseAccel") {
            mMouseAccelerationBtn->blockSignals(true);
            mMouseAccelerationBtn->setChecked(mMouseGsetting->get(kMouseAccelerationKey).toBool());
            mMouseAccelerationBtn->blockSignals(false);
        } else if(key == "locatePointer") {
            mPointerPositionBtn->blockSignals(true);
            mPointerPositionBtn->setChecked(mMouseGsetting->get(kPointerPositionKey).toBool());
            mPointerPositionBtn->blockSignals(false);
        } else if(key == "cursorSize") {
            int PointerSize = mMouseGsetting->get(kPointerSizeKey).toInt();
            if (PointerSize == 24) {
                mPointerSizeSmallRadio->setChecked(true);
            } else if (PointerSize == 36) {
                mPointerSizeMediumRadio->setChecked(true);
            } else if (PointerSize == 48) {
                mPointerSizeLargeRadio->setChecked(true);
            }
        }
    });

    connect(mDesktopGsetting,&QGSettings::changed,[=](const QString &key) {
        if (key == "cursorBlink") {
            mBlinkingCursorOnTextBtn->blockSignals(true);
            mBlinkingCursorOnTextBtn->setChecked(mDesktopGsetting->get(kBlinkCursorOnTextKey).toBool());
            mCursorSpeedFrame->setVisible(mDesktopGsetting->get(kBlinkCursorOnTextKey).toBool());
            mBlinkingCursorOnTextBtn->blockSignals(false);
        }else if (key == "cursorBlinkTime") {
            mCursorSpeedSlider->blockSignals(true);
            int mValue = mCursorSpeedSlider->maximum() - mDesktopGsetting->get(kCursorSpeedKey).toInt() + mCursorSpeedSlider->minimum();
            mCursorSpeedSlider->setValue(mValue);
            mCursorSpeedSlider->blockSignals(false);
        }
    });
}

void MouseUI::initEnableStatus()
{
    //初始化惯用手, 左手：右键为主键，右手：左键为主键
    mDominantRadioGroup->blockSignals(true);
    bool currentDominantHand = mMouseGsetting->get(kDominantHandKey).toBool();
    if (currentDominantHand == true) {
        mDominantHandRightRadio->setChecked(true);
    } else {
        mDominantHandLeftRadio->setChecked(true);
    }
    mDominantRadioGroup->blockSignals(false);

    //初始化滚轮速度
    mWheelSpeedSlider->blockSignals(true);
    mWheelSpeedSlider->setValue(mMouseGsetting->get(kWheelSpeedKey).toInt());
    mWheelSpeedSlider->blockSignals(false);

    //初始化双击间隔时长
    mDoubleClickIntervalSlider->blockSignals(true);
    mDoubleClickIntervalSlider->setValue(mMouseGsetting->get(kDoubleClickIntervalKey).toInt());
    mDoubleClickIntervalSlider->blockSignals(false);

    //初始化指针速度
    mPointerSpeedSlider->blockSignals(true);
    mPointerSpeedSlider->setValue(static_cast<int>(mMouseGsetting->get(kPointerSpeedKey).toDouble()*100));
    mPointerSpeedSlider->blockSignals(false);

    //初始化鼠标加速
    mMouseAccelerationBtn->blockSignals(true);
    mMouseAccelerationBtn->setChecked(mMouseGsetting->get(kMouseAccelerationKey).toBool());
    mMouseAccelerationBtn->blockSignals(false);

    //初始化按 ctrl 键时显示指针位置
    mPointerPositionBtn->blockSignals(true);
    mPointerPositionBtn->setChecked(mMouseGsetting->get(kPointerPositionKey).toBool());
    mPointerPositionBtn->blockSignals(false);

    //初始化指针大小
    mPointerSizeRadioGroup->blockSignals(true);
    int currentPointerSize = mMouseGsetting->get(kPointerSizeKey).toInt();
    if (currentPointerSize == 24) {
        mPointerSizeSmallRadio->setChecked(true);
    } else if (currentPointerSize == 36) {
        mPointerSizeMediumRadio->setChecked(true);
    } else if (currentPointerSize == 48) {
        mPointerSizeLargeRadio->setChecked(true);
    }
    mPointerSizeRadioGroup->blockSignals(false);

    //初始化文本区域光标闪烁
    mBlinkingCursorOnTextBtn->blockSignals(true);
    mBlinkingCursorOnTextBtn->setChecked(mDesktopGsetting->get(kBlinkCursorOnTextKey).toBool());
    mBlinkingCursorOnTextBtn->blockSignals(false);

    //初始化光标速度
    mCursorSpeedSlider->blockSignals(true);
    mCursorSpeedSlider->setValue(mCursorSpeedSlider->maximum() - mDesktopGsetting->get(kCursorSpeedKey).toInt()
                                 + mCursorSpeedSlider->minimum());
    mCursorSpeedSlider->blockSignals(false);
    mCursorSpeedFrame->setVisible(mDesktopGsetting->get(kBlinkCursorOnTextKey).toBool());
}


/* Slot */
void MouseUI::dominantHandSlot(QAbstractButton *button)
{
    QString dominantHand = button->property("dominatHand").toString();
    bool tmpLeftHand;

    // 左键主键：右手、left-handed=false，右键主键：左手、left-handed=true
    if (QString::compare(dominantHand, "left-key") == 0) {
        tmpLeftHand = false;
    } else {
        tmpLeftHand = true;
    }

    mMouseGsetting->set(kDominantHandKey, tmpLeftHand);
}

void MouseUI::wheelSpeedSlot()
{
    mMouseGsetting->set(kWheelSpeedKey, mWheelSpeedSlider->value());
}

void MouseUI::doubleClickIntervalSlot()
{
    mMouseGsetting->set(kDoubleClickIntervalKey, mDoubleClickIntervalSlider->value());
    qApp->setDoubleClickInterval(mDoubleClickIntervalSlider->value());
}

void MouseUI::pointerSpeedSlot(int value)
{
    mMouseGsetting->set(kPointerSpeedKey, static_cast<double>(value)/mPointerSpeedSlider->maximum() * 10);
}

void MouseUI::mouseAccelerationSlot(bool checked)
{
    mMouseGsetting->set(kMouseAccelerationKey, checked);
}

void MouseUI::pointerPositionSlot(bool checked)
{
    mMouseGsetting->set(kPointerPositionKey, checked);
}

void MouseUI::pointerSizeSlot(QAbstractButton *button)
{
    QString pointerSize = button->property("pointer").toString();
    int tmpPointerSize;

    if (QString::compare(pointerSize, "Small") == 0) {
        tmpPointerSize = 24;
    } else if (QString::compare(pointerSize, "Medium") == 0) {
        tmpPointerSize = 36;
    } else if (QString::compare(pointerSize, "Large") == 0) {
        tmpPointerSize = 48;
    }

    mMouseGsetting->set(kPointerSizeKey, tmpPointerSize);
}

void MouseUI::blinkCursorOnTextSlot(bool checked)
{
    mCursorSpeedFrame->setVisible(checked);
    mDesktopGsetting->set(kBlinkCursorOnTextKey, checked);
    mThemeSettings->set(kBlinkCursorOnTextKey, checked);

    if (!checked) {
        mThemeSettings->set(kCursorSpeedKey, 0);
    } else {
        int mValue = mCursorSpeedSlider->maximum() - mCursorSpeedSlider->value() + mCursorSpeedSlider->minimum();
        mThemeSettings->set(kCursorSpeedKey, mValue);
    }
}

void MouseUI::cursorSpeedSlot()
{
    mDesktopGsetting->set(kCursorSpeedKey, mCursorSpeedSlider->maximum() - mCursorSpeedSlider->value()
                          + mCursorSpeedSlider->minimum());
    mThemeSettings->set(kCursorSpeedKey, mCursorSpeedSlider->maximum() - mCursorSpeedSlider->value()
                        + mCursorSpeedSlider->minimum());
}
