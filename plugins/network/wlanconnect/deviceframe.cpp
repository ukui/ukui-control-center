#include "deviceframe.h"

DeviceFrame::DeviceFrame(QWidget *parent) : QFrame(parent)
{
    this->setFrameShape(QFrame::Box);
    this->setFixedHeight(58);
    QHBoxLayout *deviceLayout = new QHBoxLayout(this);
    setLayout(deviceLayout);
    deviceLayout->setSpacing(16);

    deviceLabel = new QLabel(this);
    dropDownLabel = new DrownLabel(this);

    deviceLayout->addWidget(deviceLabel);
    deviceLayout->addStretch();
    deviceLayout->addWidget(dropDownLabel);
}

DeviceFrame::~DeviceFrame()
{

}

