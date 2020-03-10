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
#include "shortcut.h"
#include "ui_shortcut.h"

#include <QListWidgetItem>
#include <QDebug>

#include "customlineedit.h"

#include "realizeshortcutwheel.h"

#define ITEMHEIGH 36

typedef struct _KeyEntry KeyEntry;

struct _KeyEntry : QObjectUserData{
//    int keyval;
    QString gsSchema;
    QString keyStr;
    QString valueStr;
    QString descStr;

    QString gsPath;
    QString nameStr;
    QString bindingStr;
    QString actionStr;
};

Q_DECLARE_METATYPE(KeyEntry)

//快捷键屏蔽键
QStringList forbiddenKeys = {
    //Navigation keys
    "Home",
    "Left",
    "Up",
    "Right",
    "Down",
    "Page_Up",
    "Page_Down",
    "End",
    "Tab",

    // Return
    "Return",
    "Enter",

    "Space",
};

QList<KeyEntry *> generalEntries;
QList<KeyEntry *> customEntries;

Shortcut::Shortcut()
{
    ui = new Ui::Shortcut;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("shortcut");
    pluginType = DEVICES;

    pluginWidget->setStyleSheet("background: #ffffff;");

    ui->generalListWidget->setStyleSheet("QListWidget#generalListWidget{background: #ffffff; border: none;}");

    ui->showBtn->setStyleSheet("QPushButton{background: #E9E9E9; border-radius: 4px;}"
                               "QPushButton:hover:!pressed{background: #3d6be5; border-radius: 4px;}"
                               "QPushButton:hover:pressed{background: #415FC4; border-radius: 4px;}");

    ui->customListWidget->setStyleSheet("QListWidget#customListWidget{background: #ffffff; border: none;}");

    ui->addWidget->setStyleSheet("QWidget{background: #F4F4F4; border-radius: 6px;}");

    pKeyMap = new KeyMap;

    showList << "terminal" << "screenshot" << "area-screenshot" << "peony-qt" << "logout" << "screensaver";

    setupComponent();
    initFunctionStatus();
}

Shortcut::~Shortcut()
{
    delete ui;
    delete pKeyMap;
}

QString Shortcut::get_plugin_name(){
    return pluginName;
}

int Shortcut::get_plugin_type(){
    return pluginType;
}

QWidget *Shortcut::get_plugin_ui(){
    return pluginWidget;
}

void Shortcut::plugin_delay_control(){

}

void Shortcut::setupComponent(){
    ui->addBtn->setIcon(QIcon("://img/plugins/printer/add.png"));
    ui->addBtn->setIconSize(QSize(48, 48));


    ui->generalListWidget->setFocusPolicy(Qt::NoFocus);
    ui->generalListWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->generalListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->generalListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->generalListWidget->setSpacing(0);

    ui->customListWidget->setFocusPolicy(Qt::NoFocus);
    ui->customListWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->customListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->customListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->customListWidget->setSpacing(0);
//    ui->customListWidget->setFixedHeight((showList.length() * ITEMHEIGH));

}

void Shortcut::initFunctionStatus(){
    generalEntries.clear();
    customEntries.clear();

    //使用线程获取快捷键
    pThread = new QThread;
    pWorker = new GetShortcutWorker;
    connect(pWorker, &GetShortcutWorker::generalShortcutGenerate, this, [=](QString schema, QString key, QString value){
//        qDebug() << "general shortcut" << schema << key << value;
        KeyEntry * generalKeyEntry = new KeyEntry;
        generalKeyEntry->gsSchema = schema;
        generalKeyEntry->keyStr = key;
        generalKeyEntry->valueStr = value;
        generalEntries.append(generalKeyEntry);

    });
    connect(pWorker, &GetShortcutWorker::customShortcutGenerate, this, [=](QString path, QString name, QString binding, QString action){
//        qDebug() << "custom shortcut" << path << name << binding;
        KeyEntry * customKeyEntry = new KeyEntry;
        customKeyEntry->gsSchema = KEYBINDINGS_CUSTOM_SCHEMA;
        customKeyEntry->gsPath = path;
        customKeyEntry->nameStr = name;
        customKeyEntry->bindingStr = binding;
        customKeyEntry->actionStr = action;
        customEntries.append(customKeyEntry);

    });
    connect(pWorker, &GetShortcutWorker::workerComplete, this, [=]{
        pThread->quit(); //退出事件循环
        pThread->wait(); //释放资源
    });

    pWorker->moveToThread(pThread);
    connect(pThread, &QThread::started, pWorker, &GetShortcutWorker::run);
    connect(pThread, &QThread::finished, this, [=]{
        appendGeneralItems();
        appendCustomItems();

        ui->generalListWidget->setFixedHeight(ui->generalListWidget->count() * ITEMHEIGH);
        ui->customListWidget->setFixedHeight(ui->customListWidget->count() * ITEMHEIGH);

        initGeneralItemsStyle();
        initCustomItemsStyle();
    });
    connect(pThread, &QThread::finished, pWorker, &GetShortcutWorker::deleteLater);

    pThread->start();
}

QWidget * Shortcut::buildItemWidget(QString name, QString binding){
    QWidget * baseWidget = new QWidget;
    baseWidget->setAttribute(Qt::WA_DeleteOnClose);

    QHBoxLayout * mainHorLayout = new QHBoxLayout(baseWidget);
    mainHorLayout->setSpacing(0);
    mainHorLayout->setContentsMargins(16, 0, 24, 0);

    baseWidget->setLayout(mainHorLayout);

    QLabel * nameLabel = new QLabel(baseWidget);
    nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    nameLabel->setText(name);

    CustomLineEdit * line = new CustomLineEdit(binding, baseWidget);
    line->setStyleSheet("QLineEdit{border: none;}");
    line->setAlignment(Qt::AlignRight);
    line->setText(binding);
    line->setReadOnly(true);
    line->setFixedWidth(300);
    connect(line, &CustomLineEdit::shortcutCodeSignals, this, [=](QList<int> keyCode){
        qDebug() << keyCode << getBindingName(keyCode);
        newBindingRequest(keyCode);
    });


    mainHorLayout->addWidget(nameLabel);
    mainHorLayout->addStretch();
    mainHorLayout->addWidget(line);

    return baseWidget;

}

void Shortcut::appendGeneralItems(){
    for (KeyEntry * gkeyEntry: generalEntries){
        if (showList.contains(gkeyEntry->keyStr)){
            QWidget * singleWidget = buildItemWidget(gkeyEntry->keyStr, gkeyEntry->valueStr);
            singleWidget->setUserData(Qt::UserRole, gkeyEntry);
            QListWidgetItem * item = new QListWidgetItem(ui->generalListWidget);
            item->setSizeHint(QSize(ui->generalListWidget->width(), ITEMHEIGH));
            ui->generalListWidget->setItemWidget(item, singleWidget);
        }
    }
}

void Shortcut::appendCustomItems(){
    for (KeyEntry * ckeyEntry : customEntries){
        QWidget * singleWidget = buildItemWidget(ckeyEntry->nameStr, ckeyEntry->bindingStr);
        singleWidget->setUserData(Qt::UserRole, ckeyEntry);
        QListWidgetItem * item = new QListWidgetItem(ui->customListWidget);
        item->setSizeHint(QSize(ui->customListWidget->width(), ITEMHEIGH));
        ui->customListWidget->setItemWidget(item, singleWidget);
    }
}

void Shortcut::initItemsStyle(QListWidget *listWidget){
    int total = listWidget->count();
    for (int row = 0; row < total; row++){
        QString style;
        if (1 == total){ //总数为1
            style = "QWidget{background: #F4F4F4; border: none; border-radius: 6px;}";
        } else if (0 == row && (row % 2 == 0)){ //首位
            style = "QWidget{background: #F4F4F4; border: none; border-top-left-radius: 6px; border-top-right-radius: 6px;}";
        } else if (total - 1 == row){ //末位
            if (0 == row % 2)
                style = "QWidget{background: #F4F4F4; border: none; border-bottom-left-radius: 6px; border-bottom-right-radius: 6px;}";
            else
                style = "QWidget{background: #EEEEEE; border: none; border-bottom-left-radius: 6px; border-bottom-right-radius: 6px;}";
        } else if (row % 2 == 0){
            style = "QWidget{background: #F4F4F4; border: none;}";
        } else if (row % 2 != 0){
            style = "QWidget{background: #EEEEEE; border: none;}";
        }

        QWidget * widget = listWidget->itemWidget(listWidget->item(row));
        widget->setStyleSheet(style);
    }
}

void Shortcut::initGeneralItemsStyle(){
    initItemsStyle(ui->generalListWidget);
}

void Shortcut::initCustomItemsStyle(){
    initItemsStyle(ui->customListWidget);
}

void Shortcut::newBindingRequest(QList<int> keyCode){
    QObject * object = QObject::sender();

    CustomLineEdit * current = qobject_cast<CustomLineEdit *>(object);
    QWidget * widget = current->parentWidget();
    KeyEntry * nkeyEntry = dynamic_cast<KeyEntry *>(widget->userData(Qt::UserRole));

    QString shortcutString = getBindingName(keyCode);

    //check for unmodified keys
    if (keyCode.count() == 1){
        if (shortcutString.contains(QRegExp("[a-z]")) ||
                shortcutString.contains(QRegExp("[0-9]")) ||
                keyIsForbidden(shortcutString)){
            qDebug() << "Please try with a key such as Control, Alt or Shift at the same time.";
            return;
        }
    }

    /* flag to see if the new accelerator was in use by something */
    for (KeyEntry * ckeyEntry : generalEntries){
        if (shortcutString == ckeyEntry->valueStr){
            qDebug() << QString("The shortcut \"%1\" is already used for\n\"%2\",please reset!!!").arg(shortcutString).arg(ckeyEntry->keyStr);
            return;
        }
    }

    current->setText(shortcutString);

    if (nkeyEntry->gsPath.isEmpty()){ //非自定义快捷键的修改
        const QByteArray iid(nkeyEntry->gsSchema.toLatin1().data());
        QGSettings * settings = new QGSettings(iid);

//        settings->set(nkeyEntry->keyStr, shortcutString);

        delete settings;

        //更新
        for (int index = 0; index < generalEntries.count(); index++){
            if (nkeyEntry->keyStr == generalEntries[index]->keyStr){
                generalEntries[index]->valueStr = shortcutString;
            }
        }

    } else { //自定义快捷键的修改
        const QByteArray id(KEYBINDINGS_CUSTOM_SCHEMA);
        const QByteArray idd(nkeyEntry->gsPath.toLatin1().data());
        QGSettings * settings = new QGSettings(id, idd);

//        settings->set(BINDING_KEY, shortcutString);

        delete settings;

        //更新
        for (int index = 0; index < customEntries.count(); index++){
            if (nkeyEntry->nameStr == customEntries[index]->nameStr){
                customEntries[index]->bindingStr = shortcutString;
            }
        }
    }
}

QString Shortcut::getBindingName(QList<int> keyCode){
    QStringList tmpList;
    for (int keycode : keyCode){
        if (keycode >= 16777216 && keycode <= 16777254){ //1677216=Escape; 16777254=ScrollLock
            tmpList.append(QString("<%1>").arg(pKeyMap->keycodeTokeystring(keycode)));
        }
        else if (keycode >= 48 && keycode <= 57){ // 48 = 0; 57 = 9
            QString str = pKeyMap->keycodeTokeystring(keycode);
            tmpList.append(str.split("K_").at(1));
        }
        else if (keycode >= 65 && keycode <= 90){
            QString str = pKeyMap->keycodeTokeystring(keycode);
            tmpList.append(str.toLower());
        }
        else
            tmpList.append(pKeyMap->keycodeTokeystring(keycode));
    }
    return tmpList.join("");
}

bool Shortcut::keyIsForbidden(QString key){
    for (int i = 0; i < forbiddenKeys.length(); i++){
        if (key == forbiddenKeys[i])
            return true;
    }
    return false;
}

