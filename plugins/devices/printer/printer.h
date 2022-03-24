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
#ifndef PRINTER_H
#define PRINTER_H


#include <QObject>
#include <QtPlugin>
#include <QTimer>
#include <QListWidget>

#include "shell/interface.h"
#include <ukcc/widgets/hoverwidget.h>
#include <ukcc/widgets/imageutil.h>
#include <ukcc/widgets/hoverbtn.h>
#include <ukcc/widgets/titlelabel.h>
#include <ukcc/widgets/fixlabel.h>
#include <ukcc/widgets/addbtn.h>

namespace Ui {
class Printer;
}

class Printer : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Printer();
    ~Printer();

    QString plugini18nName() Q_DECL_OVERRIDE;
    int pluginTypes() Q_DECL_OVERRIDE;
    QWidget * pluginUi() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;

public:
    void initUi(QWidget *widget);
    void initPrinterUi();
    void initTitleLabel();
    void initComponent();
    void runExternalApp();
    void clearAutoItem();

private:
    QString pluginName;
    int pluginType;
    QWidget * pluginWidget;
    QWidget *PrinterWidget;
    AddBtn * mAddWgt;
    QPushButton *mAddBtn;

    QFrame *mPrinterListFrame;

    TitleLabel *mPrinterLabel;
    QListWidget *mPrinterListWidget;
    QStringList mPrinterList;
    QVBoxLayout *mPrinterListLayout;

    bool mFirstLoad;

    QTimer *mTimer;

public slots:
    void refreshPrinterDevSlot();

signals:
    void addsignal(QString);
    void removesignal(QString);

};

#endif // PRINTER_H
