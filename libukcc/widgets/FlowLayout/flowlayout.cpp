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

#include "flowlayout.h"

#include <QWidget>
#include <QDebug>
#include <QtMath>

FlowLayout::FlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing) :
    QLayout(parent),
    m_hSpace(hSpacing),
    m_vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::FlowLayout(QWidget *parent, bool home, int margin, int hSpacing, int vSpacing):
    QLayout(parent),
    m_hSpace(hSpacing),
    m_vSpace(vSpacing),
    m_home(home)
{
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::FlowLayout(int margin, int hSpacing, int vSpacing)
    : m_hSpace(hSpacing),
      m_vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::~FlowLayout(){
    QLayoutItem * item;
    while ((item = takeAt(0))) {

    }
}

void FlowLayout::addItem(QLayoutItem *item){
    itemList.append(item);
}

int FlowLayout::horizontalSpacing() const{
    if (m_hSpace >= 0 || m_hSpace == -1) {
        return m_hSpace;
    } else {
        return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
    }
}

int FlowLayout::verticalSpacing() const{
    if (m_vSpace >= 0 || m_vSpace == -1) {
        return m_vSpace;
    } else {
        return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
    }
}

int FlowLayout::count() const{
    return itemList.size();
}

QLayoutItem * FlowLayout::itemAt(int index) const{
    return itemList.value(index);
}

QLayoutItem * FlowLayout::takeAt(int index){
    if (index >= 0 && index < itemList.size())
        return itemList.takeAt(index);
    else
        return 0;
}

Qt::Orientations FlowLayout::expandingDirections() const{
    return 0;
}

bool FlowLayout::hasHeightForWidth() const{
    return true;
}

int FlowLayout::heightForWidth(int width) const{
    int height = doLayout(QRect(0, 0, width, 0), true);
    return height;
}

void FlowLayout::setGeometry(const QRect &rect){
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

QSize FlowLayout::sizeHint() const{
    return minimumSize();
}

QSize FlowLayout::minimumSize() const{
    QSize size;
    QLayoutItem *item;
    foreach (item, itemList)
        size = size.expandedTo(item->minimumSize());

    size += QSize(2*margin(), 2*margin());
    return size;
}

int FlowLayout::doLayout(const QRect &rect, bool testOnly) const{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;

    int fillX = 0;
    bool bFillX = false;

    QLayoutItem *item;
    foreach (item, itemList) {
        QWidget *wid = item->widget();
        int spaceX = horizontalSpacing();
        if (spaceX == -1) {
            if (!bFillX) {
                bFillX = true;
                fillX = fillSpaceX(wid);
            }
            spaceX = fillX;
        }

        int spaceY = verticalSpacing();
        if (spaceY == -1 && fillX >= 0) {
            spaceY = fillX;
        } else {
            spaceY = wid->style()->layoutSpacing(
                        QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);
        }
        if (m_home) {
            spaceY = 32;
        }


        int nextX = x + item->sizeHint().width() + spaceX;
        if (nextX - spaceX > effectiveRect.right() && lineHeight > 0) {
            x = effectiveRect.x();
            y = y + lineHeight + spaceY;
            nextX = x + item->sizeHint().width() + spaceX;
            lineHeight = 0;
        }

        if (!testOnly)
            item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

        x = nextX;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }
    return y + lineHeight - rect.y() + bottom;
}

int FlowLayout::smartSpacing(QStyle::PixelMetric pm) const{
    QObject *parent = this->parent();
    if (!parent) {
        return -1;
    } else if (parent->isWidgetType()) {
        QWidget *pw = static_cast<QWidget *>(parent);
        return pw->style()->pixelMetric(pm, 0, pw);
    } else {
        return static_cast<QLayout *>(parent)->spacing();
    }
}

int FlowLayout::fillSpaceX(QWidget *wid) const{
    int num = 0;
    int x = 0;
    int numH = 0;
    int space  = 4;
    if (m_home) {
        space = 24;
    }
    int len = this->parentWidget()->width() - this->contentsMargins().left() - this->contentsMargins().right();
    while (true) {
       num++;
       if (num * (wid->width() + space) - space >= len) {  //最小间距space
           break;
       }
    }

    num = num - 1;
    if (num <= 1) {
        numH = itemList.size();
        return 32;
    }
    int height = wid->height();
    numH = ceil(double(itemList.size()) / num);
    x = len + space - num * (wid->width() + space);
    x = ceil(double(x)/(num - 1)) +space;
    x = x - 1;   //考虑边框等因素影响

    int maxY = numH * (height + x) + 32 - x;
    if (m_home) {
        maxY = numH * (height + 24);
        this->parentWidget()->parentWidget()->setFixedHeight(maxY);
    }
    this->parentWidget()->setFixedHeight(maxY);

    return x;
}

