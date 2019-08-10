#include "screenlock.h"
#include "ui_screenlock.h"

#include <QDebug>

#define BGPATH "/usr/share/backgrounds/"
#define SCREENLOCK_BG_SCHEMA "org.ukui.screensaver"
#define SCREENLOCK_BG_KEY "background"

Screenlock::Screenlock()
{
    ui = new Ui::Screenlock;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("screenlock");
    pluginType = PERSONALIZED;

    bggsettings = g_settings_new(SCREENLOCK_BG_SCHEMA);

    component_init();
    status_init();


}

Screenlock::~Screenlock()
{
    delete ui;
    g_object_unref(bggsettings);
}

QString Screenlock::get_plugin_name(){
    return pluginName;
}

int Screenlock::get_plugin_type(){
    return pluginType;
}

QWidget * Screenlock::get_plugin_ui(){
    return pluginWidget;
}

void Screenlock::component_init(){
    //背景形式
    QStringList formList;
    formList << tr("picture");
    ui->formComboBox->addItems(formList);

    //同步登录背景的控件SwitchButton
    setloginbgBtn = new SwitchButton();
    setloginbgBtn->setAttribute(Qt::WA_DeleteOnClose);
    ui->setloginbgHLayout->addWidget(setloginbgBtn);
    ui->setloginbgHLayout->addStretch();
}

void Screenlock::status_init(){
    //获取当前锁屏背景及登录背景，对比后确定按钮状态
    QString bgfilename = QString(g_settings_get_string(bggsettings, SCREENLOCK_BG_KEY));
    QPixmap bg = QPixmap(bgfilename);
    ui->bgLabel->setPixmap(bg.scaled(QSize(400, 240)));

    connect(ui->openPushBtn, SIGNAL(clicked()), this, SLOT(openpushbtn_clicked_slot()));
}

void Screenlock::openpushbtn_clicked_slot(){

    QString filters = "Image files(*.png *.jpg)";
    QFileDialog fd;
    fd.setDirectory(BGPATH);
    fd.setAcceptMode(QFileDialog::AcceptOpen);
    fd.setViewMode(QFileDialog::List);
    fd.setNameFilter(filters);
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setWindowTitle(tr("selsect screenlock background"));
    fd.setLabelText(QFileDialog::Accept, "Select");

    if (fd.exec() != QDialog::Accepted)
        return;

    QString selectedfile;
    selectedfile = fd.selectedFiles().first();

    QPixmap bg = QPixmap(selectedfile);
    ui->bgLabel->setPixmap(bg.scaled(QSize(400, 240)));

    //QString to char *
    QByteArray ba = selectedfile.toLatin1();

    g_settings_set_string(bggsettings, SCREENLOCK_BG_KEY, ba.data());
}
