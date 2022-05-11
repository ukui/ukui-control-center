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
#include "defineshortcutitem.h"
#include <ukcc/widgets/fixlabel.h>
#include <ukcc/widgets/hlineframe.h>
#include "doubleclicklineedit.h"
#include <QMenu>
#include "clickfixlabel.h"

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

#include <glib.h>
#include <glib/gi18n.h>

#define ITEMHEIGH 36
#define TITLEWIDGETHEIGH 40
#define SYSTEMTITLEWIDGETHEIGH 50
#define UKUI_STYLE_SCHEMA          "org.ukui.style"
#define SYSTEM_FONT_EKY            "system-font-size"

// 快捷键屏蔽键
QStringList forbiddenKeys = {
    // Navigation keys
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

Shortcut::Shortcut() : mFirstLoad(true)
{
    pluginName = tr("Shortcut");
    pluginType = DEVICES;
}

Shortcut::~Shortcut()
{
    if (!mFirstLoad) {
        delete ui;
        ui = nullptr;
        delete pKeyMap;
        pKeyMap = nullptr;
    }
}

QString Shortcut::plugini18nName()
{
    return pluginName;
}

int Shortcut::pluginTypes()
{
    return pluginType;
}

QWidget *Shortcut::pluginUi()
{
    if (mFirstLoad) {
        mFirstLoad = false;

        ui = new Ui::Shortcut;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);

        pKeyMap = new KeyMap;

        isCloudService = false;

        setupComponent();
        setupConnect();
        initFunctionStatus();
        connectToServer();
    }
    return pluginWidget;
}

const QString Shortcut::name() const
{
    return QStringLiteral("Shortcut");
}

bool Shortcut::isShowOnHomePage() const
{
    return false;
}

QIcon Shortcut::icon() const
{
    return QIcon::fromTheme("ukui-shortcut-key-symbolic");
}

bool Shortcut::isEnable() const
{
    return true;
}

void Shortcut::connectToServer()
{
    QThread *NetThread = new QThread;
    MThread *NetWorker = new MThread;
    NetWorker->moveToThread(NetThread);
    connect(NetThread, &QThread::started, NetWorker, &MThread::run);
    connect(NetWorker,&MThread::keychangedsignal,this,&Shortcut::shortcutChangedSlot);
    connect(NetThread, &QThread::finished, NetWorker, &MThread::deleteLater);
     NetThread->start();
}

void Shortcut::setupComponent()
{
    QLabel *AddLabel = new QLabel;
    //~ contents_path /Shortcut/Add
    AddLabel->setText(tr("Add"));       // 用于添加搜索索引
    delete AddLabel;
    //~ contents_path /Shortcut/System Shortcut
    ui->systemLabel->setText(tr("System Shortcut"));
    //~ contents_path /Shortcut/Customize Shortcut
    ui->customLabel->setText(tr("Customize Shortcut"));

    QWidget *systemTitleWidget = new QWidget;
    QVBoxLayout *systemVerLayout = new QVBoxLayout(systemTitleWidget);

    systemTitleWidget->setFixedHeight(SYSTEMTITLEWIDGETHEIGH);
    systemTitleWidget->setStyleSheet("QWidget{background: palette(window);"
                                     "border: none; border-radius: 4px}");
    systemVerLayout->setSpacing(0);
    systemVerLayout->setContentsMargins(16, 15, 19, 0);

    QLabel *titleLabel = new QLabel(systemTitleWidget);
    titleLabel->setText(tr("System Shortcut"));

    systemVerLayout->addWidget(titleLabel);
    systemVerLayout->addStretch();
    systemTitleWidget->setLayout(systemVerLayout);

    AddBtn *addBtn = new AddBtn();
    connect(addBtn, &AddBtn::clicked, this, [=](){
        addShortcutDialog *addDialog = new addShortcutDialog(generalEntries, customEntries, pluginWidget);
        addDialog->setTitleText(QObject::tr("Customize Shortcut"));

        connect(addDialog, &addShortcutDialog::shortcutInfoSignal,
                [=](QString path, QString name, QString exec, QString key){
            createNewShortcut(path, name, exec, key);
        });

        addDialog->exec();
    });

    ui->addLyt->addWidget(addBtn);
}

void Shortcut::setupConnect()
{

}

void Shortcut::initFunctionStatus()
{
    generalEntries.clear();
    customEntries.clear();

    // 使用线程获取快捷键
    pThread = new QThread;
    pWorker = new GetShortcutWorker;
    if (isCloudService == false) {
        connect(pWorker, &GetShortcutWorker::generalShortcutGenerate, this,
                [=](QString schema, QString key, QString value){
            // qDebug() << "general shortcut" << schema << key << value;
            KeyEntry *generalKeyEntry = new KeyEntry;
            generalKeyEntry->gsSchema = schema;
            generalKeyEntry->keyStr = key;
            generalKeyEntry->valueStr = value;
            generalEntries.append(generalKeyEntry);
        });
    }
    connect(pWorker, &GetShortcutWorker::customShortcutGenerate, this,
            [=](QString path, QString name, QString binding, QString action){
        // qDebug() << "custom shortcut" << path << name << binding;
        KeyEntry *customKeyEntry = new KeyEntry;
        customKeyEntry->gsSchema = KEYBINDINGS_CUSTOM_SCHEMA;
        customKeyEntry->gsPath = path;
        customKeyEntry->nameStr = name;
        customKeyEntry->bindingStr = binding;
        customKeyEntry->actionStr = action;
        customEntries.append(customKeyEntry);
    });
    connect(pWorker, &GetShortcutWorker::workerComplete, this, [=] {
        pThread->quit(); // 退出事件循环
        pThread->wait(); // 释放资源
    });

    pWorker->moveToThread(pThread);
    connect(pThread, &QThread::started, pWorker, &GetShortcutWorker::run);
    connect(pThread, &QThread::finished, this, [=] {
        QMap<QString, QString> systemMap;
        QMap<QString, QString> desktopMap;

        // 最新快捷键数据
        for (int i = 0; i < generalEntries.count(); i++) {
            if (generalEntries[i]->gsSchema == KEYBINDINGS_DESKTOP_SCHEMA) {
                desktopMap.insert(generalEntries[i]->keyStr, generalEntries[i]->valueStr);
            } else if (generalEntries[i]->gsSchema == KEYBINDINGS_SYSTEM_SCHEMA) {
                systemMap.insert(generalEntries[i]->keyStr, generalEntries[i]->valueStr);
            }
        }

        desktopMap = MergerOfTheSamekind(desktopMap);

        QMap<QString, QMap<QString, QString> > generalMaps;
        if (desktopMap.count() != 0) {
            generalMaps.insert("Desktop", desktopMap);
        }
        // 构建系统快捷键界面
        appendGeneralItems(generalMaps);

        // 构建自定义快捷键界面
        appendCustomItems();
        isCloudService = false;
    });
    connect(pThread, &QThread::finished, pWorker, &GetShortcutWorker::deleteLater);

    pThread->start();
}

QMap<QString, QString> Shortcut:: MergerOfTheSamekind(QMap<QString, QString> desktopMap)
{
    QMap<QString, QString>::iterator it = desktopMap.begin();
    for (; it != desktopMap.end(); it++) {
        QString name = it.key().at(it.key().size() - 1);
        QString name_modification = it.key().left(it.key().length() - 1);
        if (name == '2') {
            desktopMap[name_modification] = desktopMap[name_modification]+" or "+it.value();
            desktopMap.erase(it);
            it = desktopMap.begin()+1;// 除之后要将指针指向后面一个
        }
    }
    return desktopMap;
}

QWidget *Shortcut::buildGeneralWidget(QString schema, QMap<QString, QString> subShortcutsMap)
{
    GSettingsSchema *pSettings;
    QString domain;

    if (schema == "Desktop") {
        pSettings
            = g_settings_schema_source_lookup(g_settings_schema_source_new_from_directory(
                                                  "/usr/share/glib-2.0/schemas/",
                                                  g_settings_schema_source_get_default(),
                                                  FALSE, NULL),
                                              KEYBINDINGS_DESKTOP_SCHEMA,
                                              FALSE);
        domain = "ukui-settings-daemon";
    } else if (schema == "System") {
        pSettings
            = g_settings_schema_source_lookup(g_settings_schema_source_new_from_directory(
                                                  "/usr/share/glib-2.0/schemas/",
                                                  g_settings_schema_source_get_default(),
                                                  FALSE, NULL),
                                              KEYBINDINGS_SYSTEM_SCHEMA,
                                              FALSE);
        domain = "gsettings-desktop-schemas";
    } else {
        return NULL;
    }

    QWidget *pWidget = new QWidget;
    pWidget->setAttribute(Qt::WA_DeleteOnClose);
    QVBoxLayout *pVerLayout = new QVBoxLayout(pWidget);
    pVerLayout->setSpacing(0);
    pVerLayout->setContentsMargins(0, 0, 0, 0);

    QMap<QString, QString>::iterator it = subShortcutsMap.begin();

    for (int i = 0; it != subShortcutsMap.end(); it++,i++) {
        if (i != 0) {
            HLineFrame *line = new HLineFrame;
            pVerLayout->addWidget(line);
        }
        QWidget *gWidget = new QWidget;
        gWidget->setFixedHeight(52);
        gWidget->setStyleSheet(
            "QWidget{background: palette(base); border: none; border-radius: 6px}");

        QHBoxLayout *gHorLayout = new QHBoxLayout(gWidget);
        gHorLayout->setSpacing(140);
        gHorLayout->setContentsMargins(16, 0, 16, 0);

        QByteArray ba = domain.toLatin1();
        QByteArray ba1 = it.key().toLatin1();

        GSettingsSchemaKey *keyObj = g_settings_schema_get_key(pSettings, ba1.data());

        char *i18nKey;
        FixLabel *nameLabel = new FixLabel(gWidget);
        i18nKey = const_cast<char *>(g_dgettext(ba.data(), g_settings_schema_key_get_summary(
                                                    keyObj)));

        nameLabel->setText(QString(i18nKey));
        nameLabel->setToolTip(QString(i18nKey));

        FixLabel *bindingLabel = new FixLabel(gWidget);

        bindingLabel->setText(getShowShortcutString(it.value()));
        bindingLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        nameLabel->setText(QString(i18nKey));

        QSizePolicy policy;
        policy = nameLabel->sizePolicy();
        policy.setHorizontalPolicy(QSizePolicy::Ignored);
        nameLabel->setSizePolicy(policy);

        policy = bindingLabel->sizePolicy();
        policy.setHorizontalPolicy(QSizePolicy::Ignored);
        bindingLabel->setSizePolicy(policy);

        gHorLayout->addWidget(nameLabel, 1);
        gHorLayout->addWidget(bindingLabel, 1);

        gWidget->setLayout(gHorLayout);

        pVerLayout->addWidget(gWidget);

        g_settings_schema_key_unref(keyObj);
    }

    g_settings_schema_unref(pSettings);

    return pWidget;
}

void Shortcut::appendGeneralItems(QMap<QString, QMap<QString, QString> > shortcutsMap)
{
    QMap<QString, QMap<QString, QString> >::iterator it = shortcutsMap.begin();
    for (; it != shortcutsMap.end(); it++) {
        QWidget *gWidget = buildGeneralWidget(it.key(), it.value());
        if (gWidget != NULL) {
            ui->showFrame->layout()->addWidget(gWidget);
        }
    }
}

void Shortcut::appendCustomItems()
{
    for (KeyEntry *ckeyEntry : customEntries) {
        buildCustomItem(ckeyEntry);
    }
}

void Shortcut::buildCustomItem(KeyEntry *nkeyEntry)
{
    QFrame *frame = new QFrame;
    frame->setFixedHeight(60);
    ui->verticalLayout_3->addWidget(frame);
    QHBoxLayout *layout = new QHBoxLayout(frame);
    QHBoxLayout *lineEditLayout = new QHBoxLayout;
    DoubleClickLineEdit *nameLineEdit = new DoubleClickLineEdit(&customEntries,frame);
    DoubleClickShortCut *bingdingLineEdit = new DoubleClickShortCut(generalEntries, &customEntries);
    ClickFixLabel *nameLabel = new ClickFixLabel(frame);
    ClickFixLabel *bingdingLabel = new ClickFixLabel(frame);

    // 大小写字母数字中文
    QRegExp rx("[a-zA-Z0-9\u4e00-\u9fa5]+");
    QRegExpValidator *regValidator = new QRegExpValidator(rx);
    // 输入限制
    nameLineEdit->setValidator(regValidator);
    nameLineEdit->setFixedHeight(36);
    bingdingLineEdit->setFixedHeight(36);

    bingdingLineEdit->setAlignment(Qt::AlignRight);
    bingdingLabel->setAlignment(Qt::AlignRight | Qt::AlignCenter);
    HLineFrame *line = new HLineFrame;
    ui->verticalLayout_3->addWidget(line);

    QToolButton *btn = new QToolButton(frame);
    btn->setAutoRaise(true);
    btn->setPopupMode(QToolButton::InstantPopup);
    QMenu *menu = new QMenu(btn);
    btn->setMenu(menu);
    QAction *edit = new QAction(btn);
    QAction *del = new QAction(btn);
    edit->setText(tr("Edit"));
    del->setText(tr("Delete"));
    menu->addAction(edit);
    menu->addAction(del);

    connect(del, &QAction::triggered, this, [=](){
        frame->deleteLater();
        line->deleteLater();
        deleteCustomShortcut(nkeyEntry->gsPath);
        customEntries.removeOne(nkeyEntry);
        Q_EMIT updateCustomShortcut();
    });

    connect(edit, &QAction::triggered, this, [=](){
        addShortcutDialog *addDialog = new addShortcutDialog(generalEntries, customEntries, pluginWidget);
        addDialog->setSourceEnable(false);
        addDialog->setWindowTitle(QObject::tr("Edit Shortcut"));

        addDialog->setExecText(nkeyEntry->actionStr);
        addDialog->setNameText(nkeyEntry->nameStr);
        addDialog->setKeyText(nkeyEntry->bindingStr);
        addDialog->setKeyIsAvailable(3);

        connect(addDialog, &addShortcutDialog::shortcutInfoSignal,
                [=](QString path, QString name, QString exec, QString key){
            Q_UNUSED(path)
            createNewShortcut(nkeyEntry->gsPath, name, exec, key, false);
            nkeyEntry->actionStr = exec;
            nkeyEntry->nameStr   = name;
            nkeyEntry->bindingStr= key;

            nameLineEdit->setToolTip(name);
            nameLineEdit->blockSignals(true);
            nameLineEdit->setText(name);
            nameLineEdit->blockSignals(false);
            bingdingLineEdit->blockSignals(true);
            QString mKey = getShowShortcutString(addDialog->keyToLib(key));
            if (mKey.contains("Meta")) {
                mKey.replace("Meta", "Win");
            }
            bingdingLineEdit->setText(mKey);
            bingdingLineEdit->blockSignals(false);
            nameLabel->setText(name);
            bingdingLabel->setText(bingdingLineEdit->text());
        });

        addDialog->exec();
        Q_EMIT updateCustomShortcut();
    });

    btn->setIcon(QIcon::fromTheme("view-more-horizontal-symbolic"));
    btn->setProperty("useButtonPalette", true);
    btn->setFixedSize(36,36);

    layout->setContentsMargins(8,0,16,0);
    layout->setSpacing(24);
    layout->addLayout(lineEditLayout);
    layout->addWidget(btn);
//    layout->addStretch();

    lineEditLayout->setMargin(0);
    lineEditLayout->setSpacing(140);

    QSizePolicy policy;
    policy = nameLineEdit->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Ignored);
    nameLineEdit->setSizePolicy(policy);

    policy = nameLabel->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Ignored);
    nameLabel->setSizePolicy(policy);

    policy = bingdingLineEdit->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Ignored);
    bingdingLineEdit->setSizePolicy(policy);

    policy = bingdingLabel->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Ignored);
    bingdingLabel->setSizePolicy(policy);

    lineEditLayout->addWidget(nameLineEdit,1);
    lineEditLayout->addWidget(nameLabel,1);
    lineEditLayout->addWidget(bingdingLineEdit,1);
    lineEditLayout->addWidget(bingdingLabel,1);
    nameLineEdit->setVisible(false);
    bingdingLineEdit->setVisible(false);

    nameLineEdit->blockSignals(true);
    nameLineEdit->setText(nkeyEntry->nameStr);
    nameLineEdit->blockSignals(false);
    bingdingLineEdit->blockSignals(true);
    bingdingLineEdit->setText(getShowShortcutString(nkeyEntry->bindingStr));
    bingdingLineEdit->blockSignals(false);

    nameLabel->setText(nkeyEntry->nameStr);
    bingdingLabel->setText(getShowShortcutString(nkeyEntry->bindingStr));


    connect(nameLabel,&ClickFixLabel::doubleClicked,this,[=](){
        nameLabel->hide();
        nameLineEdit->show();
        nameLineEdit->setFocus();
    });

    connect(bingdingLabel,&ClickFixLabel::doubleClicked,this,[=](){
        bingdingLabel->hide();
        bingdingLineEdit->show();
        bingdingLineEdit->setFocus();
    });

    connect(nameLineEdit,&DoubleClickLineEdit::focusOut,this,[=](){
        nameLabel->show();
        nameLineEdit->hide();
    });

    connect(bingdingLineEdit,&DoubleClickShortCut::focusOut,this,[=](){
        bingdingLabel->show();
        bingdingLineEdit->hide();
    });

    connect(nameLineEdit, &DoubleClickLineEdit::strChanged, this, [=](){
        createNewShortcut(nkeyEntry->gsPath, nameLineEdit->text(), nkeyEntry->actionStr, nkeyEntry->bindingStr, false, false); //只修改
        nameLabel->setText(nameLineEdit->text());
        nkeyEntry->nameStr = nameLineEdit->text();
    });

    connect(bingdingLineEdit, &DoubleClickShortCut::shortcutChanged, this, [=](){
        createNewShortcut(nkeyEntry->gsPath, nkeyEntry->nameStr, nkeyEntry->actionStr, bingdingLineEdit->keySequence().toString(), false, true); //只修改
        bingdingLineEdit->blockSignals(true);
        bingdingLineEdit->setText(getShowShortcutString(bingdingLineEdit->text()));
        bingdingLineEdit->blockSignals(false);
        bingdingLabel->setText(bingdingLineEdit->text());
        for (int i = 0; i < customEntries.count(); i++) {
            if (customEntries[i]->nameStr == nkeyEntry->nameStr) {
                nkeyEntry->keyStr = customEntries[i]->keyStr;
                break;
            }
        }
    });;

    return;

}

QString Shortcut::keyToUI(QString key)
{
    if (key.contains("+")) {
        QStringList keys = key.split("+");
        QString keyToUI = keys.join("    ");
        return keyToUI;
    }
    return key;
}

QString Shortcut::keyToLib(QString key)
{
    if (key.contains("+")) {
        QStringList keys = key.split("+");
        if (keys.count() == 2) {
            QString lower = keys.at(1);
            QString keyToLib = "<" + keys.at(0) + ">" + lower.toLower();
            qDebug() << "count = 2,keyToLib = " << keyToLib;
            return keyToLib;
        } else if (keys.count() == 3) {
            QString lower = keys.at(2);
            QString keyToLib = "<" + keys.at(0) + ">" + "<" + keys.at(1) + ">" + lower.toLower();
            qDebug() << "count = 3,keyToLib = " << keyToLib;
            return keyToLib;
        } else if (keys.count() == 4) {
            QString lower = keys.at(3);
            QString keyToLib = "<" + keys.at(0) + ">" + "<" + keys.at(1) + ">" + "<" + keys.at(2) + ">" + lower.toLower();
            qDebug() << "count = 4,keyToLib = " << keyToLib;
            return keyToLib;
        }
    }
    qDebug() << "count = 1,keyToLib = " << key;
    return key;
}

void Shortcut::createNewShortcut(QString path, QString name, QString exec, QString key, bool buildFlag, bool convertFlag)
{
    if (key.contains("Meta")) {
        key.replace("Meta", "Win");
    }
    qDebug() << "createNewShortcut" << path << name << exec << key;
    QString availablepath;
    if (path.isEmpty()) {
        availablepath = findFreePath(); // 创建快捷键

        // 更新数据
        KeyEntry *nKeyentry = new KeyEntry;
        nKeyentry->gsPath = availablepath;
        nKeyentry->nameStr = name;
        if (convertFlag == true) {
            nKeyentry->bindingStr = keyToLib(key);
        } else {
            nKeyentry->bindingStr = key;
        }
        nKeyentry->actionStr = exec;

        customEntries.append(nKeyentry);
        if (true == buildFlag)
            buildCustomItem(nKeyentry);
    } else {
        availablepath = path; // 更新快捷键

        // 更新数据
        int i = 0;
        for (; i < customEntries.count(); i++) {
            if (customEntries[i]->gsPath == availablepath) {
                customEntries[i]->nameStr = name;
                customEntries[i]->actionStr = exec;
                if (convertFlag == true) {
                    customEntries[i]->bindingStr = keyToLib(key);
                } else {
                    customEntries[i]->bindingStr = key;
                }
                break;
            }
        }
    }

    const QByteArray id(KEYBINDINGS_CUSTOM_SCHEMA);
    const QByteArray idd(availablepath.toLatin1().data());
    QGSettings *settings = new QGSettings(id, idd, this);
    if (convertFlag == true) {
         settings->set(BINDING_KEY, keyToLib(key));
    } else {
         settings->set(BINDING_KEY, key);
    }

    settings->set(NAME_KEY, name);
    settings->set(ACTION_KEY, exec);

    delete settings;
    settings = nullptr;
    Q_EMIT updateCustomShortcut();
}

void Shortcut::deleteCustomShortcut(QString path)
{
    if (path.isEmpty())
        return;

    QProcess p(0);
    QStringList args;

    char *fullpath = path.toLatin1().data();
    QString cmd = "dconf";

    args.append("reset");
    args.append("-f");
    args.append(fullpath);
    p.execute(cmd, args);// command是要执行的命令,args是参数
    qDebug()<<"wait for finish";
    p.waitForFinished(-1);
    qDebug()<<QString::fromLocal8Bit(p.readAllStandardError());
}

bool Shortcut::keyIsForbidden(QString key)
{
    for (int i = 0; i < forbiddenKeys.length(); i++) {
        if (key == forbiddenKeys[i])
            return true;
    }
    return false;
}

void Shortcut::shortcutChangedSlot()
{
    qDebug() << "receive cloud service signal";
    while (ui->verticalLayout_3->count()) {
        QWidget *p = ui->verticalLayout_3->takeAt(0)->widget();
        ui->verticalLayout_3->removeWidget(p);
        p->deleteLater();
    }
    isCloudService = true;
    initFunctionStatus();
}

QString Shortcut::getShowShortcutString(QString src) {
    src.replace("<","");
    src.replace(">","   ");
    src.replace(" or ",tr(" or "));
    QStringList temp_list = src.split(" ");
    QString str;
    for(qint32 i = 0; i < temp_list.count(); i++) {
        str += temp_list.at(i)
                    .left(1).toUpper() +
                    temp_list.at(i)
                    .mid(1, temp_list.at(i).length() - 1).toLower();
        str += " ";
    }
    str.replace("Or","or");
    return  str;
}

