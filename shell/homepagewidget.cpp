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
#include "homepagewidget.h"
#include "ui_homepagewidget.h"

#include <QLabel>
#include <QSignalMapper>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QDebug>

#include "mainwindow.h"
#include "utils/keyvalueconverter.h"
#include "component/clicklabel.h"
#include "utils/functionselect.h"
#include "component/hoverwidget.h"

HomePageWidget::HomePageWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HomePageWidget)
{
    ui->setupUi(this);
    //获取主窗口
    this->setParent(parent);
    pmainWindow = (MainWindow *)parentWidget();

    //
    ui->listWidget->setStyleSheet("QListWidget#listWidget{border: none;}");



    //初始化首页
    initUI();
}

HomePageWidget::~HomePageWidget()
{
    delete ui;
}

void HomePageWidget::initUI(){
    ui->listWidget->setResizeMode(QListView::Adjust);
    ui->listWidget->setViewMode(QListView::IconMode);
    ui->listWidget->setMovement(QListView::Static);
    ui->listWidget->setSpacing(0);

    ui->listWidget->setFocusPolicy(Qt::NoFocus);
    ui->listWidget->setSelectionMode(QAbstractItemView::NoSelection);

//    ui->listWidget->setGridSize(QSize(360, 100));

    //构建枚举键值转换对象
    KeyValueConverter * kvConverter = new KeyValueConverter(); //继承QObject，No Delete

    //初始化功能列表数据
    FunctionSelect::initValue();

    QSignalMapper * moduleSignalMapper = new QSignalMapper(this);

    for (int moduleIndex = 0; moduleIndex < TOTALMODULES; moduleIndex++){
        //获取插件QMap
        QMap<QString, QObject *> moduleMap;
        moduleMap = pmainWindow->exportModule(moduleIndex);

        //获取当前模块名
        QString modulenameString = kvConverter->keycodeTokeystring(moduleIndex).toLower();
        QString modulenamei18nString = kvConverter->keycodeTokeyi18nstring(moduleIndex);

        ////构建首页8个模块
        //基础Widget
        QWidget * baseWidget = new QWidget;
        baseWidget->setAttribute(Qt::WA_DeleteOnClose);
        baseWidget->setObjectName("itemBaseWidget");
//        baseWidget->setStyleSheet("QWidget#itemBaseWidget{border: 1px solid #3D68E5;}");
        //baseWidget 的顶级布局
        QVBoxLayout * baseVerLayout = new QVBoxLayout(baseWidget);
        baseVerLayout->setMargin(0);
        baseVerLayout->setSpacing(0);
        //包含显示内容的布局
        QBoxLayout * majorHorLayout = new QHBoxLayout();
        majorHorLayout->setMargin(0);
        majorHorLayout->setSpacing(0);

        //内容Widget的构建
        HoverWidget * widget = new HoverWidget(modulenameString);
//        QWidget * widget = new QWidget;
        widget->setFixedSize(QSize(320, 80));
        widget->setAttribute(Qt::WA_DeleteOnClose);

        widget->setObjectName("itemWidget");
        widget->setStyleSheet("HoverWidget:hover:!pressed#itemWidget{background: #3D6BE5; border-radius: 4px;}");

        QHBoxLayout * mainHorLayout = new QHBoxLayout(widget);
        mainHorLayout->setMargin(16);
        mainHorLayout->setSpacing(16);

        QLabel * logoLabel = new QLabel(widget);
        logoLabel->setObjectName("logoLabel");
        logoLabel->setScaledContents(true);
        logoLabel->setPixmap(QPixmap(QString(":/img/homepage/%1.png").arg(modulenameString)));

        QVBoxLayout * rightVerLayout = new QVBoxLayout();
        rightVerLayout->setContentsMargins(0, 2, 0, 0);
        rightVerLayout->setSpacing(8);

        QLabel * titleLabel = new QLabel(widget);
        titleLabel->setObjectName("mptitleLabel");
        titleLabel->setText(modulenamei18nString);
        titleLabel->setStyleSheet("font-size: 16px; color: #DE000000;");
        QHBoxLayout * funcHorLayout = new QHBoxLayout();

        //循环填充模块下属功能
        QList<FuncInfo> tmpList = FunctionSelect::funcinfoList[moduleIndex];
        for (int funcIndex = 0; funcIndex < tmpList.size(); funcIndex++){
            FuncInfo single = tmpList.at(funcIndex);
            //跳过插件不存在的功能项
            if (!moduleMap.contains(single.namei18nString)){
//                qDebug() << single.namei18nString << "plugin object not found";
                continue;
            }
            //跳过不在首页显示的功能
            if (!single.mainShow)
                continue;

            ClickLabel * label = new ClickLabel(single.namei18nString, widget);
            label->setStyleSheet("font-size: 14px; color: #91434345;");

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

        //悬浮改变Widget状态
        connect(widget, &HoverWidget::enterWidget, this, [=](QString mname){
            HoverWidget * w = dynamic_cast<HoverWidget *>(QObject::sender());
            logoLabel->setPixmap(QPixmap(QString(":/img/homepage/%1Hover.png").arg(mname)));

            titleLabel->setStyleSheet("font-size: 16px; color: #F7FFFFFF;");

            QList<ClickLabel *> clabelList = w->findChildren<ClickLabel *>();
            for (ClickLabel * tmpLabel : clabelList){
                tmpLabel->setStyleSheet("font-size: 14px; color: #91FFFFFF;");
            }
        });
        //还原状态
        connect(widget, &HoverWidget::leaveWidget, this, [=](QString mname){
            HoverWidget * w = dynamic_cast<HoverWidget *>(QObject::sender());
            logoLabel->setPixmap(QPixmap(QString(":/img/homepage/%1.png").arg(mname)));

            titleLabel->setStyleSheet("font-size: 16px; color: #DE000000;");

            QList<ClickLabel *> clabelList = w->findChildren<ClickLabel *>();
            for (ClickLabel * tmpLabel : clabelList){
                tmpLabel->setStyleSheet("font-size: 14px; color: #91434345;");
            }
        });


        QListWidgetItem * item = new QListWidgetItem(ui->listWidget);
        item->setSizeHint(QSize(360, 100));
        ui->listWidget->addItem(item);
        ui->listWidget->setItemWidget(item, baseWidget);
    }
    connect(moduleSignalMapper, SIGNAL(mapped(QObject*)), pmainWindow, SLOT(functionBtnClicked(QObject*)));

}
