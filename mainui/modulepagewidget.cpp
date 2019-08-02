#include "modulepagewidget.h"
#include "ui_modulepagewidget.h"

#include "mainwindow.h"

ModulePageWidget::ModulePageWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ModulePageWidget)
{
    ui->setupUi(this);
    //set parent
    this->setParent(parent);
    pmainWindow = (MainWindow *)parentWidget();

//    QFont font;
//    font.setPointSize(14);
//    ui->modulesTitle->setFont(font);
//    ui->modulesTitle->setStyleSheet("color: #999999");

//    connect(ui->backBtn, SIGNAL(clicked(bool)), pmainWindow, SLOT(backToMain()));

    ui->widget->setStyleSheet("background-color: #ebeef0");
    ui->leftStackedWidget->setStyleSheet("border-style: none");
    ui->backBtn->setStyleSheet("border-style: none;");
//    ui->scrollArea->setStyleSheet("border: 1px solid #f5f6f7");

    connect(ui->backBtn, SIGNAL(clicked()), this, SLOT(backBtnClicked_cb()));

    initUI();
}

ModulePageWidget::~ModulePageWidget()
{
    for ( int i = 0; i < ui->leftStackedWidget->count(); i++){
        delete(ui->leftStackedWidget->widget(i));
    }
    delete ui;
}

void ModulePageWidget::initUI(){

    for (int i = 0; i < FUNCTOTALNUM; i++){
        QListWidget * leftListWidget = new QListWidget();
        connect(leftListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(itemClicked_cb(QListWidgetItem*)));

        QMap<QString, QObject *> funcMaps;
        funcMaps = pmainWindow->export_module(i);
        QStringList currentStringList = pmainWindow->subfuncList[i];

        for (int num = 0; num < currentStringList.size(); num++){
            if (!funcMaps.contains(currentStringList.at(num)))
                continue;

            QListWidgetItem * item = new QListWidgetItem(leftListWidget);
            item->setSizeHint(QSize(198,50)); //widget width 200 -  border 2 = 198
            QFont font;
            font.setPixelSize(14);
            item->setFont(font);
            item->setText(currentStringList.at(num));
            leftListWidget->addItem(item);

            CommonInterface * pluginInstance = qobject_cast<CommonInterface *>(funcMaps[currentStringList.at(num)]);
            widgetMaps.insert(currentStringList.at(num), pluginInstance->get_plugin_ui());
        }
        ui->leftStackedWidget->addWidget(leftListWidget);
    }

//    for (int i = 0; i < 2; i++){
//        QListWidget * leftListWidget = new QListWidget();
//        connect(leftListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(itemClicked_cb(QListWidgetItem*)));

//        QMap<QString, QObject *> funcMaps;
//        funcMaps = pmainWindow->export_module(i);
//        if (funcMaps.isEmpty()){
//            QListWidgetItem * item = new QListWidgetItem(leftListWidget);
//            item->setText("unavailable");
//            leftListWidget->addItem(item);
//        }
//        else{
//            QMap<QString, QObject *>::iterator it;
//            for (it = funcMaps.begin(); it != funcMaps.end(); ++it){
//                QListWidgetItem * item = new QListWidgetItem(leftListWidget);
//                item->setText(it.key());
//                leftListWidget->addItem(item);


//                CommonInterface * pluginInstance = qobject_cast<CommonInterface *>(it.value());
//                widgetMaps.insert(it.key(), pluginInstance->get_plugin_ui());
//            }
//        }
//        ui->leftStackedWidget->addWidget(leftListWidget);
//    }
}

void ModulePageWidget::setup_component(QObject * plugin){
    CommonInterface * pluginInstance = qobject_cast<CommonInterface *>(plugin);
    QString name; int type;
    name = pluginInstance->get_plugin_name();
    type = pluginInstance->get_plugin_type();

    //设置module title
    if (type == SYSTEM)
        ui->modulesTitle->setText(tr("System"));
    else if (type == DEVICES)
        ui->modulesTitle->setText(tr("Devices"));
    else if (type == PERSONALIZED)
        ui->modulesTitle->setText(tr("Personalized"));
    else if (type == NETWORK)
        ui->modulesTitle->setText(tr("Network"));
    else if (type == ACCOUNT)
        ui->modulesTitle->setText(tr("Account"));
    else if (type == TIME_LANGUAGE)
        ui->modulesTitle->setText(tr("Time Language"));
    else if (type == SECURITY_UPDATES)
        ui->modulesTitle->setText(tr("Security Updates"));
    else if (type == MESSAGES_TASK)
        ui->modulesTitle->setText(tr("Message Task"));

    ui->leftStackedWidget->setCurrentIndex(type);

    //高亮左侧边栏
    QListWidget * tmpListWidget = new QListWidget();
    tmpListWidget->setAttribute(Qt::WA_DeleteOnClose);
    tmpListWidget = (QListWidget *)ui->leftStackedWidget->currentWidget();
    for (int i = 0; i < tmpListWidget->count(); i++){
        if (name == (tmpListWidget->item(i)->text()))
            tmpListWidget->setCurrentRow(i);
    }

    if (!widgetMaps.contains(name))
        widgetMaps.insert(name, pluginInstance->get_plugin_ui()); //缓存

    if (widgetMaps[name]){
        ui->scrollArea->takeWidget();
        delete(ui->scrollArea->widget());//释放上次显示的ui
    }
    ui->scrollArea->setWidget(widgetMaps[name]);
}

void ModulePageWidget::itemClicked_cb(QListWidgetItem * item){
    if (widgetMaps.contains(item->text())){
        ui->scrollArea->takeWidget();
        delete(ui->scrollArea->widget());
        ui->scrollArea->setWidget(widgetMaps[item->text()]);
    }
    else{
        qDebug() << "plugin widget not found" ;
    }
}

void ModulePageWidget::backBtnClicked_cb(){
    QStackedWidget * tmpStackedWidget = ui->scrollArea->widget()->findChild<QStackedWidget *>("StackedWidget");
    if (tmpStackedWidget->currentIndex())
        tmpStackedWidget->setCurrentIndex(0);
    else
        pmainWindow->backToMain();
}
