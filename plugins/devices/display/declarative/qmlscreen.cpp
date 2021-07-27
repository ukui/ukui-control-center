/*
    Copyright (C) 2012  Dan Vratil <dvratil@redhat.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "qmlscreen.h"
#include "qmloutputcomponent.h"
#include "qmloutput.h"

#include <QTextStream>
#include <QDir>
#include <QFile>
#include <QTime>
#include <KF5/KScreen/kscreen/output.h>
#include <KF5/KScreen/kscreen/config.h>

#include <QTimer>
#include <sys/socket.h>
#define TABLET_MODE_SCHEMAS              "org.ukui.SettingsDaemon.plugins.tablet-mode"
#define TABLET_MODE_KEY                  "tablet-mode"

#define CONTROL_CENTER_MODE              "org.ukui.control-center.personalise"
Q_DECLARE_METATYPE(KScreen::OutputPtr)

QMLScreen::QMLScreen(QQuickItem *parent)
    : QQuickItem(parent)
{
    m_initPath =  QString("%1/%2/%3").arg(QDir::homePath()).arg(".config/ukui").arg("config.ini");//构造函数中初始化设置信息
    QSettings settings(m_initPath, QSettings::IniFormat);
    int begin_x = settings.value("qmlOutput_Position/start_x").toInt();
    int begin_y = settings.value("qmlOutput_Position/start_y").toInt();
    if (begin_x == NULL || begin_y == NULL) {
        writeInit("qmlOutput_Position","start_x",509);
        writeInit("qmlOutput_Position","start_y",108);
        start_x = settings.value("qmlOutput_Position/start_x").toInt();
        start_y = settings.value("qmlOutput_Position/start_y").toInt();
    } else {
            start_x = settings.value("qmlOutput_Position/start_x").toInt();
            start_y = settings.value("qmlOutput_Position/start_y").toInt();
    }
    if(QGSettings::isSchemaInstalled(TABLET_MODE_SCHEMAS)) {
        m_tmsettings = new QGSettings(TABLET_MODE_SCHEMAS);
    }
    m_outputTimer = new QTimer(this);
    m_screenAddTimer = new QTimer(this);
    connect(m_outputTimer,SIGNAL(timeout()),this,SLOT(mainScreenChanged()),Qt::QueuedConnection);
    connect(qApp, SIGNAL(screenAdded(QScreen *)),this,SLOT(screenAddedProcess()));
    connect(m_screenAddTimer,SIGNAL(timeout()),this,SLOT(screenAddedProcessTimer()));

    connect(qApp, SIGNAL(screenRemoved(QScreen *)),this,SLOT(screenRemovedProcess()));
    connect(this, &QMLScreen::widthChanged, this, &QMLScreen::viewSizeChanged);
    connect(this, &QMLScreen::heightChanged, this, &QMLScreen::viewSizeChanged);

}

QMLScreen::~QMLScreen()
{
    qDeleteAll(m_outputMap);
    m_outputMap.clear();
    if (m_tmsettings) {
        delete m_tmsettings;
    }
}

void QMLScreen::screenRemovedProcess() {
    //保证拔出显示器后，内显为选定状态
    Q_FOREACH (QQuickItem *item, childItems()) {
        QMLOutput *qmlOutput = qobject_cast<QMLOutput*>(item);
        if (qmlOutput->output()->name() == "eDP-1") {
            setActiveOutput(qmlOutput);
        }
    }
}

void QMLScreen::screenAddedProcess() {
    m_screenAddTimer->start();
}
void QMLScreen::screenAddedProcessTimer() {
    m_screenAddTimer->stop();
    Q_FOREACH (QQuickItem *item, childItems()) {
        QMLOutput *qmlOutput = qobject_cast<QMLOutput*>(item);
        if (!qmlOutput->output()->isPrimary()) {
            qmlOutput->setPosition(QPointF(509,108));
        } else {
            setActiveOutput(qmlOutput);
        }
    }
    writeInit("qmlOutput_Position","start_x",509);
    writeInit("qmlOutput_Position","start_y",108);
}

void QMLScreen::setDisplayWidget(DisplayWidget * displayWidget) {
    /*切换主副屏，交换位置*/
    this->dispalyWidget = displayWidget;
    connect(dispalyWidget,&DisplayWidget::primaryChange,this,[=]{
        Q_FOREACH (QMLOutput *item, m_outputMap) {
            QMLOutput *qmlOutput = qobject_cast<QMLOutput*>(item);
            if (!qmlOutput->output()->isPrimary()) {
                QSettings settings(m_initPath, QSettings::IniFormat);
                int change_x = settings.value("qmlOutput_Position/start_x").toInt();
                int change_y = settings.value("qmlOutput_Position/start_y").toInt();
                writeInit("mainScreenChange_Position","x",change_x);
                writeInit("mainScreenChange_Position","y",change_y);
//                qDebug()<<"坐标："<<change_x<<","<<change_y;
//                qmlOutput->setPosition(QPointF(change_x,change_y));
//                m_outputTimer->start();
                mainScreenChanged();
//                fixPosition(change_x,change_y);
            }
        }
    });
}

KScreen::ConfigPtr QMLScreen::config() const
{
    return m_config;
}
void QMLScreen::setConfig(const KScreen::ConfigPtr &config)
{
    qDeleteAll(m_outputMap);
    m_outputMap.clear();
    m_manuallyMovedOutputs.clear();
    m_bottommost = m_leftmost = m_rightmost = m_topmost = nullptr;
    m_connectedOutputsCount = 0;
    m_enabledOutputsCount = 0;

    if (m_config) {
        m_config->disconnect(this);
    }
    m_config = config;
    connect(m_config.data(), &KScreen::Config::outputAdded,
            this, [this](const KScreen::OutputPtr &output) {
                addOutput(output);
                updateOutputsPlacement();
            });
    connect(m_config.data(), &KScreen::Config::outputRemoved,
            this, &QMLScreen::removeOutput);
    //qDebug()<<"所要拿取的配置为------>"<<m_config<<endl;
    for (const KScreen::OutputPtr &output : m_config->outputs()) {
//        qDebug()<<"\noutput类型----debug------>"<<output<<" "<<
//                  "\noutput原始类型------->"<<QVariant::fromValue(qobject_cast<KScreen::OutputPtr>(output))<<" "<<endl;
        addOutput(output);
    }

    updateOutputsPlacement();

    for (QMLOutput *qmlOutput : m_outputMap) {
        if (qmlOutput->output()->isConnected() && qmlOutput->output()->isEnabled()) {
            //qDebug()<<"qmlOutput---->"<<qmlOutput<<endl;
            qmlOutput->dockToNeighbours();
        }
    }
}


void QMLScreen::addOutput(const KScreen::OutputPtr &output)
{
    //qDebug()<<"qmlscreen.cpp-------> output类型------->"<<output<<endl;
    QMLOutputComponent comp(qmlEngine(this), this);

    QMLOutput *qmloutput = comp.createForOutput(output);
    if (!qmloutput) {
        qWarning() << "Failed to create QMLOutput";
        return;
    }

    m_outputMap.insert(output, qmloutput);

    qmloutput->setParentItem(this);
    qmloutput->setZ(m_outputMap.count());
    int connectedCount = 0;

    Q_FOREACH (const KScreen::OutputPtr &output, m_outputMap.keys()) {
        if (output->isConnected()) {
            ++connectedCount;
        }
    }

    if (connectedCount != m_connectedOutputsCount) {
        m_connectedOutputsCount = connectedCount;
        Q_EMIT connectedOutputsCountChanged();
        updateOutputsPlacement();
    }
    connect(output.data(), &KScreen::Output::isConnectedChanged,
            this, &QMLScreen::outputConnectedChanged);
    connect(output.data(), &KScreen::Output::isEnabledChanged,
            this, &QMLScreen::outputEnabledChanged);
    connect(output.data(), &KScreen::Output::posChanged,
            this, &QMLScreen::outputPositionChanged);
    connect(qmloutput, &QMLOutput::yChanged,
            [this, qmloutput]() {
                qmlOutputMoved(qmloutput);
            });
    connect(qmloutput, &QMLOutput::xChanged,
            [this, qmloutput]() {
                qmlOutputMoved(qmloutput);
            });
    //在这里点击上面小屏幕
    connect(qmloutput, SIGNAL(clicked()),
            this, SLOT(setActiveOutput()));
    connect(qmloutput, SIGNAL(mouseReleased()),
            this, SLOT(applyOutput()));
    qmloutput->updateRootProperties();
}

void QMLScreen::applyOutput() {
//    m_csettings->set("mode",false);
    writeInit("qmlOutput_Position","start_x",start_x);
    writeInit("qmlOutput_Position","start_y",start_y);
    qDebug()<<"Will apply later";
    Q_EMIT changed();
}

void QMLScreen::removeOutput(int outputId)
{
    for (const KScreen::OutputPtr &output : m_outputMap.keys()) {
        if (output->id() == outputId) {
            QMLOutput *qmlOutput = m_outputMap.take(output);
            qmlOutput->setParentItem(nullptr);
            qmlOutput->setParent(nullptr);
            qmlOutput->destroyed();
            return;
        }
    }
}


int QMLScreen::connectedOutputsCount() const
{
    return m_connectedOutputsCount;
}

int QMLScreen::enabledOutputsCount() const
{
    return m_enabledOutputsCount;
}

QMLOutput *QMLScreen::primaryOutput() const
{
    Q_FOREACH (QMLOutput *qmlOutput, m_outputMap) {
        if (qmlOutput->output()->isPrimary()) {
            return qmlOutput;
        }
    }

    return nullptr;
}

QList<QMLOutput*> QMLScreen::outputs() const
{
    return m_outputMap.values();
}


void QMLScreen::setActiveOutput(QMLOutput *output)
{
    Q_FOREACH (QMLOutput *qmlOutput, m_outputMap) {
        if (qmlOutput->z() > output->z()) {
            qmlOutput->setZ(qmlOutput->z() - 1);
        }
    }

    output->setZ(m_outputMap.count());
    //选中屏幕
    output->setFocus(true);
    Q_EMIT focusedOutputChanged(output);
}

QSize QMLScreen::maxScreenSize() const
{
    //qDebug()<<"m_config->screen()->maxSize()--->"<<m_config->screen()->maxSize()<<endl;
    return m_config->screen()->maxSize();
}

float QMLScreen::outputScale() const
{
    return m_outputScale;
}

void QMLScreen::outputConnectedChanged()
{
    int connectedCount = 0;

    Q_FOREACH (const KScreen::OutputPtr &output, m_outputMap.keys()) {
        if (output->isConnected()) {
            ++connectedCount;
        }
    }

    if (connectedCount != m_connectedOutputsCount) {
        m_connectedOutputsCount = connectedCount;
        Q_EMIT connectedOutputsCountChanged();
        updateOutputsPlacement();
    }
}

void QMLScreen::outputEnabledChanged()
{
    const KScreen::OutputPtr output(qobject_cast<KScreen::Output*>(sender()), [](void *){});
    if (output->isEnabled()) {
        updateOutputsPlacement();
    }
    int enabledCount = 0;

    Q_FOREACH (const KScreen::OutputPtr &output, m_outputMap.keys()) {
        if (output->isEnabled()) {
            ++enabledCount;
        }
    }

    if (enabledCount == m_enabledOutputsCount) {
        m_enabledOutputsCount = enabledCount;
        Q_EMIT enabledOutputsCountChanged();
    }
}

void QMLScreen::outputPositionChanged()
{
    /* TODO: Reposition the QMLOutputs */
}
bool QMLScreen::writeInit(QString group, QString key, int value)
{
    if(group.isEmpty() || key.isEmpty())
    {
        return false;
    }else
    {
        //创建配置文件操作对象,Iniformat存储数据到INI文件中
        QSettings config(m_initPath, QSettings::IniFormat);

        //将信息写入配置文件
        config.beginGroup(group);
        config.setValue(key, value);
        config.endGroup();

        return true;
    }
}
void QMLScreen::mainScreenChanged(){
//    m_outputTimer->stop();
    QSettings settings(m_initPath, QSettings::IniFormat);
    int x = settings.value("mainScreenChange_Position/x").toInt();
    int y = settings.value("mainScreenChange_Position/y").toInt();
    int expandWidth;
    int expandHeight;
    Q_FOREACH (QQuickItem *item, childItems()) {
        QMLOutput *qmlOutput = qobject_cast<QMLOutput*>(item);
        if (qmlOutput->output()->isPrimary()) {
            primaryWidth = qmlOutput->width();
            primaryHeight = qmlOutput->height();
            primaryX = qmlOutput->x();
            primaryY = qmlOutput->y();
            qmlOutput->setPosition(QPointF(434-qmlOutput->width()/2,108));
            qDebug()<<"primaryWidth,primaryHeight,primaryX,primaryY"<<primaryWidth<<primaryHeight<<primaryX<<primaryY;
        } else {
            expandWidth = qmlOutput->width();
            expandHeight = qmlOutput->height();
            if (x >= primaryX + primaryWidth/2) {
//                qDebug()<<primaryWidth<<primaryHeight<<expandWidth<<expandHeight;
//                qDebug()<<primaryX + primaryWidth + 2<<","<<primaryY + primaryHeight/2 - expandHeight/2;
                qmlOutput->setPosition(QPointF(primaryX + primaryWidth + 2, primaryY + primaryHeight/2 - expandHeight/2));
            }
            if (x < primaryX + primaryWidth/2) {
//                qDebug()<<primaryWidth<<primaryHeight<<expandWidth<<expandHeight;
//                qDebug()<<primaryX - expandWidth - 2<<","<<primaryY + primaryHeight/2 - expandHeight/2;
                qmlOutput->setPosition(QPointF(primaryX - expandWidth - 2, primaryY + primaryHeight/2 - expandHeight/2));
            }
        }
    }
}

void QMLScreen::qmlOutputMoved(QMLOutput *qmlOutput)
{
//    qDebug()<<"qmlOutputMoved------>"<<endl;
    if (qmlOutput->isCloneMode()) {
        return;
    }
    if (!m_manuallyMovedOutputs.contains(qmlOutput))
        m_manuallyMovedOutputs.append(qmlOutput);

    updateCornerOutputs();

    if (m_leftmost) {
        m_leftmost->setOutputX(0);
    }
    if (m_topmost) {
        m_topmost->setOutputY(0);
    }
    if (qmlOutput->output()->isPrimary()) {
        qmlOutput->setPosition(QPointF(434-qmlOutput->width()/2,108));
        primaryX = qmlOutput->x();
        primaryY = qmlOutput->y();
        primaryWidth = qmlOutput->width();
        primaryHeight = qmlOutput->height();
    } else {
        expandX = qmlOutput->x();
        expandY = qmlOutput->y();
        expandWidth = qmlOutput->width();
        expandHeight = qmlOutput->height();
//        qDebug()<<"在左边的坐标:"<<primaryX + primaryWidth + 2 << "," << primaryY + primaryHeight/2 - expandHeight/2;
//        qDebug()<<"在右边的坐标："<<primaryX - expandWidth - 2 << "," << primaryY + primaryHeight/2 - expandHeight/2;
        if (qmlOutput->x() >= primaryX + primaryWidth/2) {
            qmlOutput->setPosition(QPointF(primaryX + primaryWidth + 2, primaryY + primaryHeight/2 - expandHeight/2));
        }
        if (qmlOutput->x() < primaryX + primaryWidth/2) {
            qmlOutput->setPosition(QPointF(primaryX - expandWidth - 2, primaryY + primaryHeight/2 - expandHeight/2));
        }
//        if (qmlOutput->y() > primaryY + primaryHeight) {
//            qmlOutput->setPosition(QPointF(left_x + primaryWidth/2 - expandWidth/2, primaryY + primaryHeight + 2));
//        }
//        if (qmlOutput->y() < primaryY - expandHeight) {
//            qmlOutput->setPosition(QPointF(left_x + primaryWidth/2 - expandWidth/2, primaryY - expandHeight - 2));
//        }
        start_x = qmlOutput->x();
        start_y = qmlOutput->y();
    }
    if (qmlOutput == m_leftmost) {
        Q_FOREACH (QMLOutput *other, m_outputMap) {
            if (other == m_leftmost) {
                continue;
            }

            if (!other->output()->isConnected() || !other->output()->isEnabled()) {
                continue;
            }

            other->setOutputX(float(other->x() - m_leftmost->x()) / outputScale());
        }
    } else if (m_leftmost) {
        qmlOutput->setOutputX(float(qmlOutput->x() - m_leftmost->x()) / outputScale());
    }

    if (qmlOutput == m_topmost) {
        Q_FOREACH (QMLOutput *other, m_outputMap) {
            if (other == m_topmost) {
                continue;
            }

            if (!other->output()->isConnected() || !other->output()->isEnabled()) {
                continue;
            }

            other->setOutputY(float(other->y() - m_topmost->y()) / outputScale());
        }
    } else if (m_topmost) {
        qmlOutput->setOutputY(float(qmlOutput->y() - m_topmost->y()) / outputScale());
    }
}

void QMLScreen::viewSizeChanged()
{
    updateOutputsPlacement();
}

void QMLScreen::updateCornerOutputs()
{
    m_leftmost = nullptr;
    m_topmost = nullptr;
    m_rightmost = nullptr;
    m_bottommost = nullptr;

    Q_FOREACH (QMLOutput *output, m_outputMap) {
        if (!output->output()->isConnected() || !output->output()->isEnabled()) {
            continue;
        }

        QMLOutput *other = m_leftmost;
        if (!other || output->x() < other->x()) {
            m_leftmost = output;
        }

        if (!other || output->y() < other->y()) {
            m_topmost = output;
        }

        if (!other || output->x() + output->width() > other->x() + other->width()) {
            m_rightmost = output;
        }

        if (!other || output->y() + output->height() > other->y() + other->height()) {
            m_bottommost = output;
        }
    }
}

//void QMLScreen::setOutputScale(float scale)
//{
//    if (qFuzzyCompare(scale, m_outputScale))
//        return;
//    m_outputScale = scale;
//    qDebug()<<"m_outputScale===="<<m_outputScale;
//    emit outputScaleChanged();
//}

//应该是画坐标的地方?
void QMLScreen::updateOutputsPlacement()
{
    //qDebug()<<"updateOutputsPlacement---->"<<endl;
    if (width() <= 0)
        return;
    bool isTableMode = m_tmsettings->get(TABLET_MODE_KEY).toBool();
    QSizeF initialActiveScreenSize;

    Q_FOREACH (QQuickItem *item, childItems()) {
        QMLOutput *qmlOutput = qobject_cast<QMLOutput*>(item);
        if (!qmlOutput->output()->isConnected() || !qmlOutput->output()->isEnabled()) {
            continue;
        }

        if (qmlOutput->outputX() + qmlOutput->currentOutputWidth() > initialActiveScreenSize.width()) {
           // qDebug()<<qmlOutput->outputX()<<qmlOutput->currentOutputWidth()<<initialActiveScreenSize.width()<<endl;
            initialActiveScreenSize.setWidth(qmlOutput->outputX() + qmlOutput->currentOutputWidth());
        }
        if (qmlOutput->outputY() + qmlOutput->currentOutputHeight() > initialActiveScreenSize.height()) {
            initialActiveScreenSize.setHeight(qmlOutput->outputY() + qmlOutput->currentOutputHeight());
        }
    }

    auto initialScale = outputScale();

    //qDebug() << " -----debug0--->outputScale" << initialScale;
    auto scale = initialScale;
    qreal lastX = -1.0;
    do {
        auto activeScreenSize = initialActiveScreenSize * scale;

        const QPointF offset((width() - activeScreenSize.width()) / 2.0,
                             (height() - activeScreenSize.height()) / 2.0);

       // qDebug() << " ----------debug1--->offset-->" << offset;

        lastX = -1.0;
        qreal lastY = -1.0;
        Q_FOREACH (QQuickItem *item, childItems()) {
            QMLOutput *qmlOutput = qobject_cast<QMLOutput*>(item);
            if (!qmlOutput->output()->isConnected() || !qmlOutput->output()->isEnabled() ||
                 m_manuallyMovedOutputs.contains(qmlOutput)) {
                continue;
            }
            qmlOutput->blockSignals(true);
            if (qmlOutput->output()->isPrimary()) {
                qmlOutput->setPosition(QPointF(434-qmlOutput->width()/2,108));
                primaryX = qmlOutput->x();
                primaryY = qmlOutput->y();
                primaryWidth = qmlOutput->width();
                primaryHeight = qmlOutput->height();
            } else {
                expandHeight = qmlOutput->height();
                QSettings settings(m_initPath, QSettings::IniFormat);
                int begin_x = settings.value("qmlOutput_Position/start_x").toInt();
                int begin_y = settings.value("qmlOutput_Position/start_y").toInt();
                if (begin_x == NULL && begin_y == NULL) {
                    qmlOutput->setPosition(QPointF(509,108));
                } else if (begin_x < 0 || begin_x > 10000 || begin_y < 0 || begin_y > 10000) {
                    qmlOutput->setPosition(QPointF(509,108));
                    writeInit("qmlOutput_Position","start_x",509);
                    writeInit("qmlOutput_Position","start_y",108);
                } else {
                    if (isTableMode) {
                        qmlOutput->setVisible(false);
                    }
                    qmlOutput->setPosition(QPointF(begin_x,begin_y));
                }
            }
            lastX = qMax(lastX, qmlOutput->position().x() + qmlOutput->width() / initialScale * scale);
            lastY = qMax(lastY, qmlOutput->position().y());
           // qDebug()<<"坐标---->"<<lastX<<" "<<lastY<<endl;
            qmlOutput->blockSignals(false);
        }

        Q_FOREACH (QQuickItem *item, childItems()) {
            QMLOutput *qmlOutput = qobject_cast<QMLOutput*>(item);
            if (qmlOutput->output()->isConnected() && !qmlOutput->output()->isEnabled() &&
                !m_manuallyMovedOutputs.contains(qmlOutput)) {
                qmlOutput->blockSignals(true);
                qmlOutput->setPosition(QPointF(lastX, lastY));
                lastX += qmlOutput->width() / initialScale * scale;
                qmlOutput->blockSignals(false);
            }
        }
        // calculate the scale dynamically, so all screens fit to the dialog
        if (lastX > width()) {
            scale *= 0.8;
        }
    } while (lastX > width());
    // Use a timer to avoid binding loop on width()
//    QTimer::singleShot(0, this, [scale, this] {
//        setOutputScale(scale);
//    });
}
