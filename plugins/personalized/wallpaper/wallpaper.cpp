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

#include <QDebug>

enum{
    PICTURE, //图片背景
    COLOR, //纯色背景
    SLIDESHOW //幻灯片背景
};

#define ITEMWIDTH 182
#define ITEMHEIGH 126

#define COLORITEMWIDTH 56
#define COLORITEMHEIGH 56

Wallpaper::Wallpaper()
{
    ui = new Ui::Wallpaper;
    itemDelege = new QStyledItemDelegate();
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("background");
    pluginType = PERSONALIZED;

    pluginWidget->setStyleSheet("background: #ffffff;");

    ui->previewLabel->setStyleSheet("QLabel#previewLabel{border-radius: 6px;}");

    ui->switchWidget->setStyleSheet("QWidget{background: #F4F4F4; border-radius: 6px;}");

    ui->listWidget->setStyleSheet("QListWidget#listWidget{background: #ffffff; border: none;}");
    ui->colorListWidget->setStyleSheet("QListWidget#colorListWidget{background: #ffffff; border: none;}");

    QString btnQss = QString("QPushButton{background: #E9E9E9; border-radius: 4px;}"
                             "QPushButton:hover:!pressed{background: #3d6be5; border: none; border-radius: 4px;}"
                             "QPushButton:hover:pressed{background: #415FC4; border: none; border-radius: 4px;}");
    ui->browserLocalwpBtn->setStyleSheet(btnQss);
    ui->browserOnlinewpBtn->setStyleSheet(btnQss);

    ui->resetBtn->setStyleSheet("QPushButton{border: none;}");

    //初始化gsettings
    const QByteArray id(BACKGROUND);
    bgsettings = new QGSettings(id);

    //构建xmlhandle对象
    xmlhandleObj = new XmlHandle();

    //初始化控件
    setupComponent();

    initBgFormStatus();
}

Wallpaper::~Wallpaper()
{
    delete ui;
    delete bgsettings;
    delete xmlhandleObj;

}

QString Wallpaper::get_plugin_name(){
    return pluginName;
}

int Wallpaper::get_plugin_type(){
    return pluginType;
}

QWidget *Wallpaper::get_plugin_ui(){
    return pluginWidget;
}

void Wallpaper::plugin_delay_control(){
}

void Wallpaper::setupComponent(){
    //背景形式
    QStringList formList;
    formList << tr("picture") << tr("color")/* << tr("slideshow")*/ ;
    ui->formComBox->setItemDelegate(itemDelege);
    ui->formComBox->setMaxVisibleItems(5);
    ui->formComBox->addItems(formList);

    ui->picOptionsComBox->setItemDelegate(itemDelege);
    ui->picOptionsComBox->setMaxVisibleItems(5);

    ui->previewLabel->setScaledContents(true);
    initPreviewStatus();

    ///图片背景
    ui->listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->listWidget->setResizeMode(QListView::Adjust);
    ui->listWidget->setViewMode(QListView::IconMode);
    ui->listWidget->setMovement(QListView::Static);
    ui->listWidget->setSpacing(0);
    ui->listWidget->setFixedHeight(2 * ITEMHEIGH);

    //使用线程构建本地壁纸文件；获取壁纸压缩QPixmap
    pThread = new QThread;
    pObject = new WorkerObject;
    connect(pObject, &WorkerObject::pixmapGenerate, this, [=](QPixmap pixmap, QString filename){
        appendPicWpItem(pixmap, filename);
    });
    connect(pObject, &WorkerObject::workComplete, this, [=](QMap<QString, QMap<QString, QString>> wpInfoMaps){
        wallpaperinfosMap = wpInfoMaps;
        pThread->quit(); //退出事件循环
        pThread->wait(); //释放资源
    });

    pObject->moveToThread(pThread);
    connect(pThread, &QThread::started, pObject, &WorkerObject::run);
    connect(pThread, &QThread::finished, this, [=]{
        if (ui->formComBox->currentIndex() == PICTURE){
            //设置当前壁纸
            QString filename = bgsettings->get(FILENAME).toString();
            if (picWpItemMap.contains(filename)){
                QListWidgetItem * currentItem = picWpItemMap.value(filename);
                ui->listWidget->blockSignals(true);
                ui->listWidget->setCurrentItem(currentItem);
                ui->listWidget->blockSignals(false);
            }
            //设置当前壁纸放置方式
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
    connect(pThread, &QThread::finished, pObject, &WorkerObject::deleteLater);

    pThread->start();

    //壁纸放置方式
//    QStringList layoutList;
//    layoutList << tr("wallpaper") << tr("centered") << tr("scaled") << tr("stretched") << tr("zoom") << tr("spanned");
//    ui->picOptionsComBox->addItems(layoutList);
    ui->picOptionsComBox->addItem(tr("wallpaper"), "wallpaper");
    ui->picOptionsComBox->addItem(tr("centered"), "centered");
    ui->picOptionsComBox->addItem(tr("scaled"), "scaled");
    ui->picOptionsComBox->addItem(tr("stretched"), "stretched");
    ui->picOptionsComBox->addItem(tr("zoom"), "zoom");
    ui->picOptionsComBox->addItem(tr("spanned"), "spanned");

    connect(ui->listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(picWallpaperChangedSlot(QListWidgetItem*,QListWidgetItem*)));
    connect(ui->formComBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
        ui->substackedWidget->setCurrentIndex(index);
        //显示/隐藏控件
        showComponent(index);

        //当前背景类型与当前背景页面不同
        if (index != _getCurrentBgForm()){
            if (PICTURE == index){
                //设置图片背景

                QString fileName = ui->listWidget->currentItem()->data(Qt::UserRole).toString();

                bgsettings->set(FILENAME, fileName);
                ui->listWidget->setCurrentItem(ui->listWidget->item(0));


            } else if (COLOR == index){
                //设置图片背景为空
                bgsettings->set(FILENAME, "");
                //设置纯色背景
                ui->colorListWidget->setCurrentItem(ui->colorListWidget->item(0));
            } else {

            }
        }
    });
    connect(ui->picOptionsComBox, SIGNAL(currentTextChanged(QString)), this, SLOT(wpOptionsChangedSlot(QString)));
    connect(ui->resetBtn, SIGNAL(clicked(bool)), this, SLOT(resetDefaultWallpaperSlot()));

    ///纯色背景
    QStringList colors;

    colors << "#2d7d9a" << "#018574" << "#107c10" << "#10893e" << "#038387" << "#486860" << "#525e54" << "#7e735f" << "#4c4a48" << "#000000";
    colors << "#ff8c00" << "#e81123" << "#d13438" << "#c30052" << "#bf0077" << "#9a0089" << "#881798" << "#744da9" << "#8764b8" << "#e9e9e9";

    ui->colorListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->colorListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->colorListWidget->setResizeMode(QListView::Adjust);
    ui->colorListWidget->setViewMode(QListView::IconMode);
    ui->colorListWidget->setMovement(QListView::Static);
    ui->colorListWidget->setSpacing(0);
    ui->colorListWidget->setFixedHeight(COLORITEMHEIGH * 2);

    for (QString color : colors){
        appendColWpItem(color);
    }
    connect(ui->colorListWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(colWallpaperChangedSlot(QListWidgetItem*,QListWidgetItem*)));
}

int Wallpaper::_getCurrentBgForm(){
    QString filename = bgsettings->get(FILENAME).toString();

    qDebug() << "----------" << filename;

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
    int currentIndex = _getCurrentBgForm();
    //设置当前背景形式
    ui->formComBox->setCurrentIndex(currentIndex);
    ui->substackedWidget->setCurrentIndex(currentIndex);

    //屏蔽背景放置方式无效
    ui->picOptionsComBox->hide();
    ui->picOptionsLabel->hide();

    //屏蔽纯色背景的确定按钮
    ui->cancelBtn->hide();
    ui->certainBtn->hide();

    //根据背景形式选择显示组件
    showComponent(currentIndex);
}

void Wallpaper::showComponent(int index){
    if (0 == index){ //图片
//        ui->picOptionsComBox->show();
//        ui->picOptionsLabel->show();
        ui->previewLabel->show();
        ui->previewWidget->hide();
    } else if (1 == index){ //纯色
//        ui->picOptionsComBox->hide();
//        ui->picOptionsLabel->hide();
        ui->previewLabel->hide();
        ui->previewWidget->show();
    } else { //幻灯片

    }
}

void Wallpaper::initPreviewStatus(){
    //设置图片背景的预览效果
    QString filename = bgsettings->get(FILENAME).toString();
    QByteArray ba = filename.toLatin1();
    if (g_file_test(ba.data(), G_FILE_TEST_EXISTS)){
        ui->previewLabel->setPixmap(QPixmap(filename).scaled(ui->previewLabel->size(), Qt::KeepAspectRatio));
    }

    //设置纯色背景的预览效果
    QString color = bgsettings->get(PRIMARY).toString();
    if (!color.isEmpty()){
        QString widgetQss = QString("QWidget{background: %1; border-radius: 6px;}").arg(color);
        ui->previewWidget->setStyleSheet(widgetQss);
    }
}

void Wallpaper::component_init(){
    //背景形式
    QStringList formList;
    formList << tr("picture") << tr("color")/* << tr("slideshow")*/ ;
    ui->formComBox->addItems(formList);

//    init_current_status();


    //        QSize IMAGE_SIZE(160, 120);
    //        ui->listWidget->setIconSize(IMAGE_SIZE);
    ui->listWidget->setResizeMode(QListView::Adjust);
    ui->listWidget->setViewMode(QListView::IconMode);
    ui->listWidget->setMovement(QListView::Static);
    ui->listWidget->setSpacing(10);

//    SimpleThread * thread = new SimpleThread(wallpaperinfosMap, nullptr);
//    connect(thread, &SimpleThread::widgetItemCreate, this, [=](QPixmap pixmap, QString filename){
//        append_item(pixmap, filename);
//    }, Qt::QueuedConnection);
//    connect(thread, &SimpleThread::finished, this, [=]{
//        QString filename = bgsettings->get(FILENAME).toString();
//        //当前背景形式是壁纸
//        if (ui->formComBox->currentIndex() == 0){
//            if (delItemsMap.contains(filename)){
//                QListWidgetItem * currentItem = delItemsMap.find(filename).value();
//                QWidget * widget = ui->listWidget->itemWidget(currentItem);
//                widget->setStyleSheet("QWidget{border: 5px solid #daebff}");
//                //            ui->listWidget->setItemSelected(currentItem, true); //???
//                ui->listWidget->blockSignals(true);
//                ui->listWidget->setCurrentItem(currentItem);
//                ui->listWidget->blockSignals(false);
//            }
//            //设置当前壁纸放置方式
//            if (wallpaperinfosMap.contains(filename)){
//                QMap<QString, QString> currentwpMap = (QMap<QString, QString>) wallpaperinfosMap.find(filename).value();
//                if (currentwpMap.contains("options")){
//                    QString opStr = QString::fromLocal8Bit("%1").arg(currentwpMap.find("options").value());
//                    ui->picOptionsComBox->setCurrentText(tr("%1").arg(opStr));
//                }
//            }
//        }
//    });
//    connect(thread, &SimpleThread::finished, thread, &SimpleThread::deleteLater);
//    thread->start();

    //壁纸放置方式
//    QStringList layoutList;
//    layoutList << tr("wallpaper") << tr("centered") << tr("scaled") << tr("stretched") << tr("zoom") << tr("spanned");
    ui->picOptionsComBox->addItem(tr("wallpaper"), "wallpaper");
    ui->picOptionsComBox->addItem(tr("centered"), "centered");
    ui->picOptionsComBox->addItem(tr("scaled"), "scaled");
    ui->picOptionsComBox->addItem(tr("stretched"), "stretched");
    ui->picOptionsComBox->addItem(tr("zoom"), "zoom");
    ui->picOptionsComBox->addItem(tr("spanned"), "spanned");


    //纯色
    /*ui->colorListWidget->setResizeMode(QListView::Adjust);
    ui->colorListWidget->setViewMode(QListView::IconMode);
    ui->colorListWidget->setMovement(QListView::Static);
    ui->colorListWidget->setSpacing(10);

    ui->listWidget->setStyleSheet("QListView::item:selected{border: 5px solid #ac4844}");

    QSize ITEM_SIZE(65, 65);
    //自定义item
    QString colorStr = "#99FF33";
    QWidget * widget = new QWidget();
    widget->setAttribute(Qt::WA_DeleteOnClose);
    widget->setStyleSheet(QString("background-color: %1").arg(colorStr));

    QListWidgetItem * item = new QListWidgetItem(ui->colorListWidget);
    item->setSizeHint(ITEM_SIZE);
    item->setData(Qt::UserRole, colorStr);
    ui->colorListWidget->setItemWidget(item, widget);

    QString colorStr2 = "#FFFF00";
    QLabel * widget2 = new QLabel();
    widget2->setAttribute(Qt::WA_DeleteOnClose);
    widget2->setStyleSheet(QString("background-color: %1").arg(colorStr2));

    QListWidgetItem * item2 = new QListWidgetItem(ui->colorListWidget);
    item2->setData(Qt::UserRole, colorStr2);
    item2->setSizeHint(ITEM_SIZE);
    ui->colorListWidget->setItemWidget(item2, widget2);

    ui->colorListWidget->setCurrentItem(item);
    widget->setStyleSheet(QString("background-color: %1; border: 5px solid #ac4844").arg(item->data(Qt::UserRole).toString()));

    connect(ui->colorListWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(colorwp_item_clicked(QListWidgetItem*,QListWidgetItem*)));
    */

//    connect(ui->addPushBtn, SIGNAL(clicked(bool)), this, SLOT(add_custom_wallpaper()));
//    connect(ui->delPushBtn, SIGNAL(clicked(bool)), this, SLOT(del_wallpaper()));

}

void Wallpaper::appendPicWpItem(QPixmap pixmap, QString filename){

    //
    QWidget * baseWidget = new QWidget;
    baseWidget->setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout * mainVerLayout = new QVBoxLayout(baseWidget);
    mainVerLayout->setSpacing(0);
    mainVerLayout->setMargin(0);

    QHBoxLayout * baseHorLayout = new QHBoxLayout;
    baseHorLayout->setSpacing(0);
    baseHorLayout->setMargin(0);

    QLabel * wpLable = new QLabel(baseWidget);
    QSizePolicy wpSizePolicy = wpLable->sizePolicy();
    wpSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    wpSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    wpLable->setSizePolicy(wpSizePolicy);
    wpLable->setFixedSize(pixmap.size());
    wpLable->setPixmap(pixmap);

    baseHorLayout->addWidget(wpLable);
    baseHorLayout->addStretch();

    mainVerLayout->addLayout(baseHorLayout);
    mainVerLayout->addStretch();

    baseWidget->setLayout(mainVerLayout);


    QListWidgetItem * item = new QListWidgetItem(ui->listWidget);
    item->setSizeHint(QSize(ITEMWIDTH, ITEMHEIGH));
    item->setData(Qt::UserRole, filename);
    ui->listWidget->setItemWidget(item, baseWidget);

    picWpItemMap.insert(filename, item);
}

void Wallpaper::appendColWpItem(QString color){
    QWidget * baseWidget = new QWidget;
    baseWidget->setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout * mainLayout = new QVBoxLayout(baseWidget);
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);

    QHBoxLayout * baseLayout = new QHBoxLayout;
    baseLayout->setSpacing(0);
    baseLayout->setMargin(0);

    QWidget * widget = new QWidget(baseWidget);
    widget->setFixedSize(QSize(48, 48));
    QString widgetQss = QString("QWidget{background: %1; border-radius: 4px;}").arg(color);
    widget->setStyleSheet(widgetQss);

    baseLayout->addWidget(widget);
    baseLayout->addStretch();

    mainLayout->addLayout(baseLayout);
    mainLayout->addStretch();

    baseWidget->setLayout(mainLayout);

    QListWidgetItem * item = new QListWidgetItem(ui->colorListWidget);
    item->setSizeHint(QSize(COLORITEMWIDTH, COLORITEMHEIGH));
    item->setData(Qt::UserRole, color);
    ui->colorListWidget->setItemWidget(item, baseWidget);

    //设置当前ITEM


}

void Wallpaper::picWallpaperChangedSlot(QListWidgetItem * current, QListWidgetItem *previous){
    Q_UNUSED(previous)
//    if (previous != nullptr){
//        QWidget * previousWidget = ui->listWidget->itemWidget(previous);
//        previousWidget->setStyleSheet("QWidget{border: none}");
//    }

//    QWidget * currentWidget = ui->listWidget->itemWidget(current);
//    currentWidget->setStyleSheet("QWidget{border: 5px solid #daebff}");



    QString filename = current->data(Qt::UserRole).toString();
    bgsettings->set(FILENAME, QVariant(filename));
    qDebug() << "" << filename;
    initPreviewStatus();
}

void Wallpaper::colWallpaperChangedSlot(QListWidgetItem *current, QListWidgetItem *previous){
    Q_UNUSED(previous)

    QString color = current->data(Qt::UserRole).toString();

    bgsettings->set(PRIMARY, QVariant(color));

    initPreviewStatus();

}

void Wallpaper::wpOptionsChangedSlot(QString op){
    //获取当前选中的壁纸
    QListWidgetItem * currentitem = ui->listWidget->currentItem();
    QString filename = currentitem->data(Qt::UserRole).toString();

    bgsettings->set(OPTIONS, ui->picOptionsComBox->currentData().toString());

    //更新xml数据
    if (wallpaperinfosMap.contains(filename)){
        wallpaperinfosMap[filename]["options"] = op;
    }

    //将改动保存至文件
    xmlhandleObj->xmlUpdate(wallpaperinfosMap);
}

void Wallpaper::setlistview(){
    //初始化listview
//    ui->listView->setFocusPolicy(Qt::NoFocus);
//    ui->listView->setAutoFillBackground(true);
//    ui->listView->setIconSize(QSize(160, 100));
//    ui->listView->setResizeMode(QListView::Adjust);
//    ui->listView->setModel(&wpListModel);
//    ui->listView->setViewMode(QListView::IconMode);
//    ui->listView->setSpacing(5);
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

    initPreviewStatus();
}

void Wallpaper::add_custom_wallpaper(){
    QString filters = "Wallpaper files(*.png *.jpg)";
    QFileDialog fd;
    fd.setDirectory(QString(const_cast<char *>(g_get_user_special_dir(G_USER_DIRECTORY_PICTURES))));
    fd.setAcceptMode(QFileDialog::AcceptOpen);
    fd.setViewMode(QFileDialog::List);
    fd.setNameFilter(filters);
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setWindowTitle(tr("selsect custom wallpaper file"));
    fd.setLabelText(QFileDialog::Accept, tr("Select"));
    fd.setLabelText(QFileDialog::LookIn, tr("Position: "));
    fd.setLabelText(QFileDialog::FileName, tr("FileName: "));
    fd.setLabelText(QFileDialog::FileType, tr("FileType: "));
    fd.setLabelText(QFileDialog::Reject, tr("Cancel"));

    if (fd.exec() != QDialog::Accepted)
        return;

    QString selectedfile;
    selectedfile = fd.selectedFiles().first();

    QSize IMAGE_SIZE(160, 120);
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
        ui->listWidget->setCurrentItem(picWpItemMap.find(selectedfile).value());
    }

}

void Wallpaper::del_wallpaper(){
    //获取当前选中的壁纸
    QListWidgetItem * currentitem = ui->listWidget->currentItem();
    QString filename = currentitem->data(Qt::UserRole).toString();

    //更新xml数据
    if (wallpaperinfosMap.contains(filename)){
        wallpaperinfosMap[filename]["deleted"] = "true";

        int row = ui->listWidget->row(currentitem);

        int nextrow = ui->listWidget->count() - 1 - row ? row + 1 : row - 1;

        ui->listWidget->setCurrentItem(ui->listWidget->item(nextrow));

        ui->listWidget->takeItem(row);

    }

//    将改动保存至文件
    xmlhandleObj->xmlUpdate(wallpaperinfosMap);
}
