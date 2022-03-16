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
#include <cups/cups.h>

#include <QDebug>
#include <QMouseEvent>
#include <QThread>
#include "usbthread.h"

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

        initUi(pluginWidget);

        refreshPrinterDevSlot();

        // 开辟线程监听usb插拔
        QThread *mThread = new QThread;
        UsbThread *UsbWorker = new UsbThread;
        UsbWorker->moveToThread(mThread);
        connect(mThread, &QThread::started, UsbWorker, &UsbThread::run);
        connect(UsbWorker, &UsbThread::addsignal, this, [=](){
            sleep(1);
            refreshPrinterDevSlot();
        });
        connect(UsbWorker, &UsbThread::removesignal, this, [=](){
            sleep(1);
            refreshPrinterDevSlot();
        });
        connect(mThread, &QThread::finished, UsbWorker, &UsbThread::deleteLater);
         mThread->start();
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
    cups_dest_t *dests;
    int num_dests = cupsGetDests(&dests);
    cups_dest_t *dest;
    int i;
    bool isListChange = false;
    for (i = num_dests, dest = dests; i > 0; i --, dest ++) {
        // 获取打印机状态，3为空闲，4为忙碌，5为不可用
        const char*  value = cupsGetOption("printer-state", dest->num_options, dest->options);
        qDebug()<<dest->name<<"----------------"<<value;
        if (value == nullptr)
            continue;
         // 标志位flag用来判断该打印机是否可用
         bool flag = (atoi(value) == 5 ? true : false);

        if (flag) {
            if (mPrinterList.contains(QString(dest->name))) {
                mPrinterList.removeOne(QString(dest->name));
                isListChange = true;
            }
        } else {
            if (!mPrinterList.contains(QString(dest->name))) {
                mPrinterList.append(QString(dest->name));
                isListChange = true;
            }
        }
    }
    //打印机列表内容有变化，则清空再构建一遍
    if (isListChange) {
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
