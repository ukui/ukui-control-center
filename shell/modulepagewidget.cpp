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
#include "modulepagewidget.h"
#include "ui_modulepagewidget.h"

#include <QListWidgetItem>

#include "mainwindow.h"
#include "interface.h"
#include "utils/keyvalueconverter.h"
#include "utils/functionselect.h"
#include "component/leftwidgetitem.h"

#include <QDebug>

ModulePageWidget::ModulePageWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ModulePageWidget)
{
    ui->setupUi(this);

    //设置父窗口对象
    this->setParent(parent);
    pmainWindow = (MainWindow *)parentWidget();

    //左侧Widget大小限定
    ui->leftbarWidget->setMinimumWidth(120);
    ui->leftbarWidget->setMaximumWidth(224);

    //
    ui->mtitleLabel->setStyleSheet("QLabel#mtitleLabel{font-size: 18px; color: #91000000;}");
    //左侧二级菜单样式
    ui->leftStackedWidget->setStyleSheet("border: none;");
    //上侧二级菜单样式
    ui->topStackedWidget->setStyleSheet("border: none;");
    //功能区域
    ui->scrollArea->setStyleSheet("#scrollArea{border: 0px solid;}");
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);


    //构建枚举键值转换对象
    mkvConverter = new KeyValueConverter(); //继承QObject，No Delete

    ui->topsideWidget->hide();

    initUI();

}

ModulePageWidget::~ModulePageWidget()
{
    delete ui;
}

void ModulePageWidget::initUI(){
    //设置伸缩策略
    QSizePolicy leftSizePolicy = ui->leftbarWidget->sizePolicy();
    QSizePolicy rightSizePolicy = ui->widget->sizePolicy();

    leftSizePolicy.setHorizontalStretch(1);
    rightSizePolicy.setHorizontalStretch(5);

    ui->leftbarWidget->setSizePolicy(leftSizePolicy);
    ui->widget->setSizePolicy(rightSizePolicy);

    //绑定高亮函数
    connect(this, &ModulePageWidget::widgetChanged, this, [=](QString text){highlightItem(text);});

    for (int moduleIndex = 0; moduleIndex < TOTALMODULES; moduleIndex++){
        QListWidget * leftListWidget = new QListWidget;
        leftListWidget->setAttribute(Qt::WA_DeleteOnClose);
        leftListWidget->setResizeMode(QListView::Adjust);
        leftListWidget->setFocusPolicy(Qt::NoFocus);
        leftListWidget->setSelectionMode(QAbstractItemView::NoSelection);
        leftListWidget->setSpacing(0);
        connect(leftListWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(currentLeftitemChanged(QListWidgetItem*,QListWidgetItem*)));
        QListWidget * topListWidget = new QListWidget;
        topListWidget->setAttribute(Qt::WA_DeleteOnClose);
        topListWidget->setResizeMode(QListView::Adjust);
        topListWidget->setViewMode(QListView::IconMode);
        topListWidget->setMovement(QListView::Static);
        topListWidget->setSpacing(0);

        QMap<QString, QObject *> moduleMap;
        moduleMap = pmainWindow->exportModule(moduleIndex);

        QList<FuncInfo> functionStructList = FunctionSelect::funcinfoList[moduleIndex];
        for (int funcIndex = 0; funcIndex < functionStructList.size(); funcIndex++){
            FuncInfo single = functionStructList.at(funcIndex);
            //跳过插件不存在的功能项
            if (!moduleMap.contains(single.namei18nString))
                continue;

            //填充左侧二级菜单
            LeftWidgetItem * leftWidgetItem = new LeftWidgetItem();
            leftWidgetItem->setAttribute(Qt::WA_DeleteOnClose);
            leftWidgetItem->setLabelText(single.namei18nString);
            leftWidgetItem->setLabelPixmap(QString("://img/secondaryleftmenu/%1.png").arg(single.nameString));

            QListWidgetItem * item = new QListWidgetItem(leftListWidget);
            item->setSizeHint(QSize(120, 52)); //QSize(120, 40) spacing: 12px;
            leftListWidget->setItemWidget(item, leftWidgetItem);

            strItemsMap.insert(single.namei18nString, item);

            //填充上侧二级菜单
            QListWidgetItem * topitem = new QListWidgetItem(topListWidget);
            topitem->setSizeHint(QSize(60, 60));
            topitem->setText(single.namei18nString);
            topListWidget->addItem(topitem);

            strItemsMap.insert(single.namei18nString, topitem);

            CommonInterface * pluginInstance = qobject_cast<CommonInterface *>(moduleMap.value(single.namei18nString));

            pluginInstanceMap.insert(single.namei18nString, pluginInstance);

        }

//        QStringList functionStringList = FunctionSelect::funcsList[moduleIndex];
//        for (int funcIndex = 0; funcIndex < functionStringList.size(); funcIndex++){
//            QString funcnameString = functionStringList.at(funcIndex);
//            //跳过插件不存在的功能项
//            if (!moduleMap.contains(funcnameString))
//                continue;

//            //填充左侧二级菜单
//            LeftWidgetItem * leftWidgetItem = new LeftWidgetItem(this);
//            leftWidgetItem->setLabelText(funcnameString);
//            leftWidgetItem->setLabelPixmap(QString("://img/secondaryleftmenu/%1.png").arg(funcnameString));

//            QListWidgetItem * item = new QListWidgetItem(leftListWidget);
//            item->setSizeHint(QSize(120, 40)); //测试数据
//            leftListWidget->setItemWidget(item, leftWidgetItem);

//            //填充上侧二级菜单
//            QListWidgetItem * topitem = new QListWidgetItem(topListWidget);
//            topitem->setSizeHint(QSize(60, 60));
//            topitem->setText(funcnameString);
//            topListWidget->addItem(topitem);

//            CommonInterface * pluginInstance = qobject_cast<CommonInterface *>(moduleMap.value(funcnameString));

//            pluginInstanceMap.insert(funcnameString, pluginInstance);

//        }

        ui->leftStackedWidget->addWidget(leftListWidget);
        ui->topStackedWidget->addWidget(topListWidget);
    }

    //左侧模块标题及上侧模块标题随左侧二级菜单联动
    connect(ui->leftStackedWidget, &QStackedWidget::currentChanged, this, [=](int index){
        QString titleString = mkvConverter->keycodeTokeyi18nstring(index);

        ui->mtitleLabel->setText(titleString);
        ui->mmtitleLabel->setText(titleString);

    });
}

void ModulePageWidget::switchPage(QObject *plugin){
    CommonInterface * pluginInstance = qobject_cast<CommonInterface *>(plugin);
    QString name; int type;
    name = pluginInstance->get_plugin_name();
    type = pluginInstance->get_plugin_type();

    //首次点击设置模块标题后续交给回调函数
    if (ui->mtitleLabel->text().isEmpty() || ui->mmtitleLabel->text().isEmpty()){
        QString titleString = mkvConverter->keycodeTokeyi18nstring(type);

        ui->mtitleLabel->setText(titleString);
        ui->mmtitleLabel->setText(titleString);
    }

    //设置左侧一级菜单
    pmainWindow->setModuleBtnHightLight(type);

    //设置左侧二级菜单
    ui->leftStackedWidget->setCurrentIndex(type);

    //设置上侧二级菜单
    ui->topStackedWidget->setCurrentIndex(type);

    refreshPluginWidget(pluginInstance);
}

void ModulePageWidget::refreshPluginWidget(CommonInterface *plu){
    ui->scrollArea->takeWidget();
    delete(ui->scrollArea->widget());

    ui->scrollArea->setWidget(plu->get_plugin_ui());

    //延迟操作
    plu->plugin_delay_control();

    //发出信号更新左侧及上侧的高亮item
    emit widgetChanged(plu->get_plugin_name());
}

void ModulePageWidget::highlightItem(QString text){
    QList<QListWidgetItem *> currentItemList = strItemsMap.values(text);

    if (2 > currentItemList.count())
        return;

    //高亮左侧二级菜单
    QListWidget * lefttmpListWidget = dynamic_cast<QListWidget *>(ui->leftStackedWidget->currentWidget());
    lefttmpListWidget->setCurrentItem(currentItemList.at(1)); //QMultiMap 先添加的vlaue在后面

    //高亮上侧二级菜单
    QListWidget * toptmpListWidget = dynamic_cast<QListWidget *>(ui->topStackedWidget->currentWidget());
    toptmpListWidget->setCurrentItem(currentItemList.at(0)); //QMultiMap 后添加的value在前面
}

void ModulePageWidget::currentLeftitemChanged(QListWidgetItem *cur, QListWidgetItem *pre){
    //获取当前QListWidget
    QListWidget * currentLeftListWidget = dynamic_cast<QListWidget *>(ui->leftStackedWidget->currentWidget());


    if (pre != nullptr){
        LeftWidgetItem * preWidgetItem = dynamic_cast<LeftWidgetItem *>(currentLeftListWidget->itemWidget(pre));
        //取消高亮
        preWidgetItem->setSelected(false);
    }

    LeftWidgetItem * curWidgetItem = dynamic_cast<LeftWidgetItem *>(currentLeftListWidget->itemWidget(cur));
    if (pluginInstanceMap.contains(curWidgetItem->text())){
        CommonInterface * pluginInstance = pluginInstanceMap[curWidgetItem->text()];
        refreshPluginWidget(pluginInstance);
        //高亮
        curWidgetItem->setSelected(true);
    } else {
        qDebug() << "plugin widget not fount!";
    }
}
