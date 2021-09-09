#ifndef DEVICEFRAME_H
#define DEVICEFRAME_H
#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include "drownlabel.h"

class DeviceFrame : public QFrame
{

public:
    DeviceFrame(QWidget *parent = nullptr);
    ~DeviceFrame();
public:
    QLabel * deviceLabel = nullptr;
    DrownLabel *dropDownLabel = nullptr;

private:
    bool isDropDown = false;
    int frameSize;

};

#endif // DEVICEFRAME_H
