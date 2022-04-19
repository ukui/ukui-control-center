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
#include "ukccabout.h"
#include "../utils/xatom-helper.h"

#include <QVariant>
#include <QDesktopServices>
#include <QUrl>
#include <QMouseEvent>
#include <QDebug>

UkccAbout::UkccAbout(QWidget *parent)
    : QDialog(parent) {

    setFixedSize(420, 344);
    setWindowTitle(tr("Settings"));
    initUI();
}

bool UkccAbout::eventFilter(QObject *watch, QEvent *event)
{
    if ( watch == mTipLabel_2) {
        if (event->type() == QEvent::MouseButtonPress){
            QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton){
                QDesktopServices::openUrl(QUrl(QLatin1String("mailto:support@kylinos.cn")));
            }
        }
    }
    return QDialog::eventFilter(watch, event);
}

void UkccAbout::initUI() {
    QVBoxLayout *Lyt = new QVBoxLayout(this);
    Lyt->setContentsMargins(40, 32, 32, 32);
    Lyt->setSpacing(16);

    mIconLabel = new QLabel(this);
    mIconLabel->setFixedSize(96, 96);
    mIconLabel->setPixmap(QIcon::fromTheme("ukui-control-center").pixmap(96, 96));

    mAppLabel = new QLabel(tr("Settings"),this);
    mAppLabel->setStyleSheet("QLabel{font-size: 18px; font-weight: bold}");

    QHBoxLayout *Lyt_1 = new QHBoxLayout();
    Lyt_1->setSpacing(0);
    mVersionLabel_1 = new LightLabel(tr("Version: "), this);
    mVersionLabel_2 = new LightLabel(getUkccVersion(), this);

    Lyt_1->addStretch();
    Lyt_1->addWidget(mVersionLabel_1);
    Lyt_1->addWidget(mVersionLabel_2);
    Lyt_1->addStretch();

    QHBoxLayout *Lyt_2 = new QHBoxLayout();
    Lyt_2->setSpacing(8);
    mTipLabel_1 = new LightLabel(tr("Service and Support:"), this);
    mTipLabel_2 = new LightLabel("support@kylinos.cn", this);
    mTipLabel_2->setStyleSheet("LightLabel{text-decoration: underline}");
    mTipLabel_2->installEventFilter(this);

    Lyt_2->addStretch();
    Lyt_2->addWidget(mTipLabel_1);
    Lyt_2->addWidget(mTipLabel_2);
    Lyt_2->addStretch();



    Lyt->addWidget(mIconLabel, 0, Qt::AlignHCenter);
    Lyt->addSpacing(8);
    Lyt->addWidget(mAppLabel, 0, Qt::AlignHCenter);
    Lyt->addLayout(Lyt_1);
    Lyt->addLayout(Lyt_2);
    Lyt->addStretch();
}

QString UkccAbout::getUkccVersion() {
    FILE *pp = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char *q = NULL;
    QString version = "none";

    pp = popen("dpkg -l  ukui-control-center | grep  ukui-control-center", "r");
    if(NULL == pp)
        return version;

    while((read = getline(&line, &len, pp)) != -1){
        q = strrchr(line, '\n');
        *q = '\0';

        QString content = line;
        QStringList list = content.split(" ");

        list.removeAll("");

        if (list.size() >= 3)
            version = list.at(2);
    }

    free(line);
    line = NULL;
    pclose(pp);
    return version;
}
