#include "bluetoothmainwindow.h"
#include <ukcc/widgets/titlelabel.h>

BlueToothMainWindow::BlueToothMainWindow(QWidget *parent):
    QMainWindow(parent)
{
    InitMainWindow();
}

BlueToothMainWindow::~BlueToothMainWindow()
{

}

void BlueToothMainWindow::InitMainWindow()
{
    _MCentralWidget = new QStackedWidget(this);
    this->setCentralWidget(_MCentralWidget);

    InitError0Widget();
    InitError1Widget();
    InitNormalWidget();

    _MCentralWidget->setCurrentIndex(2);
}

void BlueToothMainWindow::InitError0Widget()
{
    _MError0Widget = new QWidget(_MCentralWidget);
    QVBoxLayout *errorWidgetLayout = new QVBoxLayout(_MError0Widget);
    QLabel      *errorWidgetIcon   = new QLabel(_MError0Widget);
    QLabel      *errorWidgetTip    = new QLabel(_MError0Widget);

    _MError0Widget->setObjectName("error0Widget");

    errorWidgetLayout->setSpacing(10);
    errorWidgetLayout->setMargin(0);
    errorWidgetLayout->setContentsMargins(0,0,0,0);

    errorWidgetIcon->setFixedSize(56,56);
    errorWidgetTip->resize(200,30);
    errorWidgetTip->setFont(QFont("Noto Sans CJK SC",18,QFont::Bold));

    errorWidgetLayout->addStretch(10);

    if (QIcon::hasThemeIcon("dialog-warning")) {
        errorWidgetIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(56,56));
        errorWidgetLayout->addWidget(errorWidgetIcon,1,Qt::AlignCenter);
    }

    errorWidgetTip->setText(tr("Bluetooth adapter is abnormal !"));
    errorWidgetLayout->addWidget(errorWidgetTip,1,Qt::AlignCenter);
    errorWidgetLayout->addStretch(10);

    _MCentralWidget->addWidget(_MError0Widget);
}

void BlueToothMainWindow::InitError1Widget()
{
    _MError1Widget = new QWidget(_MCentralWidget);
    QVBoxLayout *errorWidgetLayout = new QVBoxLayout(_MError1Widget);
    QLabel      *errorWidgetIcon   = new QLabel(_MError1Widget);
    QLabel      *errorWidgetTip    = new QLabel(_MError1Widget);

    _MError1Widget->setObjectName("error1Widget");

    errorWidgetLayout->setSpacing(10);
    errorWidgetLayout->setMargin(0);
    errorWidgetLayout->setContentsMargins(0,0,0,0);

    errorWidgetIcon->setFixedSize(56,56);
    errorWidgetTip->resize(200,30);
    errorWidgetTip->setFont(QFont("Noto Sans CJK SC",18,QFont::Bold));

    errorWidgetLayout->addStretch(10);

    if (QIcon::hasThemeIcon("dialog-warning")) {
        errorWidgetIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(56,56));
        errorWidgetLayout->addWidget(errorWidgetIcon,1,Qt::AlignCenter);
    }

    errorWidgetTip->setText(tr("Bluetooth adapter not detected !"));
    errorWidgetLayout->addWidget(errorWidgetTip,1,Qt::AlignCenter);
    errorWidgetLayout->addStretch(10);

    _MCentralWidget->addWidget(_MError1Widget);
}

void BlueToothMainWindow::InitNormalWidget()
{
    _MNormalWidget = new QWidget(_MCentralWidget);
    _MNormalWidget->setObjectName("normalWidget");

    _NormalWidgetMainLayout = new QVBoxLayout(_MNormalWidget);
    _NormalWidgetMainLayout->setSpacing(40);
    _NormalWidgetMainLayout->setContentsMargins(0,0,30,10);

    _MNormalWidgetTop = new QWidget(_MNormalWidget);
    _MNormalWidgetTop->setObjectName("NormalWidgetTop");
    _MNormalWidgetTop->setMinimumSize(582,239);
    _MNormalWidgetTop->setMaximumSize(1000,239);
//    _MNormalWidgetTop->setStyleSheet("background:red;");

    _MNormalWidgetTopMiddle = new QWidget(_MNormalWidget);
    _MNormalWidgetTopMiddle->setObjectName("NormalWidgetTopMiddle");

    _MNormalWidgetTopBottom = new QWidget(_MNormalWidget);
    _MNormalWidgetTopBottom->setObjectName("frame_bottom");
    _MNormalWidgetTopBottom->setMinimumWidth(582);
    _MNormalWidgetTopBottom->setMaximumWidth(1000);
//    _MNormalWidgetTopBottom->setStyleSheet("background:white;");

    InitNormalWidgetTop();
    InitNormalWidgetMiddle();
    InitNormalWidgetBottom();

    _NormalWidgetMainLayout->addWidget(_MNormalWidgetTop,1,Qt::AlignTop);
    _NormalWidgetMainLayout->addWidget(_MNormalWidgetTopMiddle,1,Qt::AlignTop);
    _NormalWidgetMainLayout->addWidget(_MNormalWidgetTopBottom,1,Qt::AlignTop);
    _NormalWidgetMainLayout->addStretch(10);

    _MCentralWidget->addWidget(_MNormalWidget);
}

void BlueToothMainWindow::InitNormalWidgetTop()
{
    QVBoxLayout *NormalWidgetTopLayout = new QVBoxLayout(_MNormalWidgetTop);
    NormalWidgetTopLayout->setSpacing(10);
    NormalWidgetTopLayout->setContentsMargins(0,0,0,0);

    TitleLabel *label_1 = new TitleLabel(_MNormalWidgetTop);
    label_1->setText(tr("Bluetooth"));
    label_1->resize(100,25);

    NormalWidgetTopLayout->addWidget(label_1,1,Qt::AlignTop);

    QVBoxLayout *NormalWidgetFrameLayout = new QVBoxLayout(_MNormalWidgetTop);
    NormalWidgetFrameLayout->setSpacing(2);
    NormalWidgetFrameLayout->setContentsMargins(0,0,0,0);

    NormalWidgetTopLayout->addLayout(NormalWidgetFrameLayout);

    //================================ init BtSwitchFrame =======================================
    QFrame *BtSwitchFrame = new QFrame(_MNormalWidgetTop);
    BtSwitchFrame->setMinimumWidth(582);
    BtSwitchFrame->setFrameShape(QFrame::Shape::Box);
    BtSwitchFrame->setFixedHeight(50);
    BtSwitchFrame->setAutoFillBackground(true);
    NormalWidgetFrameLayout->addWidget(BtSwitchFrame,1,Qt::AlignTop);

    QHBoxLayout *BtSwitchFrameLayout = new QHBoxLayout(BtSwitchFrame);
    BtSwitchFrameLayout->setSpacing(0);
    BtSwitchFrameLayout->setContentsMargins(16,0,16,0);

    QLabel *label_2 = new QLabel(tr("Turn on :"),BtSwitchFrame);
    label_2->setStyleSheet("QLabel{\
                           width: 56px;\
                           height: 20px;\
                           font-weight: 400;\
                           line-height: 20px;}");
    BtSwitchFrameLayout->addWidget(label_2,1,Qt::AlignLeft);

    _BtNameLabel = new BluetoothNameLabel(BtSwitchFrame,300,38);
    BtSwitchFrameLayout->addWidget(_BtNameLabel,1,Qt::AlignLeft);

    _BtSwitchBtn = new SwitchButton(BtSwitchFrame);
    BtSwitchFrameLayout->addStretch(10);
    BtSwitchFrameLayout->addWidget(_BtSwitchBtn);
    //================================ end init BtSwitchFrame =======================================


    //================================ init BtAdapterListFrame ===========================================
    QFrame *BtAdapterListFrame = new QFrame(_MNormalWidgetTop);
    BtAdapterListFrame->setMinimumWidth(582);
    BtAdapterListFrame->setFrameShape(QFrame::Shape::Box);
    BtAdapterListFrame->setFixedHeight(50);
    BtAdapterListFrame->setAutoFillBackground(true);
    NormalWidgetFrameLayout->addWidget(BtAdapterListFrame,1,Qt::AlignTop);

    QHBoxLayout *BtAdapterListFrameLayout = new QHBoxLayout(BtAdapterListFrame);
    BtAdapterListFrameLayout->setSpacing(0);
    BtAdapterListFrameLayout->setContentsMargins(16,0,16,0);

    QLabel *label_3 = new QLabel(tr("Adapter List"),BtAdapterListFrame);
    label_3->setStyleSheet("QLabel{\
                           width: 56px;\
                           height: 20px;\
                           font-weight: 400;\
                           line-height: 20px;}");
    BtAdapterListFrameLayout->addWidget(label_3);

    _AdapterListSelect = new QComboBox(BtAdapterListFrame);
    _AdapterListSelect->clear();
    _AdapterListSelect->setMinimumWidth(300);
    BtAdapterListFrameLayout->addWidget(_AdapterListSelect);
    //================================ end init BtAdapterListFrame =======================================


    //================================ init BtTrayIconShowFrame ===========================================
    QFrame *BtTrayIconShowFrame = new QFrame(_MNormalWidgetTop);
    BtTrayIconShowFrame->setMinimumWidth(582);
    BtTrayIconShowFrame->setFrameShape(QFrame::Shape::Box);
    BtTrayIconShowFrame->setFixedHeight(50);
    BtTrayIconShowFrame->setAutoFillBackground(true);
    NormalWidgetFrameLayout->addWidget(BtTrayIconShowFrame,1,Qt::AlignTop);

    QHBoxLayout *BtTrayIconShowFrameLayout = new QHBoxLayout(BtTrayIconShowFrame);
    BtTrayIconShowFrameLayout->setSpacing(0);
    BtTrayIconShowFrameLayout->setContentsMargins(16,0,16,0);

    QLabel *label_4 = new QLabel(tr("Show icon on taskbar"),BtTrayIconShowFrame);
    label_4->setStyleSheet("QLabel{\
                           width: 56px;\
                           height: 20px;\
                           font-weight: 400;\
                           line-height: 20px;}");
    BtTrayIconShowFrameLayout->addWidget(label_4);


    _BtTrayIconShow = new SwitchButton(BtTrayIconShowFrame);
    BtTrayIconShowFrameLayout->addWidget(_BtTrayIconShow);
    //================================ end init BtTrayIconShowFrame =======================================


    //================================ init BtDiscoverableFrame ===========================================
    QFrame *BtDiscoverableFrame = new QFrame(_MNormalWidgetTop);
    BtDiscoverableFrame->setMinimumWidth(582);
    BtDiscoverableFrame->setFrameShape(QFrame::Shape::Box);
    BtDiscoverableFrame->setFixedHeight(50);
    BtDiscoverableFrame->setAutoFillBackground(true);
    NormalWidgetFrameLayout->addWidget(BtDiscoverableFrame,1,Qt::AlignTop);

    QHBoxLayout *BtDiscoverableFrameLayout = new QHBoxLayout(BtDiscoverableFrame);
    BtDiscoverableFrameLayout->setSpacing(0);
    BtDiscoverableFrameLayout->setContentsMargins(16,0,16,0);

    QLabel *label_5 = new QLabel(tr("Discoverable by nearby Bluetooth devices"),BtDiscoverableFrame);
    label_5->setStyleSheet("QLabel{\
                           width: 56px;\
                           height: 20px;\
                           font-weight: 400;\
                           line-height: 20px;}");
    BtDiscoverableFrameLayout->addWidget(label_5);


    _BtDiscoverable = new SwitchButton(BtDiscoverableFrame);
    BtDiscoverableFrameLayout->addWidget(_BtDiscoverable);
    //================================ end init BtDiscoverableFrame =======================================

    NormalWidgetTopLayout->addStretch(10);
}

void BlueToothMainWindow::InitNormalWidgetMiddle()
{
    QVBoxLayout *NormalWidgetMiddleLayout = new QVBoxLayout(_MNormalWidgetTopMiddle);
    NormalWidgetMiddleLayout->setSpacing(10);
    NormalWidgetMiddleLayout->setContentsMargins(0,0,0,0);

    _NormalWidgetPairedDevLayout = new QVBoxLayout();
    _NormalWidgetPairedDevLayout->setSpacing(2);
    _NormalWidgetPairedDevLayout->setContentsMargins(0,0,0,0);

    TitleLabel *middle_label = new TitleLabel(_MNormalWidgetTopMiddle);
    middle_label->setText(tr("My Devices"));
    middle_label->resize(72,25);

    NormalWidgetMiddleLayout->addWidget(middle_label,Qt::AlignTop);
    NormalWidgetMiddleLayout->addLayout(_NormalWidgetPairedDevLayout,Qt::AlignTop);
}

void BlueToothMainWindow::InitNormalWidgetBottom()
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    QVBoxLayout *_MNormalWidgetTopBottomLayout = new QVBoxLayout(_MNormalWidgetTopBottom);
    _MNormalWidgetTopBottomLayout->setSpacing(10);
    _MNormalWidgetTopBottomLayout->setContentsMargins(0,0,0,0);

    QHBoxLayout *titleLayout = new QHBoxLayout(_MNormalWidgetTopBottom);
    titleLayout->setSpacing(10);
    titleLayout->setContentsMargins(0,0,0,0);

    //~ contents_path /bluetooth/Other Devices
    TitleLabel *label_1 = new TitleLabel(_MNormalWidgetTopBottom);
    label_1->setText(tr("Other Devices"));
    label_1->resize(72,25);
    titleLayout->addWidget(label_1, 1, Qt::AlignLeft);

    _LoadIcon = new LoadingLabel(_MNormalWidgetTopBottom);
    _LoadIcon->setFixedSize(24,24);
    _LoadIcon->setTimerStart();
    titleLayout->addWidget(_LoadIcon, 1, Qt::AlignLeft);

    _AdapterListSelect = new QComboBox(_MNormalWidgetTopBottom);
    _AdapterListSelect->clear();
    _AdapterListSelect->setMinimumWidth(120);
    _AdapterListSelect->addItem(tr("All"));
    _AdapterListSelect->addItem(tr("Audio"));
    _AdapterListSelect->addItem(tr("Peripherals"));
    _AdapterListSelect->addItem(tr("PC"));
    _AdapterListSelect->addItem(tr("Phone"));
    _AdapterListSelect->addItem(tr("Other"));
    titleLayout->addStretch(10);
    titleLayout->addWidget(_AdapterListSelect, 1, Qt::AlignRight);

    _NormalWidgetCacheDevLayout  = new QVBoxLayout(_MNormalWidgetTopBottom);
    _NormalWidgetCacheDevLayout->setSpacing(2);
    _NormalWidgetCacheDevLayout->setContentsMargins(0,0,0,0);
    _NormalWidgetCacheDevLayout->setAlignment(Qt::AlignTop);

    _MNormalWidgetTopBottomLayout->addLayout(titleLayout);
    _MNormalWidgetTopBottomLayout->addLayout(_NormalWidgetCacheDevLayout);
}

void BlueToothMainWindow::InitInterfaceUIStatus()
{

}
