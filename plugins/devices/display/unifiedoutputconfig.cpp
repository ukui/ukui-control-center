#include "unifiedoutputconfig.h"
#include "resolutionslider.h"
#include "utils.h"
//#include "kcm_screen_debug.h"

#include <QComboBox>
#include <QIcon>
//#include <KLocalizedString>

#include <QLabel>
#include <QGridLayout>
#include <QSpacerItem>
#include <QCheckBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QStyledItemDelegate>
#include <QFile>

#include <KF5/KScreen/kscreen/output.h>
#include <KF5/KScreen/kscreen/config.h>

#define TABLET_MODE_SCHEMAS "org.ukui.SettingsDaemon.plugins.tablet-mode"
#define TABLET_MODE_KEY "tablet-mode"
#define AUTO_ROTATION_KEY "auto-rotation"
#define AUTO 0
#define FIXED 3

bool operator<(const QSize &s1, const QSize &s2)
{
    return s1.width() * s1.height() < s2.width() * s2.height();
}

template<>
bool qMapLessThanKey(const QSize &s1, const QSize &s2)
{
    return s1 < s2;
}


UnifiedOutputConfig::UnifiedOutputConfig(const KScreen::ConfigPtr &config, QWidget *parent)
    : OutputConfig(parent)
    , mConfig(config)
{
    const QByteArray id(TABLET_MODE_SCHEMAS);
    if (QGSettings::isSchemaInstalled(id)){
        m_tgsettings = new QGSettings(id);
    }
}

UnifiedOutputConfig::~UnifiedOutputConfig()
{
    if (m_tgsettings) {
        delete m_tgsettings;
        m_tgsettings = NULL;
    }
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
    vbox->setContentsMargins(0,0,0,0);
    vbox->setSpacing(0);
    //mTitle = new QLabel(this);
    //mTitle->setAlignment(Qt::AlignHCenter);
    //vbox->addWidget(mTitle);

    //setTitle(i18n("统一输出"));
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);


    //vbox->addStretch(0);

    KScreen::OutputPtr fakeOutput = createFakeOutput();
    mResolution = new ResolutionSlider(fakeOutput, this);



    //统一输出分辨率下拉框

    mResolution->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    mResolution->setMinimumSize(402,48);
//    mResolution->setMaximumSize(402,30);

    QLabel *resLabel = new QLabel(this);
    resLabel->setText(tr("resolution"));
    resLabel->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    resLabel->setMinimumSize(118,30);
    resLabel->setMaximumSize(118,30);

    QHBoxLayout *resLayout = new QHBoxLayout();
    resLayout->addWidget(resLabel);
    resLayout->addWidget(mResolution);
//    resLayout->addStretch();

    QFrame *resFrame = new QFrame(this);
    resFrame->setFrameShape(QFrame::Shape::NoFrame);
    resFrame->setLayout(resLayout);

    //暂时去掉分辨率调节功能
    resFrame->hide();
//    resWidget->setStyleSheet("background-color:#F4F4F4;border-radius:6px");
//    mResolution->setStyleSheet("background-color:#F8F9F9");

    resFrame->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
//    resFrame->setMinimumSize(552,50);
//    resFrame->setMaximumSize(960,50);
    resFrame->setFixedHeight(64);


    vbox->addWidget(resFrame);
//    connect(mResolution, &ResolutionSlider::resolutionChanged,
//            this, &UnifiedOutputConfig::whetherApplyResolution);

    slotResolutionChanged();

    QFrame *line_1 = new QFrame(this);
    line_1->setFrameShape(QFrame::Shape::HLine);
    line_1->setFixedHeight(1);
    vbox->addWidget(line_1);

    //方向下拉框
    mRotation = new QComboBox();
//    mRotation->setFont(ft);
    mRotation->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    mRotation->setMinimumSize(402,48);
    mRotation->setMaximumSize(16777215,48);
    mRotation->close();
    rotationRadioBtn = new SwitchButton;
//    mRotation->setStyleSheet(qss);
//    mRotation->setItemDelegate(itemDelege);
    rotationLabel = new QLabel();
    rotationLabel->setText(tr("Only tablet mode supports this feature"));
    rotationLabel->setEnabled(false);
    QLabel *rotateLabel = new QLabel();
    // ~contents_path /display/orientation
    rotateLabel->setText(tr("Auto Screen Rotation"));
//    rotateLabel->setFont(ft);
    rotateLabel->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    rotateLabel->setMinimumSize(160,30);
    rotateLabel->setMaximumSize(160,30);

    QHBoxLayout *rotateLayout = new QHBoxLayout();
    rotateLayout->addWidget(rotateLabel);
    rotateLayout->addItem(new QSpacerItem(40,10,QSizePolicy::Fixed));
    rotateLayout->addWidget(rotationLabel);
    rotateLayout->addStretch();
    rotateLayout->addWidget(rotationRadioBtn);
//    rotateLayout->addStretch();

    QFrame *rotateFrame = new QFrame(this);
    rotateFrame->setFrameShape(QFrame::Shape::NoFrame);
    rotateFrame->setLayout(rotateLayout);
//    rotateWidget->setStyleSheet("background-color:#F4F4F4;border-radius:6px");
//    mRotation->setStyleSheet("background-color:#F8F9F9");

    rotateFrame->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
//    rotateFrame->setMinimumSize(550,50);
//    rotateFrame->setMaximumSize(960,50);
    rotateFrame->setFixedHeight(64);
    rotateFrame->hide();
    mRotation->addItem( tr("arrow-up"), KScreen::Output::None);
    mRotation->addItem( tr("90° arrow-right"), KScreen::Output::Right);
    mRotation->addItem( tr("arrow-down"), KScreen::Output::Inverted);
    mRotation->addItem(tr("90° arrow-left"), KScreen::Output::Left);
    if (m_tgsettings) {
        if (m_tgsettings->get(TABLET_MODE_KEY).toBool()) {
            rotationRadioBtn->setEnabled(true);
            bool autoRation = m_tgsettings->get(AUTO_ROTATION_KEY).toBool();
            if (autoRation) {
                rotationRadioBtn->setChecked(true);
            } else {
                rotationRadioBtn->setChecked(false);
            }
        } else {
            rotationRadioBtn->setEnabled(false);
        }
    }
    connect(m_tgsettings,&QGSettings::changed,this,[=](const QString &key){
        if (key == "autoRotation") {
            if (m_tgsettings->get(TABLET_MODE_KEY).toBool()) {
                bool autoRoation = m_tgsettings->get(AUTO_ROTATION_KEY).toBool();
                if (autoRoation) {
                    rotationRadioBtn->blockSignals(true);
                    rotationRadioBtn->setChecked(true);
                    rotationRadioBtn->blockSignals(false);
                } else {
                    rotationRadioBtn->blockSignals(true);
                    rotationRadioBtn->setChecked(false);
                    rotationRadioBtn->blockSignals(false);
                }
            }
        }
        if (key == "tabletMode") {
            if (m_tgsettings->get(TABLET_MODE_KEY).toBool()) {
                rotationRadioBtn->setEnabled(true);
            } else {
                rotationRadioBtn->blockSignals(true);
                rotationRadioBtn->setChecked(false);
                rotationRadioBtn->blockSignals(false);

                rotationRadioBtn->setEnabled(false);
            }
        }
    });
    connect(rotationRadioBtn,&SwitchButton::checkedChanged,this,[=]{
        if (rotationRadioBtn->isChecked()) {
            m_tgsettings->set(AUTO_ROTATION_KEY,true);
        } else {
            m_tgsettings->set(AUTO_ROTATION_KEY,false);
        }
    });
    if (m_tgsettings) {
        if (m_tgsettings->get(TABLET_MODE_KEY).toBool()) {
            mRotation->removeItem(mRotation->findData(KScreen::Output::None));
            mRotation->removeItem(mRotation->findData(KScreen::Output::Right));
            mRotation->removeItem(mRotation->findData(KScreen::Output::Inverted));
            mRotation->removeItem(mRotation->findData(KScreen::Output::Left));
            mRotation->addItem(tr("fixed-rotation"), FIXED);
            mRotation->addItem(tr("auto-rotation"), AUTO);
        }
        connect(m_tgsettings, &QGSettings::changed, this, [ = ](const QString &keys) {
            mRotation->blockSignals(true);
            if (keys == "tabletMode") {
                if (m_tgsettings->get(TABLET_MODE_KEY).toBool()) {
                    mRotation->removeItem(mRotation->findData(KScreen::Output::None));
                    mRotation->removeItem(mRotation->findData(KScreen::Output::Right));
                    mRotation->removeItem(mRotation->findData(KScreen::Output::Inverted));
                    mRotation->removeItem(mRotation->findData(KScreen::Output::Left));
                    mRotation->addItem(tr("fixed-rotation"), FIXED);
                    mRotation->addItem(tr("auto-rotation"), AUTO);
                    if (m_tgsettings->get(AUTO_ROTATION_KEY).toBool()) {
                        mRotation->setCurrentIndex(mRotation->findData(AUTO));
                    } else {
                        mRotation->setCurrentIndex(mRotation->findData(FIXED));
                    }
                } else {
                    mRotation->removeItem(mRotation->findData(FIXED));
                    mRotation->removeItem(mRotation->findData(AUTO));
                    mRotation->addItem(tr("arrow-up"), KScreen::Output::None);
                    mRotation->addItem(tr("90° arrow-right"), KScreen::Output::Right);
                    mRotation->addItem(tr("arrow-down"), KScreen::Output::Inverted);
                    mRotation->addItem(tr("90° arrow-left"), KScreen::Output::Left);
                    mRotation->setCurrentIndex(mRotation->findData(mOutput->rotation()));
                }
            }
            if (keys == "autoRotation" && m_tgsettings->get(TABLET_MODE_KEY).toBool()) {
                if (m_tgsettings->get(AUTO_ROTATION_KEY).toBool()) {
                    mRotation->setCurrentIndex(mRotation->findData(AUTO));
                } else {
                    mRotation->setCurrentIndex(mRotation->findData(FIXED));
                }
            }
            mRotation->blockSignals(false);
        });
    }
//    connect(mRotation, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
//            this, &OutputConfig::whetherApplyRotation);
    connect(mRotation, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &UnifiedOutputConfig::whetherApplyRotation);

    if (m_tgsettings) {
        if (m_tgsettings->get(TABLET_MODE_KEY).toBool() && m_tgsettings->get(AUTO_ROTATION_KEY).toBool()) {
            mRotation->blockSignals(true);
            mRotation->setCurrentIndex(mRotation->findData(AUTO));
            mRotation->blockSignals(false);
        } else {
            mRotation->blockSignals(true);
            mRotation->setCurrentIndex(mRotation->findData(mOutput->rotation()));
            mRotation->blockSignals(false);
        }
    } else {
        mRotation->blockSignals(true);
        mRotation->setCurrentIndex(mRotation->findData(mOutput->rotation()));
        mRotation->blockSignals(false);
    }

    vbox->addWidget(rotateFrame);

    QFrame *line_2 = new QFrame(this);
    line_2->setFrameShape(QFrame::Shape::HLine);
    line_2->setFixedHeight(1);
    vbox->addWidget(line_2);

    //统一输出刷新率下拉框
    mRefreshRate = new QComboBox(this);
    mRefreshRate->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    mRefreshRate->setMinimumSize(402,48);
    mRefreshRate->setMaximumSize(16777215,48);

    QLabel *freshLabel = new QLabel(this);
    freshLabel->setText(tr("refresh rate"));
    freshLabel->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    freshLabel->setMinimumSize(118,30);
    freshLabel->setMaximumSize(118,30);

    mRefreshRate->addItem(tr("auto"), -1);
    mRefreshRate->addItem(tr("aa"), -1);


    QHBoxLayout *freshLayout = new QHBoxLayout();
    freshLayout->addWidget(freshLabel);
    freshLayout->addWidget(mRefreshRate);

    QFrame *freshFrame = new QFrame(this);
    freshFrame->setFrameShape(QFrame::Shape::NoFrame);
    freshFrame->setLayout(freshLayout);
//    freshFrame->setStyleSheet("background-color:#F4F4F4;border-radius:6px");

    vbox->addWidget(freshFrame);

    QFrame *line_3 = new QFrame(this);
    line_3->setFrameShape(QFrame::Shape::HLine);
    line_3->setFixedHeight(1);
    vbox->addWidget(line_3);

    //暂时隐藏刷新率设置
    freshFrame->setVisible(false);
    line_3->setVisible(false);

    freshFrame->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
//    freshFrame->setMinimumSize(552,50);
//    freshFrame->setMaximumSize(960,50);
    freshFrame->setFixedHeight(64);

    mRefreshRate->setEnabled(false);

    scaleCombox = new QComboBox();
    //    mRefreshRate->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
        scaleCombox->setMinimumSize(402,48);
        scaleCombox->setMaximumSize(16777215,48);
        int maxReslu = mResolution->getMaxResolution().width();

    #if QT_VERSION < QT_VERSION_CHECK(5,7,0)
        if (maxReslu >= 2000) {
            scaleCombox->addItem(tr("200%"));
        } else {
            scaleCombox->addItem(tr("100%"));
        }
    #else
        scaleCombox->addItem(tr("100% (Recommended)"));
        if (maxReslu >= 2000 && maxReslu <= 3800) {
            scaleCombox->addItem(tr("200%"));
        } else if (maxReslu >= 3800 || maxReslu >= 4000) {
            scaleCombox->addItem(tr("200%"));
            scaleCombox->addItem(tr("300%"));
        }
    #endif

        QLabel *scaleLabel = new QLabel();

        scaleLabel->setText(tr("screen zoom"));
        scaleLabel->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
        scaleLabel->setMinimumSize(118,30);
        scaleLabel->setMaximumSize(118,30);

        QHBoxLayout *scaleLayout = new QHBoxLayout();
        scaleLayout->addWidget(scaleLabel);
        scaleLayout->addWidget(scaleCombox);
    //    freshLayout->addStretch();

        QFrame *scaleFrame = new QFrame(this);
        scaleFrame->setFrameShape(QFrame::Shape::NoFrame);
        scaleFrame->setLayout(scaleLayout);

        //屏幕缩放暂时去掉
        scaleFrame->close();
    //    scaleWidget->setStyleSheet("background-color:#F4F4F4;border-radius:6px");

        scaleFrame->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
//        scaleFrame->setMinimumSize(550,50);
//        scaleFrame->setMaximumSize(960,50);
        scaleFrame->setFixedHeight(64);
        vbox->addWidget(scaleFrame);

        int scale = getScreenScale();
    #if QT_VERSION < QT_VERSION_CHECK(5,7,0)

    #else
        scaleCombox->blockSignals(true);
        scaleCombox->setCurrentIndex(0);
        if (scale <= scaleCombox->count() && scale > 0) {
    //        qDebug()<<"scale is----->"<<scale<<endl;
            scaleCombox->setCurrentIndex(scale - 1);
        }
        scaleCombox->blockSignals(false);
        slotScaleChanged();
    #endif

        connect(scaleCombox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                this, &OutputConfig::slotScaleChanged);

        if (mResolution->currentResolution().isValid()) {
            resolutionIndex = mResolution->mComboBox->currentIndex();
        }
        rotationIndex = mRotation->currentIndex();
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
            //qCDebug(KSCREEN_KCM) << smallestMode << clone->preferredMode()->size();
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
    fakeOutput->setCurrentModeId(Utils::sizeToString(commonResults.last()));
    return fakeOutput;
}

void UnifiedOutputConfig::whetherApplyResolution() {
    ApplyDialog * applydialog = new ApplyDialog(this);
    connect(applydialog, SIGNAL(do_apply()), this, SLOT(slotResolutionChanged()));
    connect(applydialog, SIGNAL(do_not_apply()), this, SLOT(slotResolutionNotChange()));
    applydialog->exec();
}

void UnifiedOutputConfig::slotResolutionNotChange() {
    mResolution->mComboBox->blockSignals(true);
    mResolution->mComboBox->setCurrentIndex(resolutionIndex);
    mResolution->mComboBox->blockSignals(false);
}

void UnifiedOutputConfig::whetherApplyRotation() {
    //避免点击统一输出时触发弹窗
    if (is_unifiedoutput) {
        is_unifiedoutput = false;
        return;
    }
    if (m_tgsettings) {
        //平板模式下开关自动旋转不触发弹窗
        if (m_tgsettings->get(TABLET_MODE_KEY).toBool()) {
            if (mRotation->currentData() == 0) {
                //打开自动旋转
                m_tgsettings->set(AUTO_ROTATION_KEY, true);
            } else if (mRotation->currentData() == 3) {
                //关闭自动旋转
                m_tgsettings->set(AUTO_ROTATION_KEY, false);
            }
            return;
        }
    }
    ApplyDialog * applydialog = new ApplyDialog(this);
    connect(applydialog, SIGNAL(do_apply()), this, SLOT(slotRotationChanged()));
    connect(applydialog, SIGNAL(do_not_apply()), this, SLOT(slotRotationNotChange()));
    applydialog->exec();
}

void UnifiedOutputConfig::slotRotationNotChange() {
    mRotation->blockSignals(true);
    mRotation->setCurrentIndex(rotationIndex);
    mRotation->blockSignals(false);
}

void UnifiedOutputConfig::slotResolutionChanged()
{
    // Ignore disconnected outputs
     if (!mResolution->currentResolution().isValid()) {
         return;
     }
    QSize size = mResolution->currentResolution();
    resolutionIndex = mResolution->mComboBox->currentIndex();

    Q_FOREACH (const KScreen::OutputPtr &clone, mClones) {
        const QString &id = findBestMode(clone, size);
        if (id.isEmpty()) {
            // FIXME: Error?
            return;
        }

        clone->setCurrentModeId(id);
    }

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

//统一输出方向信号改变
void UnifiedOutputConfig::slotRotationChangedDerived(){
    int index = mRotation->currentIndex();
    if (index == mRotation->findData(AUTO)) {
        //平板模式下选择自动转屏
        m_tgsettings->set(AUTO_ROTATION_KEY, true);
        Q_EMIT changed();
        return;
    } else {
        //平板模式下设置转屏方向，自动旋转关闭
        if (m_tgsettings) {
            if (m_tgsettings->get(TABLET_MODE_KEY).toBool()) {
                m_tgsettings->blockSignals(true);
                m_tgsettings->set(AUTO_ROTATION_KEY, false);
                m_tgsettings->blockSignals(false);
            }
        }
    }
    rotationIndex = index;
    KScreen::Output::Rotation rotation =
        static_cast<KScreen::Output::Rotation>(mRotation->itemData(index).toInt());
    auto mainOutput = mConfig->primaryOutput();
    //qDebug()<<"首屏幕输出--->"<<mainOutput<<endl;
    Q_FOREACH (const KScreen::OutputPtr &clone, mClones) {
        if(clone->isConnected()&&clone->isEnabled()){
            //mOutput = clone;

            mainOutput->setRotation(rotation);
          //  clone->setRotation(rotation);
            //qDebug()<<"是不是首屏------->"<<clone->isPrimary()<<endl;
            if(!clone->isPrimary()){
               // qDebug()<<"非首屏------>"<<clone<<endl;
//                KScreen::OutputPtr output = clone;
                clone->setRotation(rotation);
                //output->apply(mainOutput);
                //clone->apply(mainOutput);
            }
        }
    }
    Q_EMIT changed();
}


