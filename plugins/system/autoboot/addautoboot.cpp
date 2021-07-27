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
#include "addautoboot.h"
#include "ui_addautoboot.h"

#include <QDebug>
#include <QCheckBox>

//#define DESKTOPPATH "/etc/xdg/autostart/"
#define DESKTOPPATH "/usr/share/applications/"

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

AddAutoBoot::AddAutoBoot(QStringList list, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddAutoBoot)
{
    foreach(QString filename,list){
        addedList.append(filename);
    }
//    addedList = list;
//    foreach (QString filename, addedList) {
//        qDebug()<<filename;
//    }
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    ui->titleLabel->setStyleSheet("QLabel{font-size: 18px; color: palette(windowText);}");

    selectFile = "";

//    ui->frame->setStyleSheet("QFrame{background: #ffffff; border: none; border-radius: 6px;}");

//    //关闭按钮在右上角，窗体radius 6px，所以按钮只得6px
//    ui->closeBtn->setStyleSheet("QPushButton#closeBtn{background: #ffffff; border: none; border-radius: 6px;}"
//                                "QPushButton:hover:!pressed#closeBtn{background: #FA6056; border: none; border-top-left-radius: 2px; border-top-right-radius: 6px; border-bottom-left-radius: 2px; border-bottom-right-radius: 2px;}"
//                                "QPushButton:hover:pressed#closeBtn{background: #E54A50; border: none; border-top-left-radius: 2px; border-top-right-radius: 6px; border-bottom-left-radius: 2px; border-bottom-right-radius: 2px;}");

//    QString lineEditQss = QString("QLineEdit{background: #E9E9E9; border: none; border-radius: 4px;}");
//    ui->nameLineEdit->setStyleSheet(lineEditQss);
//    ui->execLineEdit->setStyleSheet(lineEditQss);
//    ui->commentLineEdit->setStyleSheet(lineEditQss);

//    QString btnQss = QString("QPushButton{background: #E9E9E9; border-radius: 4px;}"
//                             "QPushButton:checked{background: #3d6be5; border-radius: 4px;}"
//                             "QPushButton:hover:!pressed{background: #3d6be5; border-radius: 4px;}"
//                             "QPushButton:hover:pressed{background: #415FC4; border-radius: 4px;}");

//    ui->cancelBtn->setStyleSheet(btnQss);
//    ui->certainBtn->setStyleSheet(btnQss);

    ui->closeBtn->setIcon(QIcon("://img/titlebar/close.svg"));
    ui->closeBtn->setProperty("useIconHighlightEffect", true);
    ui->closeBtn->setProperty("iconHighlightEffectMode", 1);
    ui->closeBtn->setFlat(true);

    ui->closeBtn->setStyleSheet("QPushButton:hover:!pressed#closeBtn{background: #FA6056; border-radius: 4px;}"
                                "QPushButton:hover:pressed#closeBtn{background: #E54A50; border-radius: 4px;}");


//    connect(ui->openBtn, SIGNAL(clicked(bool)), this, SLOT(open_desktop_dir_slots()));
//    connect(ui->cancelBtn, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(ui->cancelBtn, &QPushButton::clicked, [=]{
//        resetBeforeClose();
        close();
    });
//    connect(ui->certainBtn, &QPushButton::clicked, this, [=]{
//        emit autoboot_adding_signals(selectFile, ui->nameLineEdit->text(), ui->execLineEdit->text(), ui->commentLineEdit->text());
//        resetBeforeClose();
//    });
    connect(ui->closeBtn, &QPushButton::clicked, [=]{
//        resetBeforeClose();
        close();
    });
//    get_all_files(DESKTOPPATH);
}
void AddAutoBoot::refreshList(QString filename){
    addedList.removeOne(filename);
    qDebug()<<"--------------------";
    foreach(QString s, addedList){
        qDebug()<<s;
    }
    this->findChild<QCheckBox *>(QString("%1button").arg(filename))->setEnabled(true);
    this->findChild<QCheckBox *>(QString("%1button").arg(filename))->setChecked(false);
//    if (ui->appsLayout->layout() != NULL) {
//        QLayoutItem* item;
//        while ((item = ui->appsLayout->layout()->takeAt(0)) != NULL )
//        {
//            delete item->widget();
//            delete item;
//        }
//    }
//    get_all_files(DESKTOPPATH);
}

void AddAutoBoot::get_all_files(QString path){
    // 获取所有文件名
    QDir dir(path);
    //设置文件过滤器
    QStringList nameFilters;
    //设置文件过滤格式
    nameFilters << "*.desktop";
    //将过滤后的文件名称存入到files列表中
    desktopFiles = dir.entryList(nameFilters, QDir::Files|QDir::Readable, QDir::Name);
    qint64 uid=static_cast<qint64>(getuid());
    QDBusInterface iface("org.freedesktop.Accounts",
                         "/org/freedesktop/Accounts",
                         "org.freedesktop.Accounts",
                         QDBusConnection::systemBus());
    QDBusReply<QDBusObjectPath> objPath=iface.call("FindUserById",uid);
    QDBusInterface useriface("org.freedesktop.Accounts",
                             objPath.value().path(),
                             "org.freedesktop.DBus.Properties",
                             QDBusConnection::systemBus());
    QDBusReply<QVariant> var=useriface.call("Get","org.freedesktop.Accounts.User","UserName");
    QString name=var.value().toString();
    QDBusInterface desktopfpListiface("com.kylin.intel.edu.appsinfo",
                             "/com/kylin/intel/edu/appsinfo",
                             "com.kylin.intel.edu.appsinfo.interface",
                             QDBusConnection::systemBus());
    //qDebug()<<"name"<<username;
    QList<QVariant> desktopfpvar=desktopfpListiface.call("GetUserAppList",name).arguments();
    QStringList  desktopfpList;
    for(int i=0;i<desktopfpvar.count();i++)
    {
        //qDebug()<<desktopfpvar.at(i).toString();
        QString s=desktopfpvar.at(i).toString();//
        if(s==NULL)
        {
            break;
        }
        desktopfpList.append(QString("%1%2.desktop").arg("/usr/share/applications/").arg(s));

        build_app_item(desktopfpList.at(i));

    }

//    foreach (QString filename, desktopFiles) {
//        QString s =QString("%1%2").arg(path).arg(filename);
//        qDebug()<<"s="<<s;
//        build_app_item(QString("%1%2").arg(path).arg(filename));
//    }
}

void AddAutoBoot::build_app_item(QString path){
//    qDebug()<<path;
    QByteArray ba;
    ba = path.toUtf8();
    //解析desktop文件
    GKeyFile * keyfile;
    char *name;
    keyfile = g_key_file_new();
    if (!g_key_file_load_from_file(keyfile, ba.data(), G_KEY_FILE_NONE, NULL)){
        g_key_file_free (keyfile);
        qDebug()<<"-----------shaole";
        return;
    }
    name = g_key_file_get_locale_string(keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, NULL, NULL);
//    if(addedList.contains(name)){
//        return;
//    }

    char * comment, * exec, * icon;
    comment = g_key_file_get_locale_string(keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_COMMENT, NULL, NULL);
    exec = g_key_file_get_string(keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_EXEC, NULL);
    icon = g_key_file_get_locale_string(keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ICON, NULL, NULL);

        //    qDebug()<<name;
        //为每个.desktop文件构建列表项
        QFrame * baseWidget = new QFrame;
        baseWidget->setFixedSize(270, 56);
    //    baseWidget->setFrameShape(QFrame::Shape::Box);
        baseWidget->setAutoFillBackground(false);
    //    baseWidget->setAttribute(Qt::WA_DeleteOnClose);
        QHBoxLayout * appLayout = new QHBoxLayout(baseWidget);
        appLayout->setContentsMargins(16,0,16,0);
        appLayout->setSpacing(16);
        QLabel * iconLabel = new QLabel();
        QLabel * nameLabel = new QLabel();
        QCheckBox * checkbox = new QCheckBox();
        checkbox->setObjectName(QString("%1button").arg(name));
        iconLabel->setFixedSize(16, 16);
        iconLabel->setScaledContents(true);
        //if (!QString(icon).isEmpty() && QIcon::hasThemeIcon(QString(icon))){
        if (!QString(icon).isEmpty()){
            QIcon currenticon = QIcon::fromTheme(QString(icon));
            iconLabel->setPixmap(currenticon.pixmap(QSize(16, 16)));
        } else {
            iconLabel->setPixmap(QPixmap(QString(":/img/plugins/autoboot/desktop.png")));
        }
        nameLabel->setText(name);
        appLayout->addWidget(iconLabel);
        appLayout->addWidget(nameLabel);
        appLayout->addStretch();
        appLayout->addWidget(checkbox);
        emit autoboot_adding_signals(path,name,exec,comment);
        if(addedList.contains(name)){
            checkbox->setChecked(true);
            checkbox->setEnabled(false);
        }
       /* connect(ui->certainBtn, &QPushButton::clicked, this, [=]{
            if(checkbox->isChecked()){
                emit autoboot_adding_signals(path,name,exec,comment);
                if(!addedList.contains(name)){
                    addedList.append(name);
                    qDebug()<<"+++++++++++++++++";
                    foreach(QString s, addedList){
                        qDebug()<<s;
                    }
                }
            }
            checkbox->setEnabled(false);
            close();
    });*/
    baseWidget->setLayout(appLayout);
    ui->appsLayout->addWidget(baseWidget);
}

//void AddAutoBoot::resetBeforeClose(){
//    ui->nameLineEdit->setText(QString());
//    ui->commentLineEdit->setText(QString());
//    ui->execLineEdit->setText(QString());
//    close();
//}

void AddAutoBoot::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect().adjusted(10, 10, -10, -10), 10, 10);

    // 画一个黑底
    QPixmap pixmap(this->rect().size());
    pixmap.fill(Qt::transparent);
    QPainter pixmapPainter(&pixmap);
    pixmapPainter.setRenderHint(QPainter::Antialiasing);
    pixmapPainter.setPen(Qt::transparent);
    pixmapPainter.setBrush(Qt::black);
    pixmapPainter.setOpacity(0.65);
    pixmapPainter.drawPath(rectPath);
    pixmapPainter.end();

    // 模糊这个黑底
    QImage img = pixmap.toImage();
    qt_blurImage(img, 10, false, false);

    // 挖掉中心
    pixmap = QPixmap::fromImage(img);
    QPainter pixmapPainter2(&pixmap);
    pixmapPainter2.setRenderHint(QPainter::Antialiasing);
    pixmapPainter2.setCompositionMode(QPainter::CompositionMode_Clear);
    pixmapPainter2.setPen(Qt::transparent);
    pixmapPainter2.setBrush(Qt::transparent);
    pixmapPainter2.drawPath(rectPath);

    // 绘制阴影
    p.drawPixmap(this->rect(), pixmap, pixmap.rect());

    // 绘制一个背景
    p.save();
    p.fillPath(rectPath,palette().color(QPalette::Base));
    p.restore();


}

AddAutoBoot::~AddAutoBoot()
{
    delete ui;
}

//void AddAutoBoot::open_desktop_dir_slots(){
//    QString filters = "Desktop files(*.desktop)";
//    QFileDialog fd;
//    fd.setDirectory(DESKTOPPATH);
//    fd.setAcceptMode(QFileDialog::AcceptOpen);
//    fd.setViewMode(QFileDialog::List);
//    fd.setNameFilter(filters);
//    fd.setFileMode(QFileDialog::ExistingFile);
//    fd.setWindowTitle(tr("select autoboot desktop"));
//    fd.setLabelText(QFileDialog::Accept, "Select");

//    if (fd.exec() != QDialog::Accepted)
//        return;

//    QString selectedfile;
//    selectedfile = fd.selectedFiles().first();
//    selectFile = selectedfile;
//    qDebug()<<selectedfile;

//    QByteArray ba;
//    ba = selectedfile.toUtf8();

//    //解析desktop文件
//    GKeyFile * keyfile;
//    char *name, * comment, * exec;

//    keyfile = g_key_file_new();
//    if (!g_key_file_load_from_file(keyfile, ba.data(), G_KEY_FILE_NONE, NULL)){
//        g_key_file_free (keyfile);
//        return;
//    }

//    name = g_key_file_get_locale_string(keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, NULL, NULL);
//    comment = g_key_file_get_locale_string(keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_COMMENT, NULL, NULL);
//    exec = g_key_file_get_string(keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_EXEC, NULL);

////    if (ui->nameLineEdit->text().isEmpty())
//        ui->nameLineEdit->setText(QString(name));
////    if (ui->execLineEdit->text().isEmpty())
//        ui->execLineEdit->setText(QString(exec));
////    if (ui->commentLineEdit->text().isEmpty())
//        ui->commentLineEdit->setText(QString(comment));

//    g_key_file_free(keyfile);
//}
