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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDir>
#include <QMainWindow>
#include <QPaintEvent>
#include <QShowEvent>
#include <QMoveEvent>
#include <QHideEvent>
#include <QPainter>
#include <QAction>
#include <QSvgRenderer>
#include <QToolTip>
#include <QPropertyAnimation>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

#include "interface.h"
#include "modulepagewidget.h"
#include "searchwidget.h"
class BorderShadowEffect;
class QLabel;
class QPushButton;
class QButtonGroup;
class KeyValueConverter;
class QWidgetResizeHandler;
class QStackedWidget;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public:
    QMap<QString, QObject *> exportModule(int);

    void bootOptionsFilter(QString opt);
    void bootOptionsSwitch(int moduleNum, int funcNum);
protected:
    void validBorder();
    void paintEvent(QPaintEvent *);
    bool eventFilter(QObject *watched, QEvent *event);
//    QRect sideBarRect();
private:
    Ui::MainWindow *ui;
    QDBusReply<bool> is_tabletmode;
    QDBusInterface *m_statusSessionDbus;

private:
    BorderShadowEffect *m_effect;
    ModulePageWidget * modulepageWidget;
    SearchWidget      * m_searchWidget;
    QButtonGroup * leftBtnGroup;
    QButtonGroup * leftMicBtnGroup; //
    bool bIsFullScreen;

    QDir pluginsDir;
//    QStringList modulesStringList;
    QList<int> moduleIndexList;
    QList<QMap<QString, QObject *>> modulesList;

    KeyValueConverter * kvConverter;

    QPushButton       * minBtn;
    QPushButton       * maxBtn;
    QPushButton       * closeBtn;

    QLabel            * blankLabel1;
    QLabel            * blankLabel2;
    QLabel            * blankLabel3;

    QLabel            * titleLabel;
    QLabel            *icon;
    QLabel            *m_queryIcon;
    QLabel            *m_queryText=nullptr;
    QPropertyAnimation *m_animation=nullptr;
    QWidget           *m_queryWid=nullptr;
    bool              m_isSearching;
    QString           m_searchKeyWords;
    QGSettings        *gsettings;
    QGSettings        *themeSetting;
    char              style[100];
private:
    void initMdm();
    void initTileBar();
    void setBtnLayout(QPushButton * &pBtn);
    void loadPlugins();
    void initShortcutKey();
    QPushButton * buildLeftsideBtn(QString bname, QString tipName);

    // load svg picture
    const QPixmap loadSvg(const QString &fileName, QString color);
    const QPixmap loadSvg_1(const QString &fileName, const int size);
    //Render icon from theme
    const QPixmap renderSvg(const QIcon &icon, QString color);
    // chang svg picture's color
    QPixmap drawSymbolicColoredPixmap(const QPixmap &source, QString color);

    bool dblOnEdge(QMouseEvent *event);
    void initStyleSheet();
    void initConnection();
    void setAppStyle();

public slots:
    void switchPage(QString moduleName);
    void animationFinishedSlot();
    void sltMessageReceived(const QString &msg);
    void pageChangeSlot();
    void mainWindow_statusDbusSlot(bool tablet_mode);
signals:
    void btnclicked();
};

#endif // MAINWINDOW_H
