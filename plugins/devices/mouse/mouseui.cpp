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
    mVlayout->setContentsMargins(0, 0, 32, 0);
    initUI();
    initConnection();
}

MouseUI::~MouseUI()
{
}

void MouseUI::initUI()
{
    mMouseTitleLabel = new TitleLabel(this);
    mMouseTitleLabel->setText(tr("Mouse"));

    setDominantHandFrame();
    setWheelSpeedFrame();
    setDoubleClickFrame();

    mPointerTitleLabel = new TitleLabel(this);
    mPointerTitleLabel->setText(tr("Pointer"));

    setPointerSpeedFrame();
    setMouseAccelerationFrame();
    setPointerPositionFrame();
    setPointerSizeFrame();

    mCursorTitleLabel = new TitleLabel(this);
    mCursorTitleLabel->setText(tr("Cursor"));

    setBlinkCursorFrame();
    setCursorSpeedFrame();

    /* add all frame to widget */
    mVlayout->addWidget(mMouseTitleLabel);
    mVlayout->addWidget(mDominantHandFrame);
    mVlayout->addWidget(mWheelSpeedFrame);
    mVlayout->addWidget(mDoubleClickIntervalFrame);
    mVlayout->addSpacing(32);
    mVlayout->setSpacing(8);

    mVlayout->addWidget(mPointerTitleLabel);
    mVlayout->addWidget(mPointerSpeedFrame);
    mVlayout->addWidget(mMouseAccelerationFrame);
    mVlayout->addWidget(mPointerPositionFrame);
    mVlayout->addWidget(mPointerSizeFrame);
    mVlayout->addSpacing(32);
    mVlayout->setSpacing(8);

    mVlayout->addWidget(mCursorTitleLabel);
    mVlayout->addWidget(mBlinkingCursorOnTextFrame);
    mVlayout->addWidget(mCursorSpeedFrame);
    mVlayout->addStretch();
}

void MouseUI::setDominantHandFrame()
{
    /* dominant hand Frame Setting */
    mDominantHandFrame = new QFrame(this);
    mDominantHandFrame->setFrameShape(QFrame::Shape::Box);
    mDominantHandFrame->setMinimumSize(550, 50);
    mDominantHandFrame->setMaximumSize(960, 50);

    QHBoxLayout *dominantHandHLayout = new QHBoxLayout();

    mDominantHandLabel = new QLabel(tr("Dominant hand"), this);
    mDominantHandLabel->setMinimumWidth(140);
    mDominantHandLeftRadio = new QRadioButton(tr("Left hand"), this);
    mDominantHandRightRadio = new QRadioButton(tr("Right hand"), this);
    mDominantHandLeftRadio->setProperty("dominatHand", "left-hand");
    mDominantHandRightRadio->setProperty("dominatHand", "right-hand");
    mDominantRadioGroup = new QButtonGroup(this);
    mDominantRadioGroup->addButton(mDominantHandLeftRadio);
    mDominantRadioGroup->addButton(mDominantHandRightRadio);
    dominantHandHLayout->addWidget(mDominantHandLabel);
    dominantHandHLayout->addWidget(mDominantHandLeftRadio);
    dominantHandHLayout->addWidget(mDominantHandRightRadio);

    mDominantHandFrame->setLayout(dominantHandHLayout);
}

void MouseUI::setWheelSpeedFrame()
{
    /* wheel speed Frame Setting */
    mWheelSpeedFrame = new QFrame(this);
    mWheelSpeedFrame->setFrameShape(QFrame::Shape::Box);
    mWheelSpeedFrame->setMinimumSize(550, 50);
    mWheelSpeedFrame->setMaximumSize(960, 50);

    QHBoxLayout *wheelSpeedHLayout = new QHBoxLayout();

    mWheelSpeedLabel = new QLabel(tr("Wheel speed"), this);
    mWheelSpeedLabel->setMinimumWidth(140);
    mWheelSpeedSlowLabel = new QLabel(tr("Slow"), this);
    mWheelSpeedSlowLabel->setMinimumWidth(40);
    mWheelSpeedSlider = new QSlider(Qt::Horizontal);
    mWheelSpeedSlider->setMinimum(1);
    mWheelSpeedSlider->setMaximum(30);
    mWheelSpeedSlider->setPageStep(3);
    mWheelSpeedFastLabel = new QLabel(tr("Fast"), this);
    mWheelSpeedFastLabel->setMinimumWidth(40);
    wheelSpeedHLayout->addWidget(mWheelSpeedLabel);
    wheelSpeedHLayout->addWidget(mWheelSpeedSlowLabel);
    wheelSpeedHLayout->addWidget(mWheelSpeedSlider);
    wheelSpeedHLayout->addWidget(mWheelSpeedFastLabel);

    mWheelSpeedFrame->setLayout(wheelSpeedHLayout);
}

void MouseUI::setDoubleClickFrame()
{
    /* Double Click Interval time Frame Setting */
    mDoubleClickIntervalFrame = new QFrame(this);
    mDoubleClickIntervalFrame->setFrameShape(QFrame::Shape::Box);
    mDoubleClickIntervalFrame->setMinimumSize(550, 50);
    mDoubleClickIntervalFrame->setMaximumSize(960, 50);

    QHBoxLayout *doubleClickIntervalHLayout = new QHBoxLayout();

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
    mDoubleClickIntervalLongLabel->setMinimumWidth(40);
    doubleClickIntervalHLayout->addWidget(mDoubleClickIntervalLabel);
    doubleClickIntervalHLayout->addWidget(mDoubleClickIntervalShortLabel);
    doubleClickIntervalHLayout->addWidget(mDoubleClickIntervalSlider);
    doubleClickIntervalHLayout->addWidget(mDoubleClickIntervalLongLabel);
    doubleClickIntervalHLayout->addWidget(new MyLabel());

    mDoubleClickIntervalFrame->setLayout(doubleClickIntervalHLayout);
}

void MouseUI::setPointerSpeedFrame()
{
    /* Pointer Speed Frame Setting */
    mPointerSpeedFrame = new QFrame(this);
    mPointerSpeedFrame->setFrameShape(QFrame::Shape::Box);
    mPointerSpeedFrame->setMinimumSize(550, 50);
    mPointerSpeedFrame->setMaximumSize(960, 50);

    QHBoxLayout *pointerSpeedHLayout = new QHBoxLayout();

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
    mPointerSpeedFastLabel->setMinimumWidth(40);
    pointerSpeedHLayout->addWidget(mPointerSpeedLabel);
    pointerSpeedHLayout->addWidget(mPointerSpeedSlowLabel);
    pointerSpeedHLayout->addWidget(mPointerSpeedSlider);
    pointerSpeedHLayout->addWidget(mPointerSpeedFastLabel);

    mPointerSpeedFrame->setLayout(pointerSpeedHLayout);
}

void MouseUI::setMouseAccelerationFrame()
{
    /* Mouse Acceleration Frame Setting */
    mMouseAccelerationFrame = new QFrame(this);
    mMouseAccelerationFrame->setFrameShape(QFrame::Shape::Box);
    mMouseAccelerationFrame->setMinimumSize(550, 50);
    mMouseAccelerationFrame->setMaximumSize(960, 50);

    QHBoxLayout *mouseAccelerationHLayout = new QHBoxLayout();

    mMouseAccelerationBtn = new SwitchButton(this);
    mMouseAccelerationLabel = new QLabel(tr("Mouse acceleration"), this);
    mMouseAccelerationLabel->setMinimumWidth(140);
    mouseAccelerationHLayout->addWidget(mMouseAccelerationLabel);
    mouseAccelerationHLayout->addStretch();
    mouseAccelerationHLayout->addWidget(mMouseAccelerationBtn);

    mMouseAccelerationFrame->setLayout(mouseAccelerationHLayout);
}

void MouseUI::setPointerPositionFrame()
{
    /* Pointer Position Frame Setting */
    mPointerPositionFrame = new QFrame(this);
    mPointerPositionFrame->setFrameShape(QFrame::Shape::Box);
    mPointerPositionFrame->setMinimumSize(550, 50);
    mPointerPositionFrame->setMaximumSize(960, 50);

    QHBoxLayout *PointerPositionHLayout = new QHBoxLayout();

    mPointerPositionLabel = new QLabel(tr("Show pointer position when pressing ctrl"), this);
    mPointerPositionLabel->setMinimumWidth(140);
    mPointerPositionBtn = new SwitchButton(this);
    PointerPositionHLayout->addWidget(mPointerPositionLabel);
    PointerPositionHLayout->addStretch();
    PointerPositionHLayout->addWidget(mPointerPositionBtn);

    mPointerPositionFrame->setLayout(PointerPositionHLayout);
}

void MouseUI::setPointerSizeFrame()
{
    /* Pointer Size Frame Setting */
    mPointerSizeFrame = new QFrame(this);
    mPointerSizeFrame->setFrameShape(QFrame::Shape::Box);
    mPointerSizeFrame->setMinimumSize(550, 50);
    mPointerSizeFrame->setMaximumSize(960, 50);

    QHBoxLayout *pointerSizeHLayout = new QHBoxLayout();

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
    pointerSizeHLayout->addWidget(mPointerSizeLabel);
    pointerSizeHLayout->addStretch();
    pointerSizeHLayout->addWidget(mPointerSizeSmallRadio);
    pointerSizeHLayout->addStretch();
    pointerSizeHLayout->addWidget(mPointerSizeMediumRadio);
    pointerSizeHLayout->addStretch();
    pointerSizeHLayout->addWidget(mPointerSizeLargeRadio);
    pointerSizeHLayout->addStretch();

    mPointerSizeFrame->setLayout(pointerSizeHLayout);
}

void MouseUI::setBlinkCursorFrame()
{
    /* Blinking Cursor On Text Frame Setting */
    mBlinkingCursorOnTextFrame = new QFrame(this);
    mBlinkingCursorOnTextFrame->setFrameShape(QFrame::Shape::Box);
    mBlinkingCursorOnTextFrame->setMinimumSize(550, 50);
    mBlinkingCursorOnTextFrame->setMaximumSize(960, 50);

    QHBoxLayout *blinkingCursorOnTextHLayout = new QHBoxLayout();

    mBlinkingCursorOnTextLabel = new QLabel(tr("Blinking cursor in text area"), this);
    mBlinkingCursorOnTextLabel->setMinimumWidth(140);
    mBlinkingCursorOnTextBtn = new SwitchButton(this);
    blinkingCursorOnTextHLayout->addWidget(mBlinkingCursorOnTextLabel);
    blinkingCursorOnTextHLayout->addStretch();
    blinkingCursorOnTextHLayout->addWidget(mBlinkingCursorOnTextBtn);

    mBlinkingCursorOnTextFrame->setLayout(blinkingCursorOnTextHLayout);
}

void MouseUI::setCursorSpeedFrame()
{
    /* Cursor Speed Frame Setting */
    mCursorSpeedFrame = new QFrame(this);
    mCursorSpeedFrame->setFrameShape(QFrame::Shape::Box);
    mCursorSpeedFrame->setMinimumSize(550, 50);
    mCursorSpeedFrame->setMaximumSize(960, 50);

    QHBoxLayout *cursorSpeedHLayout = new QHBoxLayout();

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
    mCursorSpeedFastlabel->setMinimumWidth(40);
    cursorSpeedHLayout->addWidget(mCursorSpeedlabel);
    cursorSpeedHLayout->addWidget(mCursorSpeedSlowlabel);
    cursorSpeedHLayout->addWidget(mCursorSpeedSlider);
    cursorSpeedHLayout->addWidget(mCursorSpeedFastlabel);

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

        connect(mWheelSpeedSlider, &QSlider::sliderReleased, this, &MouseUI::wheelSpeedSlot);

        connect(mDoubleClickIntervalSlider, &QSlider::sliderReleased , this, &MouseUI::doubleClickIntervalSlot);

        connect(mPointerSpeedSlider, &QSlider::valueChanged, this, &MouseUI::pointerSpeedSlot);

        connect(mMouseAccelerationBtn, &SwitchButton::checkedChanged, this, &MouseUI::mouseAccelerationSlot);

        connect(mPointerPositionBtn, &SwitchButton::checkedChanged, this, &MouseUI::pointerPositionSlot);

        connect(mPointerSizeRadioGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
                this, &MouseUI::pointerSizeSlot);

        connect(mBlinkingCursorOnTextBtn, &SwitchButton::checkedChanged, this, &MouseUI::blinkCursorOnTextSlot);

        connect(mCursorSpeedSlider, &QSlider::sliderReleased, this, &MouseUI::cursorSpeedSlot);

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
                mDominantHandLeftRadio->setChecked(true);
            } else {
                mDominantHandRightRadio->setChecked(true);
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
            } else if (PointerSize == 32) {
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
    //初始化惯用手
    mDominantRadioGroup->blockSignals(true);
    bool currentDominantHand = mMouseGsetting->get(kDominantHandKey).toBool();
    if (currentDominantHand == true) {
        mDominantHandLeftRadio->setChecked(true);
    } else {
        mDominantHandRightRadio->setChecked(true);
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
    } else if (currentPointerSize == 32) {
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
}


/* Slot */
void MouseUI::dominantHandSlot(QAbstractButton *button)
{
    QString dominantHand = button->property("dominatHand").toString();
    bool tmpLeftHand;

    if (QString::compare(dominantHand, "left-hand") == 0) {
        tmpLeftHand = true;
    } else {
        tmpLeftHand = false;
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
        tmpPointerSize = 32;
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
