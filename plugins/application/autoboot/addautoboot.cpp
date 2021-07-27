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
#include "addautoboot.h"
#include "CloseButton/closebutton.h"

#include <QDebug>
#include <QFileInfo>
#include <QSettings>
#include <QPushButton>
#include <QApplication>

// #define DESKTOPPATH "/etc/xdg/autostart/"
#define DESKTOPPATH "/usr/share/applications/"

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

AddAutoBoot::AddAutoBoot(QWidget *parent) :
    QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    initUi(this);
    initStyle();
    initConnection();
}

void AddAutoBoot::resetBeforeClose()
{
    userEditNameFlag = false;
    userEditCommentFlag = false;
    mCertainBtn->setEnabled(false);
    mHintLabel->clear();
    mAppNameEdit->setToolTip("");
    mAppBewriteEdit->setToolTip("");
    mAppPathEdit->setToolTip("");
    mAppNameEdit->setText(QString());
    mAppBewriteEdit->setText(QString());
    mAppPathEdit->setText(QString());
    close();
}

bool AddAutoBoot::getFilename(GDir *dir,const char *Name)
{
    QString filedir = "/etc/xdg/autostart/";
    const char *desktopName;
    if (dir) {
        while ((desktopName = g_dir_read_name(dir))) {
            QString filePath = filedir + QString::fromUtf8(desktopName);
            GKeyFile *keyfile = g_key_file_new();
            if (!g_key_file_load_from_file(keyfile, filePath.toLatin1().data(), G_KEY_FILE_NONE, NULL)) {
                g_key_file_free(keyfile);
                return false;
            }
            QString name = g_key_file_get_string(keyfile, G_KEY_FILE_DESKTOP_GROUP,
                                                G_KEY_FILE_DESKTOP_KEY_NAME, NULL);
            g_key_file_free(keyfile);
            if (name == QString::fromUtf8(Name)) {
               return true;
            }
        }
    }
    g_dir_close(mdir);
    return false;
}

void AddAutoBoot::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect().adjusted(10, 10, -10, -10), 10, 10);

    // 画一个黑底
    QPixmap pixmap(this->rect().size());
    pixmap.fill(Qt::transparent);
    QPainter pixmapPainter(&pixmap);
    pixmapPainter.setRenderHint(QPainter::Antialiasing);
    pixmapPainter.setPen(Qt::transparent);
    pixmapPainter.setBrush(Qt::black);
    pixmapPainter.setOpacity(0.65);
    pixmapPainter.drawPath(rectPath);
    pixmapPainter.end();

    // 模糊这个黑底
    QImage img = pixmap.toImage();
    qt_blurImage(img, 10, false, false);

    // 挖掉中心
    pixmap = QPixmap::fromImage(img);
    QPainter pixmapPainter2(&pixmap);
    pixmapPainter2.setRenderHint(QPainter::Antialiasing);
    pixmapPainter2.setCompositionMode(QPainter::CompositionMode_Clear);
    pixmapPainter2.setPen(Qt::transparent);
    pixmapPainter2.setBrush(Qt::transparent);
    pixmapPainter2.drawPath(rectPath);

    // 绘制阴影
    p.drawPixmap(this->rect(), pixmap, pixmap.rect());

    // 绘制一个背景
    p.save();
    p.fillPath(rectPath, palette().color(QPalette::Base));
    p.restore();
}

void AddAutoBoot::initUi(QDialog *AddAutoBoot)
{
     AddAutoBoot->resize(420, 308);
     QVBoxLayout *mverticalLayout = new QVBoxLayout(AddAutoBoot);
     mverticalLayout->setSpacing(16);
     mverticalLayout->setContentsMargins(32, 32, 32, 24);

     mTitleLabel = new QLabel(AddAutoBoot);
     mTitleLabel->setFixedHeight(24);

     mAppFrame = new QFrame(AddAutoBoot);
     mAppFrame->setFixedSize(356,160);
     mAppFrame->setFrameShape(QFrame::NoFrame);
     mAppFrame->setFrameShadow(QFrame::Raised);

     QGridLayout *mAppLayout = new QGridLayout(mAppFrame);
     mAppLayout->setHorizontalSpacing(8);
     mAppLayout->setVerticalSpacing(4);
     mAppNameLabel = new QLabel(mAppFrame);
     mAppPathLabel = new QLabel(mAppFrame);
     mAppBewriteLabel = new QLabel(mAppFrame);
     mAppNameEdit = new QLineEdit(mAppFrame);
     mAppPathEdit = new QLineEdit(mAppFrame);
     mAppBewriteEdit = new QLineEdit(mAppFrame);
     mOpenBtn = new QPushButton(mAppFrame);
     mHintLabel = new QLabel(mAppFrame);

     mAppLayout->addWidget(mAppNameLabel,0,0,1,2);
     mAppLayout->addWidget(mAppNameEdit,0,2,1,3);
     mAppLayout->addWidget(mAppPathLabel,1,0,1,2);
     mAppLayout->addWidget(mAppPathEdit,1,2,1,2);
     mAppLayout->addWidget(mOpenBtn,1,4,1,1);
     mAppLayout->addWidget(mAppBewriteLabel,2,0,1,2);
     mAppLayout->addWidget(mAppBewriteEdit,2,2,1,3);
     mAppLayout->addWidget(mHintLabel,3,2,1,3);

     mBtnFrame = new QFrame(AddAutoBoot);
     mBtnFrame->setFixedSize(356,36);
     mBtnFrame->setFrameShape(QFrame::NoFrame);

     QHBoxLayout *mBtnLayout = new QHBoxLayout(mBtnFrame);
     mBtnLayout->setContentsMargins(0,0,0,0);
     mBtnLayout->setSpacing(16);
     mCancelBtn = new QPushButton(mBtnFrame);
     mCancelBtn->setFixedSize(100,36);
     mCertainBtn = new QPushButton(mBtnFrame);
     mCertainBtn->setFixedSize(100,36);

     mBtnLayout->addStretch();
     mBtnLayout->addWidget(mCancelBtn);
     mBtnLayout->addWidget(mCertainBtn);

     mverticalLayout->addWidget(mTitleLabel);
     mverticalLayout->addWidget(mAppFrame);
     mverticalLayout->addSpacing(8);
     mverticalLayout->addWidget(mBtnFrame);

     retranslateUi();
}

void AddAutoBoot::initStyle()
{
    mTitleLabel->setStyleSheet("QLabel{color: palette(windowText);}");

    selectFile = "";

    mAppNameEdit->setPlaceholderText(tr("Program name"));
    mAppPathEdit->setPlaceholderText(tr("Program exec"));
    mAppBewriteEdit->setPlaceholderText(tr("Program comment"));

    mHintLabel->setAlignment(Qt::AlignLeft);
    mHintLabel->setStyleSheet("color:red;");
    mCertainBtn->setEnabled(false);

}

void AddAutoBoot::initConnection()
{
    connect(mOpenBtn, SIGNAL(clicked(bool)), this, SLOT(open_desktop_dir_slots()));
    connect(mCancelBtn, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(mAppPathEdit, SIGNAL(textEdited(QString)), this, SLOT(execLinEditSlot(QString)));

    connect(mCancelBtn, &QPushButton::clicked, [=] {
        resetBeforeClose();
    });
    connect(mCertainBtn, &QPushButton::clicked, this, [=] {
        emit autoboot_adding_signals(selectFile, mAppNameEdit->text(), mDesktopExec,
                                     mAppBewriteEdit->text(), mDesktopIcon);
        resetBeforeClose();
    });

    connect(mAppNameEdit, &QLineEdit::editingFinished, this, [=](){
        if (mAppNameEdit->text().isEmpty()) {
            userEditNameFlag = false;
        } else {        // 用户输入了程序名
            userEditNameFlag = true;
        }
    });
    connect(mAppBewriteEdit, &QLineEdit::editingFinished, this, [=](){
        if (mAppBewriteEdit->text().isEmpty()) {
            userEditCommentFlag = false;
        } else {        // 用户输入了描述
            userEditCommentFlag = true;
        }
    });

    connect(mAppNameEdit, &QLineEdit::textChanged, this, [=](){
        mAppNameEdit->setToolTip(mAppNameEdit->text());
    });
    connect(mAppBewriteEdit, &QLineEdit::textChanged, this, [=](){
        mAppBewriteEdit->setToolTip(mAppBewriteEdit->text());
    });
    connect(mAppPathEdit, &QLineEdit::textChanged, this, [=](){
        mAppPathEdit->setToolTip(mAppPathEdit->text());
    });
}

void AddAutoBoot::retranslateUi()
{
    mHintLabel->setText(QString());
    mTitleLabel->setText(QApplication::translate("AddAutoBoot", "Add autoboot program", nullptr));
    mAppNameLabel->setText(QApplication::translate("AddAutoBoot", "Program name", nullptr));
    mAppPathLabel->setText(QApplication::translate("AddAutoBoot", "Program exec", nullptr));
    mAppBewriteLabel->setText(QApplication::translate("AddAutoBoot", "Program comment", nullptr));
    mOpenBtn->setText(QApplication::translate("AddAutoBoot", "Open", nullptr));
    mCancelBtn->setText(QApplication::translate("AddAutoBoot", "Cancel", nullptr));
    mCertainBtn->setText(QApplication::translate("AddAutoBoot", "Certain", nullptr));
}

AddAutoBoot::~AddAutoBoot()
{

}

void AddAutoBoot::open_desktop_dir_slots()
{
    QString filters = tr("Desktop files(*.desktop)");
    QFileDialog fd(this);
    fd.setDirectory(DESKTOPPATH);
    fd.setAcceptMode(QFileDialog::AcceptOpen);
    fd.setViewMode(QFileDialog::List);
    fd.setNameFilter(filters);
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setWindowTitle(tr("select autoboot desktop"));
    fd.setLabelText(QFileDialog::Accept, tr("Select"));
    fd.setLabelText(QFileDialog::Reject, tr("Cancel"));
    if (fd.exec() != QDialog::Accepted)
        return;

    QString selectedfile;
    selectedfile = fd.selectedFiles().first();
    selectFile = selectedfile;

    QByteArray ba;
    ba = selectedfile.toUtf8();

    // 解析desktop文件
    GKeyFile *keyfile;
    char *name, *comment,*mname;
    bool no_display;

    keyfile = g_key_file_new();
    if (!g_key_file_load_from_file(keyfile, ba.data(), G_KEY_FILE_NONE, NULL)) {
        g_key_file_free(keyfile);
        return;
    }
    no_display = g_key_file_get_boolean(keyfile, G_KEY_FILE_DESKTOP_GROUP,
                                        G_KEY_FILE_DESKTOP_KEY_NO_DISPLAY, FALSE);
    name = g_key_file_get_string(keyfile, G_KEY_FILE_DESKTOP_GROUP,
                                        G_KEY_FILE_DESKTOP_KEY_NAME, NULL);
    mname = g_key_file_get_locale_string(keyfile, G_KEY_FILE_DESKTOP_GROUP,
                                        G_KEY_FILE_DESKTOP_KEY_NAME, NULL, NULL);
    comment = g_key_file_get_locale_string(keyfile, G_KEY_FILE_DESKTOP_GROUP,
                                           G_KEY_FILE_DESKTOP_KEY_COMMENT, NULL, NULL);
    mDesktopExec = g_key_file_get_string(keyfile, G_KEY_FILE_DESKTOP_GROUP,
                                         G_KEY_FILE_DESKTOP_KEY_EXEC, NULL);
    mDesktopIcon = g_key_file_get_string(keyfile, G_KEY_FILE_DESKTOP_GROUP,
                                         G_KEY_FILE_DESKTOP_KEY_ICON, NULL);

    if (userEditNameFlag == false) {   // 用户输入了程序名，以用户输入为准，否则以自带的为准
        mAppNameEdit->setText(QString(mname));
    }

    mAppPathEdit->setText(QString(selectedfile));
    if (userEditCommentFlag == false) {   // 用户输入了程序描述，以用户输入为准，否则以自带的为准
        mAppBewriteEdit->setText(QString(comment));
    }

    emit mAppPathEdit->textEdited(QString(selectedfile));

    mdir = g_dir_open(g_build_filename(g_get_user_config_dir(), "autostart", NULL), 0, NULL);

    if (no_display) {
        mHintLabel->setText(tr("desktop file not allowed add"));
        mHintLabel->setStyleSheet("color:red;");
        mCertainBtn->setEnabled(false);
    } else if (getFilename(mdir,name)) {
        mHintLabel->setText(tr("desktop file  already exist"));
        mHintLabel->setStyleSheet("color:red;");
        mCertainBtn->setEnabled(false);
    }

    g_key_file_free(keyfile);
}

void AddAutoBoot::execLinEditSlot(const QString &fileName)
{
    selectFile = fileName;
    QFileInfo fileInfo(fileName);
    if (fileInfo.isFile() && fileName.endsWith("desktop")) {
        mHintLabel->clear();
        mCertainBtn->setEnabled(true);

        QByteArray ba;
        ba = fileName.toUtf8();

        // 解析desktop文件
        GKeyFile *keyfile;
        char *name, *comment;

        keyfile = g_key_file_new();
        if (!g_key_file_load_from_file(keyfile, ba.data(), G_KEY_FILE_NONE, NULL)) {
            g_key_file_free(keyfile);
            return;
        }

        name = g_key_file_get_locale_string(keyfile, G_KEY_FILE_DESKTOP_GROUP,
                                            G_KEY_FILE_DESKTOP_KEY_NAME, NULL, NULL);
        mDesktopExec = g_key_file_get_string(keyfile, G_KEY_FILE_DESKTOP_GROUP,
                                             G_KEY_FILE_DESKTOP_KEY_EXEC, NULL);
        mDesktopIcon = g_key_file_get_string(keyfile, G_KEY_FILE_DESKTOP_GROUP,
                                             G_KEY_FILE_DESKTOP_KEY_ICON, NULL);
        comment = g_key_file_get_locale_string(keyfile, G_KEY_FILE_DESKTOP_GROUP,
                                               G_KEY_FILE_DESKTOP_KEY_COMMENT, NULL, NULL);

        if (userEditNameFlag == false) {   // 用户输入了程序名，以用户输入为准，否则以自带的为准
            mAppNameEdit->setText(QString(name));
        }

        mAppPathEdit->setText(fileName);
        if (userEditCommentFlag == false) {   // 用户输入了程序描述，以用户输入为准，否则以自带的为准
            mAppBewriteEdit->setText(QString(comment));
        }

        g_key_file_free(keyfile);
    } else {
        mHintLabel->setText(tr("desktop file not exist"));
        mHintLabel->setStyleSheet("color:red;");
        mCertainBtn->setEnabled(false);
    }
}
