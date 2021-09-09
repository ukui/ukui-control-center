#include "itemframe.h"
#define LAYOUT_MARGINS 0,0,0,0
#define MAIN_LAYOUT_MARGINS 0,16,0,16
ItemFrame::ItemFrame(QWidget *parent)
{
    deviceLanLayout = new QVBoxLayout(this);
    deviceLanLayout->setContentsMargins(MAIN_LAYOUT_MARGINS);
    lanItemFrame = new QFrame(this);
    lanItemFrame->setFrameShape(QFrame::Shape::NoFrame);

    lanItemLayout = new QVBoxLayout(this);
    lanItemLayout->setContentsMargins(LAYOUT_MARGINS);
    lanItemLayout->setSpacing(1);
    addLanWidget = new AddBtn(this);

    deviceLanLayout->setSpacing(1);
    setLayout(deviceLanLayout);
    lanItemFrame->setLayout(lanItemLayout);

    deviceFrame = new DeviceFrame(this);
    deviceLanLayout->addWidget(deviceFrame);
    deviceLanLayout->addWidget(lanItemFrame);
    deviceLanLayout->addWidget(addLanWidget);
}

ItemFrame::~ItemFrame()
{

}
