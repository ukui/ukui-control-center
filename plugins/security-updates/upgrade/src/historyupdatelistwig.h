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

#ifndef HISTORYUPDATELISTWIG_H
#define HISTORYUPDATELISTWIG_H

#include <QFrame>
#include <QLabel>
#include <QTextEdit>
#include <QPainter>
#include <QBoxLayout>
#include <QMouseEvent>
#include <QGSettings>
#include <QDebug>

class HistoryUpdateListWig : public QFrame
{
    Q_OBJECT
public:
    HistoryUpdateListWig(QLabel *);
    ~HistoryUpdateListWig();
    void setAttribute(const QString &mname, const QString &mstatue, const QString &mtime, const QString &mdescription, const int &myid);//赋值
    QSize getTrueSize();//获取真实大小
    void selectStyle();//选中样式
    void clearStyleSheet();//取消选中样式
    int id = 0;
    void gsettingInit();
protected:
    void mousePressEvent(QMouseEvent * event);
private:
    QHBoxLayout *hl1 = nullptr;
    QHBoxLayout *hl2 = nullptr;
    QVBoxLayout *vl1 = nullptr;
    QLabel *debName = nullptr;//app名字&版本号
    QLabel *debStatue = nullptr;//更新状态
    QString debDescription = "";//描述
    //int code = 0 ;//编码
    void initUI();//初始化UI
    void setDescription();//赋值事件
    QFont font;

    /* 监听主题字体变化 */
    QGSettings *m_pGsettingFontSize;
    QString mNameLabel;
    QString mStatusLabel;

    /* 控制悬浮框 */
    bool nameTipStatus = false;
    bool statusTipStatus = false;

    /* 右侧顶部标签 */
    QLabel *mDesTab = nullptr;
};

#endif // HISTORYUPDATELISTWIG_H
