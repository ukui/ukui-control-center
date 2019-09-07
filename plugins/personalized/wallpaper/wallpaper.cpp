#include "wallpaper.h"
#include "ui_wallpaper.h"

#include <QDebug>

Wallpaper::Wallpaper()
{
    ui = new Ui::Wallpaper;
    pluginWidget = new CustomWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("background");
    pluginType = PERSONALIZED;

    //初始化gsettings
    const QByteArray id(BACKGROUND);
    bgsettings = new QGSettings(id);

    //构建xmlhandle对象，本地xml文件不存在则自动构建
    xmlhandleObj = new XmlHandle();

    initData();
    component_init();

    //壁纸的控件listview
//    setlistview();
//    setModeldata();
}

Wallpaper::~Wallpaper()
{
    delete ui;
    delete bgsettings;
    delete xmlhandleObj;
//    QMap<QString, QListWidgetItem *>::Iterator it = delItemsMap.begin();
//    QListWidgetItem * delItem;
//    for (; it != delItemsMap.end(); it++){
//        delItem = (QListWidgetItem *)it.value();
//        delete(delItem);
//    }
    delItemsMap.clear();
}

QString Wallpaper::get_plugin_name(){
    return pluginName;
}

int Wallpaper::get_plugin_type(){
    return pluginType;
}

CustomWidget *Wallpaper::get_plugin_ui(){
    return pluginWidget;
}

void Wallpaper::plugin_delay_control(){

}

void Wallpaper::initData(){
    //解析本地xml填充数据
    localwpconf = QString("%1/%2/%3").arg(QDir::homePath()).arg(".config/ukui").arg("wallpaper.xml");
    wallpaperinfosMap = xmlhandleObj->xmlreader(localwpconf);
}

void Wallpaper::component_init(){
        QSize IMAGE_SIZE(160, 100);
        QSize ITEM_SIZE(160, 120);
        ui->listWidget->setIconSize(IMAGE_SIZE);
        ui->listWidget->setResizeMode(QListView::Adjust);
        ui->listWidget->setViewMode(QListView::IconMode);
        ui->listWidget->setMovement(QListView::Static);
        ui->listWidget->setSpacing(8);

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
            QListWidgetItem * item = new QListWidgetItem(QIcon(pixmap.scaled(IMAGE_SIZE)), "");
            item->setSizeHint(ITEM_SIZE);
            item->setData(Qt::UserRole, filename);
//            item->setToolTip(QString("%1\nfolder: %2\n").arg(wpMap.find("name").value()).arg(filename));
            delItemsMap.insert(filename, item);
            ui->listWidget->insertItem(row, item);
        }

        //背景形式
        QStringList formList;
        formList << tr("picture") << tr("slideshow") << tr("color");
        ui->formComboBox->addItems(formList);

        //壁纸放置方式
        QStringList layoutList;
        layoutList << tr("wallpaper") << tr("centered") << tr("scaled") << tr("stretched") << tr("zoom") << tr("spanned");
        ui->wpoptionsComboBox->addItems(layoutList);

        init_current_status();

        connect(ui->listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(wallpaper_item_clicked(QListWidgetItem*)));
        connect(ui->formComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(form_combobox_changed(int)));
        connect(ui->wpoptionsComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(options_combobox_changed(QString)));

}

void Wallpaper::init_current_status(){
    QString filename = bgsettings->get(FILENAME).toString();

    //设置当前背景形式
    if (filename == "")
        ui->formComboBox->setCurrentIndex(2);
    else if (filename.endsWith("xml"))
        ui->formComboBox->setCurrentIndex(1);
    else
        ui->formComboBox->setCurrentIndex(0);

    //当前背景形式是壁纸
    if (ui->formComboBox->currentIndex() == 0){
        if (delItemsMap.contains(filename)){
            QListWidgetItem * currentItem = (QListWidgetItem *)delItemsMap.find(filename).value();
//            ui->listWidget->setItemSelected(currentItem, true); //???
            ui->listWidget->setCurrentItem(currentItem);
        }
        //设置当前壁纸放置方式
        if (wallpaperinfosMap.contains(filename)){
            QMap<QString, QString> currentwpMap = (QMap<QString, QString>) wallpaperinfosMap.find(filename).value();
            if (currentwpMap.contains("options")){
                QString opStr = QString::fromLocal8Bit("%1").arg(currentwpMap.find("options").value());
                ui->wpoptionsComboBox->setCurrentText(tr("%1").arg(opStr));
            }
        }
    }

//    //当前背景形式是幻灯片
//    if (ui->formComboBox->currentIndex() == 1){
//        ;
//    }

//    //当前背景形式是纯色
//    if (ui->formComboBox->currentIndex() == 2){
//        ;
//    }
}

void Wallpaper::wallpaper_item_clicked(QListWidgetItem * item){
    QString filename = item->data(Qt::UserRole).toString();
    bgsettings->set(FILENAME, QVariant(filename));
    init_current_status();
}

void Wallpaper::form_combobox_changed(int index){
    ui->stackedWidget->setCurrentIndex(index);
    init_current_status();
}

void Wallpaper::options_combobox_changed(QString op){
    //获取当前选中的壁纸
    QListWidgetItem * currentitem = ui->listWidget->currentItem();
    QString filename = currentitem->data(Qt::UserRole).toString();
    qDebug() << filename << "----" << op;

    //更新xml数据
    if (wallpaperinfosMap.contains(filename)){
        wallpaperinfosMap[filename]["options"] = op;
    }

    //将改动保存至文件
    xmlhandleObj->xmlwriter(localwpconf, wallpaperinfosMap);
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
