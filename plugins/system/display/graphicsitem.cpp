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
#include "graphicsitem.h"

#include <QDebug>

#define SPACING 10

GraphicsItem::GraphicsItem()
{
    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);
//    QGraphicsItem::ItemIsSelectable item可选中
//    QGraphicsItem::ItemIsMovable item可移动
//    QGraphicsItem::ItemSendsGeometryChanges 使itemChange()函数有效

    selectedFlag = false; //选中状态
    monitorName = "UNKNOWN";
    monitorType = "UNKNOWN";

}

GraphicsItem::~GraphicsItem()
{
}

void GraphicsItem::setRectF(qreal width, qreal height){
    m_width = width;
    m_height = height;
}

QRectF GraphicsItem::boundingRect()const{
    return QRectF(-m_width/2-1, -m_height/2-1, m_width+2, m_height+2);
}

QPainterPath GraphicsItem::shape()const{
    QPainterPath path;
    path.addRect(QRectF(-m_width/2, -m_height/2, m_width, m_height));
    return path;
}

void GraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    Q_UNUSED(option);
    Q_UNUSED(widget);
    //设置画笔
    QPen pen;
    pen.setWidth(2);
    if (selectedFlag)
        pen.setColor(QColor("#8B8878"));
    else
        pen.setColor(QColor("#01579b"));
    painter->setPen(pen);
    painter->setBrush(QColor("#35a3dd"));
    //绘制矩形显示器
    painter->drawRect(QRectF(-m_width/2, -m_height/2, m_width, m_height));


//    int fontsize = static_cast<int>(m_width/(monitorName.length()));
    int fontsize = 14;
    //设置显示器信息字体
    QFont font;
    font.setPointSize(fontsize);
//    font.setLetterSpacing(QFont::AbsoluteSpacing, 10);
    painter->setFont(font);

    QPen textpen;
    textpen.setWidth(1);
    textpen.setColor(QColor("#ffffff"));
    painter->setPen(textpen);

    //绘制显示器信息
    painter->drawText(QRectF(-m_width/2, -m_height/2 + (m_height - fontsize*2 - SPACING)/2, m_width, fontsize + 10), Qt::AlignCenter, monitorName);
    painter->drawText(QRectF(-m_width/2, -m_height/2 + (m_height - fontsize*2 - SPACING)/2 + fontsize + SPACING, m_width, fontsize + 10), Qt::AlignCenter, monitorType);
}

QVariant GraphicsItem::itemChange(GraphicsItemChange change, const QVariant &value){

    if (change == ItemPositionChange && scene() && scene()->selectedItems().length() > 0){ //通过选中的item来判断是否是用户在拖动
        QPointF newPos = value.toPointF();
//        qDebug() << "*******newPos: " << newPos;
        QRectF rect(0, this->pos().y(), scene()->width(), 0);//水平
//        qDebug() << "******Pos: " << this->pos() << scene()->width();
//        qDebug() << "rect: " << rect.left() << rect.top() << rect.right() << rect.bottom();
//        qDebug() << rect.isEmpty();
//        qDebug() << "****length:" << scene()->selectedItems().length();
//        qDebug() << "****length2:" << scene()->items().length();

//        if (scene()->items().length() == 1){ //单屏禁止移动
//            QPointF newPos = value.toPointF();
//            newPos.setX(0);
//            newPos.setY(0);
//            return newPos;
//        }

        if (scene()->items().length() == 2){ //双屏
            QPointF newPos = value.toPointF();
            QGraphicsItem * transfastenedItem;
            QGraphicsItem * transselectedItem = scene()->selectedItems().first();

            for (QGraphicsItem * item : scene()->items()){
                if (item != transselectedItem)
                    transfastenedItem = item;
            }

            GraphicsItem * fastenedItem = dynamic_cast<GraphicsItem * >(transfastenedItem);
            GraphicsItem * selectedItem = dynamic_cast<GraphicsItem *>(transselectedItem);

            QPointF fastenedPos = fastenedItem->pos();
            QRectF rect(fastenedPos.x() - fastenedItem->itemWidth()/2 - selectedItem->itemWidth()/2, fastenedPos.y() - fastenedItem->itemHeight()/2 - selectedItem->itemHeight()/2, fastenedItem->itemWidth() + selectedItem->itemWidth(), fastenedItem->itemHeight() + selectedItem->itemHeight());
//            qDebug() << rect.left() << rect.top() << rect.right() << rect.bottom();

            if (!rect.contains(newPos)){ //设置限制移动的矩形
                newPos.setX(qMin(rect.right(), qMax(newPos.x(), rect.left())));
                newPos.setY(qMin(rect.bottom(), qMax(newPos.y(), rect.top())));

                return newPos;
            }
            else{

                QPolygonF rightPolygonF, leftPolygonF, topPolygonF, bottomPolygonF;
                rightPolygonF << rect.topRight() << rect.bottomRight() << fastenedPos;
                leftPolygonF << rect.topLeft() << rect.bottomLeft() << fastenedPos;
                topPolygonF << rect.topLeft() << rect.topRight() << fastenedPos;
                bottomPolygonF << rect.bottomLeft() << rect.bottomRight() << fastenedPos;

                if (rightPolygonF.containsPoint(newPos, Qt::OddEvenFill))
                    newPos.setX(rect.right());
                else if (topPolygonF.containsPoint(newPos, Qt::OddEvenFill))
                    newPos.setY(rect.top());
                else if (leftPolygonF.containsPoint(newPos, Qt::OddEvenFill))
                    newPos.setX(rect.left());
                else if (bottomPolygonF.containsPoint(newPos, Qt::OddEvenFill))
                    newPos.setY(rect.bottom());

                return newPos;
            }

        }

    }
    return QGraphicsItem::itemChange(change, value);
}

//void GraphicsItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event){
//    qDebug() << "drag----Enter--->";
//}

//void GraphicsItem::dragMoveEvent(QGraphicsSceneDragDropEvent *event){
//    qDebug() << "drag----Move--->";
//}

//void GraphicsItem::dragLeaveEvent(QGraphicsSceneDragDropEvent *event){
//    qDebug() << "drag----Leave---->";
//}

void GraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
//    qDebug() << "-------release-->" << pos();

    emit dragOverSignal();

    QGraphicsItem::mouseReleaseEvent(event);
}

void GraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event){
//    qDebug() << "---------press--->" << pos();
    lastpos = pos();
    QGraphicsItem::mousePressEvent(event);
}

void GraphicsItem::selected_changed(bool flag){
    if (selectedFlag != flag){
        selectedFlag = flag;
        update();
    }
}

qreal GraphicsItem::itemWidth(){
    return m_width;
}

qreal GraphicsItem::itemHeight(){
    return m_height;
}

QPointF GraphicsItem::getLastPos(){
    return lastpos;
}

void GraphicsItem::setMonitorInfo(QString name, QString type){
    monitorName = name;
    monitorType = type;
    update();
}

QString GraphicsItem::getMonitorInfo(){
    return monitorName + monitorType;
}

void GraphicsItem::setitemMoveFlags(bool status){
    setFlag(QGraphicsItem::ItemIsMovable, status);
}
