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

#include "MaskWidget/maskwidget.h"
#include <QGraphicsOpacityEffect>
#include <QDBusReply>
#include <QDBusInterface>
#include <QDebug>
#include <QDesktopServices>
#include <QProcess>
const QString kylinUrl = "https://www.ubuntukylin.com/wallpaper.html";

enum{
    PICTURE, //图片背景
    COLOR, //纯色背景
    SLIDESHOW //幻灯片背景
};

#define ITEMWIDTH 198
#define ITEMHEIGH 110

#define COLORITEMWIDTH 56
#define COLORITEMHEIGH 56

Wallpaper::Wallpaper() : mFirstLoad(true)
{
    pluginName = tr("Wallpaper");
    pluginType = PERSONALIZED;
}

Wallpaper::~Wallpaper()
{
    if (!mFirstLoad) {
        delete ui;
        delete xmlhandleObj;
        if (bgsettings){
            delete bgsettings;
        }
    }
}

QString Wallpaper::get_plugin_name(){
    return pluginName;
}

int Wallpaper::get_plugin_type(){
    return pluginType;
}

QWidget *Wallpaper::get_plugin_ui(){
    if (mFirstLoad) {
        mFirstLoad = false;
        ui = new Ui::Wallpaper;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_StyledBackground,true);
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);
        ui->formSelectMenu->setText(tr("Background Form:"));
        ui->titleLabel->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");

        //~ contents_path /wallpaper/Desktop Background
        ui->titleLabel->setText(tr("Desktop Background"));

        ui->picOptionsComBox->setMinimumHeight(48);

        //初始化控件
        setupComponent();
        //隐藏背景形式
        ui->frame->hide();
        ui->backgroundWidget->close();
        //初始化gsettings
        const QByteArray id(BACKGROUND);
        if (QGSettings::isSchemaInstalled(id)){
            bgsettings = new QGSettings(id);
            setupConnect();
            initBgFormStatus();
        }
        //构建xmlhandle对象
        xmlhandleObj = new XmlHandle();
    }
    return pluginWidget;
}

void Wallpaper::plugin_delay_control(){
}

const QString Wallpaper::name() const {

    return QStringLiteral("wallpaper");
}
void Wallpaper::setupComponent(){
    //~ contents_path /wallpaper/arrangement
    ui->picOptionsLabel->setText(tr("arrangement"));
    //    ui->browserLocalwpBtn->hide();
    //    ui->browserOnlinewpBtn->hide();
    //背景形式
    QStringList formList;
    formList << tr("picture") << tr("color")/* << tr("slideshow")*/ ;

    //预览遮罩
    MaskWidget * maskWidget = new MaskWidget(ui->previewLabel);
    maskWidget->setGeometry(0, 0, 288, 162);
    QPushButton * hoverBtn_1 = new QPushButton(maskWidget);
    QGraphicsOpacityEffect *opacityEffect_1=new QGraphicsOpacityEffect;
//    hoverBtn_1->setStyleSheet("QPushButton#hoverBtn_1{border-radius:8px;}");
    hoverBtn_1->setGraphicsEffect(opacityEffect_1);
    opacityEffect_1->setOpacity(0.65);
    hoverBtn_1->setGeometry(4,146,280,12);
    hoverBtn_1->setEnabled(false);
    QPushButton * hoverBtn_2 = new QPushButton(maskWidget);
    QGraphicsOpacityEffect *opacityEffect_2=new QGraphicsOpacityEffect;
//    hoverBtn_2->setStyleSheet("QPushButton#hoverBtn_1{border-radius:4px;}");
    hoverBtn_2->setGraphicsEffect(opacityEffect_2);
    opacityEffect_2->setOpacity(0.65);
    hoverBtn_2->setGeometry(8,8,12,12);
    hoverBtn_2->setEnabled(false);
    QPushButton * hoverBtn_3 = new QPushButton(maskWidget);
    QGraphicsOpacityEffect *opacityEffect_3=new QGraphicsOpacityEffect;
//    hoverBtn_3->setStyleSheet("QPushButton#hoverBtn_1{border-radius:4px;}");
    hoverBtn_3->setGraphicsEffect(opacityEffect_3);
    opacityEffect_3->setOpacity(0.65);
    hoverBtn_3->setGeometry(8,24,12,12);
    hoverBtn_3->setEnabled(false);
    QPushButton * hoverBtn_4 = new QPushButton(maskWidget);
    QGraphicsOpacityEffect *opacityEffect_4=new QGraphicsOpacityEffect;
//    hoverBtn_4->setStyleSheet("QPushButton#hoverBtn_1{border-radius:4px;}");
    hoverBtn_4->setGraphicsEffect(opacityEffect_4);
    opacityEffect_4->setOpacity(0.65);
    hoverBtn_4->setGeometry(8,40,12,12);
    hoverBtn_4->setEnabled(false);
    ///图片背景
    picFlowLayout = new FlowLayout(ui->picListWidget);
    picFlowLayout->setContentsMargins(11, 13, 11, 13);
    ui->picListWidget->setLayout(picFlowLayout);
    //纯色背景
    colorFlowLayout = new FlowLayout(ui->colorListWidget);
    colorFlowLayout->setContentsMargins(0, 0, 0, 0);
    ui->colorListWidget->setLayout(colorFlowLayout);

    localBgd = new HoverWidget("");
    localBgd->setObjectName("localBgd");
    //~ contents_path /wallpaper/Local wallpaper
    localBgd->setToolTip(tr("Local wallpaper"));
    localBgd->setFixedSize(136,56);
    localBgd->setStyleSheet("HoverWidget#localBgd{background: palette(base); border-radius: 12px;}HoverWidget:hover:!pressed#localBgd{background: #2FB3E8; border-radius: 12px;}");
    QHBoxLayout *addLyt_1 = new QHBoxLayout;
    QLabel * iconLabel_1 = new QLabel();
    QLabel * textLabel_1 = new QLabel(tr("Local wall..."));
    textLabel_1->setStyleSheet("color: #2FB3E8;font-weight: 400;");
    QPixmap pixgray_1 = ImageUtil::loadSvg(":/img/titlebar/openLocal.svg", "default", 24);
    iconLabel_1->setPixmap(pixgray_1);
    addLyt_1->addStretch();
    addLyt_1->addWidget(iconLabel_1);
    addLyt_1->addWidget(textLabel_1);
    addLyt_1->addStretch();
    localBgd->setLayout(addLyt_1);
    ui->horizontalLayout_4->addWidget(localBgd);

    // 悬浮改变Widget状态
    connect(localBgd, &HoverWidget::enterWidget, this, [=](QString mname){
        Q_UNUSED(mname);
        QPixmap pixgray_1 = ImageUtil::loadSvg(":/img/titlebar/openLocal.svg", "white", 24);
        iconLabel_1->setPixmap(pixgray_1);
        textLabel_1->setStyleSheet("color: palette(base);");

    });
    // 还原状态
    connect(localBgd, &HoverWidget::leaveWidget, this, [=](QString mname){
        Q_UNUSED(mname);
        QPixmap pixgray_1 = ImageUtil::loadSvg(":/img/titlebar/openLocal.svg", "default", 24);
        iconLabel_1->setPixmap(pixgray_1);
        textLabel_1->setStyleSheet("color: #2FB3E8;font-weight: 400;");
    });
    ui->titleLabel->setAlignment(Qt::AlignLeft);

    connect(localBgd, &HoverWidget::widgetClicked,[=]{
        showLocalWpDialog();
    });
    resetBgd = new HoverWidget("");
    resetBgd->setObjectName("resetBgd");
    resetBgd->setFixedSize(136,56);
    resetBgd->setStyleSheet("HoverWidget#resetBgd{background: palette(base); border-radius: 12px;}HoverWidget:hover:!pressed#resetBgd{background: #2FB3E8; border-radius: 12px;}");
    addLyt_2 = new QHBoxLayout(pluginWidget);
    QLabel * iconLabel_2 = new QLabel();
    //~ contents_path /wallpaper/Reset
    QLabel * textLabel_2 = new QLabel(tr("Reset"));
    textLabel_2->setStyleSheet("color: #2FB3E8;font-weight: 400;");
    QPixmap pixgray_2 = ImageUtil::loadSvg(":/img/titlebar/resetBgd.svg", "default", 24);
    iconLabel_2->setPixmap(pixgray_2);
    addLyt_2->addStretch();
    addLyt_2->addWidget(iconLabel_2);
    addLyt_2->addWidget(textLabel_2);
    addLyt_2->addStretch();
    resetBgd->setLayout(addLyt_2);
    ui->horizontalLayout_4->addWidget(resetBgd);
    ui->horizontalLayout_4->addItem(new QSpacerItem(16,20,QSizePolicy::Expanding));
    // 悬浮改变Widget状态
    connect(resetBgd, &HoverWidget::enterWidget, this, [=](QString mname){
        Q_UNUSED(mname);
        QPixmap pixgray_2 = ImageUtil::loadSvg(":/img/titlebar/resetBgd.svg", "white", 24);
        iconLabel_2->setPixmap(pixgray_2);
        textLabel_2->setStyleSheet("color: palette(base);font-weight: 400;");

    });
    // 还原状态
    connect(resetBgd, &HoverWidget::leaveWidget, this, [=](QString mname){
        Q_UNUSED(mname);
        QPixmap pixgray_2 = ImageUtil::loadSvg(":/img/titlebar/resetBgd.svg", "default", 24);
        iconLabel_2->setPixmap(pixgray_2);
        textLabel_2->setStyleSheet("color: #2FB3E8;font-weight: 400;");
    });
    connect(resetBgd, &HoverWidget::widgetClicked,[=]{
        GSettings * wpgsettings;
        wpgsettings = g_settings_new(BACKGROUND);
        GVariant * variant = g_settings_get_default_value(wpgsettings, FILENAME);
        gsize size = g_variant_get_size(variant);
        const char * dwp = g_variant_get_string(variant, &size);
        g_object_unref(wpgsettings);

        bgsettings->set(FILENAME, QVariant(QString(dwp)));
        bgsettings->set(OPTIONS, "scaled");

    });
    colWgt = new HoverWidget("");
    colWgt->setObjectName("colWgt");
    colWgt->setStyleSheet("HoverWidget#colWgt{background: palette(button); border-radius: 4px;}HoverWidget:hover:!pressed#colWgt{background: #3D6BE5; border-radius: 4px;}");
    addLyt = new QHBoxLayout(pluginWidget);
    QLabel * iconLabel = new QLabel();
    QLabel * textLabel = new QLabel(tr("Custom color"));
    QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "black", 12);
    iconLabel->setPixmap(pixgray);
    addLyt->addWidget(iconLabel);
    addLyt->addWidget(textLabel);
    addLyt->addStretch();
    colWgt->setLayout(addLyt);

    // 悬浮改变Widget状态
    connect(colWgt, &HoverWidget::enterWidget, this, [=](QString mname){
        Q_UNUSED(mname);
        QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "white", 12);
        iconLabel->setPixmap(pixgray);
        textLabel->setStyleSheet("color: palette(base);");

    });
    // 还原状态
    connect(colWgt, &HoverWidget::leaveWidget, this, [=](QString mname){
        Q_UNUSED(mname);
        QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "black", 12);
        iconLabel->setPixmap(pixgray);
        textLabel->setStyleSheet("color: palette(windowText);");
    });

    //壁纸放置方式
    //~ contents_path /wallpaper/padding
    ui->picOptionsComBox->addItem(tr("padding"), "scaled");
    //~ contents_path /wallpaper/tile
    ui->picOptionsComBox->addItem(tr("tile"), "wallpaper");
    //~ contents_path /wallpaper/centered
    ui->picOptionsComBox->addItem(tr("centered"), "centered");
    //~ contents_path /wallpaper/stretched
    ui->picOptionsComBox->addItem(tr("stretched"), "stretched");

    //屏蔽纯色背景的确定按钮
    ui->cancelBtn->hide();
    ui->certainBtn->hide();
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
                prePicUnit->setStyleSheet("border-width:0px;");
            }
            picUnit->changeClickedFlag(true);
            prePicUnit = picUnit;
            picUnit->setFrameShape(QFrame::Box);
            picUnit->setStyleSheet(picUnit->clickedStyleSheet);
        }

        connect(picUnit, &PictureUnit::clicked, [=](QString fn){
            if (prePicUnit != nullptr) {
                prePicUnit->changeClickedFlag(false);
                prePicUnit->setStyleSheet("border-width:0px;");
            }
            picUnit->changeClickedFlag(true);
            prePicUnit = picUnit;
            picUnit->setFrameShape(QFrame::Box);
            picUnit->setStyleSheet(picUnit->clickedStyleSheet);
            bgsettings->set(FILENAME, fn);
            ui->previewStackedWidget->setCurrentIndex(PICTURE);
        });
        picFlowLayout->addWidget(picUnit);
    });
    connect(pObject, &WorkerObject::workComplete, this, [=](QMap<QString, QMap<QString, QString>> wpInfoMaps){
        wallpaperinfosMap = wpInfoMaps;
        pThread->quit(); //退出事件循环
        pThread->wait(); //释放资源
    });
    QString pictureOptions = bgsettings->get(OPTIONS).toString();
    for (int i = 0; i < 4; i++) {
        if (pictureOptions == ui->picOptionsComBox->itemData(i)) {
            ui->picOptionsComBox->setCurrentIndex(i);
        }
    }
    connect(bgsettings,&QGSettings::changed,this,[=](const QString &key){
        if (key == "pictureOptions") {
            QString pictureOptions = bgsettings->get(OPTIONS).toString();
            for (int i = 0; i < 4; i++) {
                if (pictureOptions == ui->picOptionsComBox->itemData(i)) {
                    ui->picOptionsComBox->setCurrentIndex(i);
                }
            }
        }
    });
    pObject->moveToThread(pThread);
    connect(pThread, &QThread::started, pObject, &WorkerObject::run);
    connect(pThread, &QThread::finished, this, [=](){
        if (ui->pictureButton->isChecked()){
            //            设置当前壁纸放置方式;
            QString filename = bgsettings->get(FILENAME).toString();
            if (wallpaperinfosMap.contains(filename)){
                QMap<QString, QString> currentwpMap = wallpaperinfosMap.value(filename);
                if (currentwpMap.contains("options")){
                    QString opStr = QString::fromLocal8Bit("%1").arg(currentwpMap.value("options"));
                    ui->picOptionsComBox->blockSignals(true);
                    ui->picOptionsComBox->setCurrentText(tr("%1").arg(opStr));
                    ui->picOptionsComBox->blockSignals(false);
                }
            }
        }
    });
    connect(pThread, &QThread::finished, this, [=](){


    });

    connect(pThread, &QThread::finished, pObject, &WorkerObject::deleteLater);

    pThread->start();

    connect(ui->picOptionsComBox, SIGNAL(currentTextChanged(QString)), this, SLOT(wpOptionsChangedSlot(QString)));

    //    connect(ui->browserLocalwpBtn, &QPushButton::clicked, [=]{
    //        showLocalWpDialog();
    //    });


    //    connect(ui->browserOnlinewpBtn, &QPushButton::clicked, [=]{
    //        QDesktopServices::openUrl(QUrl(kylinUrl));
    //    });
    //    connect(ui->resetBtn, SIGNAL(clicked(bool)), this, SLOT(resetDefaultWallpaperSlot()));

    ///纯色背景
    QStringList colors;

    colors << "#ECC81D" << "#EE7C71" << "#49E17B" << "#3DD0D6" << "#2FB3E8" << "#867BE5" << "#AC7BE5" << "#D678DD" << "#DD789B" << "#EB6D6D"<< "#EB986D";
    colors << "#EBC16D" << "#C3DE6E" << "#8CDA6F" << "#6DD4A3" << "#6DD9DE" << "#5EAEF5" << "#47A2F3" << "#518CE7" << "#518CE7" << "#DFBC25"<< "#DA6155";
    colors << "#46C670" << "#3ABAC0" << "#299DCC" << "#665ACF" << "#9059CF" ;
    setColor = new clickLabel();
    setColor->setFixedSize(62,62);
    setColor->setPixmap(QPixmap("://img/plugins/wallpaper/setColor.png"));
    for (QString color : colors){
        button = new QPushButton(ui->colorListWidget);
        button->setFixedSize(QSize(62, 62));
        QString btnQss = QString("QPushButton{background: %1; border: none ; border-radius: 12px;}"
                                 "QPushButton:hover{background: %1; border: 2px solid #FFFFFF; border-radius: 12px;}").arg(color);
        button->setStyleSheet(btnQss);
        connect(button, &QPushButton::clicked, [=]{
            QString widgetQss = QString("QWidget{background: %1; border-radius: 8px;}").arg(color);
            ui->previewWidget->setStyleSheet(widgetQss);

            ///设置系统纯色背景
            bgsettings->set(PRIMARY, QVariant(color));
            bgsettings->set(SECONDARY, QVariant(color));
            bgsettings->set(FILENAME, "");

            ui->previewStackedWidget->setCurrentIndex(COLOR);
        });
        colorFlowLayout->addWidget(button);
        colorFlowLayout->addItem(new QSpacerItem(8,1,QSizePolicy::Fixed));
    }
    colorFlowLayout->addWidget(setColor);
    connect(setColor,SIGNAL(clicked()), SLOT(opencolorSelectedSlot()));
    connect(ui->pictureButton,&QRadioButton::clicked,[=]{
        ui->substackedWidget->setCurrentIndex(0);
        resetBgd->show();
        localBgd->show();
        ui->picOptionsComBox->show();
        ui->picOptionsLabel->show();
        ui->switchFrame_2->show();
        ui->colorPage->hide();
        ui->picturePage->show();
    });

    connect(ui->colorButton,&QRadioButton::clicked,[=]{
        ui->substackedWidget->setCurrentIndex(1);
        resetBgd->hide();
        localBgd->hide();
        ui->picOptionsComBox->hide();
        ui->picOptionsLabel->hide();
        ui->switchFrame_2->hide();
        ui->picturePage->hide();
        ui->colorPage->show();

    });
    //壁纸变动后改变用户属性
    connect(bgsettings, &QGSettings::changed, [=](QString key){

        initBgFormStatus();

        if (ui->pictureButton->isChecked()) {

            QString fileName = bgsettings->get(FILENAME).toString();
            setClickedPic(fileName);
        }
        //GSettings key picture-filename 这里收到 pictureFilename的返回值
        if (!QString::compare(key, "pictureFilename")){
            QString curPicname = bgsettings->get(key).toString();

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

    //设置当前背景形式
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
    if(currentIndex == 0){
        ui->pictureButton->setChecked(true);
        ui->substackedWidget->setCurrentIndex(0);
        ui->previewStackedWidget->setCurrentIndex(0);
    } else {
        ui->colorButton->setChecked(true);
        ui->substackedWidget->setCurrentIndex(1);
        ui->previewStackedWidget->setCurrentIndex(1);
    }
    //根据背景形式选择显示组件
    showComponent(currentIndex);
}

void Wallpaper::showComponent(int index){
    if (PICTURE == index){ //图片
        resetBgd->show();
        localBgd->show();
        ui->picOptionsComBox->show();
        ui->picOptionsLabel->show();
        ui->switchFrame_2->show();
    } else if (COLOR == index){ //纯色
        resetBgd->hide();
        localBgd->hide();
        ui->picOptionsComBox->hide();
        ui->picOptionsLabel->hide();
        ui->switchFrame_2->hide();
    } else { //幻灯片

    }
}

void Wallpaper::initPreviewStatus(){
    //设置图片背景的预览效果
    QString filename = bgsettings->get(FILENAME).toString();
    //    qDebug()<<"preview pic is---------->"<<filename<<endl;

    QByteArray ba = filename.toLatin1();
    //    if (g_file_test(ba.data(), G_FILE_TEST_EXISTS)){
    ui->previewLabel->setPixmap(QPixmap(filename).scaled(ui->previewLabel->size()));
    //    }

    //设置纯色背景的预览效果
    QString color = bgsettings->get(PRIMARY).toString();
    if (!color.isEmpty()){
        QString widgetQss = QString("QWidget{background: %1; border-radius: 6px;}").arg(color);
        ui->previewWidget->setStyleSheet(widgetQss);
    }
}
void Wallpaper::opencolorSelectedSlot(){
    colordialog = new ColorDialog();
    connect(colordialog,&ColorDialog::colorSelected,this,&Wallpaper::colorSelectedSlot);
    colordialog->exec();
}
//自定义颜色面板选定颜色
void Wallpaper::colorSelectedSlot(QColor color){
    qDebug() << "colorSelectedSlot" << color << color.name();

    QString widgetQss = QString("QWidget{background: %1; border-radius: 6px;}").arg(color.name());
    ui->previewWidget->setStyleSheet(widgetQss);

    ///设置系统纯色背景
    bgsettings->set(PRIMARY, QVariant(color.name()));
    bgsettings->set(FILENAME, "");

    ui->previewStackedWidget->setCurrentIndex(COLOR);
}

void Wallpaper::wpOptionsChangedSlot(QString op){
    //获取当前选中的壁纸
    //    QListWidgetItem * currentitem = ui->listWidget->currentItem();
    //    QString filename = currentitem->data(Qt::UserRole).toString();

    bgsettings->set(OPTIONS, ui->picOptionsComBox->currentData().toString());
    //更新xml数据
    //    if (wallpaperinfosMap.contains(filename)){
    //        wallpaperinfosMap[filename]["options"] = op;
    //    }

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
        wpListModel.setData(wpindex, QIcon(pixmap.scaled(QSize(198,110))), Qt::DecorationRole);
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
    //由于文件管理器画壁纸使用QPixmap解析壁纸文件，不支持tif和tiff格式，故此处去掉这两种格式的选择
//    filters<<tr("Wallpaper files(*.jpg *.jpeg *.bmp *.dib *.png *.jfif *.jpe *.gif *.tif *.tiff *.wdp)")<<tr("allFiles(*.*)");
    filters<<tr("Wallpaper files(*.jpg *.jpeg *.bmp *.dib *.png *.jfif *.jpe *.gif *.wdp)");
    QFileDialog fd(pluginWidget);

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

    if (fd.exec() != QDialog::Accepted)
        return;
    QString selectedfile;
    selectedfile = fd.selectedFiles().first();

    QStringList fileRes = selectedfile.split("/");

    QProcess * process = new QProcess();
    QString program("cp");
    QStringList arguments;
    arguments << selectedfile ;
//    arguments << "/tmp";
    arguments << QDir::homePath() + "/.config";
    process->start(program, arguments);

//    QString bgfile = "/tmp/" + fileRes.at(fileRes.length() - 1);
    QString bgfile = QDir::homePath() + "/.config/" + fileRes.at(fileRes.length() - 1);

    // TODO: chinese and space support
    //    if (g_file_test(selectedfile.toLatin1().data(), G_FILE_TEST_EXISTS)) {
    qDebug()<<bgfile;
    bgsettings->set(FILENAME, QVariant(bgfile));
    //    } else {
    //        bgsettings->reset(FILENAME);
    //    }
}

void Wallpaper::add_custom_wallpaper(){
    QString filters = "Wallpaper files(*.png *.jpg)";
    QFileDialog fd;
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

    if (fd.exec() != QDialog::Accepted)
        return;

    QString selectedfile;
    selectedfile = fd.selectedFiles().first();

    QSize IMAGE_SIZE(198, 110);
    QPixmap pixmap = QPixmap(selectedfile).scaled(IMAGE_SIZE);
    //    append_item(pixmap, selectedfile);

    if (wallpaperinfosMap.contains(selectedfile)){
        wallpaperinfosMap[selectedfile]["deleted"] = "false";
    }
    else{
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

    if (picWpItemMap.contains(selectedfile)){
        //        ui->listWidget->setCurrentItem(picWpItemMap.find(selectedfile).value());
    }

}
void Wallpaper::del_wallpaper(){
    //获取当前选中的壁纸
    //    QListWidgetItem * currentitem = ui->listWidget->currentItem();
    //    QString filename = currentitem->data(Qt::UserRole).toString();

    //更新xml数据
    //    if (wallpaperinfosMap.contains(filename)){
    //        wallpaperinfosMap[filename]["deleted"] = "true";

    //        int row = ui->listWidget->row(currentitem);

    //        int nextrow = ui->listWidget->count() - 1 - row ? row + 1 : row - 1;

    //        ui->listWidget->setCurrentItem(ui->listWidget->item(nextrow));

    //        ui->listWidget->takeItem(row);

    //    }

    //    将改动保存至文件
    xmlhandleObj->xmlUpdate(wallpaperinfosMap);
}


void Wallpaper::setClickedPic(QString fileName)
{
    if (prePicUnit != nullptr) {
        prePicUnit->changeClickedFlag(false);
        prePicUnit->setStyleSheet("border-width:0px;");
    }
    for (int i = picFlowLayout->count()-1; i>= 0; --i) {
        QLayoutItem *it = picFlowLayout->itemAt(i);
        PictureUnit *picUnit = static_cast<PictureUnit*>(it->widget());
        if (fileName == picUnit->filenameText()) {
            picUnit->changeClickedFlag(true);
            prePicUnit = picUnit;
            picUnit->setFrameShape(QFrame::Box);
            picUnit->setStyleSheet(picUnit->clickedStyleSheet);
        }
    }
}
