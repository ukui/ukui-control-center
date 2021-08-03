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

#define ITEMFIXEDHEIGH 58

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
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);

//        //~ contents_path /printer/Add Printers And Scanners
//        ui->titleLabel->setText(tr("Add Printers And Scanners"));

        initUi(pluginWidget);

        refreshPrinterDevSlot();
        mTimer = new QTimer(this);
        connect(mTimer, &QTimer::timeout, this, [=] {
            refreshPrinterDevSlot();
        });

        mTimer->start(1000);
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

void Printer::initUi(QWidget *widget)
{
    QVBoxLayout *mverticalLayout = new QVBoxLayout(widget);
    mverticalLayout->setSpacing(0);
    mverticalLayout->setContentsMargins(0, 0, 40, 100);

    PrinterWidget = new QWidget(widget);
    PrinterWidget->setMinimumSize(QSize(550, 0));
    PrinterWidget->setMaximumSize(QSize(16777215, 16777215));

    QVBoxLayout *PrinterLayout = new QVBoxLayout(PrinterWidget);
    PrinterLayout->setContentsMargins(0, 0, 0, 0);
    PrinterLayout->setSpacing(1);

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
    PrinterLayout->addSpacing(7);
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

        if (i == mPrinterList.count() - 1) {
            delete line;
            line = nullptr;
        }
        connect(mPriterBtn, &QPushButton::clicked, this, [=]() {
            runExternalApp();
        });

    }
}

void Printer::initTitleLabel()
{
}

void Printer::initComponent()
{
    mAddWgt = new HoverWidget("", pluginWidget);
    mAddWgt->setObjectName("mAddwgt");
    mAddWgt->setMinimumSize(QSize(580, 60));
    mAddWgt->setMaximumSize(QSize(16777215, 60));
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
    QHBoxLayout *addLyt = new QHBoxLayout;

    QLabel *iconLabel = new QLabel();
    QLabel *textLabel = new QLabel(tr("Add"));
    QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "black", 12);
    iconLabel->setPixmap(pixgray);
    iconLabel->setProperty("useIconHighlightEffect", true);
    iconLabel->setProperty("iconHighlightEffectMode", 1);

    addLyt->addStretch();
    addLyt->addWidget(iconLabel);
    addLyt->addWidget(textLabel);
    addLyt->addStretch();
    mAddWgt->setLayout(addLyt);

    connect(mAddWgt, &HoverWidget::widgetClicked, this, [=]() {
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
}

void Printer::refreshPrinterDevSlot()
{
    QStringList printer = QPrinterInfo::availablePrinterNames();
    bool IsListChange = false;

    for (int num = 0; num < printer.count(); num++) {
        QStringList env = QProcess::systemEnvironment();

        env << "LANG=en_US.UTF-8";

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
