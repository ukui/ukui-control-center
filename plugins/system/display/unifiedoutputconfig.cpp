#include "unifiedoutputconfig.h"
#include "resolutionslider.h"
#include "scalesize.h"
#include "utils.h"

#include <QComboBox>
#include <QIcon>
#include <QLabel>
#include <QGridLayout>
#include <QSpacerItem>
#include <QCheckBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QStyledItemDelegate>
#include <QFile>
#include <QVector>

#include <KF5/KScreen/kscreen/output.h>
#include <KF5/KScreen/kscreen/config.h>
#include <KF5/KScreen/kscreen/getconfigoperation.h>

bool operator<(const QSize &s1, const QSize &s2)
{
    return s1.width() * s1.height() < s2.width() * s2.height();
}

template<>
bool qMapLessThanKey(const QSize &s1, const QSize &s2)
{
    return s1 < s2;
}

UnifiedOutputConfig::UnifiedOutputConfig(const KScreen::ConfigPtr &config, QWidget *parent) :
    OutputConfig(parent),
    mConfig(config)
{
    mIsRestore = true;
}

UnifiedOutputConfig::~UnifiedOutputConfig()
{
}

void UnifiedOutputConfig::setOutput(const KScreen::OutputPtr &output)
{
    mOutput = output;

    mClones.clear();
    mClones.reserve(mOutput->clones().count());
    Q_FOREACH (int id, mOutput->clones()) {
        mClones << mConfig->output(id);
    }
    mClones << mOutput;

    OutputConfig::setOutput(output);
}

void UnifiedOutputConfig::initUi()
{
    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(0, 0, 0, 0);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    KScreen::OutputPtr fakeOutput = createFakeOutput();
    mResolution = new ResolutionSlider(fakeOutput, this);

    mResolution->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    mResolution->setMinimumSize(402, 30);

    //监听，否则无法处理修改分辨率/刷新率未保存
    connect(mOutput.data(), &KScreen::Output::currentModeIdChanged,
            this, &UnifiedOutputConfig::slotRestoreResoltion);

    connect(mOutput.data(), &KScreen::Output::rotationChanged,
            this, &UnifiedOutputConfig::slotRestoreRatation);

    QLabel *resLabel = new QLabel(this);
    resLabel->setText(tr("resolution"));
    resLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    resLabel->setMinimumSize(118, 30);
    resLabel->setMaximumSize(118, 30);

    QHBoxLayout *resLayout = new QHBoxLayout();
    resLayout->addSpacing(6);
    resLayout->addWidget(resLabel);
    resLayout->addWidget(mResolution);

    QFrame *resFrame = new QFrame(this);
    resFrame->setFrameShape(QFrame::Shape::Box);
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
    mRotation->setMinimumSize(402, 30);
    mRotation->setMaximumSize(16777215, 30);

    QLabel *rotateLabel = new QLabel(this);
    rotateLabel->setText(tr("orientation"));
    rotateLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    rotateLabel->setMinimumSize(118, 30);
    rotateLabel->setMaximumSize(118, 30);

    mRotation->addItem(tr("arrow-up"), KScreen::Output::None);
    mRotation->addItem(tr("90° arrow-right"), KScreen::Output::Right);
    mRotation->addItem(tr("arrow-down"), KScreen::Output::Inverted);
    mRotation->addItem(tr("90° arrow-left"), KScreen::Output::Left);

    int index = mRotation->findData(mOutput->rotation());
    mRotation->setCurrentIndex(index);

    connect(mRotation, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &UnifiedOutputConfig::slotRotationChangedDerived);

    QHBoxLayout *roatateLayout = new QHBoxLayout();
    roatateLayout->addSpacing(6);
    roatateLayout->addWidget(rotateLabel);
    roatateLayout->addWidget(mRotation);

    QFrame *rotateFrame = new QFrame(this);
    rotateFrame->setFrameShape(QFrame::Shape::Box);
    rotateFrame->setLayout(roatateLayout);

    rotateFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    rotateFrame->setMinimumWidth(552);
    rotateFrame->setFixedHeight(50);

    QFrame *line2 = setLine(rotateFrame);
    vbox->addWidget(rotateFrame);
    vbox->addWidget(line2);

    // 统一输出刷新率下拉框
    mRefreshRate = new QComboBox(this);
    mRefreshRate->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    mRefreshRate->setMinimumSize(402, 30);
    mRefreshRate->setMaximumSize(16777215, 30);

    QLabel *freshLabel = new QLabel(this);
    freshLabel->setText(tr("frequency"));
    freshLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    freshLabel->setMinimumSize(118, 30);
    freshLabel->setMaximumSize(118, 30);

    QHBoxLayout *freshLayout = new QHBoxLayout();
    freshLayout->addSpacing(6);
    freshLayout->addWidget(freshLabel);
    freshLayout->addWidget(mRefreshRate);

    QFrame *freshFrame = new QFrame(this);
    freshFrame->setFrameShape(QFrame::Shape::Box);
    freshFrame->setLayout(freshLayout);

    vbox->addWidget(freshFrame);

    freshFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    freshFrame->setMinimumWidth(552);
    freshFrame->setFixedHeight(50);

    slotResolutionChanged(mResolution->currentResolution(), true);
    connect(mRefreshRate, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, &UnifiedOutputConfig::slotRefreshRateChanged);
    QObject::connect(new KScreen::GetConfigOperation(), &KScreen::GetConfigOperation::finished, this,
                         [&](KScreen::ConfigOperation *op) {
        KScreen::ConfigPtr sConfig = qobject_cast<KScreen::GetConfigOperation *>(op)->config();
        KScreen::OutputPtr sOutput = sConfig -> primaryOutput();

        for (int i = 0; i < mRefreshRate->count(); ++i) {
            if (!sOutput.isNull() && !sOutput->currentMode().isNull() && mRefreshRate->itemText(i) == refreshRateToText(sOutput->currentMode()->refreshRate())) {
                mRefreshRate->setCurrentIndex(i);
            }
        }
    });
}

KScreen::OutputPtr UnifiedOutputConfig::createFakeOutput()
{
    // Find set of common resolutions
    QMap<QSize, int> commonSizes;
    Q_FOREACH (const KScreen::OutputPtr &clone, mClones) {
        QList<QSize> processedSizes;
        Q_FOREACH (const KScreen::ModePtr &mode, clone->modes()) {
            // Make sure we don't count some modes multiple times because of different
            // refresh rates
            if (processedSizes.contains(mode->size())) {
                continue;
            }

            processedSizes << mode->size();

            if (commonSizes.contains(mode->size())) {
                commonSizes[mode->size()]++;
            } else {
                commonSizes.insert(mode->size(), 1);
            }
        }
    }

    KScreen::OutputPtr fakeOutput(new KScreen::Output);

    // This will give us list of resolution that are shared by all outputs
    QList<QSize> commonResults = commonSizes.keys(mClones.count());
    // If there are no common resolution, fallback to smallest preferred mode
    if (commonResults.isEmpty()) {
        QSize smallestMode;
        Q_FOREACH (const KScreen::OutputPtr &clone, mClones) {
            if (!smallestMode.isValid() || clone->preferredMode()->size() < smallestMode) {
                smallestMode = clone->preferredMode()->size();
            }
        }
        commonResults << smallestMode;
    }
    std::sort(commonResults.begin(), commonResults.end());

    KScreen::ModeList modes;
    Q_FOREACH (const QSize &size, commonResults) {
        KScreen::ModePtr mode(new KScreen::Mode);
        mode->setSize(size);
        mode->setId(Utils::sizeToString(size));
        mode->setName(mode->id());
        modes.insert(mode->id(), mode);
    }
    fakeOutput->setModes(modes);
    if (!mOutput->currentModeId().isEmpty()) {
        fakeOutput->setCurrentModeId(Utils::sizeToString(mOutput->currentMode()->size()));
    } else {
        fakeOutput->setCurrentModeId(Utils::sizeToString(commonResults.last()));
    }

    return fakeOutput;
}

void UnifiedOutputConfig::slotResolutionChanged(const QSize &size, bool emitFlag)
{
    // Ignore disconnected outputs
    if (!size.isValid()) {
        return;
    }
    bool mIsModeInit = true;
    QVector<QString>Vrefresh;
    bool mIsCloneMode = isCloneMode();
    mRefreshRate->blockSignals(true);
    mRefreshRate->clear();
    mRefreshRate->blockSignals(false);
    Q_FOREACH (const KScreen::OutputPtr &clone, mClones) {
        const QString &id = findBestMode(clone, size);
        if (id.isEmpty()) {
            // FIXME: Error?
            return;
        }
        //本来就是镜像模式且当前分辨率就是选中分辨率，就不需要重新设置显示参数
        //用于镜像模式下刚打开控制面板时的显示，否则显示的不是实际刷新率而是findBestMode
        if (!mIsCloneMode || size != clone->currentMode()->size()) {
            mIsModeInit = false;
            clone->blockSignals(true); //必须加blockSignals，否则在这里就会触发currentModeIdChanged的信号
            mIsRestore = false; //修改配置会触发currentModeIdChanged时，不运行slotRestoreResoltion
            clone->setCurrentModeId(id);
            clone->setPos(QPoint(0, 0));
            clone->blockSignals(false);
        }

        QList<KScreen::ModePtr> modes;
        Q_FOREACH (const KScreen::ModePtr &mode, clone->modes()) {
            if (mode->size() == size) {
                modes << mode;
            }
        }

        QVector<QString>VrefreshTemp;
        for (int i = 0, total = modes.count(); i < total; ++i) {
           const KScreen::ModePtr mode = modes.at(i);

           bool alreadyExisted = false; //判断该显示器的刷新率是否有重复的，确保同一刷新率在一个屏幕上只出现一次
           for (int j = 0; j < VrefreshTemp.size(); ++j) {
               if (refreshRateToText(mode->refreshRate()) == VrefreshTemp[j]) {
                   alreadyExisted = true;
                   break;
               }
           }
           if (alreadyExisted == false) {   //不添加重复的项
               VrefreshTemp.append(refreshRateToText(mode->refreshRate()));
           }
        }

        for (int i = 0; i < VrefreshTemp.size(); ++i) {
            Vrefresh.append(VrefreshTemp[i]);
        }
    }

    for (int i = 0; i < Vrefresh.size(); ++i) {
        if (Vrefresh.count(Vrefresh[i]) == mClones.size()) { //该刷新率出现次数等于屏幕数，即每个屏幕都有该刷新率
            bool existFlag = false;
            for (int j = 0; j < mRefreshRate->count(); ++j) {  //已经存在就不再添加
                if (Vrefresh[i] == mRefreshRate->itemText(j)) {
                    existFlag = true;
                    break;
                }
            }
            if (existFlag == false) {  //不存在添加到容器中
                mRefreshRate->blockSignals(true);
                mRefreshRate->addItem(Vrefresh[i]);
                mRefreshRate->blockSignals(false);
            }
        }
    }

    if (mRefreshRate->count() > 1) {
        float currentRereshRate = mClones[0]->currentMode()->refreshRate();
        for (int i = 0; i < mRefreshRate->count(); i++) {
            if (refreshRateToText(currentRereshRate) == mRefreshRate->itemText(i)) {
                mRefreshRate->blockSignals(true);
                mRefreshRate->setCurrentIndex(i);
                mRefreshRate->blockSignals(false);
                break;
            }
        }
    }

    if (mRefreshRate->count() == 0) {
        mRefreshRate->blockSignals(true);
        mRefreshRate->addItem(tr("auto"), -1);
        mRefreshRate->blockSignals(false);
    }
    if (emitFlag && !mIsModeInit){
        changeItm = RESOLUTION;
        Q_EMIT changed();
    }
}

void UnifiedOutputConfig::slotRefreshRateChanged(int index)
{
    Q_FOREACH (const KScreen::OutputPtr &clone, mClones) {
        Q_FOREACH (const KScreen::ModePtr &mode, clone->modes()) {
            if (mode->size() == mResolution->currentResolution() && \
                    refreshRateToText(mode->refreshRate()) == mRefreshRate->itemText(index)) {
                mIsRestore = false;
                clone->blockSignals(true);
                clone->setCurrentModeId(mode->id());
                clone->blockSignals(false);
            }
        }
    }
    changeItm = FREQUENCY;
    Q_EMIT changed();
}

QString UnifiedOutputConfig::findBestMode(const KScreen::OutputPtr &output, const QSize &size)
{
    float refreshRate = 0;
    QString id;
    Q_FOREACH (const KScreen::ModePtr &mode, output->modes()) {
        if (mode->size() == size && mode->refreshRate() > refreshRate) {
            refreshRate = mode->refreshRate();
            id = mode->id();
        }
    }
    return id;
}

// 统一输出方向信号改变
void UnifiedOutputConfig::slotRotationChangedDerived(int index)
{
    KScreen::Output::Rotation rotation = static_cast<KScreen::Output::Rotation>(mRotation->itemData(index).toInt());
    Q_FOREACH (const KScreen::OutputPtr &clone, mClones) {
        if (clone->isConnected() && clone->isEnabled()) {
            clone->blockSignals(true);
            clone->setRotation(rotation);
            clone->setPos(QPoint(0, 0));
            clone->blockSignals(false);
        }
    }
    changeItm = ORIENTATION;
    Q_EMIT changed();
}

void UnifiedOutputConfig::slotRestoreResoltion()
{
    if (!mIsRestore || !mOutput->currentMode()) {
        mIsRestore = true;
        return;
    }
    if (mResolution->currentResolution() != mOutput->currentMode()->size()) { //分辨率改变时，触发该信号重新加载刷新率，用于修改分辨率之后但未保存
        mResolution->setResolution(mOutput->currentMode()->size()); //这里面不会触发分辨率改变信号
        slotResolutionChanged(mOutput->currentMode()->size(), false);
    } else { //分辨率未修改，刷新率修改,用于修改刷新率之后但未保存
        for (int i = 0; i < mRefreshRate->count(); i++) {
           if (refreshRateToText(mOutput->currentMode()->refreshRate()) == mRefreshRate->itemText(i)\
                   || mRefreshRate->count() == 1) {
               mRefreshRate->blockSignals(true);
               mRefreshRate->setCurrentIndex(i);
               mRefreshRate->blockSignals(false);
               break;
           }
        }
    }
}

void UnifiedOutputConfig::slotRestoreRatation()
{
    mRotation->blockSignals(true);
    mRotation->setCurrentIndex(mRotation->findData(mOutput->rotation()));
    mRotation->blockSignals(false);
}

bool UnifiedOutputConfig::isCloneMode()
{
    /*
     *不能直接用isVisible判断是否为镜像模式
     *设置镜像模式时，visiable总是true，但此时还未设置currentMode
     *导致某些情况异常
     */
    //return this->isVisible();    //显示则表示是统一输出
    if (!mClones.isEmpty() && mClones[0] && mClones[0]->currentMode()) {
        QSize cloneSize(mClones[0]->currentMode()->size());
        QPoint clonePos(mClones[0]->pos());
        Q_FOREACH (const KScreen::OutputPtr &clone, mClones) {
            if (clone->currentMode() && (clone->currentMode()->size() != cloneSize || clone->pos() != clonePos)) {
                return false;
            }
        }
        return true;
    } else {
        return false;
    }

}

QFrame *UnifiedOutputConfig::setLine(QFrame *frame)
{
    QFrame *line = new QFrame(frame);
    line->setMinimumSize(QSize(0, 1));
    line->setMaximumSize(QSize(16777215, 1));
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    return line;
}
