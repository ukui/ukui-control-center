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
#include "fonts.h"
#include "ui_fonts.h"
#include "shell/utils/mthread.h"

#include <QLabel>
#include <QStringList>
#include <QDebug>

#define INTERFACE_SCHEMA   "org.mate.interface"
#define DOC_FONT_KEY       "document-font-name"  // 用于阅读文档的默认字体的名称
#define GTK_FONT_KEY       "font-name"           // gkt+使用的默认字体
#define MONOSPACE_FONT_KEY "monospace-font-name" // 用于终端等处的等宽字体

#define STYLE_FONT_SCHEMA  "org.ukui.style"
#define SYSTEM_FONT_EKY    "system-font-size"
#define SYSTEM_NAME_KEY    "system-font"

#define UKCC_SCHEMA        "org.ukui.control-center"
#define UKCC_FONTS_EKY     "fonts-list"

QList<int> defaultsizeList = {6, 7, 8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72};

Fonts::Fonts() : mFirstLoad(true)
{
    pluginName = tr("Fonts");
    pluginType = PERSONALIZED;
}

Fonts::~Fonts()
{
    if (!mFirstLoad) {
        delete ui;
        ui = nullptr;
    }
}

QString Fonts::plugini18nName() {
    return pluginName;
}

int Fonts::pluginTypes() {
    return pluginType;
}

QWidget *Fonts::pluginUi() {
    if (mFirstLoad) {
        mFirstLoad = false;

        ui = new Ui::Fonts;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);
        ui->titleLabel->setContentsMargins(14, 0, 0, 0);

        initSearchText();

        const QByteArray styleID(STYLE_FONT_SCHEMA);
        const QByteArray id(INTERFACE_SCHEMA);
        const QByteArray ukccID(UKCC_SCHEMA);

        if (QGSettings::isSchemaInstalled(id) &&
            QGSettings::isSchemaInstalled(styleID)) {

            ifsettings    = new QGSettings(id, QByteArray(), this);
            stylesettings = new QGSettings(styleID, QByteArray(), this);
            mUkccSettings = new QGSettings(ukccID, QByteArray(), this);

            initModel();
            setupComponent();
            setupConnect();
            initFontStatus();
        }
    }
    return pluginWidget;
}

const QString Fonts::name() const {
    return QStringLiteral("Fonts");
}

bool Fonts::isShowOnHomePage() const
{
    return true;
}

QIcon Fonts::icon() const
{
    return QIcon::fromTheme("ukui-font-symbolic");
}

bool Fonts::isEnable() const
{
    return true;
}

void Fonts::initSearchText() {
    //~ contents_path /Fonts/Font size
    ui->fontSizeLabel->setText(tr("Font size"));
    //~ contents_path /Fonts/Fonts select
    ui->fontSelectLabel->setText(tr("Fonts select"));
    //~ contents_path /Fonts/Mono font
    ui->monoSelectLabel->setText(tr("Mono font"));
}

void Fonts::setupComponent(){
    QStringList fontScale;
    fontScale<< "10" << "11" << "12" << "13" << "14"
              <<"15";

    uslider  = new Uslider(fontScale);
    uslider->setRange(1,6);
    uslider->setTickInterval(1);
    uslider->setPageStep(1);

    ui->fontLayout->addWidget(uslider);
}

void Fonts::setupConnect(){
    connectToServer();
    connect(uslider, &QSlider::valueChanged, [=](int value){
        int size = sliderConvertToSize(value);

        // 获取当前字体信息
        _getCurrentFontInfo();

        // 设置字体大小
        ifsettings->set(GTK_FONT_KEY, QVariant(QString("%1 %2").arg(gtkfontStrList.at(0)).arg(size)));
        ifsettings->set(DOC_FONT_KEY, QVariant(QString("%1 %2").arg(docfontStrList.at(0)).arg(size)));
        ifsettings->set(MONOSPACE_FONT_KEY, QVariant(QString("%1 %2").arg(monospacefontStrList.at(0)).arg(size)));
        stylesettings->set(SYSTEM_FONT_EKY, QVariant(QString("%1").arg(size)));
        fontKwinSlot();
    });

    connect(ui->fontSelectComBox, &QComboBox::currentTextChanged, [=](QString text){
        // 获取当前字体信息
        _getCurrentFontInfo();
        ifsettings->set(GTK_FONT_KEY, QVariant(QString("%1 %2").arg(text).arg(gtkfontStrList.at(1))));
        ifsettings->set(DOC_FONT_KEY, QVariant(QString("%1 %2").arg(text).arg(docfontStrList.at(1))));        
        stylesettings->set(SYSTEM_NAME_KEY, QVariant(QString("%1").arg(text)));

        fontKwinSlot();
    });
    connect(ui->monoSelectComBox, &QComboBox::currentTextChanged, [=](QString text){
        // 获取当前字体信息
        _getCurrentFontInfo();
        ifsettings->set(MONOSPACE_FONT_KEY, QVariant(QString("%1 %2").arg(text).arg(monospacefontStrList.at(1))));
    });

    // 重置按钮
    connect(ui->resetBtn, &QPushButton::clicked, [=](bool checked){
        Q_UNUSED(checked)
        resetDefault();
    });

    // 监听系统字体的变化
    connect(stylesettings , &QGSettings::changed, this, [=](const QString &key) {
         if("systemFont" == key || "systemFontSize" == key) {
             uslider->setValue(stylesettings->get(SYSTEM_FONT_EKY).toInt() - 9);
             ui->fontSelectComBox->setCurrentText(stylesettings->get(SYSTEM_NAME_KEY).toString());
             ui->monoSelectComBox->setCurrentText(ifsettings->get(MONOSPACE_FONT_KEY).toString());
         }
    });

    //监听终端字体的变化
    connect(ifsettings , &QGSettings::changed, this, [=](const QString &key) {
        if ("monospaceFontName" == key) {
            QString str = ifsettings->get(MONOSPACE_FONT_KEY).toString();
            ui->monoSelectComBox->setCurrentText(str.mid(0 , str.size() - 3));
        }
    });

    connect(mUkccSettings, &QGSettings::changed, this, [=](const QString &key) {
        if ("fontsList" == key) {
            updateFontListSlot(mUkccSettings->get(key).toStringList());
        }
    });
}

void Fonts::initFontStatus(){
    // 初始化字体ComBox状态
    initGeneralFontStatus();
}

void Fonts::initGeneralFontStatus(){
    // 获取当前字体信息
    _getCurrentFontInfo();

    // 初始化字体选择ComBox(选择gtk默认字体作为标准)
    QString currentfonts = gtkfontStrList[0];
    ui->fontSelectComBox->blockSignals(true);
    ui->fontSelectComBox->setCurrentText(currentfonts);
    ui->fontSelectComBox->blockSignals(false);

    // 初始化等宽字体ComBox
    QString currentmonofont = monospacefontStrList[0];
    if ("DejaVu sans Mono" == currentmonofont) {
        currentmonofont = "DejaVu Sans Mono";
    }

    ui->monoSelectComBox->blockSignals(true);
    ui->monoSelectComBox->setCurrentText(currentmonofont);
    ui->monoSelectComBox->blockSignals(false);

    int size = fontConvertToSlider(QString(docfontStrList[1]).toInt());

    uslider->blockSignals(true);
    uslider->setValue(size);
    uslider->blockSignals(false);
}

void Fonts::_getCurrentFontInfo(){
    gtkfontStrList = _splitFontNameSize(ifsettings->get(GTK_FONT_KEY).toString());
    docfontStrList = _splitFontNameSize(ifsettings->get(DOC_FONT_KEY).toString());
    monospacefontStrList = _splitFontNameSize(ifsettings->get(MONOSPACE_FONT_KEY).toString());
}

QStringList Fonts::_splitFontNameSize(QString value) {
    QStringList valueStringList;
    QString str = value.right(1);
    QString ch0 = static_cast<QString>('0');
    QString ch9 = static_cast<QString>('9');
    if (str >= ch0 && str <= ch9) {
        QStringList tmpStringList = value.split(' ');
        QString::SectionFlag flag = QString::SectionSkipEmpty;
        valueStringList << value.section(' ', 0, tmpStringList.length() - 2, flag);
        valueStringList << value.section(' ', tmpStringList.length() - 1, tmpStringList.length() - 1, flag);
    } else {
        valueStringList << value << "10";
    }
    return valueStringList;
}

int Fonts::fontConvertToSlider(const int size) const {
    switch (size) {
    case 10:
        return 1;
        break;
    case 11:
        return 2;
        break;
    case 12:
        return 3;
        break;
    case 13:
        return 4;
        break;
    case 14:
        return 5;
        break;
    case 15:
        return 6;
        break;
    case 16:
        return 7;
        break;
    case 17:
        return 8;
        break;
    default:
        return 1;
        break;
    }
}

int Fonts::sliderConvertToSize(const int value) const
{
    switch (value) {
    case 1:
        return 10;
        break;
    case 2:
        return 11;
        break;
    case 3:
        return 12;
        break;
    case 4:
        return 13;
        break;
    case 5:
        return 14;
        break;
    case 6:
        return 15;
        break;
    case 7:
        return 16;
        break;
    case 8:
        return 17;
    default:
        return 10;
        break;
    }
}

void Fonts::resetDefault(){
    // Reset font
    ifsettings->reset(GTK_FONT_KEY);
    ifsettings->reset(DOC_FONT_KEY);
    ifsettings->reset(MONOSPACE_FONT_KEY);
    stylesettings->set(SYSTEM_FONT_EKY, 11);
    stylesettings->reset(SYSTEM_NAME_KEY);

    // 更新全部状态
    initFontStatus();
    fontKwinSlot();
}

void Fonts::connectToServer(){
    QThread *NetThread = new QThread;
    MThread *NetWorker = new MThread;
    NetWorker->moveToThread(NetThread);
    connect(NetThread, &QThread::started, NetWorker, &MThread::run);
    connect(NetWorker,&MThread::keychangedsignal,this,&Fonts::keyChangedSlot);
    connect(NetThread, &QThread::finished, NetWorker, &MThread::deleteLater);
     NetThread->start();
}

void Fonts::keyChangedSlot(const QString &key) {
    if(key == "font") {
        initFontStatus();
    }
}

void Fonts::updateFontListSlot(const QStringList &fontList)
{
    QStringList currentFontList;
    for (int i = 0; i < ui->fontSelectComBox->count(); i++) {
        currentFontList << ui->fontSelectComBox->itemText(i);
    }

    if (fontList.length() > currentFontList.length()) {
        for (int i = 0; i < fontList.length(); i++) {
            QString font = fontList.at(i);
            if (!currentFontList.contains(font)) {
                QStandardItem *monoItem = new QStandardItem(font);
                monoItem->setFont(QFont(font));
                mFontModel->appendRow(monoItem);
            }
        }
    } else {
        for (int i = 0; i < currentFontList.length(); i++) {
            QString font = currentFontList.at(i);
            if (!fontList.contains(font)) {
                mFontModel->removeRow(i);
            }
        }
    }
}

void Fonts::fontKwinSlot() {
    const int fontSize = sliderConvertToSize(uslider->value());
    const QString fontType = ui->fontSelectComBox->currentText();
    qDebug() << fontSize << fontType;
    QDBusMessage message =QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "slotFontChange");
    QList<QVariant> args;
    args.append(fontSize);
    args.append(fontType);
    message.setArguments(args);
    QDBusConnection::sessionBus().send(message);
}

void Fonts::initModel()
{
    mFontsList = mUkccSettings->get(UKCC_FONTS_EKY).toStringList();

    ui->fontSelectComBox->setModel(new QStandardItemModel());
    mFontModel = dynamic_cast<QStandardItemModel *>(ui->fontSelectComBox->model());

    ui->monoSelectComBox->setModel(new QStandardItemModel());
    mMonoModel = dynamic_cast<QStandardItemModel *>(ui->monoSelectComBox->model());

    // 导入系统字体列表
    QStringList fontfamiles = fontdb.families();
    QStringList actFontsList;
    for (QString fontValue : fontfamiles) {
        if (mFontsList.contains(fontValue)) {
            QStandardItem *standardItem = new QStandardItem(fontValue);
            standardItem->setFont(QFont(fontValue));
            mFontModel->appendRow(standardItem);
            actFontsList << fontValue;
        }

        if (fontValue.contains("Mono") && !fontValue.contains("Ubuntu",Qt::CaseInsensitive)) {
            QStandardItem *monoItem = new QStandardItem(fontValue);
            monoItem->setFont(QFont(fontValue));
            mMonoModel->appendRow(monoItem);
        }
    }

    mUkccSettings->set(UKCC_FONTS_EKY, actFontsList);
}
