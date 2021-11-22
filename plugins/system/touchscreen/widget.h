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

#ifndef WIDGET_H
#define WIDGET_H

//
#include <QWidget>
#include <QVariantMap>
#include <QGSettings>
#include <QSettings>
#include <QButtonGroup>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusReply>
#include <QStringList>
#include <QSharedPointer>
#include <QDir>

#include <KF5/KScreen/kscreen/config.h>




class QLabel;
class QMLOutput;
class QMLScreen;
class PrimaryOutputCombo;

class QPushButton;
class QComboBox;
class QStyledItemDelegate;
class XinputManager;

namespace KScreen
{
class ConfigOperation;
}

namespace Ui {
class TouchScreen;
}

class Widget : public QWidget
{
    Q_OBJECT

  public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

    void setConfig(const KScreen::ConfigPtr &config);
    KScreen::ConfigPtr currentConfig() const;

    void slotFocusedOutputChangedNoParam();
    void initConnection();
    void initui();
    QString getScreenName(QString name = "");
    bool event(QEvent *event);


  protected:


  Q_SIGNALS:
    void changed();

  private Q_SLOTS:

    void slotFocusedOutputChanged(QMLOutput *output);

    void slotOutputConnectedChanged();

    void outputAdded(const KScreen::OutputPtr &output);
    void outputRemoved(int outputId);
    void touchscreenAdded();
    void touchscreenRemoved();
    void curOutoutChanged(int index);
    void curTouchScreenChanged(int index);
    void primaryOutputSelected(int index);

  public Q_SLOTS:
    void maptooutput();
    void CalibratTouch();

  private:
    void loadQml();
    void save(QString touchname,QString touchid,QString screenname);
    void initTouchConfig(QString touchserial,QString touchname,QString screenname);
    void writeTouchConfig();
    void writeTouchConfig(QString touchname,QString touchid,QString touchserial,QString devnode ,QString screenname);
    bool Configserialisexit(QString touchserial,QString devnode,QString touchname);
    void cleanTouchConfig(int touchcount);
    int  compareserial(int touchcount);
    int comparescreenname(QString _touchserial,QString _touchname ,QString _screenname);
    void resetPrimaryCombo();
    void resettouchscreenCombo();
    void addOutputToMonitorCombo(const KScreen::OutputPtr &output);
    void addTouchScreenToTouchCombo(const QString touchscreenname );
    bool findTouchScreen();
    QString findTouchScreenName(int devicesid);
    KScreen::OutputPtr findOutput(const KScreen::ConfigPtr &config, const QVariantMap &info);

  private:
    Ui::TouchScreen *ui;
    XinputManager *m_pXinputManager;
    QMLScreen *mScreen = nullptr;
    QSettings *configIni;
    QDir *qdir;
    QString CurTouchScreenName = "";
    QString CurMonitorName = "";
    QString CurDevicesName="";
    int CurTouchscreenNum;

#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)
    KScreen::ConfigPtr mConfig ;
    KScreen::ConfigPtr mPrevConfig ;
    //这是outPutptr结果
    KScreen::OutputPtr res ;
#else
    KScreen::ConfigPtr mConfig = nullptr;
    KScreen::ConfigPtr mPrevConfig = nullptr;
    // outPutptr结果
    KScreen::OutputPtr res = nullptr;
#endif

    QButtonGroup *singleButton;


    bool mOriApply;
    bool mConfigChanged   = false;
    bool mOnBattery       = false;
    bool m_blockChanges = false;

};

#endif // WIDGET_H
