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
#include "printer.h"
#include "ui_printer.h"

#include <QtPrintSupport/QPrinterInfo>
#include <QProcess>

#include <QDebug>
#include <QMouseEvent>
#define THEME_QT_SCHEMA                  "org.ukui.style"
#define MODE_QT_KEY                      "style-name"
#define ITEMFIXEDHEIGH 58

Printer::Printer() : mFirstLoad(true)
{
    pluginName = tr("Printers");
    pluginType = DEVICES;

}

Printer::~Printer()
{
    delete ui;
}

QString Printer::get_plugin_name(){
    return pluginName;
}

int Printer::get_plugin_type(){
    return pluginType;
}

QWidget *Printer::get_plugin_ui(){
    if (mFirstLoad) {
        mFirstLoad = false;
        ui = new Ui::Printer;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_StyledBackground,true);
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);
        const QByteArray idd(THEME_QT_SCHEMA);
        if  (QGSettings::isSchemaInstalled(idd)){
            qtSettings = new QGSettings(idd);
        }

        ui->title2Label->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");


        ui->listWidget->setSpacing(0);

        refreshPrinterDev();

        pTimer = new QTimer(this);
        pTimer->setInterval(1000);
        connect(pTimer, SIGNAL(timeout()), this, SLOT(refreshPrinterDev()));

        initComponent();
    }
    return pluginWidget;
}

void Printer::plugin_delay_control(){

}

const QString Printer::name() const {

    return QStringLiteral("printer");
}

void Printer::initComponent(){

//    ui->addBtn->setIcon(QIcon("://img/plugins/printer/add.png"));
//    ui->addBtn->setIconSize(QSize(48, 48));
//    ui->addBtn->setStyleSheet("QPushButton{background-color:transparent;}");

    //~ contents_path /printer/List Of Existing Printers
    ui->title2Label->setText(tr("List Of Existing Printers"));

    addWgt = new HoverWidget("");
    addWgt->setObjectName("addwgt");
    addWgt->setMinimumSize(QSize(580, 64));
    addWgt->setMaximumSize(QSize(16777215, 64));
    addWgt->setStyleSheet("HoverWidget#addwgt{background: palette(base); border-radius: 4px;}HoverWidget:hover:!pressed#addwgt{background: #2FB3E8; border-radius: 4px;}");

    QHBoxLayout *addLyt = new QHBoxLayout;

    QLabel * iconLabel = new QLabel();

    QLabel * textLabel = new QLabel(tr("Add printers and scanners"));
    QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "black", 12);
    iconLabel->setPixmap(pixgray);
    addLyt->addWidget(iconLabel);
    addLyt->addWidget(textLabel);
    addLyt->addStretch();
    addWgt->setLayout(addLyt);

    //图标跟随主题变化
    QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
    if ("ukui-white" == currentThemeMode || "ukui-default" == currentThemeMode || "ukui-light" == currentThemeMode || "ukui-white-unity" == currentThemeMode) {
        QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "black", 12);
        iconLabel->setPixmap(pixgray);
    } else if ("ukui-dark" == currentThemeMode || "ukui-black" == currentThemeMode || "ukui-black-unity" == currentThemeMode){
        QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "white", 12);
        iconLabel->setPixmap(pixgray);
    }
    connect(qtSettings,&QGSettings::changed,this,[=](const QString &key){
        if (key == "styleName") {
            QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
            if ("ukui-white" == currentThemeMode || "ukui-default" == currentThemeMode || "ukui-light" == currentThemeMode || "ukui-white-unity" == currentThemeMode) {
                QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "black", 12);
                iconLabel->setPixmap(pixgray);
            } else if ("ukui-dark" == currentThemeMode || "ukui-black" == currentThemeMode || "ukui-black-unity" == currentThemeMode){
                QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "white", 12);
                iconLabel->setPixmap(pixgray);
            }
        }
    });
    // 还原状态
    connect(addWgt, &HoverWidget::leaveWidget, this, [=](){
        QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
        if ("ukui-white" == currentThemeMode || "ukui-default" == currentThemeMode || "ukui-light" == currentThemeMode || "ukui-white-unity" == currentThemeMode) {
            QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "black", 12);
            iconLabel->setPixmap(pixgray);
        } else if ("ukui-dark" == currentThemeMode || "ukui-black" == currentThemeMode || "ukui-black-unity" == currentThemeMode){
            QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "white", 12);
            iconLabel->setPixmap(pixgray);
        }
        textLabel->setStyleSheet("color: palette(windowText);");
    });
    // 悬浮改变Widget状态
    connect(addWgt, &HoverWidget::enterWidget, this, [=](){
        QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
        if ("ukui-white" == currentThemeMode || "ukui-default" == currentThemeMode || "ukui-light" == currentThemeMode || "ukui-white-unity" == currentThemeMode) {
            QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "white", 12);
            iconLabel->setPixmap(pixgray);
        } else if ("ukui-dark" == currentThemeMode || "ukui-black" == currentThemeMode || "ukui-black-unity" == currentThemeMode){
            QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "black", 12);
            iconLabel->setPixmap(pixgray);
        }
        textLabel->setStyleSheet("color: palette(base);");

    });

    connect(addWgt, &HoverWidget::widgetClicked, this, [=](QString mname){
        runExternalApp();
    });
    ui->addLyt->addWidget(addWgt);

    pTimer->start();

//    connect(ui->addBtn, &QPushButton::clicked, this, [=]{
//        runExternalApp();
//    });
}

void Printer::refreshPrinterDev(){

    ui->listWidget->clear();

    QStringList printer = QPrinterInfo::availablePrinterNames();

    for (int num = 0; num < printer.count(); num++){

        QWidget * baseWidget = new QWidget;
        baseWidget->setAttribute(Qt::WA_DeleteOnClose);

        QVBoxLayout * baseVerLayout = new QVBoxLayout(baseWidget);
        baseVerLayout->setSpacing(0);
        baseVerLayout->setContentsMargins(0, 0, 0, 8);

        QWidget * pdWidget = new QWidget(baseWidget);

        QHBoxLayout * pdHorLayout = new QHBoxLayout(pdWidget);
        pdHorLayout->setSpacing(8);
        pdHorLayout->setContentsMargins(16, 16, 0, 0);

        QLabel * pdIconLabel = new QLabel(pdWidget);
        QSizePolicy iconSizePolicy = pdIconLabel->sizePolicy();
        iconSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
        iconSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
        pdIconLabel->setSizePolicy(iconSizePolicy);
        pdIconLabel->setFixedSize(QSize(24, 24));
        pdIconLabel->setScaledContents(true);
        pdIconLabel->setPixmap(QPixmap("://img/plugins/printer/printer.png"));

        QLabel * pdLabel = new QLabel(pdWidget);
        QSizePolicy txtSizePolicy = pdLabel->sizePolicy();
        txtSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
        txtSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
        pdLabel->setSizePolicy(txtSizePolicy);
        pdLabel->setScaledContents(true);
        pdLabel->setText(printer.at(num));

        pdHorLayout->addWidget(pdIconLabel);
        pdHorLayout->addWidget(pdLabel);
        pdHorLayout->addStretch();

        pdWidget->setLayout(pdHorLayout);

        //
        baseVerLayout->addWidget(pdWidget);
        baseVerLayout->addStretch();

        baseWidget->setLayout(baseVerLayout);


        QListWidgetItem * item = new QListWidgetItem(ui->listWidget);
        item->setSizeHint(QSize(ui->listWidget->width() - 4, ITEMFIXEDHEIGH));
        ui->listWidget->setItemWidget(item, baseWidget);
    }
    ui->listWidget->setFixedHeight(ITEMFIXEDHEIGH*printer.count());
    //ui->listWidget->resize(ui->listWidget->width(),ITEMFIXEDHEIGH*printer.count());
    //qDebug()<<"ITEMFIXEDHEIGH*printer.count()"<<ITEMFIXEDHEIGH*printer.count();
}

void Printer::runExternalApp(){
    QString cmd = "system-config-printer";

    QProcess process(this);
    process.startDetached(cmd);
}

//bool Printer::eventFilter(QObject *watched, QEvent *event)
//{
//    if (watched == ui->addFrame){
//        if (event->type() == QEvent::MouseButtonPress){
//            QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
//            if (mouseEvent->button() == Qt::LeftButton){
//                runExternalApp();
//                return true;
//            } else
//                return false;
//        }
//    }
//    return QObject::eventFilter(watched, event);
//}
