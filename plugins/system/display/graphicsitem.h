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
#ifndef GRAPHICSITEM_H
#define GRAPHICSITEM_H

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QPainter>
#include <QPolygonF>


class GraphicsItem : public QGraphicsItem
{

public:
    explicit GraphicsItem();
    ~GraphicsItem();

    QRectF boundingRect()const;
    void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget);
    QPainterPath shape()const;

    void setRectF(qreal width, qreal height);
    void setMonitorInfo(QString name, QString type);
    void setitemMoveFlags(bool status);

    void selected_changed(bool flag);

    qreal itemWidth();
    qreal itemHeight();

protected:
//    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent *event) Q_DECL_OVERRIDE;
//    virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) Q_DECL_OVERRIDE;
//    virtual void dragMoveEvent(QGraphicsSceneDragDropEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    qreal m_width;
    qreal m_height;

    bool selectedFlag;

    QString monitorName;
    QString monitorType;
};

#endif // GRAPHICSITEM_H
