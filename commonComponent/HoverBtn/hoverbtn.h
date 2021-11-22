/*
 * Copyright 2021 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef HOVERBTN_H
#define HOVERBTN_H

#include <QWidget>
#include <QEvent>
#include <QString>
#include <QFrame>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QPropertyAnimation>

class HoverBtn : public QWidget
{
    Q_OBJECT
public:
    HoverBtn(QString mname, bool isHide, QWidget *parent = nullptr);
    HoverBtn(QString mname, QString detailName, QWidget *parent = nullptr);
    ~HoverBtn();

public:
    QString mName;
    QString mDetailName;
    QPushButton *mAbtBtn;

    QFrame *mInfoItem;

    QLabel *mPitIcon;
    QLabel *mPitLabel;
    QLabel *mDetailLabel;

    QHBoxLayout *mHLayout;

    QTimer *mMouseTimer;

    bool mAnimationFlag = false;
    bool mIsHide;

    int  mHideWidth;

    QPropertyAnimation *mEnterAction = nullptr;
    QPropertyAnimation *mLeaveAction = nullptr;

private:
    void initUI();
    void initAnimation();

protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void enterEvent(QEvent * event);
    virtual void leaveEvent(QEvent * event);
    virtual void mousePressEvent(QMouseEvent * event);

Q_SIGNALS:
    void widgetClicked(QString name);
    void enterWidget(QString name);
    void leaveWidget(QString name);
    void resize();
};

#endif // HOVERBTN_H
