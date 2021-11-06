#include "gesture.h"
#include "ui_gesture.h"
#include <QDir>
Gesture::Gesture() : mFirstLoad(true)
{
  pluginName = tr("Gesture");
  pluginType = DEVICES;
  QString path=":/img/plugins/gesture/gesture.ini";
  gesturesetting=new QSettings(path,QSettings::IniFormat);
}

Gesture::~Gesture()
{
  if (!mFirstLoad)
  {

      delete ui;

//      delete gifFlowLayout;
  }
  delete gesturesetting;
}
QString Gesture::plugini18nName() {
    return pluginName;
}

int Gesture::pluginTypes() {
    return pluginType;
}

QWidget *Gesture::pluginUi() {
    if (mFirstLoad) {
        mFirstLoad = false;
        ui = new Ui::Gesture;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);



        init();
        additem();

    }
    return pluginWidget;
}

const QString Gesture::name() const {
    return QStringLiteral("Gesture");
}

bool Gesture::isShowOnHomePage() const
{
    return false;
}

QIcon Gesture::icon() const
{
    return QIcon();
}

bool Gesture::isEnable() const
{
    return true;
}

void Gesture::init()
{
    //标题
    currentLabel = new TitleLabel();
    currentLabel->setText(tr("Gesture"));
    ui->horizontalLayout->addWidget(currentLabel);
//    currentLabel->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");
    gifFlowLayout=new FlowLayout(ui->frame,-1, 20,20);
    gifFlowLayout->setContentsMargins(0, 0, 0, 0);
//    gifFlowLayout->setSpacing(100);
//    ui->frame->setLayout(gifFlowLayout);


}
void Gesture::additem()
{
    gesturesetting->beginGroup("gesture");
    QStringList giflist= gesturesetting->allKeys();
    QMap<QString,int> list;
    for (int i = 0;  i< giflist.count();i ++) {
        list.insert(giflist.at(i),gesturesetting->value(giflist.at(i)).toInt());
    }
    gesturesetting->sync();
    gesturesetting->endGroup();
    qDebug()<<giflist.count();
    for(int i=0;i<giflist.count();i++){
        qDebug()<<"=="<<giflist.at(i);
        Itemwidget * item=new Itemwidget(list.key(i),ui->frame);
        gifFlowLayout->addWidget(item);
    }
}
