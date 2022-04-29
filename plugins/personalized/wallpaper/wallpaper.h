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
#ifndef WALLPAPER_H
#define WALLPAPER_H

#include <QObject>
#include <QtPlugin>

#include <QThread>
#include <QPixmap>
#include <QListWidgetItem>
#include <QFileDialog>
#include <QStyledItemDelegate>

#include <QGSettings>
#include <QSettings>
#include <QString>

#include "shell/interface.h"
#include <ukcc/widgets/flowlayout.h>
#include <ukcc/widgets/hoverwidget.h>
#include <ukcc/widgets/imageutil.h>
#include "xmlhandle.h"
#include "component/custdomitemmodel.h"
#include "simplethread.h"
#include "workerobject.h"
#include "colordialog.h"
#include "pictureunit.h"
#include "../../../shell/utils/utils.h"
/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

#include <glib.h>
#include <gio/gio.h>

#define BACKGROUND "org.mate.background"

// 图片文件路径
#define FILENAME "picture-filename"
// 图片不透明度
#define OPACITY "picture-opacity"
// 图片放置方式
#define OPTIONS "picture-options"
// 主色
#define PRIMARY "primary-color"
// 副色
#define SECONDARY "secondary-color"

const QString ScaledKey    = "scaled";    // 填充
const QString WallpaperKey = "wallpaper"; // 平铺
const QString CenteredKey  = "centered";  // 居中
const QString StretchedKey = "stretched"; // 拉伸


namespace Ui {
class Wallpaper;
}

class Wallpaper : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Wallpaper();
    ~Wallpaper();

    QString plugini18nName() Q_DECL_OVERRIDE;
    int pluginTypes() Q_DECL_OVERRIDE;
    QWidget * pluginUi() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;

public:
    void initSearchText();
    void setupComponent();
    void setupConnect();
    void initBgFormStatus();
    int _getCurrentBgForm();
    void initPreviewStatus();

    void showLocalWpDialog();

    void showComponent(int index);

    void setClickedPic(QString fileName);
private:
    void setLockBackground(QString bg);
    void initBgOption(); // 背景放置方式
    void hideComponent();

private:
    Ui::Wallpaper *ui;
    PictureUnit * prePicUnit;
    QString pluginName;
    int pluginType;
    QWidget * pluginWidget;

private:
    FlowLayout * picFlowLayout;
    FlowLayout * colorFlowLayout;
    int picNum = 0;
private:

    QMap<QString, QMap<QString, QString> > wallpaperinfosMap;

    XmlHandle * xmlhandleObj;
    ColorDialog * colordialog;
    QGSettings * bgsettings;
    QSettings  *mLockLoginSettings;
    QString localwpconf;
    QMap<QString, QListWidgetItem*> delItemsMap;

    CustdomItemModel wpListModel;

    //尝试mode view
    void setModeldata();

private:
    QThread * pThread;
    WorkerObject * pObject;

    QMap<QString, QListWidgetItem*> picWpItemMap;

    bool settingsCreate;
    bool mFirstLoad;

public slots:
    void resetDefaultWallpaperSlot();
    void wpOptionsChangedSlot(QString op);
    void colorSelectedSlot(QColor color);

    void add_custom_wallpaper();
    void del_wallpaper();
};

#endif // WALLPAPER_H
