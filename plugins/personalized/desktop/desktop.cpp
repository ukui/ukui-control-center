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
#include "desktop.h"
#include "ui_desktop.h"

#include <QGSettings>
#include "SwitchButton/switchbutton.h"
#include "realizedesktop.h"
#include "commonComponent/listDelegate/listdelegate.h"

#include <QDebug>
#include <QPushButton>
#include <QFileInfo>
#include <QFile>
#include <QProcess>
#include <QSettings>
#include <QTextCodec>
#include <QtDBus/QDBusConnection>

#define DESKTOP_SCHEMA       "org.ukui.control-center.desktop"

#define COMPUTER_VISIBLE_KEY "computer-icon-visible"
#define HOME_VISIBLE_KEY     "home-icon-visible"
#define NETWORK_VISIBLE_KEY  "network-icon-visible"
#define TRASH_VISIBLE_KEY    "trash-icon-visible"
#define VOLUMES_VISIBLE_KEY  "volumes-visible"

#define MENU_FULL_SCREEN_KEY "menufull-screen"
#define COMPUTER_LOCK_KEY    "computer-icon-locking"
#define PERSONAL_LOCK_KEY    "personal-icon-locking"
#define SETTINGS_LOCK_KEY    "settings-icon-locking"
#define TRASH_LOCK_KEY       "trash-icon-locking"

#define PANEL_SETTINGS      "org.ukui.panel.settings"
#define PANEL_POSITION_KEY  "panelposition"
#define ICON_SIZE_KEY       "iconsize"
#define PANEL_SIZE_KEY      "panelsize"
#define QUICKLAUNCH_APP_NUM "quicklaunchappsnumber"

Desktop::Desktop() : mFirstLoad(true)
{
    pluginName = tr("Desktop");
    pluginType = PERSONALIZED;
}

Desktop::~Desktop()
{
    if (!mFirstLoad) {
        delete ui;
        qDeleteAll(vecGsettings);
        vecGsettings.clear();
    }
}

QString Desktop::get_plugin_name() {
    return pluginName;
}

int Desktop::get_plugin_type() {
    return pluginType;
}

QWidget *Desktop::get_plugin_ui() {
    if (mFirstLoad) {
        mFirstLoad = false;

        ui = new Ui::Desktop;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);

        ui->listWidget->setItemDelegate(new ListDelegate(this));

        ui->titleLabel->setStyleSheet("QLabel{font-size: 18px; color: palette(windowText);}");
        ui->title2Label->setStyleSheet("QLabel{font-size: 18px; color: palette(windowText);}");
        ui->title3Label->setStyleSheet("QLabel{font-size: 18px; color: palette(windowText);}");
        ui->menuLabel->setStyleSheet("QLabel{font-size: 18px; color: palette(windowText);}");

        ui->titleLabel->setVisible(false);

        ui->deskComputerFrame->setVisible(false);
        ui->deskTrashFrame->setVisible(false);
        ui->deskHomeFrame->setVisible(false);
        ui->deskVolumeFrame->setVisible(false);
        ui->deskNetworkFrame->setVisible(false);

        ui->titleLabel->setVisible(false);

        ui->deskComputerFrame->setVisible(false);
        ui->deskTrashFrame->setVisible(false);
        ui->deskHomeFrame->setVisible(false);
        ui->deskVolumeFrame->setVisible(false);
        ui->deskNetworkFrame->setVisible(false);

        ui->title2Label->hide();
        ui->fullScreenMenuFrame->setVisible(false);

        const QByteArray id(DESKTOP_SCHEMA);
        if (QGSettings::isSchemaInstalled(id)) {
            dSettings = new QGSettings(id, QByteArray(), this);
        }
        cmd = QSharedPointer<QProcess>(new QProcess());
        initSearchText();
        initTranslation();
        setupComponent();
        setupConnect();
        initVisibleStatus();
        initLockingStatus();
        initTraySettings();
        initPanelSetUI();
    }
    return pluginWidget;
}

void Desktop::plugin_delay_control(){

}

const QString Desktop::name() const {

    return QStringLiteral("desktop");
}

void Desktop::initSearchText() {
    //~ contents_path /desktop/Icon Show On Desktop
    ui->titleLabel->setText(tr("Icon Show On Desktop"));
    //~ contents_path /desktop/Tray icon
    ui->title3Label->setText(tr("Tray icon"));
}

void Desktop::initTranslation() {
    iconMap.insert("ukui-volume-control-applet-qt", "audio-card");
    iconMap.insert("kylin-nm", "network-workgroup");
    iconMap.insert("indicator-china-weather", "indicator-china-weather");
    iconMap.insert("ukui-flash-disk", "drive-removable-media");
    iconMap.insert("ukui-power-manager-tray", "cs-power");
    iconMap.insert("fcitx", "fcitx");
    iconMap.insert("blueman", "preferences-system-bluetooth");
    iconMap.insert("kylin-video", "kylin-video");
    iconMap.insert("kylin-screenshoot", "kylin-screenshoot");
    iconMap.insert("Onboard", "onboard");

    disList<<"ukui-sidebar"<<"kylin-nm"<<"ukui-volume-control-applet-qt"<<"update-notifier"<<"software-update-available"
          <<"blueman-tray"<<"ukui-power-manager"<<"ukui-settings-daemon"<<"blueman-applet"
         <<"ErrorApplication"<<"livepatch";
}

void Desktop::setupComponent() {

    ui->deskComputerLabel->setPixmap(QPixmap("://img/plugins/desktop/computer.png"));
    ui->deskHomeLabel->setPixmap(QPixmap("://img/plugins/desktop/homefolder.png"));
    ui->deskTrashLabel->setPixmap(QPixmap("://img/plugins/desktop/trash.png"));
    ui->deskNetworkLabel->setPixmap(QPixmap("://img/plugins/desktop/default.png"));
    ui->deskVolumeLabel->setPixmap(QPixmap("://img/plugins/desktop/default.png"));

    deskComputerSwitchBtn = new SwitchButton(pluginWidget);
    ui->deskComputerHorLayout->addWidget(deskComputerSwitchBtn);

    deskTrashSwitchBtn = new SwitchButton(pluginWidget);
    ui->deskTrashHorLayout->addWidget(deskTrashSwitchBtn);

    deskHomeSwitchBtn = new SwitchButton(pluginWidget);
    ui->deskHomeHorLayout->addWidget(deskHomeSwitchBtn);

    deskVolumeSwitchBtn = new SwitchButton(pluginWidget);
    ui->deskVolumeHorLayout->addWidget(deskVolumeSwitchBtn);

    deskNetworkSwitchBtn = new SwitchButton(pluginWidget);
    ui->deskNetworkHorLayout->addWidget(deskNetworkSwitchBtn);

    ui->menuComputerLabel->setPixmap(QPixmap::fromImage(QIcon::fromTheme("computer").pixmap(32,32).toImage()));
    ui->menuFilesystemLabel->setPixmap(QPixmap::fromImage(QIcon::fromTheme("user-home").pixmap(32,32).toImage()));
    ui->menuSettingsLabel->setPixmap(QPixmap::fromImage(QIcon::fromTheme("ukui-control-center").pixmap(32,32).toImage()));
    ui->menuTrashLabel->setPixmap(QPixmap::fromImage(QIcon::fromTheme("user-trash").pixmap(32,32).toImage()));


    fullMenuSwitchBtn = new SwitchButton(pluginWidget);
    ui->fullScreenMenuLayout->addWidget(fullMenuSwitchBtn);

    menuComputerSwitchBtn = new SwitchButton(pluginWidget);
    ui->menuComputerHorLayout->addWidget(menuComputerSwitchBtn);

    menuTrashSwitchBtn = new SwitchButton(pluginWidget);
    ui->menuTrashHorLayout->addWidget(menuTrashSwitchBtn);

    menuFilesystemSwitchBtn = new SwitchButton(pluginWidget);
    ui->menuFilesystemHorLayout->addWidget(menuFilesystemSwitchBtn);

    menuSettingSwitchBtn = new SwitchButton(pluginWidget);
    ui->menuSettingHorLayout->addWidget(menuSettingSwitchBtn);
}

void Desktop::setupConnect(){
    QStringList keys = dSettings->keys();
    connect(deskComputerSwitchBtn, &SwitchButton::checkedChanged, this, [=](bool checked){dSettings->set(COMPUTER_VISIBLE_KEY, checked);});
    connect(deskTrashSwitchBtn, &SwitchButton::checkedChanged, this, [=](bool checked){dSettings->set(TRASH_VISIBLE_KEY, checked);});
    connect(deskHomeSwitchBtn, &SwitchButton::checkedChanged, this, [=](bool checked){dSettings->set(HOME_VISIBLE_KEY, checked);});
    connect(deskVolumeSwitchBtn, &SwitchButton::checkedChanged, this, [=](bool checked){dSettings->set(VOLUMES_VISIBLE_KEY, checked);});
    connect(deskNetworkSwitchBtn, &SwitchButton::checkedChanged, this, [=](bool checked){dSettings->set(NETWORK_VISIBLE_KEY, checked);});

    connect(fullMenuSwitchBtn, &SwitchButton::checkedChanged, [=](bool checked) {
        if (keys.contains("menufullScreen")) {
            dSettings->set(MENU_FULL_SCREEN_KEY, checked);
        }
    });

    connect(menuComputerSwitchBtn, &SwitchButton::checkedChanged, [=](bool checked) {
        dSettings->set(COMPUTER_LOCK_KEY, checked);
    });

    connect(menuFilesystemSwitchBtn, &SwitchButton::checkedChanged, [=](bool checked) {
        if (keys.contains("personalIconLocking")) {
            dSettings->set(PERSONAL_LOCK_KEY, checked);
        }
    });

    connect(menuSettingSwitchBtn, &SwitchButton::checkedChanged, [=](bool checked) {
        dSettings->set(SETTINGS_LOCK_KEY, checked);
    });

    connect(menuTrashSwitchBtn, &SwitchButton::checkedChanged, [=](bool checked) {
        dSettings->set(TRASH_LOCK_KEY, checked);
    });

    QDBusConnection::sessionBus().connect(QString(), QString("/org/kylinssoclient/path"),
                                          QString("org.freedesktop.kylinssoclient.interface"),
                                          "keyChanged", this, SLOT(slotCloudAccout(QString)));
}

void Desktop::slotCloudAccout(const QString &key) {
    if (key == "ukui-menu") {
        initVisibleStatus();
        initLockingStatus();
    }
}

void Desktop::initVisibleStatus() {
    deskComputerSwitchBtn->blockSignals(true);
    deskHomeSwitchBtn->blockSignals(true);
    deskTrashSwitchBtn->blockSignals(true);
    deskVolumeSwitchBtn->blockSignals(true);
    deskNetworkSwitchBtn->blockSignals(true);

    deskComputerSwitchBtn->setChecked(dSettings->get(COMPUTER_VISIBLE_KEY).toBool());
    deskHomeSwitchBtn->setChecked(dSettings->get(HOME_VISIBLE_KEY).toBool());
    deskTrashSwitchBtn->setChecked(dSettings->get(TRASH_VISIBLE_KEY).toBool());
    deskVolumeSwitchBtn->setChecked(dSettings->get(VOLUMES_VISIBLE_KEY).toBool());
    deskNetworkSwitchBtn->setChecked(dSettings->get(NETWORK_VISIBLE_KEY).toBool());

    deskComputerSwitchBtn->blockSignals(false);
    deskHomeSwitchBtn->blockSignals(false);
    deskTrashSwitchBtn->blockSignals(false);
    deskVolumeSwitchBtn->blockSignals(false);
    deskNetworkSwitchBtn->blockSignals(false);
}

void Desktop::initLockingStatus() {
    menuComputerSwitchBtn->blockSignals(true);
    menuFilesystemSwitchBtn->blockSignals(true);
    menuSettingSwitchBtn->blockSignals(true);
    menuTrashSwitchBtn->blockSignals(true);

    QStringList keys = dSettings->keys();
    if (keys.contains("menufullScreen")) {
        fullMenuSwitchBtn->setChecked(dSettings->get(MENU_FULL_SCREEN_KEY).toBool());
    }

    if (keys.contains("personalIconLocking")) {
        menuFilesystemSwitchBtn->setChecked(dSettings->get(PERSONAL_LOCK_KEY).toBool());
    }

    menuSettingSwitchBtn->setChecked(dSettings->get(SETTINGS_LOCK_KEY).toBool());
    menuTrashSwitchBtn->setChecked(dSettings->get(TRASH_LOCK_KEY).toBool());
    menuComputerSwitchBtn->setChecked(dSettings->get(COMPUTER_LOCK_KEY).toBool());

    menuComputerSwitchBtn->blockSignals(false);
    menuFilesystemSwitchBtn->blockSignals(false);
    menuSettingSwitchBtn->blockSignals(false);
    menuTrashSwitchBtn->blockSignals(false);
}

void Desktop::initTrayStatus(QString name, QIcon icon, QGSettings *gsettings) {
    QMap<QString, QIcon> desktopMap = desktopConver(name);
    if (desktopMap.isEmpty()) {
        return;
    }
    nameList.append(name);

    QVBoxLayout * baseVerLayout = new QVBoxLayout();
    baseVerLayout->setSpacing(1);

    QFrame * devFrame = new QFrame(pluginWidget);
    devFrame->setObjectName(name);
    devFrame->setFrameShape(QFrame::Shape::Box);
    devFrame->setMinimumWidth(550);
    devFrame->setMaximumWidth(960);
    devFrame->setMinimumHeight(50);
    devFrame->setMaximumHeight(50);

    QHBoxLayout * devHorLayout = new QHBoxLayout();
    devHorLayout->setSpacing(8);
    devHorLayout->setContentsMargins(16, 0, 16, 0);

    QPushButton * iconBtn = new QPushButton(pluginWidget);

    iconBtn->setStyleSheet("QPushButton{background-color:transparent;border-radius:4px}"
                                       "QPushButton:hover{background-color: transparent ;color:transparent;}");

    QSizePolicy iconSizePolicy = iconBtn->sizePolicy();
    iconSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    iconSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    iconBtn->setSizePolicy(iconSizePolicy);
    iconBtn->setIconSize(QSize(32, 32));
    if (icon.isNull()) {
        icon = desktopMap.values().at(0).isNull() ? QIcon::fromTheme("application-x-desktop") : desktopMap.values().at(0);
    }
    iconBtn->setIcon(icon);

    QLabel * nameLabel = new QLabel(pluginWidget);
    QSizePolicy nameSizePolicy = nameLabel->sizePolicy();
    nameSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    nameSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    nameLabel->setSizePolicy(nameSizePolicy);
    nameLabel->setScaledContents(true);
    nameLabel->setText(desktopMap.keys().at(0));

    SwitchButton *appSwitch = new SwitchButton(pluginWidget);
    if (disList.contains(name)) {
        appSwitch->setEnabled(false);
    }

    devHorLayout->addWidget(iconBtn);
    devHorLayout->addWidget(nameLabel);
    devHorLayout->addStretch();

    devHorLayout->addWidget(appSwitch);
    devFrame->setLayout(devHorLayout);
    baseVerLayout->addWidget(devFrame);
    baseVerLayout->addStretch();

    QListWidgetItem * item = new QListWidgetItem(ui->listWidget);

    item->setSizeHint(QSize(QSizePolicy::Fixed, 50));
    item->setFlags(Qt::ItemFlag::ItemIsSelectable);
    item->setData(Qt::UserRole, QVariant(name));
    ui->listWidget->setItemWidget(item, devFrame);

    QString status = gsettings->get(TRAY_ACTION_KEY).toString();
    if ("tray" == status) {
        appSwitch->setChecked(true);
    } else {
        appSwitch->setChecked(false);
    }

    connect(appSwitch, &SwitchButton::checkedChanged, [=](bool checked) {
        if (!checked) {
            gsettings->set(TRAY_ACTION_KEY, "tray");
            gsettings->set(TRAY_RECORD_KEY, "tray");
        } else {
            gsettings->set(TRAY_ACTION_KEY, "storage");
            gsettings->set(TRAY_RECORD_KEY, "storage");
        }
    });
}

void Desktop::initTraySettings() {
    QString action;
    QString name;
    int winID;

    QList<char *> trayList = listExistsCustomDesktopPath();

    int itemCount  = 0;
    ui->listWidget->setSpacing(1);
    ui->listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    for (int i = 0; i < trayList.length(); i++) {
        const QByteArray id(TRAY_SCHEMA);
        QGSettings * traySettings = nullptr;
        QString path = QString("%1%2").arg(TRAY_SCHEMA_PATH).arg(QString(trayList.at(i)));;

        if (QGSettings::isSchemaInstalled(id)) {
            traySettings = new QGSettings(id, path.toLatin1().data());

            connect(traySettings, &QGSettings::changed, this, [=] (const QString &key) {

                QString status = traySettings->get(key).toString();
                if ("action" == key) {
                    QString status = traySettings->get(key).toString();
                    QString itemName = traySettings->get(TRAY_NAME_KEY).toString();
                    if ("freeze" == status) {
                        removeTrayItem(itemName);
                    } else if(!nameList.contains(itemName)) {
                        addTrayItem(traySettings);
                    }
                }
            });
            vecGsettings << traySettings;
            QStringList keys = traySettings->keys();

            if (keys.contains(static_cast<QString>(TRAY_NAME_KEY)) &&
                    keys.contains(static_cast<QString>(TRAY_ACTION_KEY))) {
                name = traySettings->get(TRAY_NAME_KEY).toString();
                action = traySettings->get(TRAY_ACTION_KEY).toString();
                winID = traySettings->get(TRAY_BINDING_KEY).toInt();
            }

            if (!("" == name || "freeze" == action || disList.contains(name))) {
                QIcon icon;
                if (!iconMap[name].isEmpty()) {
                    icon = QIcon::fromTheme(iconMap[name]);
                }
                itemCount++;

                initTrayStatus(name, icon, traySettings);
            }
        }
    }
    ui->listWidget->setFixedHeight(itemCount * 55);
}

QMap<QString, QIcon> Desktop::desktopConver(QString processName) {

    QMap<QString, QIcon> desktopMap;
    if (isFileExist("/etc/xdg/autostart/"+processName + ".desktop") ||
            isFileExist("/usr/share/applications/"+processName+".desktop")) {
        QString autoName = desktopToName("/etc/xdg/autostart/"+processName+".desktop");
        QIcon autoIcon   = desktopToIcon("/etc/xdg/autostart/"+processName+".desktop");
        QString appName  = desktopToName("/usr/share/applications/"+processName+".desktop");
        QIcon appIcon    = desktopToIcon("/usr/share/applications/"+processName+".desktop");
        if (autoName != "") {
            desktopMap.insert(autoName, autoIcon);
        } else if (appName != "") {
            desktopMap.insert(appName, appIcon);
        }
    } else if (isFileExist("/etc/xdg/autostart/" + processName.toLower() + ".desktop") ||
               isFileExist("/usr/share/applications/" + processName.toLower() + ".desktop")){
        QString autoName = desktopToName("/etc/xdg/autostart/" + processName.toLower() + ".desktop");
        QIcon autoIcon = desktopToIcon("/etc/xdg/autostart/" + processName.toLower() + ".desktop");

        QString appName  = desktopToName("/usr/share/applications/" + processName.toLower() + ".desktop");
        QIcon appIcon  = desktopToIcon("/usr/share/applications/" + processName.toLower() + ".desktop");
        if (autoName != "") {
            desktopMap.insert(autoName, autoIcon);
        } else if (appName != "") {
            desktopMap.insert(appName, appIcon);
        }
    } else {
        connect(cmd.get(), &QProcess::readyReadStandardOutput, this, [&]() {
            desktopMap = readOuputSlot();
        });
        // ????????????
        connect(cmd.get() , SIGNAL(readyReadStandardError()) , this , SLOT(readErrorSlot()));

        QString inputCmd = QString("grep -nr %1 /usr/share/applications/  /etc/xdg/autostart/\n").arg(processName);
        cmd->start(inputCmd);
        cmd->waitForFinished(-1);
    }
    return desktopMap;
}

bool Desktop::isFileExist(QString fullFileName) {
    QFileInfo fileInfo(fullFileName);
    if (fileInfo.isFile()) {
        return true;
    }
    return false;
}

void Desktop::removeTrayItem(QString itemName) {
    for (int i = 0; i < ui->listWidget->count(); i++) {
        QListWidgetItem * item = ui->listWidget->item(i);
        QString value = item->data(Qt::UserRole).toString();
        if (value == itemName) {
            ui->listWidget->takeItem(i);
            break;
        }
    }

    for (int i = 0; i < nameList.length(); i++) {
        if (nameList.at(i) == itemName) {
            nameList.removeAt(i);
            break;
        }
    }
}

void Desktop::addTrayItem(QGSettings * trayGSetting) {

    QString name   = trayGSetting->get(TRAY_NAME_KEY).toString();
    QString action = trayGSetting->get(TRAY_ACTION_KEY).toString();

    if (!("" == name || "freeze" == action || disList.contains(name))){
        QIcon icon;
        if (!iconMap[name].isEmpty()) {
            icon = QIcon::fromTheme(iconMap[name]);
        } else {
            icon = QIcon::fromTheme("application-x-desktop");
        }
        initTrayStatus(name, icon, trayGSetting);
    }
}

QString Desktop::desktopToName(QString desktopfile) {
    const QString locale = QLocale::system().name();
    const QString localeName = "Name[" + locale +"]";
    const QString genericName = "GenericName[" + locale +"]";
    QSettings desktopSettings(desktopfile, QSettings::IniFormat);

    desktopSettings.setIniCodec(QTextCodec::codecForName("UTF-8"));
    desktopSettings.beginGroup("Desktop Entry");

    QString desktopName = desktopSettings.value(localeName, "").toString();
    if (desktopName.isEmpty()) {
        desktopName = desktopSettings.value(genericName, "").toString();
    }

    desktopSettings.endGroup();
    return desktopName;
}

QIcon Desktop::desktopToIcon(const QString &desktopfile) {
    QSettings desktopSettings(desktopfile, QSettings::IniFormat);

    desktopSettings.setIniCodec(QTextCodec::codecForName("UTF-8"));
    desktopSettings.beginGroup("Desktop Entry");

    QString iconName = desktopSettings.value("Icon", "").toString();

    desktopSettings.endGroup();
    return QIcon::fromTheme(iconName);
}

QMap<QString, QIcon> Desktop::readOuputSlot() {
    QString str;
    QMap<QString, QIcon> desktopMap;
    QFile file("/tmp/desktopprocess.txt");
    QString output=cmd->readAllStandardOutput().data();

    // ?????????????????????????????????
    file.open(QIODevice::ReadWrite | QIODevice::Text);
    file.write(output.toUtf8());
    file.close();

    file.open(QIODevice::ReadWrite | QIODevice::Text);
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        str = line;
        if(str.contains(".desktop:") && str.contains(":Exec")){
            str = str.section(".desktop", 0, 0)+".desktop";
            desktopMap.insert(desktopToName(str), desktopToIcon(str));
        }
    }
    file.close();
    file.remove();
    return desktopMap;
}

void Desktop::readErrorSlot() {
    qWarning() << "read desktop file name failed";
}

void Desktop::initPanelSetUI()
{
    QFrame * panelSetupFrame = new QFrame();
    panelSetupFrame->setMaximumWidth(960);
    panelSetupFrame->setFixedHeight(50);
    panelSetupFrame->setFrameShape(QFrame::Shape::Box);
    QFrame * panelSetupPositionFrame=new QFrame();
    panelSetupPositionFrame->setFixedHeight(50);
    panelSetupPositionFrame->setMaximumWidth(960);
    panelSetupPositionFrame->setFrameShape(QFrame::Shape::Box);
    quicklaunchNumFrame=new QFrame();
    quicklaunchNumFrame->setFixedHeight(50);
    quicklaunchNumFrame->setMaximumWidth(960);
    quicklaunchNumFrame->setFrameShape(QFrame::Shape::Box);

    hLayoutPanelSet = new QHBoxLayout();
    hLayoutPanelSizeSet = new QHBoxLayout();
    hLayoutPanelPositionSet = new QHBoxLayout();
    hLayoutQuicklaunchNumSet = new QHBoxLayout();
    labelPanelTitle=new QLabel();
    labelPanelSize=new QLabel();
    labelPanelPosition=new QLabel();
    labelQuicklaunchNum=new QLabel();
    comboBoxPanelSize=new QComboBox();
    comboBoxPanelPosition=new QComboBox();
    spinBoxQuickLaunchNum=new QSpinBox();

    labelPanelTitle->setText(tr("Panel Set Up"));
    labelPanelSize->setText(tr("Panel Size"));
    labelPanelPosition->setText(tr("Panel Position"));
    labelQuicklaunchNum->setText(tr("Max Quicklaunch Number"));

    panelSetupFrame->setLayout(hLayoutPanelSizeSet);
    panelSetupPositionFrame->setLayout(hLayoutPanelPositionSet);
    quicklaunchNumFrame->setLayout(hLayoutQuicklaunchNumSet);


    hLayoutPanelSet->addWidget(labelPanelTitle);
    hLayoutPanelSizeSet->addWidget(labelPanelSize);
    hLayoutPanelSizeSet->addWidget(comboBoxPanelSize);
    hLayoutPanelPositionSet->addWidget(labelPanelPosition);
    hLayoutPanelPositionSet->addWidget(comboBoxPanelPosition);
    hLayoutQuicklaunchNumSet->addWidget(labelQuicklaunchNum);
    hLayoutQuicklaunchNumSet->addWidget(spinBoxQuickLaunchNum);

    ui->verticalLayout_4->addLayout(hLayoutPanelSet);
    ui->verticalLayout_4->addLayout(hLayoutPanelSizeSet);
    ui->verticalLayout_4->addLayout(hLayoutPanelPositionSet);
    ui->verticalLayout_4->addLayout(hLayoutQuicklaunchNumSet);

    ui->verticalLayout_4->addWidget(panelSetupFrame);
    ui->verticalLayout_4->addWidget(panelSetupPositionFrame);
    ui->verticalLayout_4->addWidget(quicklaunchNumFrame);
    labelPanelTitle->setStyleSheet("QLabel{font-size: 18px; color: palette(windowText);}");

    initPanelSetItem();
}

void Desktop::initPanelSetItem()
{
    const QByteArray id(PANEL_SETTINGS);
    settings = new QGSettings(id, QByteArray(), this);
    mPanelGSettings = settings->keys();
    QStringList mPanleSizeList;
    mPanleSizeList<<tr("Small Size")<<tr("Mudium Size")<<tr("Large Size");
    QStringList mPanelPositionList;
    mPanelPositionList<<tr("Bottom")<<tr("Top")<<tr("Left")<<tr("Right");

    comboBoxPanelSize->addItems(mPanleSizeList);
    comboBoxPanelSize->setCurrentIndex(getPanelSize());
    comboBoxPanelPosition->addItems(mPanelPositionList);
    comboBoxPanelPosition->setCurrentIndex(getPanelPosition());

    if (mPanelGSettings.contains(QUICKLAUNCH_APP_NUM)) {
        spinBoxQuickLaunchNum->setValue(settings->get(QUICKLAUNCH_APP_NUM).toInt());
    } else {
        quicklaunchNumFrame->setVisible(false);
    }


    QObject::connect(settings, &QGSettings::changed, this, [=] (const QString &key){
        if(key == PANEL_POSITION_KEY){
            comboBoxPanelPosition->setCurrentIndex(getPanelPosition());
        }
        if(key == PANEL_SIZE_KEY){
            comboBoxPanelSize->setCurrentIndex(getPanelSize());
        }
    });
    connect(comboBoxPanelSize, SIGNAL(currentIndexChanged(int)), this, SLOT(panelSizeComboboxChangedSlot(int)));
    connect(comboBoxPanelPosition, SIGNAL(currentIndexChanged(int)), this, SLOT(panelPositionComboboxChangedSlot(int)));
    connect(spinBoxQuickLaunchNum,SIGNAL(valueChanged(int)),this,SLOT(quicklaunchNumSpinBoxChangedSlot(int)));
}
int  Desktop::getPanelSize()
{
    switch(settings->get(PANEL_SIZE_KEY).toInt())
    {
    case 68 ... 72:
        return 1;
        break;
    case 90 ... 94:
        return 2;
        break;
    default:
        return 0;
        break;
    }
}

int Desktop::getPanelPosition()
{
    switch(settings->get(PANEL_POSITION_KEY).toInt()){
    case 1:
        return 1;
        break;
    case 2:
        return 2;
        break;
    case 3:
        return 2;
        break;
    default:
        return 0;
        break;
    }
}
void Desktop::panelSizeComboboxChangedSlot(int size)
{
    switch(size){
    case 0:
        settings->set(PANEL_SIZE_KEY,46);
        settings->set(ICON_SIZE_KEY,32);
        break;
    case 1:
        settings->set(PANEL_SIZE_KEY,70);
        settings->set(ICON_SIZE_KEY,48);
        break;
    case 2:
        settings->set(PANEL_SIZE_KEY,92);
        settings->set(ICON_SIZE_KEY,64);
        break;
    }
}

void Desktop::panelPositionComboboxChangedSlot(int position)
{
    switch(position){
    case 1:
        settings->set(PANEL_POSITION_KEY,1);
        break;
    case 2:
        settings->set(PANEL_POSITION_KEY,2);
        break;
    case 3:
        settings->set(PANEL_POSITION_KEY,3);
        break;
    default:
        settings->set(PANEL_POSITION_KEY,0);
        break;
    }
}

void Desktop::quicklaunchNumSpinBoxChangedSlot(int size)
{
    QTimer::singleShot(1000,[=] { int s=size; settings->set(QUICKLAUNCH_APP_NUM,s);});
}
