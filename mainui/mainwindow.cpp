#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //background color
    QPalette bgPalette;
    bgPalette.setBrush(QPalette::Window, QBrush(Qt::gray));
//    this->setPalette(bgPalette);

    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Widget);

    connect(ui->minBtn, SIGNAL(clicked()), this, SLOT(showMinimized()));
    connect(ui->closeBtn, SIGNAL(clicked(bool)), this, SLOT(close()));

    loadPlugins();

    mainpageWidget = new MainPageWidget(this);
    ui->stackedWidget->addWidget(mainpageWidget);

    modulepageWidget = new ModulePageWidget(this);
    ui->stackedWidget->addWidget(modulepageWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::mousePressEvent(QMouseEvent * event){
    mousePress = false;
    if (event->button() == Qt::LeftButton){
        mousePress = true;
        dragPos = event->globalPos() - pos();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event){
    if (event->button() & Qt::LeftButton){
        mousePress = false;
//        setWindowOpacity(1); //设置透明度,1不透明
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event){
    if (event->buttons() & Qt::LeftButton){
        if (mousePress){
            move(event->globalPos() - dragPos);
//            setWindowOpacity(0.5);
        }
    }
}

void MainWindow::backToMain(){
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::pluginClicked_cb(QObject * plugin){
    ui->stackedWidget->setCurrentIndex(1);
    modulepageWidget->setup_component(plugin);
}

void MainWindow::loadPlugins(){
//    moduleDir = QDir(qApp->applicationDirPath());
//    moduleDir.cd("plugins");
//    foreach (QString dirname, moduleDir.entryList(QDir::Dirs)) {
//        if (dirname == "." || dirname == "..")
//            continue;
//        pluginDir = QDir(moduleDir.currentPath());
//        pluginDir.cd("plugins");
//        pluginDir.cd(dirname);

//        //遍历
//        QMap<QString, QObject *> pluginsMaps;
//        foreach (QString fileName, pluginDir.entryList(QDir::Files)){
//            QPluginLoader loader(pluginDir.absoluteFilePath(fileName));
//            QObject * plugin = loader.instance();
//            if (plugin){
//                CommonInterface * pluginInstance = qobject_cast<CommonInterface *>(plugin);
//                pluginsMaps.insert(pluginInstance->get_plugin_name(), plugin);
//            }
//        }
//        modulesList.append(pluginsMaps);
//    }

    for (int index = 0; index < FUNCTOTALNUM; index++){
        QMap<QString, QObject *> pluginsMaps;
        QStringList pluginStringList;
        modulesList.append(pluginsMaps);
        subfuncList.append(pluginStringList);
    }

    pluginsDir = QDir(qApp->applicationDirPath());
    pluginsDir.cd("plugins");

    foreach (QString fileName, pluginsDir.entryList(QDir::Files)){
//        qDebug() << fileName;
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QObject * plugin = loader.instance();
        if (plugin){
            CommonInterface * pluginInstance = qobject_cast<CommonInterface *>(plugin);
            modulesList[pluginInstance->get_plugin_type()].insert(pluginInstance->get_plugin_name(), plugin);
        }
    }

//    QMap<QString, QObject *> tmpMaps;
//    QMap<QString, QObject *>::iterator it;
//    tmpMaps = modulesList[DEVICES];
//    for (it = tmpMaps.begin(); it != tmpMaps.end(); it++){
//        qDebug() << it.key() << "------";
//    }
}

QMap<QString, QObject *> MainWindow::export_module(int type){
    QMap<QString, QObject *> emptyMaps;
    if (type < modulesList.length())
        return modulesList[type];
    else
        return emptyMaps;
}
