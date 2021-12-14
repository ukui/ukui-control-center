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
#ifndef CHANGEFACEINTELDIALOG_H
#define CHANGEFACEINTELDIALOG_H

#include <QDialog>
#include <QObject>
#include <QDir>
#include <QListWidgetItem>
#include <QFileDialog>
#include <QPainter>
#include <QPainterPath>
#include <QDBusInterface>
#include <QDBusReply>
#include <ukcc/widgets/flowlayout.h>
#include <QButtonGroup>
#include <QGSettings/QGSettings>
#include <ukcc/widgets/imageutil.h>

#define UKUI_QT_STYLE                      "org.ukui.style"
#define UKUI_STYLE_KEY                     "style-name"

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

extern "C" {
#include <glib.h>
#include <gio/gio.h>
}

namespace Ui {
class ChangeFaceIntelDialog;
}

class ChangeFaceIntelDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeFaceIntelDialog(QWidget *parent = 0);
    ~ChangeFaceIntelDialog();

    void setFace(QString iconfile);
    void setUsername(QString username);
    void setRealname(QString realname);
    void setAccountType(QString atype);
    void setHistoryFacesPath(QString path);

    void loadSystemFaces();

    void showLocalFaceDialog();

    QMap<QString, QListWidgetItem *> delitemMap;

    QString confirmFile;
    void loadHistoryFaces();
    QString historyFacesPath;
    QDBusInterface * sysinterface;
    int historyCount;
    FlowLayout * historyFacesFlowLayout;

protected:
    void paintEvent(QPaintEvent *);

private:
    Ui::ChangeFaceIntelDialog *ui;
    QString m_username;
    QPixmap PixmapToRound(const QPixmap &src, int radius);
    QButtonGroup *btnsGroup;
    QPushButton *old_delBtn = nullptr;
    QPixmap pixmapAdjustLabel(QString iconfile);
    QPushButton *m_closeBtn;
    bool m_isNightMode;
    QGSettings *m_style = nullptr;
    void getThemeStyle(QString key);
Q_SIGNALS:
    void face_file_send(QString file, QString username);
};

#endif // CHANGEFACEINTELDIALOG_H
