#ifndef ITEMFRAME_H
#define ITEMFRAME_H
#include <QFrame>
#include <QVBoxLayout>
#include "deviceframe.h"
#include <AddBtn/addbtn.h>
#include "addnetbtn.h"

class ItemFrame : public QFrame
{
    Q_OBJECT
public:
    ItemFrame(QWidget *parent = nullptr);
    ~ItemFrame();
    QVBoxLayout * deviceLanLayout = nullptr;
    DeviceFrame * deviceFrame = nullptr;
    QFrame * lanItemFrame = nullptr;
    QVBoxLayout * lanItemLayout = nullptr;
    AddNetBtn * addLanWidget = nullptr;
};

#endif // ITEMFRAME_H