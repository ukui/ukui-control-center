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
#include <QProcess>

#include <KF5/KScreen/kscreen/output.h>
#include <KF5/KScreen/kscreen/edid.h>

#include <ukcc/widgets/combobox.h>

double mScaleres = 0;
QList<float> kRadeonRate{59.9402, 29.98};
CONFIG changeItm = INIT;

OutputConfig::OutputConfig(QWidget *parent) :
    QWidget(parent),
    mOutput(nullptr)
{
    initRadeon();
    initDpiConnection();
}

OutputConfig::OutputConfig(const KScreen::OutputPtr &output, QWidget *parent) :
    QWidget(parent)
{
    initDpiConnection();
    setOutput(output);
}

OutputConfig::~OutputConfig()
{
}

QFrame *OutputConfig::setLine(QFrame *frame)
{
    QFrame *line = new QFrame(frame);
    line->setMinimumSize(QSize(0, 1));
    line->setMaximumSize(QSize(16777215, 1));
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    return line;
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
//    vbox->setSpacing(2);
    vbox->setSpacing(0);

    // 分辨率下拉框
    mResolution = new ResolutionSlider(mOutput, this);
    mResolution->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    mResolution->setFixedHeight(36);

    QLabel *resLabel = new QLabel(this);
    //~ contents_path /Display/resolution
    resLabel->setText(tr("resolution"));
    resLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    resLabel->setFixedSize(118, 36);

    QHBoxLayout *resLayout = new QHBoxLayout();
    resLayout->setContentsMargins(14,0,7,0);
    resLayout->addWidget(resLabel);
    resLayout->addWidget(mResolution);


    QFrame *resFrame = new QFrame(this);
    resFrame->setFrameShape(QFrame::Shape::NoFrame);
    resFrame->setLayout(resLayout);

    resFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    resFrame->setMinimumWidth(552);
    resFrame->setFixedHeight(50);

    QFrame *line1 = setLine(resFrame);

    vbox->addWidget(resFrame);
    vbox->addWidget(line1);

    connect(mResolution, &ResolutionSlider::resolutionChanged,
            this, [=](QSize size, bool emitFlag){
                slotResolutionChanged(size, emitFlag);
            });

    // 方向下拉框
    mRotation = new QComboBox(this);
    mRotation->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    mRotation->setFixedHeight(36);

    QLabel *rotateLabel = new QLabel(this);
    //~ contents_path /Display/orientation
    rotateLabel->setText(tr("orientation"));
    rotateLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    rotateLabel->setFixedSize(118, 36);

    QHBoxLayout *rotateLayout = new QHBoxLayout();
    rotateLayout->setContentsMargins(14,0,7,0);
    rotateLayout->addWidget(rotateLabel);

    rotateLayout->addWidget(mRotation);

    QFrame *rotateFrame = new QFrame(this);
    rotateFrame->setFrameShape(QFrame::Shape::Box);
    rotateFrame->setLayout(rotateLayout);

    rotateFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    rotateFrame->setMinimumWidth(552);
    rotateFrame->setFixedHeight(50);

    mRotation->addItem(tr("arrow-up"), KScreen::Output::None);
    mRotation->addItem(tr("90° arrow-right"), KScreen::Output::Right);
    mRotation->addItem(tr("90° arrow-left"), KScreen::Output::Left);
    mRotation->addItem(tr("arrow-down"), KScreen::Output::Inverted);
    connect(mRotation, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &OutputConfig::slotRotationChanged);
    mRotation->setCurrentIndex(mRotation->findData(mOutput->rotation()));

    QFrame *line2 = setLine(rotateFrame);
    vbox->addWidget(rotateFrame);
    vbox->addWidget(line2);

    // 刷新率下拉框
    mRefreshRate = new QComboBox(this);
    mRefreshRate->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    mRefreshRate->setFixedHeight(36);

    QLabel *freshLabel = new QLabel(this);
    //~ contents_path /Display/frequency
    freshLabel->setText(tr("frequency"));
    freshLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    freshLabel->setFixedSize(118, 36);

    QHBoxLayout *freshLayout = new QHBoxLayout();
    freshLayout->setContentsMargins(14, 0, 7, 0);
    freshLayout->addWidget(freshLabel);
    freshLayout->addWidget(mRefreshRate);

    QFrame *freshFrame = new QFrame(this);
    freshFrame->setFrameShape(QFrame::Shape::Box);
    freshFrame->setLayout(freshLayout);

    freshFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    freshFrame->setMinimumWidth(550);
    freshFrame->setFixedHeight(50);

    vbox->addWidget(freshFrame);

    slotResolutionChanged(mResolution->currentResolution(), true);
    connect(mRefreshRate, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &OutputConfig::slotRefreshRateChanged);

    // 缩放率下拉框
    QFrame *scaleFrame = new QFrame(this);
    scaleFrame->setFrameShape(QFrame::Shape::Box);

    scaleFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    scaleFrame->setMinimumWidth(550);
    scaleFrame->setFixedHeight(50);


    QHBoxLayout *scaleLayout = new QHBoxLayout(scaleFrame);
    scaleLayout->setContentsMargins(14, 0, 7, 0);

    mScaleCombox = new QComboBox(this);
    mScaleCombox->setObjectName("scaleCombox");
    mScaleCombox->setFixedHeight(36);

    QLabel *scaleLabel = new QLabel(this);
    //~ contents_path /Display/screen zoom
    scaleLabel->setText(tr("screen zoom"));
    scaleLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    scaleLabel->setFixedSize(118, 36);

    scaleLayout->addWidget(scaleLabel);
    scaleLayout->addWidget(mScaleCombox);

    vbox->addWidget(scaleFrame);
    scaleFrame->hide();

    initConnection();
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
    //监听，否则无法处理修改分辨率/刷新率未保存
    connect(mOutput.data(), &KScreen::Output::currentModeIdChanged, this, [=]() {
        if (!mIsRestore || !mOutput->currentMode()) {
            mIsRestore = true;
            return;
        }
        //分辨率改变时，触发该信号重新加载刷新率，用于修改分辨率之后但未保存
        if (mResolution->currentResolution() != mOutput->currentMode()->size()) {
            mResolution->setResolution(mOutput->currentMode()->size());
            slotResolutionChanged(mOutput->currentMode()->size(), false);
        } else {
            //分辨率未修改，刷新率修改,用于修改刷新率之后但未保存
            for (int i = 0; i < mRefreshRate->count(); i++) {
                if (mRefreshRate->count() == 1 || \
                        refreshRateToText(mOutput->currentMode()->refreshRate()) == mRefreshRate->itemText(i)) {
                    mRefreshRate->blockSignals(true);
                    mRefreshRate->setCurrentIndex(i);
                    mRefreshRate->blockSignals(false);
                    break;
                }
            }
        }
    });

    connect(mOutput.data(), &KScreen::Output::isEnabledChanged,
            this, [=](){
        slotEnableWidget();
    });
}

void OutputConfig::initDpiConnection()
{
    QByteArray id(SCALE_SCHEMAS);
    if (QGSettings::isSchemaInstalled(SCALE_SCHEMAS)) {
        mDpiSettings = new QGSettings(id, QByteArray(), this);
        connect(mDpiSettings, &QGSettings::changed, this, [=](QString key) {
            if (!key.compare("scalingFactor", Qt::CaseSensitive)) {
                slotDPIChanged(key);
            }

        });
    }
}

QString OutputConfig::scaleToString(double scale)
{
    return QString::number(scale * 100) + "%";
}

void OutputConfig::initRadeon()
{
    QProcess process;
    process.start("lspci -v");
    process.waitForFinished();
    QString output = process.readAll();
    output = output.simplified();
    mIsRadeon = output.contains("radeon", Qt::CaseInsensitive);
    qDebug() << Q_FUNC_INFO << mIsRadeon;
}

void OutputConfig::setOutput(const KScreen::OutputPtr &output)
{
    mOutput = output;
    initUi();
}

KScreen::OutputPtr OutputConfig::output() const
{
    return mOutput;
}

//只修改刷新率时，不应该运行此函数
void OutputConfig::slotResolutionChanged(const QSize &size, bool emitFlag)
{
    // Ignore disconnected outputs
    if (!size.isValid()) {
        return;
    }
    bool mIsModeInit = false;
    bool isRadeonRate = false;
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
    mRefreshRate->clear();
    mRefreshRate->blockSignals(false);

    for (int i = 0, total = modes.count(); i < total; ++i) {
        const KScreen::ModePtr mode = modes.at(i);

        isRadeonRate = false;
        bool alreadyExisted = false;
        for (int j = 0; j < mRefreshRate->count(); ++j) {
            if (refreshRateToText(mode->refreshRate()) == mRefreshRate->itemText(j)) {
                alreadyExisted = true;
                break;
            }
        }

        if ((mIsRadeon && kRadeonRate.contains(mode->refreshRate())))
            isRadeonRate = true;

        if (alreadyExisted == false && !isRadeonRate) {   //不添加已经存在的项
            mRefreshRate->blockSignals(true);
            mRefreshRate->addItem(refreshRateToText(mode->refreshRate()), mode->id());
            mRefreshRate->blockSignals(false);
        }

        // If selected refresh rate is other then what we consider the "Auto" value
        // - that is it's not the highest resolution - then select it, otherwise
        // we stick with "Auto"
        if (mode == selectMode && mRefreshRate->count() > 0) {
            mRefreshRate->blockSignals(true);
            mRefreshRate->setCurrentIndex(mRefreshRate->count() - 1);
            mRefreshRate->blockSignals(false);
        }
    }

    if (mRefreshRate->count() == 0) {
        mRefreshRate->blockSignals(true);
        mRefreshRate->addItem(tr("auto"), -1);
        mRefreshRate->blockSignals(false);
    } else {
        if (-1 == mRefreshRate->currentIndex()) {
            modeID = mRefreshRate->itemData(0).toString();
            // 避免选择50hz以下刷新率为空
            mRefreshRate->blockSignals(true);
            mRefreshRate->setCurrentIndex(0);
            mRefreshRate->blockSignals(false);
        }
    }

    mOutput->setCurrentModeId(modeID);

    if (!mIsModeInit) {
        mIsRestore = false;
        if (emitFlag) {
            changeItm = RESOLUTION;
            Q_EMIT changed();
        }
    }
}

void OutputConfig::slotRotationChanged(int index)
{
    KScreen::Output::Rotation rotation
        = static_cast<KScreen::Output::Rotation>(mRotation->itemData(index).toInt());
    mOutput->blockSignals(true);
    mOutput->setRotation(rotation);
    mOutput->blockSignals(false);

    changeItm = ORIENTATION;
    Q_EMIT toSetScreenPos();//要在save之前修正坐标
    Q_EMIT changed();
}

void OutputConfig::slotRefreshRateChanged(int index)
{
    QString modeId;
    modeId = mRefreshRate->itemData(index).toString();
    qDebug() << "(slotRefreshRateChanged)modeId is:" << modeId << endl;
    mOutput->blockSignals(true);
    mIsRestore = false;
    mOutput->setCurrentModeId(modeId);
    mOutput->blockSignals(false);
    changeItm = FREQUENCY;
    Q_EMIT changed();
}

void OutputConfig::slotScaleChanged(int index)
{
    Q_EMIT scaleChanged(mScaleCombox->itemData(index).toDouble());
}

void OutputConfig::slotDPIChanged(QString key)
{
    double scale = mDpiSettings->get(key).toDouble();
    if (mScaleCombox) {
        if (mScaleCombox->findData(scale) == -1) {
            mScaleCombox->addItem(scaleToString(scale), scale);
        }
        mScaleCombox->blockSignals(true);
        mScaleCombox->setCurrentText(scaleToString(scale));
        mScaleCombox->blockSignals(false);

    }
}

void OutputConfig::slotEnableWidget()
{
    bool isEnable = mOutput.data()->isEnabled();
    mResolution->setEnabled(isEnable);
    mRotation->setEnabled(isEnable);
    mRefreshRate->setEnabled(isEnable);
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

QString OutputConfig::refreshRateToText(float refreshRate)
{
    QRegExp rx;
    rx.setPattern("(\\.){0,1}0+$");
    return tr("%1 Hz").arg((QString::number(refreshRate,'f',2)).replace(rx,""));
}
