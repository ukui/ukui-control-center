﻿/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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
#include "homepagewidget.h"
#include "ui_homepagewidget.h"

#include <QLabel>
#include <QSignalMapper>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QDBusInterface>
#include <QDebug>
#include <QScrollBar>
#include <QStringList>

#include <QSvgRenderer>
#include "mainwindow.h"
#include "utils/keyvalueconverter.h"
#include "component/clicklabel.h"
#include "utils/functionselect.h"
#include "component/hoverwidget.h"
#include "./utils/utils.h"
#include "../commonComponent/FlowLayout/flowlayout.h"

#define STYLE_FONT_SCHEMA  "org.ukui.style"

const QStringList KexcludeModule{"update","security","application","search_f"};

HomePageWidget::HomePageWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HomePageWidget)
{
    qApp->installEventFilter(this);
    ui->setupUi(this);
    // 获取主窗口
    this->setParent(parent);
    pmainWindow = (MainWindow *)parentWidget();

    // 初始化首页
    initUI();
    ui->scrollArea->viewport()->setAttribute(Qt::WA_TranslucentBackground);
    ui->scrollArea->setStyleSheet("QScrollArea{background-color: transparent;}");
    ui->scrollArea->viewport()->setStyleSheet("background-color: transparent;");
    ui->scrollArea->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    ui->scrollArea->verticalScrollBar()->setVisible(false);
    const QByteArray styleID(STYLE_FONT_SCHEMA);
    QGSettings *stylesettings = new QGSettings(styleID, QByteArray(), this);
    connect(stylesettings,&QGSettings::changed, [=](QString key) {
        if ("systemFont" == key || "systemFontSize" == key) {
            if (ui->scrollArea->layout() != NULL) {
                QLayoutItem *item;
                while ((item = ui->scrollAreaWidgetContents_5->layout()->takeAt(0)) != NULL) {
                    if(item->widget()) {
                       item->widget()->setParent(NULL);
                    }
                    delete item;
                    item = nullptr;
                }
            }
            // 删掉滚动区域布局
            delete ui->scrollAreaWidgetContents_5->layout();
            initUI();
        }
    });

}

HomePageWidget::~HomePageWidget()
{
    delete ui;
    ui = nullptr;
}

void HomePageWidget::initUI() {
    FlowLayout * flowLayout = new FlowLayout(ui->scrollAreaWidgetContents_5, true,0);
    mModuleMap = Utils::getModuleHideStatus();

    //构建枚举键值转换对象
    KeyValueConverter * kvConverter = new KeyValueConverter(); //继承QObject，No Delete

    //初始化功能列表数据
    FunctionSelect::loadHomeModule();

    QSignalMapper * moduleSignalMapper = new QSignalMapper(this);

    for (int moduleIndex = 0; moduleIndex < TOTALMODULES; moduleIndex++) {
        //获取插件QMap
        QMap<QString, QObject *> moduleMap;
        moduleMap = pmainWindow->exportModule(moduleIndex);

        //获取当前模块名
        QString modulenameString = kvConverter->keycodeTokeystring(moduleIndex).toLower();
        QString modulenamei18nString = kvConverter->keycodeTokeyi18nstring(moduleIndex);
        if ((mModuleMap.keys().contains(modulenameString) && !mModuleMap[modulenameString].toBool())
                || (Utils::isTablet() && KexcludeModule.contains(modulenameString))) {
            continue;
        }

        //构建首页10个模块
        //基础Widget
        QWidget * baseWidget = new QWidget();
        baseWidget->setAttribute(Qt::WA_DeleteOnClose);
        baseWidget->setObjectName("itemBaseWidget");
        baseWidget->setStyleSheet("QWidget#itemBaseWidget{background: palette(base);}");
        //baseWidget 的顶级布局
        QVBoxLayout * baseVerLayout = new QVBoxLayout(baseWidget);
        baseVerLayout->setMargin(0);
        baseVerLayout->setSpacing(0);
        //包含显示内容的布局
        QBoxLayout * majorHorLayout = new QHBoxLayout();
        majorHorLayout->setMargin(0);
        majorHorLayout->setSpacing(0);

        //内容Widget的构建
        QPushButton * widget = new QPushButton();
        QString moduleName = modulenameString;
        QString picModuleName = modulenameString;
        widget->setMinimumWidth(320);
        widget->setMinimumHeight(100);
        widget->setAttribute(Qt::WA_DeleteOnClose);
        widget->setProperty("useButtonPalette", true);

        if (picModuleName == "search_f") {
            picModuleName = "search";
        } else if (picModuleName == "datetime") {
            picModuleName = "time-language";
        }
        widget->setObjectName("itemWidget");
        widget->setStyleSheet("QPushButton:!checked{background-color: palette(base)}");

        connect(widget, &QPushButton::clicked, [=]() {
            int moduleIndex = kvConverter->keystringTokeycode(moduleName);

            //获取模块的第一项跳转
            QString firstFunc;
            QList<FuncInfo> tmpList = FunctionSelect::funcinfoListHomePage[moduleIndex];
            for (FuncInfo tmpStruct : tmpList) {
                QString sysVersion = "/etc/apt/ota_version";
                QFile file(sysVersion);
                bool isIntel = file.exists();
                if ((isIntel && tmpStruct.namei18nString == "User Info")
                      || (!isIntel && tmpStruct.namei18nString == "User Info Intel")) {
                    continue;
                }

                if (moduleMap.keys().contains(tmpStruct.namei18nString)) {
                    if (mModuleMap.isEmpty() || mModuleMap[tmpStruct.nameString.toLower()].toBool()) {
                        firstFunc = tmpStruct.namei18nString;
                        //跳转
                        pmainWindow->functionBtnClicked(moduleMap.value(firstFunc));
                        break;
                    }
                }
            }
        });

        QHBoxLayout * mainHorLayout = new QHBoxLayout(widget);
        mainHorLayout->setMargin(16);
        mainHorLayout->setSpacing(16);

        QLabel * logoLabel = new QLabel(widget);
        logoLabel->setFixedSize(48, 48);
        logoLabel->setObjectName("logoLabel");
        logoLabel->setScaledContents(true);

        QString path = (QString(":/img/homepage/kylin-settings-%1.png").arg(picModuleName));
        QPixmap pix;
        pix.load(path);
        logoLabel->setPixmap(pix);

        QVBoxLayout * rightVerLayout = new QVBoxLayout();
        rightVerLayout->setContentsMargins(0, 2, 0, 0);
        rightVerLayout->setSpacing(8);

        QLabel * titleLabel = new QLabel(widget);
        titleLabel->setObjectName("mptitleLabel");
        titleLabel->setText(modulenamei18nString);

        QHBoxLayout * funcHorLayout = new QHBoxLayout();

        uint AllWidth = 0;
        //循环填充模块下属功能
        if (moduleIndex >= FunctionSelect::funcinfoListHomePage.size()) {
            continue;
        }

        QList<FuncInfo> tmpList = FunctionSelect::funcinfoListHomePage[moduleIndex];
        for (int funcIndex = 0; funcIndex < tmpList.size(); funcIndex++){
            FuncInfo single = tmpList.at(funcIndex);
            //跳过插件不存在的功能项
            if (!moduleMap.contains(single.namei18nString)){
                continue;
            }
            //跳过不在首页显示的功能
            if (!single.mainShow)
                continue;

            if (mModuleMap.keys().contains(single.nameString.toLower())) {
                if (!mModuleMap[single.nameString.toLower()].toBool()) {
                    continue;
                }
            }

            QString textName = single.namei18nString;

            ClickLabel * label = new ClickLabel(textName, widget);
            label->setStyleSheet("color: palette(Shadow)");

            AllWidth += label->width();
            if (AllWidth > 200) {
                delete label;
                label = nullptr;
                continue;
            }


            connect(label, SIGNAL(clicked()), moduleSignalMapper, SLOT(map()));
            moduleSignalMapper->setMapping(label, moduleMap[single.namei18nString]);

            funcHorLayout->addWidget(label);
        }

        rightVerLayout->addStretch();
        rightVerLayout->addWidget(titleLabel);
        rightVerLayout->addLayout(funcHorLayout);
        rightVerLayout->addStretch();

        mainHorLayout->addWidget(logoLabel);
        mainHorLayout->addLayout(rightVerLayout);
        mainHorLayout->addStretch();

        widget->setLayout(mainHorLayout);

        majorHorLayout->addWidget(widget);
        majorHorLayout->addStretch();

        baseVerLayout->addLayout(majorHorLayout);
        baseVerLayout->addStretch();

        baseWidget->setLayout(baseVerLayout);

        flowLayout->addWidget(baseWidget);


    }
    connect(moduleSignalMapper, SIGNAL(mapped(QObject*)), pmainWindow, SLOT(functionBtnClicked(QObject*)));
}

const QPixmap HomePageWidget::loadSvg(const QString &fileName, COLOR color)
{
    int size = 48;
    const auto ratio = qApp->devicePixelRatio();
    if ( 2 == ratio) {
        size = 96;
    } else if (3 == ratio) {
        size = 144;
    }
    QPixmap pixmap(size, size);
    QSvgRenderer renderer(fileName);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);
    renderer.render(&painter);
    painter.end();

    pixmap.setDevicePixelRatio(ratio);
    return drawSymbolicColoredPixmap(pixmap, color);
}

QPixmap HomePageWidget::drawSymbolicColoredPixmap(const QPixmap &source, COLOR cgColor)
{
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                if ( WHITE == cgColor) {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                } else if( BLACK == cgColor) {
                    color.setRed(0);
                    color.setGreen(0);
                    color.setBlue(0);
                    img.setPixelColor(x, y, color);
                } else if (GRAY == cgColor) {
                    color.setRed(152);
                    color.setGreen(163);
                    color.setBlue(164);
                    img.setPixelColor(x, y, color);
                } else if(HIGHLIGHT == cgColor){
                    QColor highLightColor = palette().color(QPalette::Highlight);
                    color.setRed(highLightColor.red());
                    color.setGreen(highLightColor.green());
                    color.setBlue(highLightColor.blue());
                    img.setPixelColor(x, y, color);
                } 
                else {
                    return source;
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}


bool HomePageWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseMove) {
        if (ui->scrollArea->geometry().contains(this->mapFromGlobal(QCursor::pos()))) {
            ui->scrollArea->verticalScrollBar()->setVisible(true);
        } else {
            ui->scrollArea->verticalScrollBar()->setVisible(false);
        }
    }
     return QObject::eventFilter(watched, event);
}

