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
#include "wallpaper.h"
#include "ui_wallpaper.h"
#include "pictureunit.h"
#include <ukcc/widgets/maskwidget.h>

#include <QDBusReply>
#include <QDBusInterface>
#include <QDebug>
#include <QDesktopServices>
#include <QProcess>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <ukcc/widgets/addbtn.h>

const QString kylinUrl = "https://www.ubuntukylin.com/wallpaper.html";

enum{
    PICTURE,   // 图片背景
    COLOR,     // 纯色背景
    SLIDESHOW  // 幻灯片背景
};


#define ITEMWIDTH 182
#define ITEMHEIGH 126

#define COLORITEMWIDTH 56
#define COLORITEMHEIGH 56

Wallpaper::Wallpaper() : mFirstLoad(true)
{
    pluginName = tr("Background");
    pluginType = PERSONALIZED;
    prePicUnit = nullptr;
}

Wallpaper::~Wallpaper()
{
    if (!mFirstLoad) {
        delete ui;
        ui = nullptr;
        delete xmlhandleObj;
        xmlhandleObj = nullptr;
    }
}

QString Wallpaper::plugini18nName() {
    return pluginName;
}

int Wallpaper::pluginTypes() {
    return pluginType;
}

QWidget *Wallpaper::pluginUi() {
    if (mFirstLoad) {
        mFirstLoad = false;

        ui = new Ui::Wallpaper;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);

        settingsCreate = false;
        initSearchText();
        // 初始化控件
        setupComponent();
        // 初始化gsettings
        const QByteArray id(BACKGROUND);
        if (QGSettings::isSchemaInstalled(id)){
            settingsCreate = true;

            bgsettings = new QGSettings(id, QByteArray(), this);
            setupConnect();
            initBgFormStatus();
            initBgOption();
            hideComponent();
        }
        // 构建xmlhandle对象
        xmlhandleObj = new XmlHandle();
    } else {
        ui->picFrame->resize(ui->picFrame->size() - QSize(1,1));
        ui->picFrame->resize(ui->picFrame->size() + QSize(1,1));
    }
    return pluginWidget;
}

const QString Wallpaper::name() const {

    return QStringLiteral("Wallpaper");
}

bool Wallpaper::isShowOnHomePage() const
{
    return true;
}

QIcon Wallpaper::icon() const
{
    return QIcon();
}

bool Wallpaper::isEnable() const
{
    return true;
}

void Wallpaper::initSearchText() {
    ui->selectLabel->setText(tr("Background"));
    //~ contents_path /Wallpaper/Browse
    ui->browserLocalwpBtn->setText(tr("Browse"));
    //~ contents_path /Wallpaper/Online Picture
    ui->onlineBtn->setText(tr("Online Picture"));
    ui->onlineBtn->setFocusPolicy(Qt::NoFocus);
    ui->onlineBtn->setContentsMargins(0,0,0,0);
    ui->onlineBtn->setCursor( QCursor(Qt::PointingHandCursor));
    ui->onlineBtn->setStyleSheet("QPushButton{background: transparent;border-radius: 4px;text-decoration: underline;} ");
    connect( ui->onlineBtn, &QPushButton::clicked, this,[=] {
        QDesktopServices::openUrl(QUrl(QLatin1String("https://www.ubuntukylin.com/wallpaper.html")));
    });
    //~ contents_path /Wallpaper/Reset To Default
    ui->resetBtn->setText(tr("Reset To Default"));
}

void Wallpaper::setupComponent(){

    QString name = qgetenv("USER");
    if (name.isEmpty()) {
        name = qgetenv("USERNAME");
    }

    QString lockfilename = "/var/lib/lightdm-data/" + name + "/ukui-greeter.conf";
    mLockLoginSettings = new QSettings(lockfilename, QSettings::IniFormat, this);

    // 背景形式
    QStringList formList;
    formList << tr("picture") << tr("color")/* << tr("slideshow")*/ ;
    ui->formComBox->addItem(formList.at(0), PICTURE);
    ui->formComBox->addItem(formList.at(1), COLOR);


    // 图片背景
    ui->picFrame->adjustSize();
    picFlowLayout = new FlowLayout(ui->picFrame,16, -1, -1);
    ui->picFrame->setLayout(picFlowLayout);

    // 纯色背景
    colorFlowLayout = new FlowLayout(ui->colorListWidget,16, -1, -1);
    ui->colorListWidget->setLayout(colorFlowLayout);

    // 背景放置方式
    QStringList optionList;
    optionList << tr("scaled") << tr("wallpaper") << tr("centered") << tr("stretched");
    ui->showModeComboBox->addItem(optionList.at(0), "scaled");
    ui->showModeComboBox->addItem(optionList.at(1), "wallpaper");
    ui->showModeComboBox->addItem(optionList.at(2), "centered");
    ui->showModeComboBox->addItem(optionList.at(3), "stretched");

    AddBtn *addBtn = new AddBtn();
    ui->horizontalLayout_7->addWidget(addBtn);

    // 打开自定义颜色面板
    connect(addBtn, &AddBtn::clicked,[=](){
        colordialog = new ColorDialog(pluginWidget);
        connect(colordialog,&ColorDialog::colorSelected,this,&Wallpaper::colorSelectedSlot);
        colordialog->exec();
    });
}

void Wallpaper::setupConnect(){
    //使用线程构建本地壁纸文件；获取壁纸压缩QPixmap
    pThread = new QThread;
    pObject = new WorkerObject;
    QString bgFileName = bgsettings->get(FILENAME).toString();
    connect(pObject, &WorkerObject::pixmapGenerate, this, [=](QPixmap pixmap, QString filename){
        PictureUnit * picUnit = new PictureUnit;
        picUnit->setPixmap(pixmap);
        picUnit->setFilenameText(filename);
        if (bgFileName == filename) {
            if (prePicUnit != nullptr) {
                prePicUnit->changeClickedFlag(false);
                prePicUnit->setStyleSheet("border-width: 0px;");
            }
            picUnit->changeClickedFlag(true);
            prePicUnit = picUnit;
            picUnit->setFrameShape(QFrame::Box);
            picUnit->setStyleSheet(picUnit->clickedStyleSheet);
        }

        connect(picUnit, &PictureUnit::clicked, [=](QString fn){
            if(prePicUnit != nullptr)
            {
                prePicUnit->changeClickedFlag(false);
                prePicUnit->setStyleSheet("border-width: 0px;");
            }
            picUnit->changeClickedFlag(true);
            prePicUnit = picUnit;
            picUnit->setFrameShape(QFrame::Box);
            picUnit->setStyleSheet(picUnit->clickedStyleSheet);
            bgsettings->set(FILENAME, fn);
            setLockBackground("");
            ui->previewStackedWidget->setCurrentIndex(PICTURE);
        });

        picFlowLayout->addWidget(picUnit);
        picNum++;
    });
    connect(pObject, &WorkerObject::workComplete, this, [=](QMap<QString, QMap<QString, QString>> wpInfoMaps){
        wallpaperinfosMap = wpInfoMaps;
        pThread->quit(); //退出事件循环
        pThread->wait(); //释放资源
    });

    pObject->moveToThread(pThread);
    connect(pThread, &QThread::started, pObject, &WorkerObject::run);
    connect(pThread, &QThread::finished, this, [=](){


    });

    connect(pThread, &QThread::finished, pObject, &WorkerObject::deleteLater);

    pThread->start();


    connect(ui->browserLocalwpBtn, &QPushButton::clicked, [=]{
        showLocalWpDialog();
    });

    connect(ui->resetBtn, SIGNAL(clicked(bool)), this, SLOT(resetDefaultWallpaperSlot()));

    // 纯色背景
    QStringList colors;

    colors << "#2d7d9a" << "#018574" << "#107c10" << "#10893e" << "#038387" << "#486860" << "#525e54" << "#7e735f" << "#4c4a48" << "#000000";
    colors << "#ff8c00" << "#e81123" << "#d13438" << "#c30052" << "#bf0077" << "#9a0089" << "#881798" << "#744da9" << "#8764b8" << "#e9e9e9";

    for (QString color : colors){
        QPushButton * button = new QPushButton(ui->colorListWidget);
        button->setFixedSize(QSize(48, 48));
        QString btnQss = QString("QPushButton{background: %1; border: none; border-radius: 4px;}").arg(color);
        button->setStyleSheet(btnQss);

        connect(button, &QPushButton::clicked, [=]{
            QString widgetQss = QString("QWidget{background: %1; border-radius: 6px;}").arg(color);
            ui->previewWidget->setStyleSheet(widgetQss);

            // 设置系统纯色背景
            bgsettings->set(FILENAME, "");
            bgsettings->set(PRIMARY, QVariant(color));
            bgsettings->set(SECONDARY, QVariant(color));
            setLockBackground(color);

            ui->previewStackedWidget->setCurrentIndex(COLOR);
        });
        colorFlowLayout->addWidget(button);
    }

#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)
    connect(ui->formComBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index){
#else
    connect(ui->formComBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
#endif
        Q_UNUSED(index)
        //切换
        int currentPage = ui->formComBox->currentData(Qt::UserRole).toInt();
        if (currentPage == PICTURE) {
            ui->colorFrame->setHidden(true);
            ui->picFrame->setHidden(false);
            ui->bottomWidget->setHidden(false);
        } else if (currentPage == COLOR){
            ui->picFrame->setHidden(true);
            ui->colorFrame->setHidden(false);
            ui->bottomWidget->setHidden(true);
        }
    });
    //壁纸变动后改变用户属性
    connect(bgsettings, &QGSettings::changed, [=](QString key){
        initBgFormStatus();
        if (ui->formComBox->currentIndex() == PICTURE) {
            QString filename = bgsettings->get(FILENAME).toString();
            setClickedPic(filename);
        }
        if (key == "pictureOptions") {
            QString option = bgsettings->get(OPTIONS).toString();
            int index = ui->showModeComboBox->findData(option);
            ui->showModeComboBox->blockSignals(true);
            ui->showModeComboBox->setCurrentIndex(index);
            ui->showModeComboBox->blockSignals(false);
        }
        //GSettings key picture-filename 这里收到 pictureFilename的返回值
        if (!QString::compare(key, "pictureFilename")){
            QString curPicname = bgsettings->get(key).toString();
            if (curPicname == "") {
                if (prePicUnit != nullptr) {
                    prePicUnit->changeClickedFlag(false);
                    prePicUnit->setStyleSheet("border-width: 0px;");
                }
            }
            QDBusInterface * interface = new QDBusInterface("org.freedesktop.Accounts",
                                             "/org/freedesktop/Accounts",
                                             "org.freedesktop.Accounts",
                                             QDBusConnection::systemBus());

            if (!interface->isValid()){
                qCritical() << "Create /org/freedesktop/Accounts Client Interface Failed " << QDBusConnection::systemBus().lastError();
                return;
            }

            QDBusReply<QDBusObjectPath> reply =  interface->call("FindUserByName", g_get_user_name());
            QString userPath;
            if (reply.isValid()){
                userPath = reply.value().path();
            }
            else {
                qCritical() << "Call 'GetComputerInfo' Failed!" << reply.error().message();
                return;
            }

            QDBusInterface * useriFace = new QDBusInterface("org.freedesktop.Accounts",
                                                            userPath,
                                                            "org.freedesktop.Accounts.User",
                                                            QDBusConnection::systemBus());

            if (!useriFace->isValid()){
                qCritical() << QString("Create %1 Client Interface Failed").arg(userPath) << QDBusConnection::systemBus().lastError();
                return;
            }

            QDBusMessage msg = useriFace->call("SetBackgroundFile", curPicname);
            if (!msg.errorMessage().isEmpty())
                qDebug() << "update user background file error: " << msg.errorMessage();
        }
    });
}

int Wallpaper::_getCurrentBgForm(){
    QString filename = bgsettings->get(FILENAME).toString();

    int current = 0;

    // 设置当前背景形式
    if (filename.isEmpty()){
        current = COLOR;
    } else if (filename.endsWith("xml")){
        current = SLIDESHOW;
    } else {
        current = PICTURE;
    }

    return current;
}

void Wallpaper::initBgFormStatus(){
    initPreviewStatus();


    int currentIndex = _getCurrentBgForm();
    //设置当前背景形式
    ui->formComBox->blockSignals(true);
    ui->formComBox->setCurrentIndex(currentIndex);
    ui->formComBox->blockSignals(false);
    if (ui->formComBox->currentIndex() == PICTURE) {
        ui->colorFrame->setHidden(true);
        ui->picFrame->setHidden(false);
        ui->bottomWidget->setHidden(false);
    } else if (ui->formComBox->currentIndex() == COLOR){
        ui->picFrame->setHidden(true);
        ui->colorFrame->setHidden(false);
        ui->bottomWidget->setHidden(true);
    }

    ui->previewStackedWidget->setCurrentIndex(currentIndex);

    //根据背景形式选择显示组件
    showComponent(currentIndex);
}

void Wallpaper::showComponent(int index) {
    Q_UNUSED(index);
}

void Wallpaper::setLockBackground(QString bg) {
    mLockLoginSettings->beginGroup("greeter");
    mLockLoginSettings->setValue("color", bg);
    mLockLoginSettings->endGroup();
}

void Wallpaper::initBgOption()
{
    QString option = bgsettings->get(OPTIONS).toString();
    int index = ui->showModeComboBox->findData(option);
    ui->showModeComboBox->setCurrentIndex(index);
    connect(ui->showModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        bgsettings->set(OPTIONS, ui->showModeComboBox->itemData(index).toString());
    });
}

void Wallpaper::hideComponent()
{
    ui->onlineBtn->setHidden(Utils::isZJY());
}

void Wallpaper::initPreviewStatus(){
    // 设置图片背景的预览效果
    QString filename = bgsettings->get(FILENAME).toString();
    QFileInfo fileinfo = QFileInfo(filename);
    if(fileinfo.suffix() == "dib")
    {
        QFile::copy(filename,".TEMP.bmp");
        ui->previewLabel->setPixmap(QPixmap(".TEMP.bmp").scaled(ui->previewLabel->size()));
    }
    else
    {
        ui->previewLabel->setPixmap(QPixmap(filename).scaled(ui->previewLabel->size()));
    }
    // 设置纯色背景的预览效果
    QString color = bgsettings->get(PRIMARY).toString();
    if (!color.isEmpty()){
        QString widgetQss = QString("QWidget{background: %1;}").arg(color);
        ui->previewWidget->setStyleSheet(widgetQss);
    }
}

// 自定义颜色面板选定颜色
void Wallpaper::colorSelectedSlot(QColor color){
    qDebug() << "colorSelectedSlot" << color << color.name();

    QString widgetQss = QString("QWidget{background: %1;}").arg(color.name());
    ui->previewWidget->setStyleSheet(widgetQss);

    // 设置系统纯色背景
    bgsettings->set(FILENAME, "");
    bgsettings->set(PRIMARY, QVariant(color.name()));
    setLockBackground(color.name());

    ui->previewStackedWidget->setCurrentIndex(COLOR);
}

void Wallpaper::wpOptionsChangedSlot(QString op){
    Q_UNUSED(op)
    //将改动保存至文件
    xmlhandleObj->xmlUpdate(wallpaperinfosMap);
}

void Wallpaper::setModeldata(){
    QMap<QString, QMap<QString, QString> >::iterator iters = wallpaperinfosMap.begin();
    for (int row = 0; iters != wallpaperinfosMap.end(); iters++, row++){
        if (QString(iters.key()) == "head") //跳过xml的头部信息
            continue;
        QMap<QString, QString> wpMap = (QMap<QString, QString>)iters.value();
        QString delstatus = QString(wpMap.find("deleted").value());
        if (delstatus == "true") //跳过被删除的壁纸
            continue;

        QString filename = QString(iters.key());
        QPixmap pixmap(filename);

        wpListModel.insertRows(row, 1, QModelIndex());
        QModelIndex wpindex = wpListModel.index(row, 0, QModelIndex());
        wpListModel.setData(wpindex, QIcon(pixmap.scaled(QSize(160,100))), Qt::DecorationRole);
        wpListModel.setData(wpindex, QString("%1\nfolder: %2\n").arg(wpMap.find("name").value()).arg(filename), Qt::ToolTipRole);
    }
}

void Wallpaper::resetDefaultWallpaperSlot(){
    GSettings * wpgsettings;
    wpgsettings = g_settings_new(BACKGROUND);
    GVariant * variant = g_settings_get_default_value(wpgsettings, FILENAME);
    gsize size = g_variant_get_size(variant);
    const char * dwp = g_variant_get_string(variant, &size);
    g_object_unref(wpgsettings);

    bgsettings->set(FILENAME, QVariant(QString(dwp)));
    setClickedPic(QString(dwp));
}

void Wallpaper::showLocalWpDialog(){
    QStringList filters;
    filters<<tr("Wallpaper files(*.jpg *.jpeg *.bmp *.dib *.png *.jfif *.jpe *.gif *.tif *.tiff *.wdp)");
    QFileDialog fd(pluginWidget);
    QList<QUrl> usb_list = fd.sidebarUrls();
    int sidebarNum = 8;// 最大添加U盘数，可以自己定义
    QString home_path = QDir::homePath().section("/", -1, -1);
    QString mnt = "/media/" + home_path + "/";
    QDir mntDir(mnt);
    mntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList file_list = mntDir.entryInfoList();
    QList<QUrl> mntUrlList;
    for (int i = 0; i < sidebarNum && i < file_list.size(); ++i) {
        QFileInfo fi = file_list.at(i);
        mntUrlList << QUrl("file://" + fi.filePath());
    }

    QFileSystemWatcher m_fileSystemWatcher(&fd);
    m_fileSystemWatcher.addPath("/media/" + home_path + "/");
    connect(&m_fileSystemWatcher, &QFileSystemWatcher::directoryChanged, &fd,
            [=, &sidebarNum, &mntUrlList, &usb_list, &fd](const QString path) {
        QDir m_wmntDir(path);
        m_wmntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
        QFileInfoList m_wfilist = m_wmntDir.entryInfoList();
        mntUrlList.clear();
        for (int i = 0; i < sidebarNum && i < m_wfilist.size(); ++i) {
            QFileInfo m_fi = m_wfilist.at(i);
            mntUrlList << QUrl("file://" + m_fi.filePath());
        }
        fd.setSidebarUrls(usb_list + mntUrlList);
        fd.update();
    });

    connect(&fd, &QFileDialog::finished, &fd, [=, &usb_list, &fd]() {
        fd.setSidebarUrls(usb_list);
    });

    fd.setDirectory(QString(const_cast<char *>(g_get_user_special_dir(G_USER_DIRECTORY_PICTURES))));
    fd.setAcceptMode(QFileDialog::AcceptOpen);
    fd.setViewMode(QFileDialog::List);
    fd.setNameFilters(filters);
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setWindowTitle(tr("select custom wallpaper file"));
    fd.setLabelText(QFileDialog::Accept, tr("Select"));
    fd.setLabelText(QFileDialog::LookIn, tr("Position: "));
    fd.setLabelText(QFileDialog::FileName, tr("FileName: "));
    fd.setLabelText(QFileDialog::FileType, tr("FileType: "));
    fd.setLabelText(QFileDialog::Reject, tr("Cancel"));

    fd.setSidebarUrls(usb_list + mntUrlList);

    if (fd.exec() != QDialog::Accepted)
        return;
    QString selectedfile;
    selectedfile = fd.selectedFiles().first();

    QStringList fileRes = selectedfile.split("/");

    QProcess * process = new QProcess();
    QString program("cp");
    QStringList arguments;
    arguments << selectedfile ;
    arguments << "/tmp";
    process->start(program, arguments);

    QString bgfile = "/tmp/" + fileRes.at(fileRes.length() - 1);

    // TODO: chinese and space support
    bgsettings->set(FILENAME, selectedfile);
    if (prePicUnit != nullptr) {  //去掉选定标记
        prePicUnit->changeClickedFlag(false);
        prePicUnit->setStyleSheet("border-width: 0px;");
    }
}

void Wallpaper::add_custom_wallpaper(){
    QString filters = "Wallpaper files(*.png *.jpg)";
    QFileDialog fd(pluginWidget);

    fd.setDirectory(QString(const_cast<char *>(g_get_user_special_dir(G_USER_DIRECTORY_PICTURES))));
    fd.setAcceptMode(QFileDialog::AcceptOpen);
    fd.setViewMode(QFileDialog::List);
    fd.setNameFilter(filters);
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setWindowTitle(tr("select custom wallpaper file"));
    fd.setLabelText(QFileDialog::Accept, tr("Select"));
    fd.setLabelText(QFileDialog::LookIn, tr("Position: "));
    fd.setLabelText(QFileDialog::FileName, tr("FileName: "));
    fd.setLabelText(QFileDialog::FileType, tr("FileType: "));
    fd.setLabelText(QFileDialog::Reject, tr("Cancel"));


    QString selectedfile;
    selectedfile = fd.selectedFiles().first();

    if (wallpaperinfosMap.contains(selectedfile)){
        wallpaperinfosMap[selectedfile]["deleted"] = "false";
    } else {
        QMap<QString, QString> tmpinfo;
        tmpinfo.insert("artist", "(none)");
        tmpinfo.insert("deleted", "false");
        tmpinfo.insert("filename", selectedfile);
        tmpinfo.insert("name", selectedfile.split("/").last());
        tmpinfo.insert("options", "zoom");
        tmpinfo.insert("pcolor", "#000000");
        tmpinfo.insert("scolor", "#000000");
        tmpinfo.insert("shade_type", "solid");
        wallpaperinfosMap.insert(selectedfile, tmpinfo);

    }
    xmlhandleObj->xmlUpdate(wallpaperinfosMap);
}

void Wallpaper::del_wallpaper(){
    // 将改动保存至文件
    xmlhandleObj->xmlUpdate(wallpaperinfosMap);
}

void Wallpaper::setClickedPic(QString fileName) {
    if (prePicUnit != nullptr) {
        prePicUnit->changeClickedFlag(false);
        prePicUnit->setStyleSheet("border-width: 0px;");
    }
    for (int i = picFlowLayout->count() - 1; i >= 0; --i) { 
        QLayoutItem *it      = picFlowLayout->itemAt(i);
        PictureUnit *picUnit = static_cast<PictureUnit*>(it->widget());
        if (fileName == picUnit->filenameText()) {
            picUnit->changeClickedFlag(true);
            prePicUnit = picUnit;
            picUnit->setFrameShape(QFrame::Box);
            picUnit->setStyleSheet(picUnit->clickedStyleSheet);
        }
    }
}
