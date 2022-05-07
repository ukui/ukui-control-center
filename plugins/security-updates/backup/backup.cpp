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
#include "backup.h"
#include "ui_backup.h"

#include <QProcess>
#include <QFile>
#include <QDebug>

#include "../../../shell/utils/utils.h"

const QString kDesktopfp = "/usr/share/applications/yhkylin-backup-tools.desktop";

#ifdef signals
#undef signals
#endif

extern "C" {
#include <gio/gdesktopappinfo.h>
#include <glib.h>
}

Backup::Backup() : mFirstLoad(true)
{
    pluginName = tr("Backup");
    pluginType = UPDATE;
}

Backup::~Backup()
{

}

QString Backup::plugini18nName()
{
    return pluginName;
}

int Backup::pluginTypes()
{
    return pluginType;
}

QWidget *Backup::pluginUi()
{
    if (mFirstLoad) {
        mFirstLoad = false;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        initUi(pluginWidget);
        initTitleLabel();
        initConnection();
    }
    return pluginWidget;
}

const QString Backup::name() const
{
    return QStringLiteral("Backup");
}

bool Backup::isShowOnHomePage() const
{
    return true;
}

QIcon Backup::icon() const
{
    return QIcon();
}

bool Backup::isEnable() const
{
    return QFile::exists(kDesktopfp);
}

void Backup::initUi(QWidget *widget)
{
    QVBoxLayout *mverticalLayout = new QVBoxLayout(widget);
    mverticalLayout->setSpacing(8);
    mverticalLayout->setContentsMargins(0, 0, 0, 0);

    mTitleLabel = new TitleLabel(widget);

    QFrame *mBackFrame = new QFrame(widget);
    mBackFrame->setMinimumSize(QSize(550, 80));
    mBackFrame->setMaximumSize(QSize(16777215, 80));
    mBackFrame->setFrameShape(QFrame::Box);
    QHBoxLayout *mBackLayout = new QHBoxLayout(mBackFrame);
    mBackLayout->setContentsMargins(16, 0, 16, 0);
    mBackLayout->setSpacing(48);
    mBackBtn = new FixButton(widget);
    mBackBtn->setFixedWidth(120);
    FixLabel *mBackLabel_1 = new FixLabel(tr("Backup") , mBackFrame);
    mBackLabel_1->setContentsMargins(0 , 12 , 0 , 0);
    LightLabel *mBackLabel_2 = new LightLabel(tr("Back up your files to other drives and restore them when the source files are lost, "
                                                                                             "damaged, or deleted to ensure the integrity of the system.") , mBackFrame);
    mBackLabel_2->setContentsMargins(0 , 0 , 0 , 12);
    QVBoxLayout *mverticalLayout_1 = new QVBoxLayout;
    mverticalLayout_1->setSpacing(0);
    mverticalLayout_1->setContentsMargins(0 , 0 , 0 , 0);
    mverticalLayout_1->addWidget(mBackLabel_1);
    mverticalLayout_1->addWidget(mBackLabel_2 );

    mBackLayout->addLayout(mverticalLayout_1);
    mBackLayout->addWidget(mBackBtn);

    QFrame *mRestoreFrame = new QFrame(widget);
    mRestoreFrame->setMinimumSize(QSize(550, 80));
    mRestoreFrame->setMaximumSize(QSize(16777215, 80));
    mRestoreFrame->setFrameShape(QFrame::Box);
    QHBoxLayout *mRestoreLayout = new QHBoxLayout(mRestoreFrame);
    mRestoreLayout->setContentsMargins(16, 0, 16, 0);
    mRestoreLayout->setSpacing(48);
    mRestoreBtn = new FixButton(widget);
    mRestoreBtn->setFixedWidth(120);
    FixLabel *mRestoreLabel_1 = new FixLabel(tr("Restore") , mRestoreFrame);
    mRestoreLabel_1->setContentsMargins(0 , 12 , 0 , 0);
    LightLabel *mRestoreLabel_2 = new LightLabel(tr("View the backup list and restore the backup file to the system") , mRestoreFrame);
    mRestoreLabel_2->setContentsMargins(0 , 0 , 0 , 12);
    QVBoxLayout *mverticalLayout_2 = new QVBoxLayout;
    mverticalLayout_2->setSpacing(0);
    mverticalLayout_2->setContentsMargins(0 , 0 , 0 , 0);
    mverticalLayout_2->addWidget(mRestoreLabel_1);
    mverticalLayout_2->addWidget(mRestoreLabel_2 );

    mRestoreLayout->addLayout(mverticalLayout_2);
    mRestoreLayout->addWidget(mRestoreBtn);

    mverticalLayout->addWidget(mTitleLabel);
    mverticalLayout->addWidget(mBackFrame);
    mverticalLayout->addWidget(mRestoreFrame);
    mverticalLayout->addStretch();
}

void Backup::initTitleLabel()
{
    mTitleLabel->setText(tr("Backup and Restore"));
     //~ contents_path /Backup/Begin backup
    mBackBtn->setText(tr("Begin backup"));
     //~ contents_path /Backup/Begin restore
    mRestoreBtn->setText(tr("Begin restore"));
}

void Backup::initConnection()
{
    if (Utils::isCommunity()) {
        connect(mBackBtn, &QPushButton::clicked, this, [=](bool checked){
            Q_UNUSED(checked)
            communitySlot();
        });

        connect(mRestoreBtn, &QPushButton::clicked, this, [=](bool checked){
            Q_UNUSED(checked)
            communitySlot();
        });
    } else {
        connect(mBackBtn, &QPushButton::clicked, this, [=](bool checked){
            Q_UNUSED(checked)
            btnClickedSlot();
        });

        connect(mRestoreBtn, &QPushButton::clicked, this, [=](bool checked){
            Q_UNUSED(checked)
            restoreSlot();
        });
    }
}

void Backup::btnClickedSlot()
{
    QString desktopfp = "/usr/share/applications/yhkylin-backup-tools.desktop";
    GDesktopAppInfo *desktopAppInfo = g_desktop_app_info_new_from_filename(desktopfp.toLocal8Bit().data());
    g_app_info_launch(G_APP_INFO(desktopAppInfo), nullptr, nullptr, nullptr);
    g_object_unref(desktopAppInfo);
}

void Backup::restoreSlot()
{
    QString desktopfp = "/usr/share/applications/yhkylin-backup-tools.desktop";
    GDesktopAppInfo *desktopAppInfo = g_desktop_app_info_new_from_filename(desktopfp.toLocal8Bit().data());

    GList *arg = NULL;
    arg = g_list_append(arg, gpointer("--restore"));
    g_app_info_launch_uris(G_APP_INFO(desktopAppInfo), arg, nullptr, nullptr);
    g_object_unref(desktopAppInfo);
}

void Backup::communitySlot()
{
    QString cmd = "/usr/bin/deja-dup";
    QProcess process(this);
    process.startDetached(cmd);
}
