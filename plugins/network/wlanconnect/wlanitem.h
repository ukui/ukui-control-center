#ifndef WLANITEM_H
#define WLANITEM_H
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QVariantMap>
#include <QTimer>
#include <QDebug>
#include <QGSettings>
#include <QImage>
#include "Label/fixlabel.h"
#include "commonComponent/InfoButton/infobutton.h"

class WlanItem : public QPushButton
{
public:
    WlanItem(QWidget *parent = nullptr);
    ~WlanItem();
public:
    QLabel * iconLabel = nullptr;
    InfoButton * infoLabel = nullptr;
    FixLabel * titileLabel = nullptr;
    FixLabel * statusLabel = nullptr;
    int waitPage;
    int countCurrentTime;
public:
    void startLoading();
    void setWaitPage(int waitPage);
    void setCountCurrentTime(int countCurrentTime);
    void waitAnimStep();
    void stopLoading();

    bool loading = false;
private:
    QTimer *waitTimer = nullptr;
    QGSettings *themeGsettings = nullptr;

};

#endif // WLANITEM_H
