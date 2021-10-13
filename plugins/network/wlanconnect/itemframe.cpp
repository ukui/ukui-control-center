#include "itemframe.h"
#define LAYOUT_MARGINS 0,0,0,0
#define MAIN_LAYOUT_MARGINS 0,16,0,16
ItemFrame::ItemFrame(QString devName, QWidget *parent)
{
    deviceLanLayout = new QVBoxLayout(this);
    deviceLanLayout->setContentsMargins(MAIN_LAYOUT_MARGINS);
    lanItemFrame = new QFrame(this);
    lanItemFrame->setFrameShape(QFrame::Shape::NoFrame);
    lanItemFrame->setContentsMargins(LAYOUT_MARGINS);

    lanItemLayout = new QVBoxLayout(this);
    lanItemLayout->setContentsMargins(LAYOUT_MARGINS);
    lanItemLayout->setSpacing(1);

    deviceLanLayout->setSpacing(1);
    setLayout(deviceLanLayout);
    lanItemFrame->setLayout(lanItemLayout);

    deviceFrame = new DeviceFrame(devName, this);
    deviceLanLayout->addWidget(deviceFrame);
    deviceLanLayout->addWidget(lanItemFrame);

    //下拉按钮
    connect(deviceFrame->dropDownLabel, &DrownLabel::labelClicked, this, [=](){
        if (!deviceFrame->dropDownLabel->isChecked) {
            qDebug() << devName << " list show";
            lanItemFrame->show();
            deviceFrame->dropDownLabel->setDropDownStatus(true);
        } else {
            qDebug() << devName << " list hide";
            lanItemFrame->hide();
            deviceFrame->dropDownLabel->setDropDownStatus(false);
        }
    });
}

ItemFrame::~ItemFrame()
{

}
