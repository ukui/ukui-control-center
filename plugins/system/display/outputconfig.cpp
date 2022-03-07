#include "outputconfig.h"
#include "resolutionslider.h"
#include "utils.h"
#include "scalesize.h"

#include <QStringBuilder>
#include <QFormLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QMessageBox>

#include <QComboBox>
#include <QGSettings>
#include <QDBusInterface>

#include <KF5/KScreen/kscreen/output.h>
#include <KF5/KScreen/kscreen/edid.h>

#include "ComboBox/combobox.h"
#include "shell/utils/utils.h"

OutputConfig::OutputConfig(QWidget *parent) :
    QWidget(parent),
    mOutput(nullptr)
{

}

OutputConfig::OutputConfig(const KScreen::OutputPtr &output, QWidget *parent) :
    QWidget(parent)
{
    setOutput(output);
}

OutputConfig::~OutputConfig()
{
}

void OutputConfig::setTitle(const QString &title)
{
    mTitle->setText(title);
}

void OutputConfig::initUi()
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(2);

    // 分辨率下拉框
    mResolution = new ResolutionSlider(mOutput, this);
    mResolution->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QLabel *resLabel = new QLabel(this);
    //~ contents_path /display/resolution
    resLabel->setText(tr("resolution"));
    resLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    resLabel->setFixedSize(118, 30);

    QHBoxLayout *resLayout = new QHBoxLayout();
    resLayout->addWidget(resLabel);
    resLayout->addWidget(mResolution);

    QFrame *resFrame = new QFrame(this);
    resFrame->setFrameShape(QFrame::Shape::Box);
    resFrame->setLayout(resLayout);

    resFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    resFrame->setMinimumSize(552, 50);
    resFrame->setMaximumSize(960, 50);

    vbox->addWidget(resFrame);

    connect(mResolution, &ResolutionSlider::resolutionChanged,
            this, [=](QSize size, bool emitFlag){
                slotResolutionChanged(size, emitFlag);
            });

    // 方向下拉框
    mRotation = new QComboBox(this);
    mRotation->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QLabel *rotateLabel = new QLabel(this);
    //~ contents_path /display/orientation
    rotateLabel->setText(tr("orientation"));
    rotateLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    rotateLabel->setFixedSize(118, 30);

    QHBoxLayout *rotateLayout = new QHBoxLayout();
    rotateLayout->addWidget(rotateLabel);

    rotateLayout->addWidget(mRotation);

    mRotateFrame = new QFrame(this);
    mRotateFrame->setFrameShape(QFrame::Shape::Box);
    mRotateFrame->setLayout(rotateLayout);

    mRotateFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    mRotateFrame->setMinimumSize(550, 50);
    mRotateFrame->setMaximumSize(960, 50);

    mRotation->addItem(tr("arrow-up"), KScreen::Output::None);
    mRotation->addItem(tr("90° arrow-right"), KScreen::Output::Right);
    mRotation->addItem(tr("90° arrow-left"), KScreen::Output::Left);
    mRotation->addItem(tr("arrow-down"), KScreen::Output::Inverted);
    connect(mRotation, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &OutputConfig::slotRotationChanged);
    mRotation->setCurrentIndex(mRotation->findData(mOutput->rotation()));

    vbox->addWidget(mRotateFrame);

    // 刷新率下拉框
    mRefreshRate = new QComboBox(this);

    QLabel *freshLabel = new QLabel(this);
    //~ contents_path /display/frequency
    freshLabel->setText(tr("frequency"));
    freshLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    freshLabel->setFixedSize(118, 30);

    QHBoxLayout *freshLayout = new QHBoxLayout();
    freshLayout->addWidget(freshLabel);
    freshLayout->addWidget(mRefreshRate);

    QFrame *freshFrame = new QFrame(this);
    freshFrame->setFrameShape(QFrame::Shape::Box);
    freshFrame->setLayout(freshLayout);

    freshFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    freshFrame->setMinimumSize(550, 50);
    freshFrame->setMaximumSize(960, 50);

    vbox->addWidget(freshFrame);

    slotResolutionChanged(mResolution->currentResolution(), true);
    connect(mRefreshRate, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &OutputConfig::slotRefreshRateChanged);

    initConnection();
    slotEnableWidget();
}

double OutputConfig::getScreenScale()
{
    double scale = 1.0;
    if (QGSettings::isSchemaInstalled(SCALE_SCHEMAS)) {
        if (mDpiSettings->keys().contains("scalingFactor")) {
            scale = mDpiSettings->get(SCALE_KEY).toDouble();
        }
    }
    return scale;
}

void OutputConfig::initConnection()
{
    connect(mOutput.data(), &KScreen::Output::isConnectedChanged,
            this, [=]() {
        if (!mOutput->isConnected()) {
            setVisible(false);
        }
    });

    connect(mOutput.data(), &KScreen::Output::rotationChanged,
            this, [=]() {
        const int index = mRotation->findData(mOutput->rotation());
        mRotation->blockSignals(true);
        mRotation->setCurrentIndex(index);
        mRotation->blockSignals(false);
    });

    connect(mOutput.data(), &KScreen::Output::currentModeIdChanged,
            this, [=]() {
        if (mOutput->currentMode()) {
            if (mRefreshRate && mOutput->currentMode()->size() != mResolution->currentResolution()) {
                slotResolutionChanged(mOutput->currentMode()->size(), false);
                mResolution->setResolution(mOutput->currentMode()->size());
            } else if (mRefreshRate &&
                       mOutput->currentMode()->size() == mResolution->currentResolution() &&
                       tr("%1 Hz").arg(QLocale().toString(qRound(mOutput->currentMode()->refreshRate()))) != mRefreshRate->itemText(mRefreshRate->currentIndex())) {
                /* 修改一次刷新率会触发三次currentModeIdChanged(不确定是否所有机型均会如此，因此这里添加保护)
                 * 例如从50HZ  ->  60HZ,mRefreshRate显示的为60HZ
                 * 第一次Mode获取到的为60HZ，不会进入此条件
                 * 第二次Mode获取到的为50HZ，会进入此条件，但mIsManualForRefreshRate为true，故不会修改mRefreshRate，并将mIsManualForRefreshRate置为false
                 * 第三次Mode获取到的为60HZ，不会进入此条件
                 * 综上可以保证手动修改刷新率时，不会去修改mRefreshRate的显示文本，并且将mIsManualForRefreshRate重新置为了false
                 * 这样可以保证修改刷新率然后选择不保存时，能够修改mRefreshRate的显示文本
                */
                if (!mIsManualForRefreshRate) {
                    mRefreshRate->blockSignals(true);
                    int dataNum = mRefreshRate->findText(tr("%1 Hz").arg(QLocale().toString(qRound(mOutput->currentMode()->refreshRate()))));
                    if (dataNum >= 0)
                        mRefreshRate->setCurrentIndex(dataNum);
                    mRefreshRate->blockSignals(false);
                } else {
                    mIsManualForRefreshRate = false;
                }
            }
        }
    });

    connect(mOutput.data(), &KScreen::Output::isEnabledChanged, this, [=](){
       slotEnableWidget();
    });
}

void OutputConfig::hideComponent()
{
    mRotateFrame->setVisible(Utils::isDell());
}

void OutputConfig::setOutput(const KScreen::OutputPtr &output)
{
    mOutput = output;
    initUi();
    hideComponent();
}

KScreen::OutputPtr OutputConfig::output() const
{
    return mOutput;
}

void OutputConfig::slotResolutionChanged(const QSize &size, bool emitFlag)
{
    // Ignore disconnected outputs
    if (!size.isValid()) {
        return;
    }
    bool mIsModeInit = false;
    QString modeID;
    KScreen::ModePtr selectMode;
    KScreen::ModePtr currentMode = mOutput->currentMode();
    QList<KScreen::ModePtr> modes;

    Q_FOREACH (const KScreen::ModePtr &mode, mOutput->modes()) {
        //初始化时,currentMode可能为空(比如刚插上屏幕)
        if (!currentMode || (currentMode && currentMode->size() == size)) {
            if (currentMode) {
                selectMode = currentMode;
            }
            mIsModeInit = true;
        }
        if (mode->size() == size) {
            if (!mIsModeInit || !currentMode) {
                selectMode = mode;
            }
            modes << mode;
        }
    }

    modeID = selectMode->id();
    mRefreshRate->blockSignals(true);
    // Don't remove the first "Auto" item - prevents ugly flicker of the combobox
    // when changing resolution
    mRefreshRate->clear();

    for (int i = 0, total = modes.count(); i < total; ++i) {
        const KScreen::ModePtr mode = modes.at(i);

        bool alreadyExisted = false;
        for (int j = 0; j < mRefreshRate->count(); ++j) {
            if (tr("%1 Hz").arg(QLocale().toString(qRound(mode->refreshRate()))) == mRefreshRate->itemText(j)) {
                alreadyExisted = true;
                break;
            }
        }
        if (alreadyExisted == false) {   //不添加已经存在的项
            mRefreshRate->addItem(tr("%1 Hz").arg(QLocale().toString(qRound(mode->refreshRate()))), mode->id());
        }

        // If selected refresh rate is other then what we consider the "Auto" value
        // - that is it's not the highest resolution - then select it, otherwise
        // we stick with "Auto"
        if (mode == selectMode && mRefreshRate->count() > 1) {
            // i + 1 since 0 is auto
            mRefreshRate->setCurrentIndex(mRefreshRate->count() - 1);
        }
    }

    if (mRefreshRate->count() == 0) {
        mRefreshRate->addItem(tr("auto"), -1);
    } else if (-1 == mRefreshRate->currentIndex() || emitFlag) {
        modeID = mRefreshRate->itemData(0).toString();
    }
    mRefreshRate->blockSignals(false);

    if (!mIsModeInit)
        mOutput->setCurrentModeId(modeID);

    if (emitFlag)
        Q_EMIT changed();
}

void OutputConfig::slotRotationChanged(int index)
{
    KScreen::Output::Rotation rotation
        = static_cast<KScreen::Output::Rotation>(mRotation->itemData(index).toInt());
    mOutput->blockSignals(true);
    mOutput->setRotation(rotation);
    mOutput->blockSignals(false);

    Q_EMIT changed();
}

void OutputConfig::slotRefreshRateChanged(int index)
{
    mIsManualForRefreshRate = true;
    QString modeId;
    if (index == -1) {
        modeId = mRefreshRate->itemData(0).toString();
    } else {
        modeId = mRefreshRate->itemData(index).toString();
    }
    qDebug() << "modeId is:" << modeId << endl;
    mOutput->setCurrentModeId(modeId);

    Q_EMIT changed();
}

void OutputConfig::slotEnableWidget()
{
    if (mOutput.data()->isEnabled()) {
        mResolution->setEnabled(true);
        mRotation->setEnabled(true);
        mRefreshRate->setEnabled(true);
    } else {
        mResolution->setEnabled(false);
        mRotation->setEnabled(false);
        mRefreshRate->setEnabled(false);
    }
}

void OutputConfig::setShowScaleOption(bool showScaleOption)
{
    mShowScaleOption = showScaleOption;
    if (mOutput) {
        initUi();
    }
}

bool OutputConfig::showScaleOption() const
{
    return mShowScaleOption;
}

// 拿取配置
void OutputConfig::initConfig(const KScreen::ConfigPtr &config)
{
    mConfig = config;
}
