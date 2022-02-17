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
#include <cups/cups.h>
#include "usbthread.h"

#include <QtPrintSupport/QPrinterInfo>
#include <QProcess>
#include <QThread>

#include <QDebug>
#include <QMouseEvent>

#define ITEMFIXEDHEIGH 58

Printer::Printer() : mFirstLoad(true)
{
    pluginName = tr("Printer");
    pluginType = DEVICES;
}

Printer::~Printer()
{
    if (!mFirstLoad) {
        delete ui;
        ui = nullptr;
    }
}

QString Printer::get_plugin_name()
{
    return pluginName;
}

int Printer::get_plugin_type()
{
    return pluginType;
}

QWidget *Printer::get_plugin_ui()
{
    if (mFirstLoad) {
        mFirstLoad = false;
        ui = new Ui::Printer;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);

        //~ contents_path /printer/Add Printers And Scanners
        ui->titleLabel->setText(tr("Add Printers And Scanners"));

        // 禁用选中效果
        ui->listWidget->setFocusPolicy(Qt::NoFocus);
        ui->listWidget->setSelectionMode(QAbstractItemView::NoSelection);

        initTitleLabel();
        initComponent();

        refreshPrinterDevSlot();

        // 开辟线程监听usb插拔
        QThread *mThread = new QThread;
        UsbThread *UsbWorker = new UsbThread;
        UsbWorker->moveToThread(mThread);
        connect(mThread, &QThread::started, UsbWorker, &UsbThread::run);
        connect(UsbWorker,&UsbThread::keychangedsignal,this,&Printer::refreshPrinterDevSlot);
        connect(mThread, &QThread::finished, UsbWorker, &UsbThread::deleteLater);
         mThread->start();
    }
    return pluginWidget;
}

void Printer::plugin_delay_control()
{
}

const QString Printer::name() const
{
    return QStringLiteral("printer");
}

void Printer::initTitleLabel()
{
    ui->listWidget->setSpacing(1);
}

void Printer::initComponent()
{
    mAddWgt = new HoverWidget("", pluginWidget);
    mAddWgt->setObjectName("mAddwgt");
    mAddWgt->setMinimumSize(QSize(580, 50));
    mAddWgt->setMaximumSize(QSize(960, 50));
    QPalette pal;
    QBrush brush = pal.highlight();  //获取window的色值
    QColor highLightColor = brush.color();
    QString stringColor = QString("rgba(%1,%2,%3)") //叠加20%白色
           .arg(highLightColor.red()*0.8 + 255*0.2)
           .arg(highLightColor.green()*0.8 + 255*0.2)
           .arg(highLightColor.blue()*0.8 + 255*0.2);

    mAddWgt->setStyleSheet(QString("HoverWidget#mAddwgt{background: palette(button);\
                                   border-radius: 4px;}\
                                   HoverWidget:hover:!pressed#mAddwgt{background: %1;\
                                   border-radius: 4px;}").arg(stringColor));

    ui->listWidget->setStyleSheet("QListWidget::Item:hover{background:palette(base);}");

    QHBoxLayout *addLyt = new QHBoxLayout;

    QLabel *iconLabel = new QLabel();
    QLabel *textLabel = new QLabel(tr("Add printers and scanners"));
    QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "black", 12);
    iconLabel->setPixmap(pixgray);
    iconLabel->setProperty("useIconHighlightEffect", true);
    iconLabel->setProperty("iconHighlightEffectMode", 1);

    addLyt->addWidget(iconLabel);
    addLyt->addWidget(textLabel);
    addLyt->addStretch();
    mAddWgt->setLayout(addLyt);

    connect(mAddWgt, &HoverWidget::widgetClicked, this, [=](QString mname) {
        Q_UNUSED(mname)
        runExternalApp();
    });

    // 悬浮改变Widget状态
    connect(mAddWgt, &HoverWidget::enterWidget, this, [=](){

        iconLabel->setProperty("useIconHighlightEffect", false);
        iconLabel->setProperty("iconHighlightEffectMode", 0);
        QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "white", 12);
        iconLabel->setPixmap(pixgray);
        textLabel->setStyleSheet("color: white;");
    });

    // 还原状态
    connect(mAddWgt, &HoverWidget::leaveWidget, this, [=](){

        iconLabel->setProperty("useIconHighlightEffect", true);
        iconLabel->setProperty("iconHighlightEffectMode", 1);
        QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "black", 12);
        iconLabel->setPixmap(pixgray);
        textLabel->setStyleSheet("color: palette(windowText);");
    });
    ui->addLyt->addWidget(mAddWgt);
}

void Printer::refreshPrinterDevSlot()
{
    cups_dest_t *dests;
    int num_dests = cupsGetDests(&dests);
    cups_dest_t *dest;
    int i;
    for (i = num_dests, dest = dests; i > 0; i --, dest ++) {
        // 获取打印机状态，3为空闲，4为忙碌，5为不可用
        const char*  value = cupsGetOption("printer-state", dest->num_options, dest->options);
//        qDebug()<<dest->name<<"----------------"<<value;
        if (value == nullptr)
            continue;
         // 标志位flag用来判断该打印机是否可用，flag1用来决定是否新增窗口(为真则加)
         bool flag = (atoi(value) == 5 ? true : false);
         bool flag1 = true;

         // 遍历窗口列表，判断列表中是否已经存在该打印机，若存在，便判断该打印机是否可用，不可用则从列表中删除该打印机窗口
         for (int j = 0; j < ui->listWidget->count(); j++) {
             QString itemData = ui->listWidget->item(j)->data(Qt::UserRole).toString();
             if (!itemData.compare(QString(dest->name))) {
                 if (flag) {
                     ui->listWidget->takeItem(j);
                     flag1 = false;
                     break;
                 }
                 flag1 = false;
                 break;
             }
         }

         //
         if (!flag && flag1) {
             HoverBtn *printerItem = new HoverBtn(QString(dest->name), pluginWidget);
             printerItem->installEventFilter(this);
             connect(printerItem,&HoverBtn::resize,[=](){
                 setLabelText(printerItem->mPitLabel,QString(dest->name));
             });

             QIcon printerIcon = QIcon::fromTheme("printer");
             printerItem->mPitIcon->setPixmap(printerIcon.pixmap(printerIcon.actualSize(QSize(24, 24))));
             QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
             item->setData(Qt::UserRole, QString(dest->name));

             item->setSizeHint(QSize(QSizePolicy::Expanding, 50));
             ui->listWidget->setItemWidget(item, printerItem);
         }

   }
}

void Printer::runExternalApp()
{
    QString cmd = "system-config-printer";

    QProcess process(this);
    process.startDetached(cmd);
}

void Printer::setLabelText(QLabel *label, QString text)
{
    QFontMetrics  fontMetrics(label->font());
    int fontSize = fontMetrics.width(text);
    if (fontSize > label->width()) {
        label->setText(fontMetrics.elidedText(text, Qt::ElideRight, label->width()));
        label->setToolTip(text);
    } else {
        label->setText(text);
        label->setToolTip("");
    }
}
bool Printer::eventFilter(QObject *obj, QEvent *event)
{
    QString strObjName(obj->metaObject()->className());
    if (strObjName == "HoverBtn") {
        if (event->type() == QEvent::Resize) {
            HoverBtn *mBtn = static_cast<HoverBtn *>(obj);
            if (mBtn) {
                mBtn->mPitLabel->setFixedWidth(mBtn->width() - 50);
                emit mBtn->resize();
            }
        }
        return false;
    }
}
