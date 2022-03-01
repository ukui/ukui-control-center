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
#include <QDesktopWidget>
#include <QScrollBar>
#include "delegate.h"
#include "mainwindow.h"
#include "interface.h"
#include "utils/keyvalueconverter.h"
#include "utils/functionselect.h"
#include "component/leftwidgetitem.h"

#include <QDebug>
#define THEME_QT_SCHEMA                  "org.ukui.style"
#define MODE_QT_KEY                      "style-name"

ModulePageWidget::ModulePageWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ModulePageWidget)
{
    qtSettings = NULL;
    ui->setupUi(this);
    //关闭菜单响应
    ui->scrollArea->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    ui->scrollArea->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    //设置父窗口对象
    this->setParent(parent);
    pmainWindow = (MainWindow *)parentWidget();
    pmainWindow->resize(1200,675);
    pmainWindow->setMinimumSize(1000,675);
    ui->leftbarWidget->setMinimumWidth(285);
    //左侧二级菜单样式
    ui->leftbarWidget->setStyleSheet("QWidget#leftbarWidget{border: none;background:transparent;}");
    ui->leftStackedWidget->setObjectName("leftStackedWidget");
    ui->leftStackedWidget->setStyleSheet("QFrame#leftStackedWidget{border: none;background:transparent;}");
    //功能区域
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //初始化记录标志位
    flagBit = true;


    //构建枚举键值转换对象
    mkvConverter = new KeyValueConverter(); //继承QObject，No Delete

//    ui->topsideWidget->hide();

    initUI();


}
LeftWidget::LeftWidget(QWidget *parent) : QListWidget(parent) {
}

LeftWidget::~LeftWidget() {
}
void LeftWidget::wheelEvent(QWheelEvent* event)
{
//    //通过修改系数调整滚轮敏感度，系数越大滑动距离越长
//    int degress = event->angleDelta().ry() * 0.03;
//    printf("%s %d vs:%d de:%d: delta%d ry:%d\n", __FUNCTION__, __LINE__,verticalScrollBar()->value(),degress,event->delta(),event->angleDelta().ry());

//    if(event->orientation() == Qt::Vertical)
//        verticalScrollBar()->setValue(verticalScrollBar()->value() - degress);
    static int touchPadMoveDistance = 0;
    int step = 0;
    int newValue = 0;
    int oldValue = verticalScrollBar()->value();

    if(event->delta() > maxDeltaPerCall || event->delta() < maxDeltaPerCall*-1){
        goto END;
    }

    touchPadMoveDistance -= event->delta();

    if(touchPadMoveDistance > perStepDelta){
        step = touchPadMoveDistance/perStepDelta;
        touchPadMoveDistance = 0;
    }
    else if(touchPadMoveDistance < perStepDelta*-1){
        step = touchPadMoveDistance/perStepDelta;
        touchPadMoveDistance = 0;
    }
    else{
        goto END;
    }
    newValue = oldValue +step;

    newValue = newValue>verticalScrollBar()->maximum()? verticalScrollBar()->maximum():newValue;
    newValue = newValue<verticalScrollBar()->minimum()? verticalScrollBar()->minimum():newValue;

    verticalScrollBar()->setValue(newValue);
END:
    event->accept();
}

ModulePageWidget::~ModulePageWidget()
{
    delete ui;
    if (qtSettings)
        delete qtSettings;
}

bool ModulePageWidget::eventFilter(QObject *watched, QEvent *event)
{

    if (event->type() == QEvent::MouseMove) {
        if (leftListWidget->geometry().contains(this->mapFromGlobal(QCursor::pos()) - QPoint(6, 12))) {
            leftListWidget->verticalScrollBar()->setVisible(true);
        } else {
            leftListWidget->verticalScrollBar()->setVisible(false);
        }
    }
    return QObject::eventFilter(watched,event);
}

void ModulePageWidget::initUI(){
    //初始化控制面板打开的首页
    QString firstFunc;
    QList<FuncInfo> tmpList = FunctionSelect::funcinfoList[0];
    for (FuncInfo tmpStruct : tmpList){
        firstFunc = tmpStruct.namei18nString;
        if (pmainWindow->exportModule(0).keys().contains(tmpStruct.namei18nString)){
            CommonInterface * pluginInstance = qobject_cast<CommonInterface *>(pmainWindow->exportModule(0).value(firstFunc));
            refreshPluginWidget(pluginInstance);
            break;
        }
    }
    //设置伸缩策略
    QSizePolicy leftSizePolicy = ui->leftbarWidget->sizePolicy();
    QSizePolicy rightSizePolicy = ui->widget->sizePolicy();

    leftSizePolicy.setHorizontalStretch(1);
    rightSizePolicy.setHorizontalStretch(5);

    ui->leftbarWidget->setSizePolicy(leftSizePolicy);
    ui->widget->setSizePolicy(rightSizePolicy);
    leftListWidget = new LeftWidget(this);
    leftListWidget->setItemDelegate(new Delegate(this));
    leftListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //关闭菜单响应
    leftListWidget->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    leftListWidget->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    leftListWidget->verticalScrollBar()->setVisible(false);
    for (int moduleIndex = 0; moduleIndex < TOTALMODULES; moduleIndex++){
        QString titleString = mkvConverter->keycodeTokeyi18nstring(moduleIndex);
        QString titleString_1 ="    "+titleString;
        leftListWidget->setObjectName("leftWidget");
        leftListWidget->setStyleSheet("QListWidget{border: none;background:transparent;}"
                                      "QListWidget::Item:hover{background:transparent;}");

        QListWidgetItem * titleItem = new QListWidgetItem();
        titleItem->setData(Qt::UserRole, "title");

        titleItem->setText(titleString_1);

        titleItem->setFlags((Qt::ItemFlag)0);
        const QByteArray idd(THEME_QT_SCHEMA);
        if  (QGSettings::isSchemaInstalled(idd) && !qtSettings){
            qtSettings = new QGSettings(idd);
        }
        currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
        if ("ukui-white" == currentThemeMode || "ukui-default" == currentThemeMode || "ukui-light" == currentThemeMode || "ukui-white-unity" == currentThemeMode) {
            titleItem->setTextColor(QColor(34,34,34));
        } else if ("ukui-dark" == currentThemeMode || "ukui-black" == currentThemeMode || "ukui-black-unity" == currentThemeMode){
            titleItem->setTextColor(QColor(255,255,255));
        }
        connect(qtSettings, &QGSettings::changed, this, [=](const QString &key){
            currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
            if ("ukui-white" == currentThemeMode || "ukui-default" == currentThemeMode || "ukui-light" == currentThemeMode || "ukui-white-unity" == currentThemeMode) {
                titleItem->setTextColor(QColor(34,34,34));
            } else if ("ukui-dark" == currentThemeMode || "ukui-black" == currentThemeMode || "ukui-black-unity" == currentThemeMode){
                titleItem->setTextColor(QColor(255,255,255));
            }
        });
        if(titleString != QString::fromLocal8Bit("Account")) {
            leftListWidget->addItem(titleItem);
        }
        leftListWidget->setAttribute(Qt::WA_DeleteOnClose);
        leftListWidget->setResizeMode(QListView::Adjust);
        leftListWidget->setFocusPolicy(Qt::NoFocus);
        leftListWidget->setSelectionMode(QAbstractItemView::NoSelection);
        leftListWidget->setSpacing(0);
        leftListWidget->viewport()->setAttribute(Qt::WA_TranslucentBackground);
        leftListWidget->setStyleSheet("QListWidget{background-color: transparent}");
        leftListWidget->viewport()->setStyleSheet("background-color: transparent");
        leftListWidget->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
        connect(leftListWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(currentLeftitemChanged(QListWidgetItem*,QListWidgetItem*)));

        connect(leftListWidget, &QListWidget::itemClicked, this, &ModulePageWidget::changePageSlot);
        QListWidget * topListWidget = new QListWidget;
        topListWidget->setAttribute(Qt::WA_DeleteOnClose);
        topListWidget->setResizeMode(QListView::Adjust);
        topListWidget->setViewMode(QListView::IconMode);
        topListWidget->setMovement(QListView::Static);
        topListWidget->setSpacing(6);

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
            currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
            if ("ukui-white" == currentThemeMode || "ukui-default" == currentThemeMode || "ukui-light" == currentThemeMode || "ukui-white-unity" == currentThemeMode) {
                leftWidgetItem->setLabelPixmap(QString("://img/secondaryleftmenu/%1.svg").arg(single.nameString), single.nameString, "default");
            } else if ("ukui-dark" == currentThemeMode || "ukui-black" == currentThemeMode || "ukui-black-unity" == currentThemeMode){
                leftWidgetItem->setLabelPixmap(QString("://img/secondaryleftmenu/%11White.svg").arg(single.nameString), single.nameString, "default");
            }
            connect(qtSettings, &QGSettings::changed, this, [=](const QString &key){
                if (key == "styleName") {
                    currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
                    if ("ukui-white" == currentThemeMode || "ukui-default" == currentThemeMode || "ukui-light" == currentThemeMode || "ukui-white-unity" == currentThemeMode) {
                        leftWidgetItem->setLabelPixmap(QString("://img/secondaryleftmenu/%1.svg").arg(single.nameString), single.nameString, "default");
                    } else if ("ukui-dark" == currentThemeMode || "ukui-black" == currentThemeMode || "ukui-black-unity" == currentThemeMode){
                        leftWidgetItem->setLabelPixmap(QString("://img/secondaryleftmenu/%1White.svg").arg(single.nameString), single.nameString, "default");
                    }
                }
            });

            QListWidgetItem * item = new QListWidgetItem(leftListWidget);
            item->setSizeHint(QSize(268,56)); //QSize(120, 40) spacing: 12px;
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

        ui->leftStackedWidget->addWidget(leftListWidget);
    }

    //左侧二级菜单标题及上侧二级菜单标题随功能页变化联动
    connect(ui->leftStackedWidget, &QStackedWidget::currentChanged, this, [=](int index){
        QString titleString = mkvConverter->keycodeTokeyi18nstring(index);
    });
    QString name=tmpList.at(0).namei18nString;
    QList<QListWidgetItem *> currentItemList = strItemsMap.values(name);
    qDebug()<<name;
    firstItem=currentItemList.at(1);
    changePageSlot(currentItemList.at(1));
    qApp->installEventFilter(this);
}

void ModulePageWidget::switchPage(QObject *plugin, bool recorded){

    CommonInterface * pluginInstance = qobject_cast<CommonInterface *>(plugin);
    QString name; int type;
    name = pluginInstance->get_plugin_name();
    type = pluginInstance->get_plugin_type();

    //通过设置标志位确定是否记录打开历史
    flagBit = recorded;

    //设置左侧菜单
    ui->leftStackedWidget->setCurrentIndex(type);


    QListWidget * lefttmpListWidget = dynamic_cast<QListWidget *>(ui->leftStackedWidget->currentWidget());
    if (lefttmpListWidget->currentItem() != nullptr){
        LeftWidgetItem * widget = dynamic_cast<LeftWidgetItem *>(lefttmpListWidget->itemWidget(lefttmpListWidget->currentItem()));
        //待打开页与QListWidget的CurrentItem相同
        if (QString::compare(widget->text(), name) == 0){
            refreshPluginWidget(pluginInstance);
        }
    }
    //设置左侧及上侧的当前Item及功能Widget
    highlightItem(name);

}

void ModulePageWidget::refreshPluginWidget(CommonInterface *plu){
    ui->scrollArea->takeWidget();
    delete(ui->scrollArea->widget());

    ui->scrollArea->setWidget(plu->get_plugin_ui());
    //延迟操作
    plu->plugin_delay_control();

    //记录打开历史
    if (flagBit){
        FunctionSelect::pushRecordValue(plu->get_plugin_type(), plu->get_plugin_name());
    }

    //恢复标志位
    flagBit = true;
}

void ModulePageWidget::highlightItem(QString text){
    QList<QListWidgetItem *> currentItemList = strItemsMap.values(text);

    if (2 > currentItemList.count())
        return;

    //高亮左侧二级菜单
    QListWidget * lefttmpListWidget = dynamic_cast<QListWidget *>(ui->leftStackedWidget->currentWidget());
    lefttmpListWidget->setCurrentItem(currentItemList.at(1)); //QMultiMap 先添加的vlaue在后面
}

void ModulePageWidget::changePageSlot(QListWidgetItem *cur)
{
    if (mSelectItem != cur) {
        mSelectItem = cur;
    } else {
        return;
    }

    QListWidget * currentLeftListWidget = dynamic_cast<QListWidget *>(ui->leftStackedWidget->currentWidget());
    for (int i = 0; i < currentLeftListWidget->count(); i++) {
        QListWidgetItem *item = currentLeftListWidget->item(i);
        LeftWidgetItem *itemWidget = dynamic_cast<LeftWidgetItem *>(currentLeftListWidget->itemWidget(item));
        if (itemWidget != nullptr) {
            if (itemWidget->is_seletced) {
                itemWidget->setSelected(false);
                itemWidget->setLabelTextIsWhite(false);
                itemWidget->isSetLabelPixmapWhite(false);
            }
        }
    }

    LeftWidgetItem * curWidgetItem = dynamic_cast<LeftWidgetItem *>(currentLeftListWidget->itemWidget(cur));
    if ((curWidgetItem != nullptr) && pluginInstanceMap.contains(curWidgetItem->text())){
        CommonInterface * pluginInstance = pluginInstanceMap[curWidgetItem->text()];
        refreshPluginWidget(pluginInstance);
        // 高亮
        curWidgetItem->setSelected(true);
        curWidgetItem->setLabelTextIsWhite(true);
        curWidgetItem->isSetLabelPixmapWhite(true);
    }
    emit pageChangeSignal();
}
