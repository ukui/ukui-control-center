/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include <QWidget>

#include "about.h"
#include "ui_about.h"
#include "memoryentry.h"

#include <QProcess>
#include <QFile>
#include <QDebug>
#include <QStorageInfo>
#include <sys/types.h>
#include <unistd.h>

#include <QLibrary>
#include <QPluginLoader>
#include "HpQRCodeInterface.h"
const QString TYPEVERSION = "Kylin V10";
const QString UbuntuVesion = "Ubuntu 20.04 LTS";
const QString UbuntuVesionEnhance = "Ubuntu 20.04.1 LTS";

About::About()  : mFirstLoad(true)
{
    //~ contents_path /about/About & Support
    pluginName = tr("About & Support");
    pluginType = OTHER;
}

About::~About()
{
    if (!mFirstLoad) {
        delete ui;
    }
}
void About::initUI(){
    QString locale = QLocale::system().name();
    QGraphicsOpacityEffect *opacityEffect_0=new QGraphicsOpacityEffect;
    QGraphicsOpacityEffect *opacityEffect_1=new QGraphicsOpacityEffect;
    QGraphicsOpacityEffect *opacityEffect_2=new QGraphicsOpacityEffect;
    QGraphicsOpacityEffect *opacityEffect_3=new QGraphicsOpacityEffect;
    QGraphicsOpacityEffect *opacityEffect_4=new QGraphicsOpacityEffect;
    QGraphicsOpacityEffect *opacityEffect_5=new QGraphicsOpacityEffect;
    QGraphicsOpacityEffect *opacityEffect_6=new QGraphicsOpacityEffect;
    ui->versionContent->setGraphicsEffect(opacityEffect_0);
    ui->systemVersionLabel->setGraphicsEffect(opacityEffect_1);
    ui->kernalContent->setGraphicsEffect(opacityEffect_2);
    ui->cpuContent->setGraphicsEffect(opacityEffect_3);
    ui->memoryContent->setGraphicsEffect(opacityEffect_4);
    ui->diskContent->setGraphicsEffect(opacityEffect_5);
    ui->desktopContent->setGraphicsEffect(opacityEffect_6);
    opacityEffect_0->setOpacity(0.65);
    opacityEffect_1->setOpacity(0.65);
    opacityEffect_2->setOpacity(0.65);
    opacityEffect_3->setOpacity(0.65);
    opacityEffect_4->setOpacity(0.65);
    opacityEffect_5->setOpacity(0.65);
    opacityEffect_6->setOpacity(0.65);
    ui->titleLabel->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");

//    ui->pushButton_2->setText(tr("Learn more HP user manual>>"));
//    ui->pushButton_2->setStyleSheet("background: transparent;color:#2FB3E8;font-size:16px;font-family:Microsoft YaHei;"
//                  "border-width:1px;border-style:none none solid none;border-color:#2FB3E8;");
    ui->frame->setObjectName("frame");
    ui->frame->setStyleSheet("QFrame#frame{border-radius:12px;background-color:palette(base)}");
    ui->frame_2->setObjectName("frame2");
    ui->frame_2->setStyleSheet("QFrame#frame2{border-radius:12px;background-color:palette(base)}");
    ui->label_9->setStyleSheet("background:transparent");
    ui->label_9->setText(tr("Wechat code scanning obtains HP professional technical support"));
    ui->label_9->setWordWrap(true);

    QPixmap pixmap=(QString(":/help-app.png"));
    pixmap = pixmap.scaled(ui->label->size());
    ui->label->setStyleSheet("background:transparent");
    ui->label->setPixmap(pixmap);

    ui->label_2->setStyleSheet("background:transparent");
    ui->label_2->setText(tr("See more about Kylin Tianqi edu platform"));
    ui->label_2->setWordWrap(true);
    ui->pushButton->setText(tr("See user manual>>"));
    ui->pushButton->setStyleSheet("background: transparent;color:#2FB3E8;font-size:16px;font-family:Microsoft YaHei;"
                  "border-width:1px;border-style:none none solid none;border-color:#2FB3E8;");
    ui->label_4->setText(tr("Support"));
    if ("zh_CN" == locale) {
        ui->versionLabel->setMinimumSize(80,0);
        ui->versionLabel->setMaximumSize(80,16777215);
        ui->kernalLabel->setMinimumSize(80,0);
        ui->kernalLabel->setMaximumSize(80,16777215);
        ui->systemVersion->setMaximumSize(80,16777215);
        ui->cpuLabel->setMinimumSize(80,0);
        ui->cpuLabel->setMaximumSize(80,16777215);
        ui->memoryLabel->setMaximumSize(80,16777215);
        ui->label_3->setMaximumSize(80,16777215);
        ui->label_6->setMaximumSize(80,16777215);
        ui->diskLabel->setMaximumSize(80,16777215);
    } else if ("en_US" == locale) {
        ui->versionLabel->setMinimumSize(95,0);
        ui->versionLabel->setMaximumSize(95,16777215);
        ui->kernalLabel->setMinimumSize(95,0);
        ui->kernalLabel->setMaximumSize(95,16777215);
        ui->systemVersion->setMaximumSize(95,16777215);
        ui->cpuLabel->setMinimumSize(95,0);
        ui->cpuLabel->setMaximumSize(95,16777215);
        ui->memoryLabel->setMaximumSize(95,16777215);
        ui->label_3->setMaximumSize(95,16777215);
        ui->label_6->setMaximumSize(95,16777215);
        ui->diskLabel->setMaximumSize(95,16777215);
    }
#ifdef WIN32
    QPluginLoader loader("../HpQRCodePlugin/hp-qrcode-plugind.dll");
#else
    QPluginLoader loader("/usr/lib/x86_64-linux-gnu/hp-qrcode-plugin/libhp-qrcode-plugin.so");

#endif
    QObject *plugin = loader.instance();
    if (plugin) {
        app = qobject_cast<hp::QRCodeInterface*>(plugin);
        ui->widget_7 = app->createWidget(ui->widget_4);
        //ui->widget_7->setParent(ui->widget);
       // ui->label
        //QVBoxLayout* mainLayout = new QVBoxLayout(this);
        //mainLayout->addWidget(w);
        //ui->horizontalLayout->addWidget(w);
         // ui->widget->addWidget(w);
        //std::function<void(int, QString)> cb = std::bind(&MainWindow::getCallBack, this);
//         app->registerCallBack([this](int status, QString msg)
//         {
//            getCallBack(status, msg);
//         });
    }
    else{
        qDebug() << "加载插件失败";
    }

    ui->pushButton_2->setText(tr("Learn more HP user manual>>"));
    ui->pushButton_2->setStyleSheet("background: transparent;color:#2FB3E8;font-size:16px;font-family:Microsoft YaHei;"
                  "border-width:1px;border-style:none none solid none;border-color:#2FB3E8;");
    connect(ui->pushButton_2,&QPushButton::clicked,this,&About::on_pushButton_2_clicked);
}
QString About::get_plugin_name(){
    return pluginName;
}

int About::get_plugin_type(){
    return pluginType;
}

QWidget *About::get_plugin_ui(){
    if (mFirstLoad) {
        mFirstLoad = false;
        ui = new Ui::About;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_StyledBackground,true);
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);
        connect(ui->pushButton,&QPushButton::clicked,this,&About::on_pushButton_clicked);

        _data_init();
        initUI();
        initSearchText();
        setupDesktopComponent();
        setupKernelCompenent();
        setupSystemVersion();
        setupVersionCompenent();
        setupSerialComponent();
    }
    return pluginWidget;
}

void About::plugin_delay_control(){

}

const QString About::name() const {
    return QStringLiteral("about");
}

void About::_data_init(){
    QStringList infoList = computerinfo.split("\n\n");
    QString available;
    if (infoList.length() > 1){
        available = infoList.at(1);
    }
    else {
        available = "";
    }
    if (available != ""){
        for (QString line : available.split("\n")){
            if (!line.contains(":"))
                continue;
            QStringList lineList = line.split(":");
            infoMap.insert(lineList.at(0).simplified(), lineList.at(1).simplified());
        }
    }
}

void About::setupDesktopComponent() {
    //获取当前桌面环境
    QString dEnv;
    foreach (dEnv, QProcess::systemEnvironment()){
        if (dEnv.startsWith("XDG_CURRENT_DESKTOP"))
            break;
    }

    //设置当前桌面环境信息
    if (!dEnv.isEmpty()) {
        QString desktop = dEnv.section("=", -1, -1);
        ui->desktopContent->setText(desktop);
    }

//    QProcess *userPro = new QProcess();
//    userPro->start("whoami");
//    userPro->waitForFinished();

//    std::string output = userPro->readAll().toStdString();

//    QString name = qgetenv("USER");
//    if (name.isEmpty()) {
//        name = qgetenv("USERNAME");
//    }
//    ui->userContent->setText(name);
    /*获取用户名（微信名）*/
    qlonglong uid = getuid();
    QDBusInterface user("org.freedesktop.Accounts",
                        "/org/freedesktop/Accounts",
                        "org.freedesktop.Accounts",
                        QDBusConnection::systemBus());
    QDBusMessage result = user.call("FindUserById", uid);
    QString userpath = result.arguments().value(0).value<QDBusObjectPath>().path();
    userInterface = new QDBusInterface ("org.freedesktop.Accounts",
                                          userpath,
                                        "org.freedesktop.Accounts.User",
                                        QDBusConnection::systemBus());
    QString userName = userInterface->property("RealName").value<QString>();

    ui->userContent->setText(userName);
}
void About::setupSystemVersion() {
    QString sysVersion = "/etc/apt/ota_version";
    QFile file(sysVersion);
    if (file.exists() == false)
        return;

    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray t = file.readAll();
    file.close();
    QString content = t;
    QStringList versionResult1;
    foreach (QString versionResult, content.split("\n")) {
        if (versionResult == NULL)
            continue;
        versionResult1.append(versionResult);
    }
    QString content1 = versionResult1.at(1);
    content1.replace('"', "");
    QString content2 = content1.split(" = ").at(1);
    ui->systemVersionLabel->setText(content2);
}
void About::setupKernelCompenent() {
//    ui->systemVersionLabel->setText(sysVersionResult1);
    //获取系统内核
    QProcess process_1;
    process_1.start("cat /proc/version");
    process_1.waitForFinished();
    QString result = process_1.readAllStandardOutput();
    QString result1 = result.split(" ").at(0);
    QString result2 = result.split(" ").at(2);
    QString kernel = result1 + " " + result2;
    //获取CPU信息
    QProcess process_2;
    process_2.start("cat /proc/cpuinfo");
    process_2.waitForFinished();
    QString cpuInfo = process_2.readAllStandardOutput();
    QString cpuRestult1 =  cpuInfo.split("model name	: ").at(1);
    QString cpuRestult2 =  cpuRestult1.split("stepping").at(0);
    QString cpuRestult3 =  cpuRestult2.split("\n").at(0);
    //获取内存信息
    QProcess process_3;
    process_3.start("free -m");
    process_3.waitForFinished();
    QString memSize = process_3.readAllStandardOutput();
    QString memResult = "";
    for (int count = 1;count < memSize.length();count++){
        if (memSize.at(count) == " "){
            if (memSize.at(count - 1)  != " "){
                memResult = memResult + " ";
            } else {
                continue;
            }
        } else {
            memResult = memResult + memSize.at(count);
        }
    }
    QStringList menInfo = memResult.replace("\n"," ").split(" ");
    int16_t memTotal = (menInfo.at(7).toDouble()+1024) / 1024;
    double memAvaliable = menInfo.at(12).toDouble() / 1024;
    QString str = QString::number(memTotal, 10) + "GB (" + QString::number(memAvaliable, 'f', 1)+ "GB "+tr("avaliable") +")";
    //获取硬盘信息
    QProcess process_4;
    process_4.start("lsblk");
    process_4.waitForFinished();
    QString diskSize = process_4.readAllStandardOutput();
    foreach (QString diskResult, diskSize.split("\n")) {
        if (diskResult == NULL)
            continue;
        diskResult.replace(QRegExp("[\\s]+"), " ");
        diskInfo = diskResult.split(" ");
        if (diskInfo.at(5) == "disk" && (!diskInfo.at(0).contains("fd")) &&
               (diskInfo.at(2)!="1")) { //过滤掉可移动硬盘
            QStringList totalSize;
            totalSize.append(diskInfo.at(3));
            disk2.insert(diskInfo.at(0),totalSize); //硬盘信息分类存储，用以兼容多硬盘电脑
        }
    }
    QProcess process_5;
    process_5.start("df -l ");
    process_5.waitForFinished();
    QString diskSize2 = process_5.readAllStandardOutput();
    double availSize=0;
    QStringList diskInfo2;
    QMap<QString, QStringList>::iterator iter;
    for(iter=disk2.begin();iter!=disk2.end();iter++)
    {
        foreach (QString diskResult, diskSize2.split("\n")) {
            if (diskResult == NULL)
                continue;
            diskResult.replace(QRegExp("[\\s]+"), " ");
            diskInfo2 = diskResult.split(" ");
            if(diskInfo2.at(0).contains(iter.key())){
                availSize += diskInfo2.at(3).toInt();
            }
        }
        QString diskAvailable = QString::number((availSize/1024/1024), 'f', 1) + "G";
        iter.value().append(diskAvailable);
        availSize = 0;
    }

    int count = 0;
    for(iter=disk2.begin();iter!=disk2.end();iter++){
        if (disk2.size() == 1) {
            ui->diskContent->show();
            ui->diskLabel->show();
            ui->diskContent->setText(iter.value().at(0) + "B (" + iter.value().at(1) + "B "+ tr("avaliable") +")");
        }
        else {
            ui->diskContent->hide();
            ui->diskLabel->hide();
            QHBoxLayout * layout = new QHBoxLayout;
            QLabel *label = new QLabel;
            label->setText(tr("Disk") + QString::number(count + 1));
            QLabel *diskLabel = new QLabel;
            diskLabel->setText(iter.value().at(0) + "B (" + iter.value().at(1) + "B "+ tr("avaliable") +")");
            QString locale = QLocale::system().name();
            if (locale == "zh_CN") {
               label->setMinimumSize(60,0);
               label->setMaximumSize(60,16777215);
            } else {
               label->setMinimumSize(75,0);
               label->setMaximumSize(75,16777215);
            }

            layout->addWidget(label);
            layout->addWidget(diskLabel);
            layout->addStretch();
            ui->verticalLayout_8->addLayout(layout);

        }
    }



    ui->cpuContent->setText(cpuRestult3);
//    ui->cpuContent->setWordWrap(true);
    ui->kernalContent->setText(kernel);
    ui->kernalContent->setWordWrap(true);
    ui->memoryContent->setText(str);
    ui->memoryContent->setWordWrap(true);
}

void About::setupVersionCompenent() {
    QString versionPath = "/etc/os-release";
    QStringList osRes =  readFile(versionPath);
    QString version;

    QProcess process;
    process.start("dpkg -l ukui-control-center");
    process.waitForFinished();
    QString vNumber = process.readAllStandardOutput();
    QStringList vNumberInfo;
    foreach (QString vNumberResult, vNumber.split("\n")) {
        if (vNumberResult == NULL)
            continue;
        vNumberResult.replace(QRegExp("[\\s]+"), " ");
        vNumberInfo = vNumberResult.split(" ");
    }
    QString versionResault = vNumberInfo.at(2);
    QString versionInfo;
    if (versionResault.contains(" + ")) {
        versionInfo = versionResault.split("+").at(1);
    } else {
        versionInfo = versionResault;
    }
    for (QString str : osRes) {
        if (str.contains("PRETTY_NAME=")) {
            int index = str.indexOf("PRETTY_NAME=");
            int startIndex = index + 13;
            int length = str.length() - startIndex - 1;
            version = str.mid(startIndex, length);
        }
    }
    if (UbuntuVesion == version) {
        version = "UbuntuKylin 20.04 LTS";
    } else if (UbuntuVesionEnhance == version) {
        version = "UbuntuKylin 20.04.1 LTS";
    }
    ui->versionContent->setText(tr("Kylin Linux Desktop(EDU) V10"));
    //        qDebug()<<"version----->"<<version<<endl;
    //        ui->activeFrame->setVisible(false);
    //        ui->trialButton->setVisible(false);
    //设置桌面环境LOGO
    QPixmap pixmap=(QString("://img/dropArrow/Logo.png"));
    ui->logoLabel->setStyleSheet("background:transparent");
    //        ui->logoLabel->setFixedSize(220,220);
    ui->logoLabel->setPixmap(pixmap);
    //隐藏激活相关功能
    ui->activeFrame->setVisible(false);
    ui->trialButton->setVisible(false);
}
const QPixmap About::loadSvg(const QString &fileName, QString color)
{
    int size = 24;
    const auto ratio = qApp->devicePixelRatio();
    if ( 2 == ratio) {
        size = 48;
    } else if (3 == ratio) {
        size = 96;
    }
    QPixmap pixmap(size, size);
    QSvgRenderer renderer(fileName);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);
    renderer.render(&painter);
    painter.end();

    pixmap.setDevicePixelRatio(ratio);
    return drawSymbolicColoredPixmap(pixmap, color);
}
QPixmap About::drawSymbolicColoredPixmap(const QPixmap &source, QString cgColor)
{
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                if ("white" == cgColor) {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                } else if ("black" == cgColor) {
                    color.setRed(0);
                    color.setGreen(0);
                    color.setBlue(0);
//                    color.setAlpha(0.1);
                    color.setAlphaF(0.9);
                    img.setPixelColor(x, y, color);
                } else if ("gray" == cgColor) {
                    color.setRed(152);
                    color.setGreen(163);
                    color.setBlue(164);
                    img.setPixelColor(x, y, color);
                } else if ("blue" == cgColor){
                    color.setRed(61);
                    color.setGreen(107);
                    color.setBlue(229);
                    img.setPixelColor(x, y, color);
                } else {
                    return source;
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}
void About::setupSerialComponent() {
//    ui->trialButton->setFlat(true);
//    ui->trialButton->setStyleSheet("text-align: left");
    //ubuntukylin youker DBus interface
    QDBusInterface *activeInterface = new QDBusInterface("org.freedesktop.activation",
                                     "/org/freedesktop/activation",
                                     "org.freedesktop.activation.interface",
                                     QDBusConnection::systemBus());
    if (!activeInterface->isValid()) {
        qDebug() << "Create active Interface Failed When Get Computer info: " << QDBusConnection::systemBus().lastError();
        return;
    }

    int status;
    QDBusReply<int> activeStatus;
    activeStatus  = activeInterface ->call("status");
    if (!activeStatus.isValid()) {
        qDebug()<<"activeStatus is invalid"<<endl;
    } else {
        status = activeStatus.value();
    }


    QString serial;
    QDBusReply<QString> serialReply;
    serialReply  = activeInterface ->call("serial_number");
    if (!serialReply.isValid()) {
        qDebug()<<"serialReply is invalid"<<endl;
    } else {
        serial = serialReply.value();
    }

    qDebug()<<"status and serial is----->"<<status<<" "<<serial<<endl;

    if (status != 1) {
        ui->activeContent->setText(tr("Inactivated"));
    } else {
        ui->activeContent->setText(tr("Activated"));
        ui->activeButton->hide();
    }

    ui->serviceContent->setText(serial);

    connect(ui->activeButton, &QPushButton::clicked, this, &About::runActiveWindow);
    connect(ui->trialButton, &QPushButton::clicked, this, &About::showPdf);
}

QStringList About::readFile(QString filepath) {
    QStringList fileCont;
    QFile file(filepath);
    if(file.exists()) {
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "ReadFile() failed to open" << filepath;
            return QStringList();
        }
        QTextStream textStream(&file);
        while(!textStream.atEnd()) {
            QString line= textStream.readLine();
            line.remove('\n');
            fileCont<<line;
        }
        file.close();
        return fileCont;
    } else {
        qWarning() << filepath << " not found"<<endl;
        return QStringList();
    }
}

void About::initSearchText() {
    //~ contents_path /about/System Summary
    ui->titleLabel->setText(tr("System Summary"));
    //~ contents_path /about/Support
    ui->label_4->setText(tr("Support"));
    ui->versionLabel->setText(tr("Version"));
    ui->kernalLabel->setText(tr("Kernel"));
    ui->cpuLabel->setText(tr("CPU"));
    ui->memoryLabel->setText(tr("Memory"));
    ui->diskLabel->setText(tr("Disk"));
    ui->systemVersion->setText(tr("Version Number"));
    ui->systemVersion->setWordWrap(true);
    ui->label_3->setText(tr("Desktop"));
    ui->label_6->setText(tr("User"));
}


void About::_call_dbus_get_computer_info(){
    interface = new QDBusInterface("com.kylin.assistant.qsessiondbus",
                                     "/com/kylin/assistant/qsessiondbus",
                                     "com.kylin.assistant.qsessiondbus",
                                     QDBusConnection::systemBus());

    if (!interface->isValid()){
        qCritical() << "Create Client Interface Failed When Get Computer info: " << QDBusConnection::systemBus().lastError();
        return;
    }

//    QDBusConnection::systemBus().connect("com.control.center.qt.systemdbus",
//                                         "/",
//                                         "com.control.center.interface",
//                                         "computerinfo", this,
//                                         SLOT(get(QString)));

    QDBusReply<QString> reply =  interface->call("GetComputerInfo");
    if (reply.isValid()){
        computerinfo =  reply.value();
    }
    else {
        qCritical() << "Call 'GetComputerInfo' Failed!" << reply.error().message();
    }


    //async
//    QDBusPendingCall async = interface->asyncCall("GetComputerInfo");
//    QDBusPendingCallWatcher * watcher = new QDBusPendingCallWatcher(async, this);

//    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(call_finished_slot(QDBusPendingCallWatcher*)));
}

//void About::call_finished_slot(QDBusPendingCallWatcher * call){
//    qDebug() << "----------------start------------>";
//    QDBusPendingReply<QString> reply = *call;
//    if (!reply.isError()){
//        QString info = reply.argumentAt<0>();
//        qDebug() << "-----------0--->" << "\n" << info;
//    }
//    else{
//        qDebug() << reply.error().message();
//    }
//    call->deleteLater();
//}


void About::runActiveWindow() {
    QString cmd = "kylin-activation";

    QProcess process(this);
    process.startDetached(cmd);
}

void About::showPdf() {
    QString cmd = "atril /usr/share/man/statement.pdf.gz";
    QProcess process(this);
    process.startDetached(cmd);
}


void About::on_pushButton_clicked()
{
    QString cmd = "/usr/bin/kylin-user-guide";
    QProcess process(this);
    process.startDetached(cmd);
}

void About::on_pushButton_2_clicked()
{
    QString cmd = "/usr/share/hp-document/hp-document";
    QProcess process(this);
    process.startDetached(cmd);
}
//void About::getCallBack(int status, QString msg)
//{
//    qDebug() << "status"<< status <<"  msg is "<<msg;
//    //ui->pushButton_3->setText(msg);
//    if(msg=="二维码已失效")
//    {
//        //ui->pushButton_3->show();
//        ui->pushButton_2->setText(tr("QR code is invalid, click refresh"));
//        ui->pushButton_2->setStyleSheet("background: transparent;color:#FB5050;font-size:16px;font-family:Microsoft YaHei;"
//                                        "border-width:1px;border-style:none none solid none;border-color:#FB5050;");

//        ui->pushButton_2->setEnabled(true);
//        disconnect(ui->pushButton_2,&QPushButton::clicked,this,&About::on_pushButton_2_clicked);
//        connect(ui->pushButton_2,&QPushButton::clicked,this,&About::on_pushButton_3_clicked);
//    }
//    else if(msg=="成功")
//    {
//        ui->pushButton_2->setText(tr("Learn more HP user manual>>"));
//        ui->pushButton_2->setStyleSheet("background: transparent;color:#2FB3E8;font-size:16px;font-family:Microsoft YaHei;"
//                      "border-width:1px;border-style:none none solid none;border-color:#2FB3E8;");
//        ui->pushButton_2->setEnabled(true);
//        disconnect(ui->pushButton_2,&QPushButton::clicked,this,&About::on_pushButton_3_clicked);
//        connect(ui->pushButton_2,&QPushButton::clicked,this,&About::on_pushButton_2_clicked);
//    }
//    else
//    {
//        ui->pushButton_2->setText(tr("QR code is invalid, click refresh"));
//        ui->pushButton_2->setStyleSheet("background: transparent;color:#FB5050;font-size:16px;font-family:Microsoft YaHei;"
//                                        "border-width:1px;border-style:none none solid none;border-color:#FB5050;");
//        ui->pushButton_2->setEnabled(true);

//    }


//}

//void About::on_pushButton_3_clicked()
//{
//    app->refreshWidget(ui->widget_7);
//}
