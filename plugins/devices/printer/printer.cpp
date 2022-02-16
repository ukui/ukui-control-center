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

#include <QtPrintSupport/QPrinterInfo>
#include <QProcess>

#include <QDebug>
#include <QMouseEvent>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <errno.h>
#include <unistd.h>
#include <regex>
#include <iostream>

#define ITEMFIXEDHEIGH 58
#define UEVENT_BUFFER_SIZE 2048

Printer::Printer() : mFirstLoad(true)
{
    pluginName = tr("Printer");
    pluginType = DEVICES;
}

Printer::~Printer()
{
    if (!mFirstLoad) {
    }
}

QString Printer::plugini18nName()
{
    return pluginName;
}

int Printer::pluginTypes()
{
    return pluginType;
}

QWidget *Printer::pluginUi()
{
    if (mFirstLoad) {
        mFirstLoad = false;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);

//        //~ contents_path /Printer/Add Printers And Scanners
//        ui->titleLabel->setText(tr("Add Printers And Scanners"));

        initUi(pluginWidget);

        refreshPrinterDevSlot();
         hotplug_sock = init_hotplug_sock();
        QtConcurrent::run([=](){
            while(1)
            {
                char buf[UEVENT_BUFFER_SIZE*2] = {0};
                // recv 会等待usb信号连入
                recv(hotplug_sock, &buf, sizeof(buf), 0);
                usbDeviceIdentify(QString(buf));
            }
        });
    }
    return pluginWidget;
}

const QString Printer::name() const
{
    return QStringLiteral("Printer");
}

bool Printer::isShowOnHomePage() const
{
    return false;
}

QIcon Printer::icon() const
{
    return QIcon();
}

bool Printer::isEnable() const
{
    return true;
}

void Printer::initUi(QWidget *widget)
{
    QVBoxLayout *mverticalLayout = new QVBoxLayout(widget);
    mverticalLayout->setSpacing(0);
    mverticalLayout->setContentsMargins(0, 0, 0, 0);

    PrinterWidget = new QWidget(widget);
    PrinterWidget->setMinimumSize(QSize(550, 0));
    PrinterWidget->setMaximumSize(QSize(16777215, 16777215));

    QVBoxLayout *PrinterLayout = new QVBoxLayout(PrinterWidget);
    PrinterLayout->setContentsMargins(0, 0, 0, 0);
    PrinterLayout->setSpacing(0);

    mPrinterLabel = new TitleLabel(PrinterWidget);
    mPrinterLabel->setText(tr("Printers And Scanners"));

    mPrinterListFrame = new QFrame(PrinterWidget);
    mPrinterListFrame->setMinimumSize(QSize(550, 0));
    mPrinterListFrame->setMaximumSize(QSize(16777215, 16777215));
    mPrinterListFrame->setFrameShape(QFrame::Box);

    mPrinterListLayout = new QVBoxLayout(mPrinterListFrame);
    mPrinterListLayout->setContentsMargins(0, 0, 0, 0);
    mPrinterListLayout->setSpacing(0);

    initComponent();

    PrinterLayout->addWidget(mPrinterLabel);
    PrinterLayout->addSpacing(8);
    PrinterLayout->addWidget(mPrinterListFrame);
    PrinterLayout->addWidget(mAddWgt);

    mverticalLayout->addWidget(PrinterWidget);
    mverticalLayout->addStretch();

}

void Printer::initPrinterUi()
{
    clearAutoItem();
    for (int i = 0; i < mPrinterList.count(); i++) {
        QPushButton *mPriterBtn = new QPushButton(mPrinterListFrame);
        mPriterBtn->setProperty("useButtonPalette", true);
        mPriterBtn->setMinimumSize(QSize(580, 60));
        mPriterBtn->setMaximumSize(QSize(16777215, 60));
        mPriterBtn->setStyleSheet("QPushButton:!checked{background-color: palette(base)}");
        QHBoxLayout *mPrinterLyt = new QHBoxLayout(mPriterBtn);
        mPrinterLyt->setSpacing(16);

        QLabel *iconLabel = new QLabel(mPriterBtn);
        QIcon printerIcon = QIcon::fromTheme("printer");
        iconLabel->setPixmap(printerIcon.pixmap(printerIcon.actualSize(QSize(24, 24))));
        FixLabel *textLabel = new FixLabel(mPriterBtn);
        textLabel->setText(mPrinterList.at(i));

        mPrinterLyt->addWidget(iconLabel);
        mPrinterLyt->addWidget(textLabel,Qt::AlignLeft);

        QFrame *line = new QFrame(mPrinterListFrame);
        line->setMinimumSize(QSize(0, 1));
        line->setMaximumSize(QSize(16777215, 1));
        line->setLineWidth(0);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        mPrinterListLayout->addWidget(mPriterBtn);
        mPrinterListLayout->addWidget(line);

        connect(mPriterBtn, &QPushButton::clicked, this, [=]() {
            runExternalApp();
        });

    }
}

void Printer::initTitleLabel()
{
    //~ contents_path /Printer/Add
    tr("Add");       // 用于添加搜索索引
}

void Printer::initComponent()
{
    mAddWgt = new AddBtn(pluginWidget);

    connect(mAddWgt, &AddBtn::clicked, this, [=]() {
        runExternalApp();
    });

}

void Printer::refreshPrinterDevSlot()
{
    qDebug()<<"---------------------------";
    QStringList printer = QPrinterInfo::availablePrinterNames();
    bool IsListChange = false;

    for (int num = 0; num < printer.count(); num++) {
        QStringList env = QProcess::systemEnvironment();

        env << "LANG=en_US";

        QProcess *process = new QProcess;
        process->setEnvironment(env);
        process->start("lpstat -p "+printer.at(num));
        process->waitForFinished();

        QString ba = process->readAllStandardOutput();
        delete process;
        QString printer_stat = QString(ba.data());

        bool flag = printer_stat.contains("disable", Qt::CaseSensitive)
                    || printer_stat.contains("Unplugged or turned off", Qt::CaseSensitive);

        if (flag) {
            if (mPrinterList.contains(printer.at(num))) {
                mPrinterList.removeOne(printer.at(num));
                IsListChange = true;
            }
        } else {
            if (!mPrinterList.contains(printer.at(num))) {
                mPrinterList.append(printer.at(num));
                IsListChange = true;
            }
        }
    }
    //打印机列表内容有变化，则清空再构建一遍
    if (IsListChange) {
        initPrinterUi();
    }

    if (mPrinterList.count() == 0) {
        mPrinterListFrame->setVisible(false);
    } else {
        mPrinterListFrame->setVisible(true);
    }
}

void Printer::runExternalApp()
{
    QString cmd = "system-config-printer";

    QProcess process(this);
    process.startDetached(cmd);
}

void Printer::clearAutoItem()
{
    if (mPrinterListLayout->layout() != NULL) {
        QLayoutItem *item;
        while ((item = mPrinterListLayout->layout()->takeAt(0)) != NULL)
        {
            delete item->widget();
            delete item;
            item = nullptr;
        }
    }
}

int Printer::init_hotplug_sock()
{
    struct sockaddr_nl snl;
    const int buffersize = 16 * 1024 * 1024;
    int retval;
    memset(&snl, 0x00, sizeof(struct sockaddr_nl));
    snl.nl_family = AF_NETLINK;
    snl.nl_pid = getpid();
    snl.nl_groups = 1;
    int hotplug_sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (hotplug_sock == -1)
    {
        printf("error getting socket: %s", strerror(errno));
        return -1;
    }
    /* set receive buffersize */
    setsockopt(hotplug_sock, SOL_SOCKET, SO_RCVBUFFORCE, &buffersize, sizeof(buffersize));
    retval = bind(hotplug_sock, (struct sockaddr *) &snl, sizeof(struct sockaddr_nl));
    if (retval < 0) {
        printf("bind failed: %s", strerror(errno));
        close(hotplug_sock);
        hotplug_sock = -1;
        return -1;
    }
    return hotplug_sock;
}

void Printer::usbDeviceIdentify(const QString &str){
    if (  str.indexOf("bind") == 0
          &&  str.contains("pci")
          && !str.right( str.size() -1 - str.lastIndexOf('/') ).contains(":")
          && !str.right( str.size() -1 - str.lastIndexOf('/') ).contains(".")
        ) {
            refreshPrinterDevSlot();
        }
        // unbind@/devices/pci0000:00/0000:00:11.0/0000:02:01.0/usb1/1-1
        if (  str.contains("unbind")
          &&  str.contains("pci")
          && !str.right( str.size() -1 - str.lastIndexOf('/') ).contains(":")
          && !str.right( str.size() -1 - str.lastIndexOf('/') ).contains(".")
        ) {
            refreshPrinterDevSlot();
        }
        return;
}
