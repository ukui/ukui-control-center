#include "autoboot.h"
#include "ui_autoboot.h"

AutoBoot::AutoBoot(){
    ui = new Ui::AutoBoot;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("autoboot");
    pluginType = SYSTEM;


    ui->tableWidget->setStyleSheet("background-color: #f5f6f7");
    //列头高度
    ui->tableWidget->horizontalHeader()->setMinimumHeight(38);

    //列宽
    ui->tableWidget->setColumnWidth(0, 440);
    ui->tableWidget->setColumnWidth(1, 318);

    // 隐藏行头
    ui->tableWidget->verticalHeader()->hide();

    // 选择整行
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    // 行平均填充
//    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    initUI();
}

AutoBoot::~AutoBoot()
{
    delete ui;
}

QString AutoBoot::get_plugin_name(){
    return pluginName;
}

int AutoBoot::get_plugin_type(){
    return pluginType;
}

QWidget * AutoBoot::get_plugin_ui(){
    return pluginWidget;
}

void AutoBoot::checkbox_changed_cb(QString bname){
    foreach (QString key, appgroupMultiMaps.keys()) {
        if (key == bname){
            if (((QCheckBox *)appgroupMultiMaps.value(key))->isChecked()){
                ((QCheckBox *)appgroupMultiMaps.value(key))->setText("already boot");
                if (appMaps.contains(bname)){
                    QMap<QString, AutoApp>::iterator it = appMaps.find(bname);
                    if (it.value().enable && localappMaps.contains(bname)){
                        QMap<QString, AutoApp>::iterator localit = localappMaps.find(bname);
                        QByteArray tranpath = localit.value().path.toUtf8();
                        g_remove(tranpath.data());
                        localappMaps.remove(bname);

                        QMap<QString, AutoApp>::iterator updateit = statusMaps.find(bname);
                        updateit.value().enable = it.value().enable;
                    }
                }
            }
            else{
                ((QCheckBox *)appgroupMultiMaps.value(key))->setText("already stop");
                if (localappMaps.contains(bname)){
                    QMap<QString, AutoApp>::iterator localit = localappMaps.find(bname);
                    if (localit.value().enable){ //删除后重新创建
                        qDebug() << "unlawful";
                        QByteArray tranpath = localit.value().path.toUtf8();
                        g_remove(tranpath.data());
                        localappMaps.remove(bname);
                    }
                }
                _stop_autoapp(bname);
            }
        }
    }
}


void AutoBoot::initUI(){
    _walk_config_dirs();

    appgroupMultiMaps.clear();
    ui->tableWidget->setRowCount(appMaps.count());

    QSignalMapper * checkboxSignalMapper = new QSignalMapper(this);
    QMap<QString, AutoApp>::iterator it = statusMaps.begin();
    for (int i = 0; it != statusMaps.end(); it++, i++){
        QTableWidgetItem * item = new QTableWidgetItem(it.value().qicon, it.value().name);
        ui->tableWidget->setItem(i, 0, item);


        QCheckBox * checkBox = new QCheckBox();
        checkBox->QAbstractButton::setChecked(it.value().enable);
        if (checkBox->isChecked())
            checkBox->setText(tr("already boot"));
        else
            checkBox->setText(tr("already stop"));
        connect(checkBox, SIGNAL(clicked()), checkboxSignalMapper, SLOT(map()));
        checkboxSignalMapper->setMapping(checkBox, it.key());
        appgroupMultiMaps.insert(it.key(), checkBox);
        ui->tableWidget->setCellWidget(i, 1, checkBox);

        //设置行高
        ui->tableWidget->setRowHeight(i, 46);
    }
    connect(checkboxSignalMapper, SIGNAL(mapped(QString)), this, SLOT(checkbox_changed_cb(QString)));
}

gboolean AutoBoot::_stop_autoapp(QString bname){
    char * dstpath;
    GFile * srcfile;
    GFile * dstfile;

    QMap<QString, AutoApp>::iterator it = appMaps.find(bname);
    QByteArray tranpath = it.value().path.toUtf8();
    srcfile = g_file_new_for_path(tranpath.data());

    QByteArray tranbname = it.value().bname.toUtf8();
    dstpath = g_build_filename(g_get_user_config_dir(), "autostart", tranbname.data(), NULL);
    dstfile = g_file_new_for_path(dstpath);

    if (!g_file_copy(srcfile, dstfile, G_FILE_COPY_NONE, NULL, NULL, NULL, NULL)){
        qDebug() << "Could not copy desktop file";
        g_object_unref(srcfile);
        g_object_unref(dstfile);
        g_free(dstpath);
        return FALSE;
    }

    GKeyFile * keyfile;
    GError * error;
    keyfile = g_key_file_new();
    error = NULL;
    g_key_file_load_from_file(keyfile, dstpath, G_KEY_FILE_KEEP_COMMENTS, &error);


    if (error){
        g_error_free(error);
        qDebug() << "keyfile load from file error";
    }

    g_key_file_set_boolean(keyfile, G_KEY_FILE_DESKTOP_GROUP, APP_KEY_FILE_DESKTOP_KEY_AUTOSTART_ENABLE, false);

    if (!_key_file_to_file(keyfile, dstpath))
        qDebug() << "Could not save desktop file";

    g_key_file_free(keyfile);

    AutoApp addapp;
    addapp = _app_new(dstpath);

    localappMaps.insert(addapp.bname, addapp);
    QMap<QString, AutoApp>::iterator updateit = statusMaps.find(addapp.bname);
    updateit.value().enable = addapp.enable;

    g_object_unref(srcfile);
    g_object_unref(dstfile);
    return TRUE;
}

gboolean AutoBoot::_key_file_to_file(GKeyFile *keyfile, const gchar *path){
    GError * werror;
    gchar * data;
    gsize length;
    gboolean res;

    werror = NULL;
    data = g_key_file_to_data(keyfile, &length, &werror);

    if (werror)
        return FALSE;

    res = g_file_set_contents(path, data, length, &werror);
    g_free(data);

    if (werror)
        return FALSE;

    return res;

}

gboolean AutoBoot::_key_file_get_shown(GKeyFile *keyfile, const char *currentdesktop){
    char ** only_show_in, ** not_show_in;
    gboolean found;

    if (!currentdesktop)
        return TRUE;

    only_show_in = g_key_file_get_string_list(keyfile, G_KEY_FILE_DESKTOP_GROUP,
                                              G_KEY_FILE_DESKTOP_KEY_ONLY_SHOW_IN, NULL, NULL);
    if (only_show_in){
        found = FALSE;
        for (int i = 0; only_show_in[i] != NULL; i++){
            if (g_strcmp0(currentdesktop, only_show_in[i]) == 0){
                found = TRUE;
                break;
            }
        }
        g_strfreev(only_show_in);
        if (!found)
            return FALSE;
    }

    not_show_in = g_key_file_get_string_list(keyfile, G_KEY_FILE_DESKTOP_GROUP,
                                             G_KEY_FILE_DESKTOP_KEY_NOT_SHOW_IN, NULL, NULL);
    if (not_show_in){
        found = FALSE;
        for (int i = 0; not_show_in[i] != NULL; i++){
            if (g_strcmp0(currentdesktop, not_show_in[i]) == 0){
                found = TRUE;
                break;
            }
        }
        g_strfreev(not_show_in);
        if (found)
            return FALSE;
    }

    return TRUE;
}

gboolean AutoBoot::_key_file_get_boolean(GKeyFile *keyfile, const gchar *key, gboolean defaultvalue){
    GError * error;
    gboolean retval;

    error = NULL;
    retval = g_key_file_get_boolean(keyfile, G_KEY_FILE_DESKTOP_GROUP, key, &error);

    if (error != NULL){
        retval = defaultvalue;
        g_error_free(error);
    }
    return retval;
}

AutoApp AutoBoot::_app_new(const char *path){
    AutoApp app;
    GKeyFile * keyfile;
    char * bname, * obpath, *name, * comment, * exec, * icon;
    bool  hidden, no_display, enable, shown;

    app.bname = "";
    keyfile = g_key_file_new();
    if (!g_key_file_load_from_file(keyfile, path, G_KEY_FILE_NONE, NULL)){
        g_key_file_free (keyfile);
        return app;
    }

    bname = g_path_get_basename(path);
    obpath = g_strdup(path);
    hidden = _key_file_get_boolean(keyfile, G_KEY_FILE_DESKTOP_KEY_HIDDEN, FALSE);
    no_display = _key_file_get_boolean(keyfile, G_KEY_FILE_DESKTOP_KEY_NO_DISPLAY, FALSE);
    enable = _key_file_get_boolean(keyfile, APP_KEY_FILE_DESKTOP_KEY_AUTOSTART_ENABLE, TRUE);
    shown = _key_file_get_shown(keyfile, g_getenv("XDG_CURRENT_DESKTOP"));
    name = g_key_file_get_locale_string(keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, NULL, NULL);
    comment = g_key_file_get_locale_string(keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_COMMENT, NULL, NULL);
    exec = g_key_file_get_string(keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_EXEC, NULL);
    icon = g_key_file_get_locale_string(keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ICON, NULL, NULL);

    app.bname = QString::fromUtf8(bname);
    app.path = QString::fromUtf8(obpath);

    app.hidden = hidden;
    app.no_display = no_display;
    app.shown = shown;
    app.enable = enable;

    app.name = QString::fromUtf8(name);
    app.comment = QString::fromUtf8(comment);
    app.exec = QString::fromUtf8(exec);
    if (icon){
        if (QIcon::hasThemeIcon(QString(icon))){
            app.qicon = QIcon::fromTheme(QString(icon));
        }
        else{
            QString absolutepath = "/usr/share/pixmaps/" + QString(icon);
            app.qicon = QIcon(absolutepath);
        }
    }
    else
        app.qicon = QIcon(QString(":/image/default.png"));

    g_free(bname);
    g_free(obpath);
    g_key_file_free(keyfile);

    return app;
}

void AutoBoot::_walk_config_dirs(){
    const char * const * systemconfigdirs;
    char * localconfigdir;
    GDir * dir;
    const char * name;

    appMaps.clear();

    systemconfigdirs = g_get_system_config_dirs(); //获取系统配置目录
    for (int i = 0; systemconfigdirs[i]; i++){
        char * path;
        path = g_build_filename(systemconfigdirs[i], "autostart", NULL);
        dir = g_dir_open(path, 0, NULL);
        if (!dir)
            continue;
        while ((name = g_dir_read_name(dir))) {
            AutoApp app;
            char * desktopfilepath;
            if (!g_str_has_suffix(name, ".desktop"))
                continue;
            desktopfilepath = g_build_filename(path, name, NULL);
            app = _app_new(desktopfilepath);
            if (app.bname == "" || app.hidden || app.no_display || !app.shown ||
                    app.exec == "/usr/bin/ukui-settings-daemon") //gtk控制面板屏蔽ukui-settings-daemon,猜测禁止用户关闭
                continue;
            appMaps.insert(app.bname, app);
            g_free (desktopfilepath);
        }
        g_dir_close(dir);
    }

    localappMaps.clear();
    localconfigdir = g_build_filename(g_get_user_config_dir(), "autostart", NULL);
    dir = g_dir_open(localconfigdir, 0, NULL);
    if (dir){
        while ((name = g_dir_read_name(dir))) {
            AutoApp localapp;
            char * localdesktopfilepath;
            if (!g_str_has_suffix(name, ".desktop"))
                continue;
            localdesktopfilepath = g_build_filename(localconfigdir, name, NULL);
            localapp = _app_new(localdesktopfilepath);

            localappMaps.insert(localapp.bname, localapp); 
            g_free(localdesktopfilepath);
        }
        g_dir_close(dir);
    }
    update_app_status();
}

void AutoBoot::update_app_status(){
    statusMaps.clear();

    QMap<QString, AutoApp>::iterator it = appMaps.begin();
    for (; it != appMaps.end(); it++){
        statusMaps.insert(it.key(), it.value());
    }

    QMap<QString, AutoApp>::iterator localit = localappMaps.begin();
    for (; localit != localappMaps.end(); localit++){
        if (appMaps.contains(localit.key())){
            if (localit.value().hidden || localit.value().no_display || !localit.value().shown)
                statusMaps.remove(localit.key());
            QMap<QString, AutoApp>::iterator updateit = statusMaps.find(localit.key());
            updateit.value().enable = localit.value().enable;
        }
        else
            statusMaps.insert(localit.key(), localit.value());
    }
}
