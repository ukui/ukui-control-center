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

#include "mainwindow.h"

#include "../plugins/pluginsComponent/customwidget.h"
#include "../plugins/pluginsComponent/publicdata.h"

ModulePageWidget::ModulePageWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ModulePageWidget)
{
    ui->setupUi(this);
    //set parent
    this->setParent(parent);
    pmainWindow = (MainWindow *)parentWidget();

//    QFont font;
//    font.setPointSize(14);
//    ui->modulesTitle->setFont(font);
//    ui->modulesTitle->setStyleSheet("color: #999999");

//    connect(ui->backBtn, SIGNAL(clicked(bool)), pmainWindow, SLOT(backToMain()));

    ui->widget->setStyleSheet("background-color: #ebeef0");
    ui->leftStackedWidget->setStyleSheet("border-style: none");
    ui->backBtn->setStyleSheet("border-style: none;");
    ui->scrollArea->setStyleSheet("#scrollArea{border: 0px solid;}");

    connect(ui->backBtn, SIGNAL(clicked()), this, SLOT(backBtnClicked_cb()));

    initUI();
}

ModulePageWidget::~ModulePageWidget()
{
    for ( int i = 0; i < ui->leftStackedWidget->count(); i++){
        delete(ui->leftStackedWidget->widget(i));
    }
    delete ui;
}

void ModulePageWidget::initUI(){

    PublicData * publicdata = new PublicData();

    for (int i = 0; i < FUNCTOTALNUM; i++){
        QListWidget * leftListWidget = new QListWidget();
        connect(leftListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(itemClicked_cb(QListWidgetItem*)));

        QMap<QString, QObject *> funcMaps;
        funcMaps = pmainWindow->export_module(i);
        QStringList currentStringList = publicdata->subfuncList[i];
        QStringList currentlogoStringList = publicdata->en_subfuncList[i];

        for (int num = 0; num < currentStringList.size(); num++){
            if (!funcMaps.contains(currentStringList.at(num)))
                continue;

            ListWidgetItem * widgetitem = new ListWidgetItem(this);
            widgetitem->setLabelPixmap(QString("://dynamic/leftsidebar/%1.svg").arg(currentlogoStringList.at(num)));
            widgetitem->setLabelText(currentStringList.at(num));
            QListWidgetItem * item = new QListWidgetItem(leftListWidget);
            item->setSizeHint(QSize(198,50));
            leftListWidget->setItemWidget(item, widgetitem);
//            QListWidgetItem * item = new QListWidgetItem(leftListWidget);
//            item->setSizeHint(QSize(198,50)); //widget width 200 -  border 2 = 198
//            item->setIcon(QIcon(QString("://dynamic/leftsidebar/%1.svg").arg(currentStringList.at(num))));
//            QFont font;
//            font.setPixelSize(14);
//            item->setFont(font);
//            item->setText(currentStringList.at(num));
//            leftListWidget->addItem(item);

            CommonInterface * pluginInstance = qobject_cast<CommonInterface *>(funcMaps[currentStringList.at(num)]);
            CustomWidget * widget = pluginInstance->get_plugin_ui();
            //绑定每个插件的currentChange信号，更新返回按钮的文字显示
            QStackedWidget * stackwidget = widget->findChild<QStackedWidget *>("StackedWidget");
            connect(stackwidget, SIGNAL(currentChanged(int)), this, SLOT(update_backbtn_text_slot(int)));
            connect(widget, SIGNAL(transmit(QString,int,int)), this, SLOT(toggle_plugin_slot(QString,int,int)));

            pluginInstanceMap.insert(currentStringList.at(num), pluginInstance);
        }
        ui->leftStackedWidget->addWidget(leftListWidget);
    }

    //pushbutton添加布局实现调整图标、文字距离调整
    backiconLabel = new QLabel(ui->backBtn);
//    backiconLabel->setFixedSize(QSize(16, 16));
    QSizePolicy iconpolicy = backiconLabel->sizePolicy();
    iconpolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    iconpolicy.setVerticalPolicy(QSizePolicy::Fixed);
    backiconLabel->setSizePolicy(iconpolicy);
    backiconLabel->setScaledContents(true);
    backiconLabel->setPixmap(QPixmap(":/back.svg"));

    backtextLabel = new QLabel(ui->backBtn);
    QSizePolicy policy = backtextLabel->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Fixed);
    policy.setVerticalPolicy(QSizePolicy::Fixed);
    backtextLabel->setSizePolicy(policy);
    backtextLabel->setScaledContents(true);
    backtextLabel->setText(tr("CCMainPage"));
    QHBoxLayout * btnLayout = new QHBoxLayout();
    btnLayout->addWidget(backiconLabel);
    btnLayout->addWidget(backtextLabel);
    btnLayout->addStretch();
    btnLayout->setSpacing(10);
    btnLayout->setContentsMargins(20,0,0,0);

    ui->backBtn->setLayout(btnLayout);

    delete publicdata;
}

void ModulePageWidget::switch_modulepage(QObject * plugin, int page){ //page 默认为0
    CommonInterface * pluginInstance = qobject_cast<CommonInterface *>(plugin);
    QString name; int type;
    name = pluginInstance->get_plugin_name();
    type = pluginInstance->get_plugin_type();

    //设置module title
    if (type == SYSTEM)
        ui->modulesTitle->setText(tr("System"));
    else if (type == DEVICES)
        ui->modulesTitle->setText(tr("Devices"));
    else if (type == PERSONALIZED)
        ui->modulesTitle->setText(tr("Personalized"));
    else if (type == NETWORK)
        ui->modulesTitle->setText(tr("Network"));
    else if (type == ACCOUNT)
        ui->modulesTitle->setText(tr("Account"));
    else if (type == TIME_LANGUAGE)
        ui->modulesTitle->setText(tr("Time Language"));
    else if (type == SECURITY_UPDATES)
        ui->modulesTitle->setText(tr("Security Updates"));
    else if (type == MESSAGES_TASK)
        ui->modulesTitle->setText(tr("Message Task"));

    ui->leftStackedWidget->setCurrentIndex(type);

    //高亮左侧边栏
//    QListWidget * tmpListWidget = new QListWidget();
//    tmpListWidget->setAttribute(Qt::WA_DeleteOnClose);
    QListWidget * tmpListWidget = dynamic_cast<QListWidget *>(ui->leftStackedWidget->currentWidget());
    for (int i = 0; i < tmpListWidget->count(); i++){
        ListWidgetItem * widgetitem = dynamic_cast<ListWidgetItem *>(tmpListWidget->itemWidget(tmpListWidget->item(i)));
        if (name == (widgetitem->text())){
            tmpListWidget->setCurrentRow(i);
        }
    }

    update_plugin_widget(pluginInstance, page);
}

void ModulePageWidget::update_backbtn_text(int index){
    if (index != 0)
        backtextLabel->setText(tr("UpperLevel"));
    else
        backtextLabel->setText(tr("CCMainPage"));
}

void ModulePageWidget::update_plugin_widget(CommonInterface *plu, int page){
    ui->scrollArea->takeWidget();
    delete(ui->scrollArea->widget()); //释放上次显示的ui

    ui->scrollArea->setWidget(plu->get_plugin_ui());

    //更新返回按钮text
    QStackedWidget * stackwidget = plu->get_plugin_ui()->findChild<QStackedWidget *>("StackedWidget");
    stackwidget->setCurrentIndex(page);

    update_backbtn_text(page);

    plu->plugin_delay_control(); //执行一些界面显示后的操作
}

void ModulePageWidget::itemClicked_cb(QListWidgetItem * item){
    QListWidget * tmpListWidget = dynamic_cast<QListWidget *>(ui->leftStackedWidget->currentWidget());
    ListWidgetItem * widgetitem = dynamic_cast<ListWidgetItem *>(tmpListWidget->itemWidget(item));
    if (pluginInstanceMap.contains(widgetitem->text())){
        CommonInterface * pluginInstance = pluginInstanceMap[widgetitem->text()];
        update_plugin_widget(pluginInstance, 0); //切换左侧边栏，stackwidget的index一定为0
    }
    else{
        qDebug() << "plugin widget not found" ;
    }
}

void ModulePageWidget::backBtnClicked_cb(){
    QStackedWidget * tmpStackedWidget = ui->scrollArea->widget()->findChild<QStackedWidget *>("StackedWidget");
    if (tmpStackedWidget->currentIndex())
        tmpStackedWidget->setCurrentIndex(0);
    else
        pmainWindow->backToMain();
}

void ModulePageWidget::toggle_plugin_slot(QString pluginname, int plugintype, int page){    //插件中按钮点击后的跳转槽函数
    QMap<QString, QObject *> funcMaps;
    funcMaps = pmainWindow->export_module(plugintype);

    if (funcMaps.contains(pluginname)){
        switch_modulepage(funcMaps[pluginname], page);
    }
    else{
        qDebug() << "plugin instance" << pluginname <<"not found";
    }
}

void ModulePageWidget::update_backbtn_text_slot(int index){
    update_backbtn_text(index);
}
