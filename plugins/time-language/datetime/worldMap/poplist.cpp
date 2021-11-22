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

#include "poplist.h"
#include "poplistdelegate.h"

#include <QKeyEvent>
#include <QtAlgorithms>
#include <QDebug>
#include <QApplication>

PopList::PopList(QWidget* parent) : QFrame ()
{
    m_zoneinfo = new ZoneInfo;
    this->initUI();
}

void PopList::initUI() {
    m_listView = new QListView(this);
    m_listModel = new QStringListModel(this);

    m_listView->setContentsMargins(0,4,0,4);

    m_listView->setModel(m_listModel);
    // 隐藏横竖拖动条
    m_listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_listView->setUniformItemSizes(true);
    m_listView->setSelectionMode(QListView::SingleSelection);
    // 不启用条目编辑
    m_listView->setEditTriggers(QListView::NoEditTriggers);

    PopListDelegate *listDelegate = new PopListDelegate(this);
    m_listView->setItemDelegate(listDelegate);
    m_listView->setMouseTracking(true);
    m_listView->setStyleSheet(m_zoneinfo->readRile(":/list.css"));

    setContentsMargins(0, 0, 0, 0);
    // 透明
    setAttribute(Qt::WA_TranslucentBackground,true);
    // 弹出式顶层窗口
    this->setFocusPolicy(Qt::StrongFocus);
    setWindowFlags(Qt::Popup);

    connect(m_listView, &QListView::pressed,
            this, &PopList::listActiveSlot);
}

QStringList PopList::stringList() {
    return m_listModel->stringList();
}

void PopList::keyPressEvent(QKeyEvent *event) {
    if(event->key() == Qt::Key_Escape) {
        this->hide();
    }
    QWidget::keyPressEvent(event);    

}

bool PopList::eventFilter(QObject *obj, QEvent *event) {
    if(event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent) {
            if (!this->geometry().contains(mouseEvent->pos())){
                this->hide();
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

void PopList::hideEvent(QHideEvent *event) {
    qApp->removeEventFilter(this);

    this->releaseKeyboard();
    QWidget::hideEvent(event);
    emit this->listHide();
}

void PopList::showEvent(QShowEvent *event) {
    qApp->installEventFilter(this);
    QFrame::showEvent(event);
}

void PopList::showPopLists(QPoint pos) {
    QSize size = m_listView->size();

    //弹出列表移动到鼠标上方
    this->move(pos.x() - size.width() / 2 -5, pos.y() - size.height() - 8);
    this->show();

    //全局键盘事件？
    this->grabKeyboard();
}

void PopList::setStringList(QStringList strings) {
    m_listModel->setStringList(strings);

    int realWidth = 60; // 菜单栏最大宽度,初始化为60
    const QFontMetrics mt(m_listView->font());
    for(QString& str : strings) {
        int itemWidth = mt.width(str);
        realWidth = qMax(realWidth, itemWidth);
    }

    // 文本宽度加margin(40)
    int width = realWidth + 40;
    // 文本高度加底部间距
    int height = 24 * strings.length() + 18;

    this->resize(width, height);

    m_listView->adjustSize();
    m_listView->resize(width, height);
}

void PopList::listActiveSlot(QModelIndex index) {
    Q_ASSERT(index.isValid());

    if(index.isValid()) {
        emit this->listAactive(index.row());
    }
}
