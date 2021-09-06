#include "operatingmode.h"
#include "ui_operatingmode.h"

OperatingMode::OperatingMode() : mFirstLoad(true)
{
    pluginName = tr("Operation Mode");
    pluginType = SYSTEM;
}

OperatingMode::~OperatingMode() {
    if (!mFirstLoad) {
        delete ui;
    }
}

QWidget *OperatingMode::get_plugin_ui() {
    if (mFirstLoad) {
        mFirstLoad = false;
        ui = new Ui::OperatingMode;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_StyledBackground,true);
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);

        m_statusSessionDbus = new QDBusInterface("com.kylin.statusmanager.interface",
                                                  "/",
                                                  "com.kylin.statusmanager.interface",
                                                  QDBusConnection::sessionBus(), this);
        QDBusReply<bool> is_tabletmode = m_statusSessionDbus->call("get_current_tabletmode");
        if(is_tabletmode){
            cur_mode = "pad";
        } else {
            cur_mode = "pc";
        }
        initConnection();
        initComponent();
    }
    return pluginWidget;
}

QString OperatingMode::get_plugin_name() {
    return pluginName;
}

int OperatingMode::get_plugin_type() {
    return pluginType;
}

void OperatingMode::plugin_delay_control() {

}

const QString OperatingMode::name() const {

    return QStringLiteral("OperatingMode");
}

void OperatingMode::initConnection() {

    connect(m_statusSessionDbus, SIGNAL(mode_change_signal(bool)), this, SLOT(dbusConnect(bool)));
}

void OperatingMode::dbusConnect(bool tablet_mode)
{
    if(tablet_mode){
        padFrame->setStyleSheet("QFrame#padFrame{background: rgba(47, 179, 232, 0.1);"
                               "border: 2px solid #2FB3E8;"
                               "border-radius: 8px;"
                               "color: palette(text)}");
        pcFrame->setStyleSheet("QFrame#pcFrame{background: palette(base);"
                                "border: none;"
                                "border-radius: 8px;"
                                "color: palette(text)}");
    } else {
        pcFrame->setStyleSheet("QFrame#pcFrame{background: rgba(47, 179, 232, 0.1);"
                               "border: 2px solid #2FB3E8;"
                               "border-radius: 8px;"
                               "color: palette(text)}");
        padFrame->setStyleSheet("QFrame#padFrame{background: palette(base);"
                                "border: none;"
                                "border-radius: 8px;"
                                "color: palette(text)}");
    }
}

void OperatingMode::initComponent() {
    //标题行
    titleFrame = new QFrame;
    QHBoxLayout * titleLyt = new QHBoxLayout(titleFrame);
    titleLyt->setContentsMargins(16, 0, 0, 0);
    QLabel * titleLabel = new QLabel;
    ////~ contents_path /OperatingMode/Operating Mode
    titleLabel->setText(tr("Operating Mode"));
    titleLabel->setFixedSize(120, 20);
    titleLabel->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");
    titleFrame->setLayout(titleLyt);
    titleLyt->addWidget(titleLabel);
    titleLyt->addStretch();


    //操作模式切换界面
    modeFrame = new QFrame;
    modeFrame->setFrameShape(QFrame::Shape::NoFrame);
    modeFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    modeFrame->setMinimumWidth(0);
    modeFrame->setMaximumWidth(16777215);
    modeFrame->setFixedHeight(132);
    QHBoxLayout * modeLyt = new QHBoxLayout(modeFrame);
    modeLyt->setContentsMargins(0,0,0,0);

    //PC模式
    pcFrame = new QFrame;
    pcFrame->setSizeIncrement(QSizePolicy::Expanding, QSizePolicy::Expanding);
//    pcFrame->setFixedSize(322, 104);
    pcFrame->setMinimumWidth(0);
    pcFrame->setMaximumWidth(16777215);
    pcFrame->setMinimumHeight(132);
    pcFrame->setMaximumHeight(132);
    pcFrame->setFrameShape(QFrame::Shape::NoFrame);
    pcFrame->setAutoFillBackground(true);
    QHBoxLayout * pcLyt = new QHBoxLayout(pcFrame);
    pcLyt->setContentsMargins(0,0,0,0);
    pcLyt->setSpacing(16);
    QLabel * iconLabel = new QLabel;
    iconLabel->setPixmap(QPixmap("://img/plugins/operatingmode/PCMode.png"));
    QFrame * pcdscFrame = new QFrame;
    QVBoxLayout *pcdscLyt = new QVBoxLayout(pcdscFrame);
    pcdscLyt->setSpacing(8);
    QLabel * pcTitleLabel = new QLabel;
    ////~ contents_path /OperatingMode/PC Mode
    pcTitleLabel->setText(tr("PC Mode"));
    pcTitleLabel->setStyleSheet("font-size: 16px;"
                                "font-weight: bold;"
                                "line-height: 24px;");
//    pcTitleLabel->setFont(QFont("",14,75));
    QLabel * pcdscLabel = new QLabel;
    pcdscLabel->setText(tr("Operate with mouse and keyboard"));
//    pcdscLabel->setWordWrap(true);
    pcdscLyt->addStretch();
    pcdscLyt->addWidget(pcTitleLabel);
    pcdscLyt->addWidget(pcdscLabel);
    pcdscLyt->addStretch();

    pcLyt->addStretch();
    pcLyt->addWidget(iconLabel);
    pcLyt->addWidget(pcdscFrame);
    pcLyt->addStretch();
    pcFrame->setLayout(pcLyt);
    pcFrame->installEventFilter(this);

    //平板模式
    padFrame = new QFrame;
    padFrame->setSizeIncrement(QSizePolicy::Expanding, QSizePolicy::Expanding);
//    pcFrame->setFixedSize(322, 104);
    padFrame->setMinimumWidth(0);
    padFrame->setMaximumWidth(16777215);
    padFrame->setMinimumHeight(132);
    padFrame->setMaximumHeight(132);
    padFrame->setFrameShape(QFrame::Shape::NoFrame);
    padFrame->setAutoFillBackground(true);
    QHBoxLayout * padLyt = new QHBoxLayout(padFrame);
    padLyt->setContentsMargins(0,0,0,0);
    padLyt->setSpacing(16);
    QLabel * iconLabel_2 = new QLabel;
    iconLabel_2->setPixmap(QPixmap("://img/plugins/operatingmode/PadMode.png"));
    QFrame * paddscFrame = new QFrame;
    QVBoxLayout *paddscLyt = new QVBoxLayout(paddscFrame);
    paddscLyt->setSpacing(8);
    QLabel * padTitleLabel = new QLabel;
    ////~ contents_path /OperatingMode/Pad Mode
    padTitleLabel->setText(tr("Pad Mode"));
    padTitleLabel->setStyleSheet("font-size: 16px;"
                                 "font-weight: bold;"
                                 "line-height: 24px;");
//    padTitleLabel->setFont(QFont("",14,75));
    QLabel * paddscLabel = new QLabel;
    paddscLabel->setText(tr("Operate with fingers"));
//    pcdscLabel->setWordWrap(true);
    paddscLyt->addStretch();
    paddscLyt->addWidget(padTitleLabel);
    paddscLyt->addWidget(paddscLabel);
    paddscLyt->addStretch();

    padLyt->addStretch();
    padLyt->addWidget(iconLabel_2);
    padLyt->addWidget(paddscFrame);
    padLyt->addStretch();
    padFrame->setLayout(padLyt);
    padFrame->installEventFilter(this);

    modeLyt->addWidget(pcFrame);
    modeLyt->addWidget(padFrame);

    //自动切换
    autoFrame = new QFrame;
    autoFrame->setFrameShape(QFrame::Shape::Box);
    QHBoxLayout * autoLyt = new QHBoxLayout(autoFrame);
    autoLyt->setSpacing(16);
    autoLyt->setContentsMargins(16,20,16,20);
    QLabel * autoLabel = new QLabel;
    ////~ contents_path /OperatingMode/switch automatically
    autoLabel->setText(tr("switch automatically"));
    autoLabel->setStyleSheet("font-weight: 400;"
                            "line-height: 24px;"
                            "font-size: 16px;");
    QLabel * dscLabel = new QLabel;
    dscLabel->setText(tr("Switch automatically when screen rotated."));
    dscLabel->setStyleSheet("font-weight: 400;"
                            "line-height: 24px;"
                            "font-size: 16px;");
    SwitchButton * autoBtn = new SwitchButton;
    autoLyt->addWidget(autoLabel);
    autoLyt->addWidget(dscLabel);
    autoLyt->addStretch();
    autoLyt->addWidget(autoBtn);
    autoFrame->setLayout(autoLyt);

    ui->mainLayout->addWidget(titleFrame);
    ui->mainLayout->addWidget(modeFrame);

    ui->mainLayout->addWidget(autoFrame);
    ui->mainLayout->addStretch();

    //暂时隐藏设置自动转换模式的功能
    autoFrame->setVisible(true);

    pcFrame->setObjectName("pcFrame");
    padFrame->setObjectName("padFrame");
    if(cur_mode == QString::fromLocal8Bit("pc")){
        pcFrame->setStyleSheet("QFrame#pcFrame{background: rgba(47, 179, 232, 0.1);"
                               "border: 2px solid #2FB3E8;"
                               "border-radius: 8px;"
                               "color: palette(text)}");
        padFrame->setStyleSheet("QFrame#padFrame{background: palette(base);"
                                "border: none;"
                                "border-radius: 8px;"
                                "color: palette(text)}");
    } else if(cur_mode == QString::fromLocal8Bit("pad")) {
        padFrame->setStyleSheet("QFrame#padFrame{background: rgba(47, 179, 232, 0.1);"
                               "border: 2px solid #2FB3E8;"
                               "border-radius: 8px;"
                               "color: palette(text)}");
        pcFrame->setStyleSheet("QFrame#pcFrame{background: palette(base);"
                                "border: none;"
                                "border-radius: 8px;"
                                "color: palette(text)}");
    }







    //init autoBtn status
    if(QGSettings::isSchemaInstalled("org.kylin-mode-switch"))
    {
        qDebug()<<"org.kylin-mode-switch"<<"=="<<QGSettings::isSchemaInstalled("org.kylin-mode-switch");
        QGSettings * mode_monitor =  new QGSettings("org.kylin-mode-switch", QByteArray(),this);
        if(mode_monitor->get("modemonitor").toBool()){
            qDebug()<<"modemonitor"<<"true";
            autoBtn->setChecked(true);
        } else {
            qDebug()<<"modemonitor"<<"true";
            autoBtn->setChecked(false);
        }
        connect(autoBtn, &SwitchButton::checkedChanged, this, [=](){
            if(autoBtn->isChecked()){
                qDebug()<<"isChecked"<<"true";
                mode_monitor->set("modemonitor", true);
            } else {
                mode_monitor->set("modemonitor", false);
                qDebug()<<"isChecked"<<"false";
            }
        });
    }else{
        qDebug()<<"org.kylin-mode-switch"<<"==="<<QGSettings::isSchemaInstalled("org.kylin-mode-switch");

    }



}

bool OperatingMode::eventFilter(QObject *w, QEvent *e){
    if(w == pcFrame) {
        if (e->type() == QEvent::MouseButtonPress) {
//            if(cur_mode == QString::fromLocal8Bit("pad")){
            QDBusReply<bool> is_tabletmode = m_statusSessionDbus->call("get_current_tabletmode");
            if(is_tabletmode) {
//                m_statusSessionDbus->call("set_tabletmode", false, "ukcc", "set_tabletmode");
                pcFrame->setStyleSheet("QFrame#pcFrame{background: rgba(47, 179, 232, 0.1);"
                                        "border: 2px solid #2FB3E8;"
                                        "border-radius: 8px;"
                                        "color: palette(text)}");
                padFrame->setStyleSheet("QFrame#padFrame{background: palette(base);"
                                        "border: none;"
                                        "border-radius: 8px;"
                                        "color: palette(text)}");
                cur_mode = "pc";
            }
//            qDebug()<<"pc pressed";
            return true;
        }
    } else if(w == padFrame){
        if (e->type() == QEvent::MouseButtonPress) {
//            if(cur_mode == QString::fromLocal8Bit("pc")){
            QDBusReply<bool> is_tabletmode = m_statusSessionDbus->call("get_current_tabletmode");
            if(!is_tabletmode) {
//                m_statusSessionDbus->call("set_tabletmode", true, "ukcc", "set_tabletmode");
                padFrame->setStyleSheet("QFrame#padFrame{background: rgba(47, 179, 232, 0.1);"
                                        "border: 2px solid #2FB3E8;"
                                        "border-radius: 8px;"
                                        "color: palette(text)}");
                pcFrame->setStyleSheet("QFrame#pcFrame{background: palette(base);"
                                        "border: none;"
                                        "border-radius: 8px;"
                                        "color: palette(text)}");
                cur_mode = "pad";
            }
//            qDebug()<<"pad pressed";
            return true;
        }
    }
    return QObject::eventFilter(w,e);
}


