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
    ui->scrollArea->setStyleSheet("#scrollArea{border: 0px solid;}");

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

            ListWidgetItem * widgetitem = new ListWidgetItem(this);
            widgetitem->setLabelPixmap(QString("://dynamic/leftsidebar/%1.svg").arg(currentStringList.at(num)));
            widgetitem->setLabelText(currentStringList.at(num));
            QListWidgetItem * item = new QListWidgetItem(leftListWidget);
            item->setSizeHint(QSize(198,50));
            leftListWidget->setItemWidget(item, widgetitem);
//            QListWidgetItem * item = new QListWidgetItem(leftListWidget);
//            item->setSizeHint(QSize(198,50)); //widget width 200 -  border 2 = 198
//            item->setIcon(QIcon(QString("://dynamic/leftsidebar/%1.svg").arg(currentStringList.at(num))));
//            QFont font;
//            font.setPixelSize(14);
//            item->setFont(font);
//            item->setText(currentStringList.at(num));
//            leftListWidget->addItem(item);

            CommonInterface * pluginInstance = qobject_cast<CommonInterface *>(funcMaps[currentStringList.at(num)]);
            QWidget * widget = pluginInstance->get_plugin_ui();
            QStackedWidget * stackwidget = widget->findChild<QStackedWidget *>("StackedWidget");
            connect(stackwidget, SIGNAL(currentChanged(int)), this, SLOT(update_backbtn_text_slot(int)));
            widgetMaps.insert(currentStringList.at(num), widget);
        }
        ui->leftStackedWidget->addWidget(leftListWidget);
    }

    //pushbutton添加布局实现调整图标、文字距离调整
    backiconLabel = new QLabel(ui->backBtn);
//    backiconLabel->setFixedSize(QSize(16, 16));
    QSizePolicy iconpolicy = backiconLabel->sizePolicy();
    iconpolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    iconpolicy.setVerticalPolicy(QSizePolicy::Fixed);
    backiconLabel->setSizePolicy(iconpolicy);
    backiconLabel->setScaledContents(true);
    backiconLabel->setPixmap(QPixmap(":/back.svg"));

    backtextLabel = new QLabel(ui->backBtn);
    QSizePolicy policy = backtextLabel->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Fixed);
    policy.setVerticalPolicy(QSizePolicy::Fixed);
    backtextLabel->setSizePolicy(policy);
    backtextLabel->setScaledContents(true);
    backtextLabel->setText(tr("CCMainPage"));
    QHBoxLayout * btnLayout = new QHBoxLayout();
    btnLayout->addWidget(backiconLabel);
    btnLayout->addWidget(backtextLabel);
    btnLayout->addStretch();
    btnLayout->setSpacing(10);
    btnLayout->setContentsMargins(20,0,0,0);

    ui->backBtn->setLayout(btnLayout);


//    ui->backBtn->setText(tr("backtoMain"));

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
//    QListWidget * tmpListWidget = new QListWidget();
//    tmpListWidget->setAttribute(Qt::WA_DeleteOnClose);
    QListWidget * tmpListWidget = dynamic_cast<QListWidget *>(ui->leftStackedWidget->currentWidget());
    for (int i = 0; i < tmpListWidget->count(); i++){
        ListWidgetItem * widgetitem = dynamic_cast<ListWidgetItem *>(tmpListWidget->itemWidget(tmpListWidget->item(i)));
        if (name == (widgetitem->text()))
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

void ModulePageWidget::update_backbtn_text(int index){
    if (index != 0)
        backtextLabel->setText(tr("UpperLevel"));
    else
        backtextLabel->setText(tr("CCMainPage"));
}

void ModulePageWidget::itemClicked_cb(QListWidgetItem * item){
    QListWidget * tmpListWidget = dynamic_cast<QListWidget *>(ui->leftStackedWidget->currentWidget());
    ListWidgetItem * widgetitem = dynamic_cast<ListWidgetItem *>(tmpListWidget->itemWidget(item));
    if (widgetMaps.contains(widgetitem->text())){
        QWidget * pluginWidget = widgetMaps[widgetitem->text()];
        ui->scrollArea->takeWidget();
        delete(ui->scrollArea->widget());
        ui->scrollArea->setWidget(pluginWidget);

        //更新返回按钮text
        QStackedWidget * stackwidget = pluginWidget->findChild<QStackedWidget *>("StackedWidget");
        update_backbtn_text(stackwidget->currentIndex());
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

void ModulePageWidget::update_backbtn_text_slot(int index){
    update_backbtn_text(index);
}
