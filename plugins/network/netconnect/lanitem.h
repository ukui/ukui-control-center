#ifndef LANITEM_H
#define LANITEM_H
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QVariantMap>
#include <QTimer>
#include <QDebug>
#include <QImage>
#include "Label/fixlabel.h"
#include "commonComponent/InfoButton/infobutton.h"

class LanItem : public QPushButton
{
public:
    LanItem(QWidget *parent = nullptr);
    ~LanItem();
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

};

#endif // LANITEM_H
