#include <QWidget>
#include <QDebug>

#include "defaultapp.h"
#include "ui_defaultapp.h"
#include "addappdialog.h"

#define BROWSERTYPE "x-scheme-handler/http"
#define MAILTYPE "x-scheme-handler/mailto"
#define IMAGETYPE "image/png"
#define AUDIOTYPE "audio/x-vorbis+ogg"
#define VIDEOTYPE "video/x-ogm+ogg"
#define TEXTTYPE "text/plain"

#define DESKTOPPATH "/usr/share/applications/"

DefaultApp::DefaultApp(){
    ui = new Ui::DefaultAppWindow;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("defaultapp");
    pluginType = SYSTEM;

    initUI();
}

DefaultApp::~DefaultApp(){
    delete ui;
}

QString DefaultApp::get_plugin_name(){
    return pluginName;
}

int DefaultApp::get_plugin_type(){
    return pluginType;
}

QWidget * DefaultApp::get_plugin_ui(){
    return pluginWidget;
}

void DefaultApp::initUI(){
    // BROWSER
    int browserindex = -1;
    QString currentbrowser(kylin_software_defaultprograms_getdefaultappid(BROWSERTYPE)); //获取当前

    AppList * list = kylin_software_defaultprograms_getappidlist(BROWSERTYPE); //获取可选列表
    for (int i = 0; list[i].appid != NULL; i++){
        QString single(list[i].appid);
        QByteArray ba = (DESKTOPPATH + single).toUtf8();
        GDesktopAppInfo * browserinfo = g_desktop_app_info_new_from_filename(ba.constData());
        QString appname = g_app_info_get_name(G_APP_INFO(browserinfo));
//        qDebug() << appname;
        const char * iconname = g_icon_to_string(g_app_info_get_icon(G_APP_INFO(browserinfo)));
        QIcon appicon;
        if (QIcon::hasThemeIcon(QString(iconname)))
            appicon = QIcon::fromTheme(QString(iconname));

        ui->browserComBoBox->addItem(appicon, appname, single);
        if (currentbrowser == single)
            browserindex = i;
    }
    ui->browserComBoBox->addItem("add", "add");
    ui->browserComBoBox->setCurrentIndex(browserindex);
    connect(ui->browserComBoBox, SIGNAL(itemchangedSignal(int)), this, SLOT(browserComBoBox_changed_cb(int)));

    // MAIL
    int mailindex = -1;
    QString currentmail(kylin_software_defaultprograms_getdefaultappid(MAILTYPE));

    list = kylin_software_defaultprograms_getappidlist(MAILTYPE);
    for (int i = 0; list[i].appid != NULL; i++){
        QString single(list[i].appid);
        ui->mailComBoBox->addItem(single);
        if (currentmail == single)
            mailindex = i;
    }
    ui->mailComBoBox->setCurrentIndex(mailindex);
    connect(ui->mailComBoBox, SIGNAL(currentTextChanged(QString)), this, SLOT(mailComBoBox_changed_cb(QString)));

    // IMAGE
    int imageindex = -1;
    QString currentimage(kylin_software_defaultprograms_getdefaultappid(IMAGETYPE));

    list = kylin_software_defaultprograms_getappidlist(IMAGETYPE);
    for (int i = 0; list[i].appid != NULL; i++){
        QString single(list[i].appid);
        ui->imageComBoBox->addItem(single);
        if (currentimage == single)
            imageindex = i;
    }
    ui->imageComBoBox->setCurrentIndex(imageindex);
    connect(ui->imageComBoBox, SIGNAL(currentTextChanged(QString)), this, SLOT(imageComBoBox_changed_cb(QString)));

    // AUDIO
    int audioindex = -1;
    QString currentaudio(kylin_software_defaultprograms_getdefaultappid(AUDIOTYPE));

    list = kylin_software_defaultprograms_getappidlist(AUDIOTYPE);
    for (int i = 0; list[i].appid != NULL; i++){
        QString single(list[i].appid);
        ui->audioComBoBox->addItem(single);
        if (currentaudio == single)
            audioindex = i;
    }
    ui->audioComBoBox->setCurrentIndex(audioindex);
    connect(ui->audioComBoBox, SIGNAL(currentTextChanged(QString)), this, SLOT(audioComBoBox_changed_cb(QString)));

    // VIDEO
    int videoindex =-1;
    QString currentvideo(kylin_software_defaultprograms_getdefaultappid(VIDEOTYPE));

    list = kylin_software_defaultprograms_getappidlist(VIDEOTYPE);
    for (int i = 0; list[i].appid != NULL; i++){
        QString single(list[i].appid);
        ui->videoComBoBox->addItem(single);
        if (currentvideo == single)
            videoindex = i;
    }
    ui->videoComBoBox->setCurrentIndex(videoindex);
    connect(ui->videoComBoBox, SIGNAL(currentTextChanged(QString)), this, SLOT(videoComBoBox_changed_cb(QString)));

    // TEXT
    int textindex = -1;
    QString currenttext(kylin_software_defaultprograms_getdefaultappid(TEXTTYPE));

    list = kylin_software_defaultprograms_getappidlist(TEXTTYPE);
    for (int i = 0; list[i].appid != NULL; i++){
        QString single(list[i].appid);
        ui->textComBoBox->addItem(single);
        if (currenttext == single)
            textindex = i;
        free(list[i].appid);
    }
    ui->textComBoBox->setCurrentIndex(textindex);
    connect(ui->textComBoBox, SIGNAL(currentTextChanged(QString)), this, SLOT(textComBoBox_changed_cb(QString)));
    free(list);
}

void DefaultApp::browserComBoBox_changed_cb(int index){
//    QString appid = ui->browserComBoBox->currentData().toString();
    QString appid = ui->browserComBoBox->itemData(index).toString();
    if (appid == "add"){
//        AddAppDialog * dialog = new AddAppDialog();
//        dialog->show();
        qDebug() << "add clicked";
    }
    else{
        QByteArray ba = appid.toUtf8(); // QString to char *
        kylin_software_defaultprograms_setwebbrowsers(ba.data());
    }
}

void DefaultApp::mailComBoBox_changed_cb(QString appid){
    qDebug() << "-------->" << appid;
    QByteArray ba = appid.toUtf8(); // QString to char *
    kylin_software_defaultprograms_setmailreaders(ba.data());
}

void DefaultApp::imageComBoBox_changed_cb(QString appid){
    qDebug() << "-------->" << appid;
    QByteArray ba = appid.toUtf8(); // QString to char *
    kylin_software_defaultprograms_setimageviewers(ba.data());
}

void DefaultApp::audioComBoBox_changed_cb(QString appid){
    qDebug() << "-------->" << appid;
    QByteArray ba = appid.toUtf8(); // QString to char *
    kylin_software_defaultprograms_setaudioplayers(ba.data());
}

void DefaultApp::videoComBoBox_changed_cb(QString appid){
    qDebug() << "-------->" << appid;
    QByteArray ba = appid.toUtf8(); // QString to char *
    kylin_software_defaultprograms_setvideoplayers(ba.data());
}

void DefaultApp::textComBoBox_changed_cb(QString appid){
    qDebug() << "-------->" << appid;
    QByteArray ba = appid.toUtf8(); // QString to char *
    kylin_software_defaultprograms_settexteditors(ba.data());
}
