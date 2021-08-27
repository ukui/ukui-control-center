#include "biometricswidget.h"
#include "ui_biometricswidget.h"
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusReply>
#include <QDBusArgument>
#include <QDBusMetaType>
#include <QProcess>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QToolButton>
#include <QDebug>
#include <QDBusInterface>
#include <QLineEdit>
#include <QMessageBox>
#include <unistd.h>
#include "SwitchButton/switchbutton.h"
#include "ImageUtil/imageutil.h"
#include "namelabel.h"
#include "changefeaturename.h"
#include "changepwddialog.h"
#include "elipsemaskwidget.h"
#include "passwdcheckutil.h"
#include <polkit-qt5-1/polkitqt1-authority.h>
#include <glib.h>
#define DEFAULTFACE "/usr/share/ukui/faces/default.png"

#define ITEMHEIGH 52
BiometricsWidget::BiometricsWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BiometricsWidget)
{
    ui->setupUi(this);
    initBioComonent();
}

BiometricsWidget::~BiometricsWidget()
{
    delete ui;
}

void BiometricsWidget::initBioComonent()
{
    m_biometricProxy = new BiometricProxy(this);


    serviceInterface = new QDBusInterface(DBUS_SERVICE,
                                          DBUS_PATH,
                                          DBUS_INTERFACE
                                          , QDBusConnection::systemBus());
    serviceInterface->setTimeout(2147483647); /* 微秒 */

    addBioFeatureWidget = new HoverWidget("");
    addBioFeatureWidget->setObjectName("addBioFeatureWidget");
    addBioFeatureWidget->setMinimumSize(QSize(580, 50));
    addBioFeatureWidget->setMaximumSize(QSize(960, 50));
    QPalette pal;
    QBrush brush = pal.highlight();  //获取window的色值
    QColor highLightColor = brush.color();
    QString stringColor = QString("rgba(%1,%2,%3)") //叠加20%白色
           .arg(highLightColor.red()*0.8 + 255*0.2)
           .arg(highLightColor.green()*0.8 + 255*0.2)
           .arg(highLightColor.blue()*0.8 + 255*0.2);

    addBioFeatureWidget->setStyleSheet(QString("HoverWidget#addBioFeatureWidget{background: palette(button);\
                                   border-radius: 4px;}\
                                   HoverWidget:hover:!pressed#addBioFeatureWidget{background: %1;\
                                   border-radius: 4px;}").arg(stringColor));
    QHBoxLayout *addBioFeatureLayout = new QHBoxLayout;

    QLabel * iconLabel = new QLabel();
    QLabel * textLabel = new QLabel(tr("Add biometric feature"));
    QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "black", 12);
    iconLabel->setPixmap(pixgray);
    iconLabel->setProperty("useIconHighlightEffect", true);
    iconLabel->setProperty("iconHighlightEffectMode", 1);
    addBioFeatureLayout->addWidget(iconLabel);
    addBioFeatureLayout->addWidget(textLabel);
    addBioFeatureLayout->addStretch();
    addBioFeatureWidget->setLayout(addBioFeatureLayout);

    // 悬浮改变Widget状态
    connect(addBioFeatureWidget, &HoverWidget::enterWidget, this, [=](){

        iconLabel->setProperty("useIconHighlightEffect", false);
        iconLabel->setProperty("iconHighlightEffectMode", 0);
        QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "white", 12);
        iconLabel->setPixmap(pixgray);
        textLabel->setStyleSheet("color: white;");
    });

    // 还原状态
    connect(addBioFeatureWidget, &HoverWidget::leaveWidget, this, [=](){

        iconLabel->setProperty("useIconHighlightEffect", true);
        iconLabel->setProperty("iconHighlightEffectMode", 1);
        QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "black", 12);
        iconLabel->setPixmap(pixgray);
        textLabel->setStyleSheet("color: palette(windowText);");
    });

    connect(addBioFeatureWidget, &HoverWidget::widgetClicked, this, [=](QString mname) {
        Q_UNUSED(mname);
        showEnrollDialog();
    });

    ui->addFeatureLayout->addWidget(addBioFeatureWidget);

    ui->bioFeatureListWidget->setStyleSheet("QListWidget::Item:hover{background:palette(base);}");
    ui->bioFeatureListWidget->setSpacing(0);

    ui->bioFeatureListWidget->setFixedHeight(biometricFeatureMap.count()*ITEMHEIGH - biometricFeatureMap.count()*6);

    connect(ui->biometrictypeBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onbiometricTypeBoxCurrentIndexChanged(int)));

    connect(ui->biometricDeviceBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onbiometricDeviceBoxCurrentIndexChanged(int)));

    connect(ui->toolButton, &QToolButton::clicked, this, [=](){
        QProcess process(this);
        process.startDetached("/usr/bin/biometric-manager");
    });

//    ui->biometricMoreBtn->setText("...");
//    connect(ui->biometricMoreBtn, &QPushButton::clicked, this, [=](){
//        biometricShowMoreInfoDialog();
//    });

    updateDevice();

    if(m_biometricProxy && m_biometricProxy->isValid())
    {
        connect(m_biometricProxy, &BiometricProxy::USBDeviceHotPlug,
            this, &BiometricsWidget::onBiometricUSBDeviceHotPlug);
    }

    enableBiometricBtn = new SwitchButton(ui->enableBiometricFrame);
    enableBiometricBtn->setChecked(getBioStatus());
    ui->enableBiometricLayout->addWidget(enableBiometricBtn);
    connect(enableBiometricBtn, &SwitchButton::checkedChanged, [=](bool checked){
        QProcess process;
        if(checked){
            process.start("bioctl enable");
            process.waitForFinished(3000);
        }else{
            process.start("bioctl disable");
            process.waitForFinished(3000);
        }
    });

    mBiometricWatcher = nullptr;
    if(!mBiometricWatcher){
        mBiometricWatcher = new QFileSystemWatcher(this);
        mBiometricWatcher->addPath(UKUI_BIOMETRIC_SYS_CONFIG_PATH);
        connect(mBiometricWatcher,&QFileSystemWatcher::fileChanged,this,[=](const QString &path){
            mBiometricWatcher->addPath(UKUI_BIOMETRIC_SYS_CONFIG_PATH);
            enableBiometricBtn->blockSignals(true);
            enableBiometricBtn->setChecked(getBioStatus());
            enableBiometricBtn->blockSignals(false);
        });
    }

    connect(ui->changePasswordPB_3, &QPushButton::clicked, this, [=](bool checked){
        Q_UNUSED(checked)
        showChangePwdDialog();
    });
    initUserInfo();
    pcgThread = new PwdChangeThread;
    ui->passwordFrame_2->hide();
}

void BiometricsWidget::initUserInfo()
{
    QDBusInterface iface("org.freedesktop.Accounts", "/org/freedesktop/Accounts",
                         "org.freedesktop.Accounts",QDBusConnection::systemBus());

    QDBusReply<QDBusObjectPath> userPath = iface.call("FindUserById", (qint64)getuid());
    if(!userPath.isValid())
        qWarning() << "Get UserPath error:" << userPath.error();
    else {
        m_user.current = true;
        m_user.logined = false;
        m_user.autologin = false;

        QString path = userPath.value().path();
        QDBusInterface iproperty("org.freedesktop.Accounts",
                                 path,
                                 "org.freedesktop.DBus.Properties",
                                 QDBusConnection::systemBus());
        QDBusReply<QMap<QString, QVariant> > reply = iproperty.call("GetAll", "org.freedesktop.Accounts.User");
        if (reply.isValid()){
            QMap<QString, QVariant> propertyMap;
            propertyMap = reply.value();
            m_user.username = propertyMap.find("UserName").value().toString();
            m_user.realname = propertyMap.find("RealName").value().toString();

            if (m_user.realname.isEmpty()){
                m_user.realname = propertyMap.find("UserName").value().toString();
            }

            if (m_user.username == QString(g_get_user_name())) {
                m_user.current = true;
                m_user.logined = true;
                m_user.noPwdLogin = false;
            }
            m_user.accounttype = propertyMap.find("AccountType").value().toInt();
            m_user.iconfile = propertyMap.find("IconFile").value().toString();
            if(m_user.iconfile.isEmpty()){
                m_user.iconfile = DEFAULTFACE;
            }
            char * iconpath = m_user.iconfile.toLatin1().data();
            if (!g_file_test(iconpath, G_FILE_TEST_EXISTS)){
                m_user.iconfile = DEFAULTFACE;
            }

            m_user.passwdtype = propertyMap.find("PasswordMode").value().toInt();
            m_user.uid = propertyMap.find("Uid").value().toInt();
            m_user.autologin = false;
            m_user.objpath = path;

        }
        else
            qDebug() << "reply failed";
    }

}

QString BiometricsWidget::_accountTypeIntToString(int type){
    QString atype;
    if (type == STANDARDUSER)
        atype = tr("Standard");
    else if (type == ADMINISTRATOR)
        atype = tr("Admin");
    else if (type == ROOT)
        atype = tr("root");

    return atype;
}

void BiometricsWidget::showChangePwdDialog()
{
        ChangePwdDialog * dialog = new ChangePwdDialog(m_user.current, m_user.username,this);
        dialog->setFace(m_user.iconfile);
        dialog->setUsername(m_user.realname);
        dialog->setAccountType(_accountTypeIntToString(m_user.accounttype));

        connect(dialog, &ChangePwdDialog::passwd_send, this, [=](QString pwd){

                changeUserPwd(pwd, m_user.username);

        });
        connect(dialog, &ChangePwdDialog::passwd_send2, this, [=](QString pwd){

            PolkitQt1::Authority::Result result;

            result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
                        "org.control.center.qt.systemdbus.action",
                        PolkitQt1::UnixProcessSubject(QCoreApplication::applicationPid()),
                        PolkitQt1::Authority::AllowUserInteraction);

            if (result == PolkitQt1::Authority::Yes){
                changeUserPwd(pwd, m_user.username);
            }


        });
        dialog->exec();

}

void BiometricsWidget::changeUserPwd(QString pwd, QString username){
    //上层已做判断，这里不去判断而直接获取
 //   UserInfomation user = allUserInfoMap.value(username);

//    UserDispatcher * userdispatcher  = new UserDispatcher(user.objpath); //继承QObject不再删除
//    QString result = userdispatcher->change_user_pwd(pwd, "");

    QDBusInterface * tmpSysinterface = new QDBusInterface("com.control.center.qt.systemdbus",
                                                          "/",
                                                          "com.control.center.interface",
                                                          QDBusConnection::systemBus());

    if (!tmpSysinterface->isValid()){
        qCritical() << "Create Client Interface Failed When : " << QDBusConnection::systemBus().lastError();
        return;
    }
    QDBusReply<int> reply = tmpSysinterface->call("setPid", QCoreApplication::applicationPid());
    if (reply.isValid()){
        tmpSysinterface->call("changeOtherUserPasswd", username, pwd);
    }

    delete tmpSysinterface;
    tmpSysinterface = nullptr;
}

bool BiometricsWidget::getBioStatus()
{
    QProcess process;
    process.start("bioctl status");
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    if (output.contains("enable", Qt::CaseInsensitive)) {
        return true;
    }
    else {
        return false;
    }
}

void BiometricsWidget::setBioStatus(bool status)
{
    enableBiometricBtn->setChecked(true);
}

void BiometricsWidget::onBiometricUSBDeviceHotPlug(int drvid, int action, int deviceNum)
{
    int savedDeviceId = -1;
    if(currentDevice)
        savedDeviceId = currentDevice->id;

    int savedCount = 0;
    for(int type : deviceMap.keys())
        savedCount += deviceMap.value(type).count();

    switch(action)
    {
    case ACTION_ATTACHED:
    {
        //插入设备后，需要更新设备列表
        updateDevice();
        if(savedDeviceId >= 0)
            setCurrentDevice(savedDeviceId);
        break;
    }
    case ACTION_DETACHED:
    {
        updateDevice();
    }

    }
}

void BiometricsWidget::setBiometricDeviceVisible(bool visible)
{
    if(!visible)
    {
        //ui->bioTitleWidget->hide();
        //ui->biometricWidget->hide();
        ui->biometricTypeFrame->hide();
        ui->biometricDeviceFrame->hide();
        ui->addFeatureWidget->hide();
        ui->bioFeatureListWidget->hide();
    }else{
        //ui->bioTitleWidget->show();
        //ui->biometricWidget->show();
        ui->biometricTypeFrame->show();
        ui->biometricDeviceFrame->show();
        ui->addFeatureWidget->show();
        ui->bioFeatureListWidget->show();
    }
}

void BiometricsWidget::updateDevice()
{
    deviceMap.clear();
    DeviceList deviceList = m_biometricProxy->GetDevList();

    QString default_name = GetDefaultDevice(QString(qgetenv("USER")));

    for(auto pDeviceInfo : deviceList)
    {
        deviceMap[pDeviceInfo->deviceType].push_back(pDeviceInfo);
    }
    ui->biometrictypeBox->clear();
    for(int type : deviceMap.keys())
    {
        ui->biometrictypeBox->addItem(DeviceType::getDeviceType_tr(type), type);
    }
    if(deviceMap.size() > 0)
    {
        DeviceInfoPtr ptr = findDeviceByName(default_name);
        if(ptr)
        {
            setCurrentDevice(default_name);
        }else{
            int index = deviceMap.keys().at(0);
            setCurrentDevice(deviceMap[index].at(0));
        }
    }

    if(deviceMap.size()<=0)
        setBiometricDeviceVisible(false);
    else
        setBiometricDeviceVisible(true);
}

void BiometricsWidget::setCurrentDevice(int drvid)
{
    DeviceInfoPtr pDeviceInfo = findDeviceById(drvid);
    if(pDeviceInfo)
    {
        setCurrentDevice(pDeviceInfo);
    }
}

void BiometricsWidget::setCurrentDevice(const QString &deviceName)
{
    DeviceInfoPtr pDeviceInfo = findDeviceByName(deviceName);
    if(pDeviceInfo)
    {
        setCurrentDevice(pDeviceInfo);
    }
}

void BiometricsWidget::setCurrentDevice(const DeviceInfoPtr &pDeviceInfo)
{
    this->currentDevice = pDeviceInfo;
    ui->biometrictypeBox->setCurrentText(DeviceType::getDeviceType_tr(pDeviceInfo->deviceType));
    ui->biometricDeviceBox->setCurrentText(pDeviceInfo->shortName);

}

DeviceInfoPtr BiometricsWidget::findDeviceById(int drvid)
{
    for(int type : deviceMap.keys())
    {
        DeviceList &deviceList = deviceMap[type];
        auto iter = std::find_if(deviceList.begin(), deviceList.end(),
                                 [&](DeviceInfoPtr ptr){
            return ptr->id == drvid;
        });
        if(iter != deviceList.end())
        {
            return *iter;
        }
    }
    return DeviceInfoPtr();
}

DeviceInfoPtr BiometricsWidget::findDeviceByName(const QString &name)
{
    for(int type : deviceMap.keys())
    {
        DeviceList &deviceList = deviceMap[type];
        auto iter = std::find_if(deviceList.begin(), deviceList.end(),
                                 [&](DeviceInfoPtr ptr){
            return ptr->shortName == name;
        });
        if(iter != deviceList.end())
        {
            return *iter;
        }
    }
    return DeviceInfoPtr();
}

bool BiometricsWidget::deviceExists(int drvid)
{
    return (findDeviceById(drvid) != nullptr);
}

bool BiometricsWidget::deviceExists(const QString &deviceName)
{
    return (findDeviceByName(deviceName) != nullptr);
}

void BiometricsWidget::onbiometricTypeBoxCurrentIndexChanged(int index)
{
    if(index < 0 || index >= deviceMap.keys().size())
    {
        return;
    }

    int type = ui->biometrictypeBox->itemData(index).toInt();
    ui->biometricDeviceBox->clear();

    for(auto &deviceInfo : deviceMap.value(type))
    {
        ui->biometricDeviceBox->addItem(deviceInfo->shortName);
    }
}

void BiometricsWidget::onbiometricDeviceBoxCurrentIndexChanged(int index)
{
    if(index < 0)
    {
        return;
    }

    int type = ui->biometrictypeBox->currentData().toInt();

    DeviceInfoPtr deviceInfo = deviceMap.value(type).at(index);
    QList<QVariant> args;

    currentDevice = deviceInfo;

    args << QVariant(deviceInfo->id)
        << QVariant((int)getuid()) << QVariant(0) << QVariant(-1);
    serviceInterface->callWithCallback("GetFeatureList", args, this,
                        SLOT(updateFeatureListCallback(QDBusMessage)),
                        SLOT(errorCallback(QDBusError)));
}

void BiometricsWidget::updateFeatureListCallback(QDBusMessage callbackReply)
{
    QList<QDBusVariant> qlist;
    FeatureInfo *featureInfo;
    int listsize;

    ui->bioFeatureListWidget->clear();
    biometricFeatureMap.clear();

    QList<QVariant> variantList = callbackReply.arguments();
    listsize = variantList[0].value<int>();
    variantList[1].value<QDBusArgument>() >> qlist;
    for (int i = 0; i < listsize; i++) {
        featureInfo = new FeatureInfo;
        qlist[i].variant().value<QDBusArgument>() >> *featureInfo;
        addFeature(featureInfo);
    }
    updateFeatureList();
}

void BiometricsWidget::updateFeatureList()
{
    ui->bioFeatureListWidget->setFixedHeight(biometricFeatureMap.count()*ITEMHEIGH + biometricFeatureMap.count()*6);
}

void BiometricsWidget::errorCallback(QDBusError error)
{

}

void BiometricsWidget::showEnrollDialog()
{
    if(ui->biometricDeviceBox->count() <= 0 || ui->biometrictypeBox->count() <= 0)
        return ;

    int index = ui->biometricDeviceBox->currentIndex();
    int type = ui->biometrictypeBox->currentData().toInt();

    if(index < 0|| type < 0)
        return ;

    DeviceInfoPtr deviceInfo = deviceMap.value(type).at(index);

    if(!deviceInfo)
        return ;

    BiometricEnrollDialog * dialog = new BiometricEnrollDialog(serviceInterface,deviceInfo->deviceType,deviceInfo->id,getuid());
    //gdxfp显示指纹图片
    if(deviceInfo->shortName == "gdxfp")
        dialog->setProcessed(true);

    if(deviceInfo->deviceType == BIOTYPE_FACE)
        dialog->setIsFace(true);

    int num=1;
    QStringList list = m_biometricProxy->getFeaturelist(deviceInfo->id,getuid(),0,-1);
    QString featurename;
    while(1){
        featurename = DeviceType::getDeviceType_tr(deviceInfo->deviceType) + QString::number(num);
        if(!list.contains(featurename))
            break;
        num++;
    }
    dialog->enroll(deviceInfo->id,getuid(),-1,featurename);

    onbiometricDeviceBoxCurrentIndexChanged(ui->biometricDeviceBox->currentIndex());

}

void BiometricsWidget::showVerifyDialog(FeatureInfo *featureinfo)
{
    DeviceInfoPtr deviceInfoPtr = findDeviceByName(featureinfo->device_shortname);
    if(!deviceInfoPtr)
            return ;

    BiometricEnrollDialog * dialog = new BiometricEnrollDialog(serviceInterface,deviceInfoPtr->deviceType,deviceInfoPtr->id,getuid());

    if(deviceInfoPtr->shortName == "huawei")
        dialog->setProcessed(true);

    if(deviceInfoPtr->deviceType == BIOTYPE_FACE)
        dialog->setIsFace(true);

    dialog->verify(deviceInfoPtr->id,getuid(),featureinfo->index);
}

void BiometricsWidget::biometricShowMoreInfoDialog()
{
    if(ui->biometricDeviceBox->count() <= 0 || ui->biometrictypeBox->count() <= 0)
        return ;

    int index = ui->biometricDeviceBox->currentIndex();
    int type = ui->biometrictypeBox->currentData().toInt();

    if(index < 0|| type < 0)
        return ;

    DeviceInfoPtr deviceInfo = deviceMap.value(type).at(index);

    if(!deviceInfo)
        return ;

    BiometricMoreInfoDialog * dialog = new BiometricMoreInfoDialog(deviceInfo);
    dialog->exec();
}

void BiometricsWidget::renameFeaturedone(FeatureInfo *featureinfo ,QString newname)
{
     QListWidgetItem *item = biometricFeatureMap.value(featureinfo->index_name);
     ui->bioFeatureListWidget->takeItem(ui->bioFeatureListWidget->row(item));
     biometricFeatureMap.remove(featureinfo->index_name);


     featureinfo->index_name = newname;
     addFeature(featureinfo);

}

void BiometricsWidget::deleteFeaturedone(FeatureInfo *featureinfo)
{

    QListWidgetItem *item = biometricFeatureMap.value(featureinfo->index_name);

    ui->bioFeatureListWidget->takeItem(ui->bioFeatureListWidget->row(item));
    biometricFeatureMap.remove(featureinfo->index_name);

    updateFeatureList();
}

void BiometricsWidget::addFeature(FeatureInfo *featureinfo)
{
    HoverWidget * baseWidget = new HoverWidget(featureinfo->index_name);
    baseWidget->setMinimumSize(550,50);
    baseWidget->setMaximumSize(960,50);
    baseWidget->setAttribute(Qt::WA_DeleteOnClose);

    //ui->currentUserFrame->setContentsMargins(16,0,16,0);

    QHBoxLayout * baseVerLayout = new QHBoxLayout(baseWidget);
    baseVerLayout->setSpacing(0);
    baseVerLayout->setMargin(0);

    QHBoxLayout * baseHorLayout = new QHBoxLayout();
    baseHorLayout->setSpacing(16);
    baseHorLayout->setMargin(0);

    QFrame * widget = new QFrame(baseWidget);
    widget->setFrameShape(QFrame::Shape::Box);
    widget->setFixedHeight(50);

    QHBoxLayout * mainHorLayout = new QHBoxLayout(widget);
    mainHorLayout->setSpacing(16);
    mainHorLayout->setContentsMargins(16, 0, 16, 0);

//    QWidget *nameWidget = new QWidget(widget);
//    QHBoxLayout *nameLayout = new QHBoxLayout(nameWidget);

//    QLabel * nameLabel = new QLabel(widget);
//    QSizePolicy nameSizePolicy = nameLabel->sizePolicy();
//    nameSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
//    nameSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
//    nameLabel->setSizePolicy(nameSizePolicy);
//    nameLabel->setText(featureinfo->index_name);

//    QLabel  *changeNameLabel = new QLabel(widget);
//    changeNameLabel->setMinimumSize(QSize(15,22));
//    changeNameLabel->setMaximumSize(QSize(15,22));
//    changeNameLabel->setPixmap(QIcon::fromTheme("document-edit-symbolic").pixmap(changeNameLabel->size()));
//    nameLayout->addWidget(nameLabel);
//    nameLayout->addWidget(changeNameLabel);

    NameLabel *nameLabel = new NameLabel(widget);
    nameLabel->setText(featureinfo->index_name);

    QString btnQss = QString("QPushButton{background: #ffffff; border-radius: 4px;}");
/*
    QLineEdit *renameEdit = new QLineEdit(widget);
    renameEdit->setFixedWidth(240);
    renameEdit->setText(featureinfo->index_name);
    renameEdit->hide();

    connect(renameEdit, &QLineEdit::editingFinished, this, [=](){
        renameEdit->hide();
        nameLabel->show();
        QString rename = renameEdit->text();
        if(rename == "" || rename == featureinfo->index_name)
            return;

        DeviceInfoPtr deviceInfoPtr = findDeviceByName(featureinfo->device_shortname);
        if(!deviceInfoPtr)
                return ;
        bool res = m_biometricProxy->renameFeature(deviceInfoPtr->id,getuid(),featureinfo->index,rename);
        renameFeaturedone(featureinfo,rename);
    });

    QPushButton * renameBtn = new QPushButton(widget);

    renameBtn->setFixedHeight(36);
    renameBtn->setMinimumWidth(88);
    renameBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    renameBtn->setText(tr("Rename"));
*/
    connect(nameLabel, &NameLabel::clicked, this, [=](){
        DeviceInfoPtr deviceInfoPtr = findDeviceByName(featureinfo->device_shortname);
        if(!deviceInfoPtr)
                return ;

        QStringList names = m_biometricProxy->getFeaturelist(deviceInfoPtr->id,getuid());
        ChangeFeatureName * dialog = new ChangeFeatureName(names);
        connect(dialog, &ChangeFeatureName::sendNewName, [=](QString rename){
            if(rename == "" || rename == featureinfo->index_name)
                return;

            DeviceInfoPtr deviceInfoPtr = findDeviceByName(featureinfo->device_shortname);
            if(!deviceInfoPtr)
                    return ;
            bool res = m_biometricProxy->renameFeature(deviceInfoPtr->id,getuid(),featureinfo->index,rename);
            renameFeaturedone(featureinfo,rename);
        });
        dialog->exec();
    });
/*
    renameBtn->hide();

    QPushButton * verifyBtn = new QPushButton(widget);

    verifyBtn->setFixedHeight(36);
    verifyBtn->setMinimumWidth(88);
    verifyBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    verifyBtn->setText(tr("Verify"));

    connect(verifyBtn, &QPushButton::clicked, this, [=](bool checked){
        Q_UNUSED(checked)
        showVerifyDialog(featureinfo);
    });

    renameBtn->hide();
    verifyBtn->hide();
*/
    mainHorLayout->addWidget(nameLabel);
//    mainHorLayout->addWidget(renameEdit);
    mainHorLayout->addStretch();
//    mainHorLayout->addWidget(renameBtn);
//    mainHorLayout->addWidget(verifyBtn);

    widget->setLayout(mainHorLayout);

    QPushButton * delBtn = new QPushButton(baseWidget);
    delBtn->setFixedSize(70, 36);
    delBtn->setText(tr("Delete"));
//    delBtn->setStyleSheet("QPushButton{background: #FA6056; border-radius: 4px}");
    delBtn->hide();
    connect(delBtn, &QPushButton::clicked, this, [=](bool checked){
        Q_UNUSED(checked)
        DeviceInfoPtr deviceInfoPtr = findDeviceByName(featureinfo->device_shortname);
        if(!deviceInfoPtr)
                return ;
        bool res = m_biometricProxy->deleteFeature(deviceInfoPtr->id,getuid(),featureinfo->index,featureinfo->index);
        if(!res){
             deleteFeaturedone(featureinfo);
        }
    });

    connect(baseWidget, &HoverWidget::enterWidget, this, [=](QString name){
        Q_UNUSED(name)
//        renameBtn->show();
    //驱动gdxfp不支持验证
//        if(featureinfo->device_shortname != "gdxfp")
//        verifyBtn->show();
        delBtn->show();
    });
    connect(baseWidget, &HoverWidget::leaveWidget, this, [=](QString name){
        Q_UNUSED(name)
//        renameBtn->hide();
//        verifyBtn->hide();
        delBtn->hide();
    });

    baseHorLayout->addWidget(widget);
    baseHorLayout->addWidget(delBtn, Qt::AlignVCenter);
    baseHorLayout->addSpacing(4);

    baseVerLayout->addLayout(baseHorLayout);

    baseWidget->setLayout(baseVerLayout);

    QListWidgetItem * item = new QListWidgetItem(ui->bioFeatureListWidget);
    item->setSizeHint(QSize(QSizePolicy::Expanding, ITEMHEIGH));
    item->setData(Qt::UserRole, QVariant(featureinfo->index_name));
    ui->bioFeatureListWidget->setItemWidget(item, baseWidget);

    biometricFeatureMap.insert(featureinfo->index_name, item);
}
