#include <QtWidgets>

#include "display.h"
#include "ui_display.h"

#include <QDebug>

time_t bak_timestamp = 0;

typedef struct _ResolutionValue{
    int width;
    int height;
}ResolutionValue;

Q_DECLARE_METATYPE(MateRROutputInfo *)
Q_DECLARE_METATYPE(MateRRRotation)
Q_DECLARE_METATYPE(ResolutionValue)

DisplaySet::DisplaySet(){
    ui = new Ui::DisplayWindow();
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("display");

    pluginType = SYSTEM;

    brightnessgsettings = g_settings_new(POWER_MANAGER_SCHEMA);

    monitor_init();
    component_init();
    status_init();

}

DisplaySet::~DisplaySet(){
//    delete pluginWidget;
    delete ui;

    g_object_unref(brightnessgsettings);
    gtk_widget_destroy(monitor.window);
}

QWidget * DisplaySet::get_plugin_ui(){
    return pluginWidget;
}

QString DisplaySet::get_plugin_name(){
    return pluginName;
}

int DisplaySet::get_plugin_type(){
    return pluginType;
}

MateRROutputInfo * DisplaySet::_get_output_for_window(MateRRConfig *configuration, GdkWindow *window){
    GdkRectangle win_rect;
    int i;
    int largest_area;
    int largest_index;
    MateRROutputInfo **outputs;

    gdk_window_get_geometry (window, &win_rect.x, &win_rect.y, &win_rect.width, &win_rect.height);
    largest_area = 0;
    largest_index = -1;

    outputs = mate_rr_config_get_outputs (configuration);
    for (i = 0; outputs[i] != NULL; i++){
        GdkRectangle output_rect, intersection;

        mate_rr_output_info_get_geometry (outputs[i], &output_rect.x, &output_rect.y, &output_rect.width, &output_rect.height);
        if (mate_rr_output_info_is_connected (outputs[i]) && gdk_rectangle_intersect (&win_rect, &output_rect, &intersection)){
            int area;

            area = intersection.width * intersection.height;
            if (area > largest_area)
            {
            largest_area = area;
            largest_index = i;
            }
        }
    }

    if (largest_index != -1)
        return outputs[largest_index];
    else
        return NULL;
}

MateRRMode ** DisplaySet::_get_current_modes(){
    MateRROutput *output;

    if (mate_rr_config_get_clone (monitor.current_configuration)){
        return mate_rr_screen_list_clone_modes (monitor.screen);
    }
    else{
        if (!monitor.current_output)
            return NULL;

        output = mate_rr_screen_get_output_by_name (monitor.screen, mate_rr_output_info_get_name (monitor.current_output));

        if (!output)
            return NULL;

        return mate_rr_output_list_modes (output);
    }
}

void DisplaySet::monitor_init(){

    gtk_init(NULL, NULL);
    GError * error;
    error = NULL;
    monitor.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    monitor.screen = mate_rr_screen_new(gdk_screen_get_default(), &error);
    monitor.current_configuration = mate_rr_config_new_current(monitor.screen, NULL);
//    monitor.labeler = mate_rr_labeler_new(monitor.current_configuration);

    monitor.current_output = _get_output_for_window (monitor.current_configuration, gtk_widget_get_window(monitor.window));
}


void DisplaySet::rebuild_monitor_combo(){
    ui->monitorComboBox->clear();
    //monitor init
    int monitor_num = 0;
    MateRROutputInfo **outputs;

    outputs = mate_rr_config_get_outputs (monitor.current_configuration);
    for (int i = 0; outputs[i] != NULL; ++i){
        MateRROutputInfo *output = outputs[i];
        if (mate_rr_output_info_is_connected(output)){
            QString monitorname = QString(mate_rr_output_info_get_display_name (output)) + QString(mate_rr_output_info_get_name (output));
            ui->monitorComboBox->addItem(monitorname, QVariant::fromValue(output));
            monitor_num++;
        }
    }

    if (monitor_num <= 1)
        ui->monitorComboBox->setEnabled(false);

    //monitor status init
    if (monitor.current_output){
        QString monitorname = QString(mate_rr_output_info_get_display_name (monitor.current_output)) + QString(mate_rr_output_info_get_name (monitor.current_output));
        ui->monitorComboBox->setCurrentText(monitorname);
    }
}

void DisplaySet::rebuild_resolution_combo(){
    ui->resolutionComboBox->clear();
    //resolution init
    QStringList resolutionStringList;

    MateRRMode ** modes;

    if (!(modes = _get_current_modes ()) || !monitor.current_output || !mate_rr_output_info_is_active (monitor.current_output)){
        ui->resolutionComboBox->setEnabled(false);
    }
    else{
        ui->resolutionComboBox->setEnabled(true);

        int index = 0;
        for (int i = 0; modes[i] != NULL; ++i){
            int width, height;

            width = mate_rr_mode_get_width (modes[i]);
            height = mate_rr_mode_get_height (modes[i]);
            if(width <= 800 || height <= 600)
                continue;
            QString r = QString::number(width) + "*" + QString::number(height);
            if (!resolutionStringList.contains(r)){ //去重
                resolutionStringList.append(r);
                ResolutionValue value;
                value.width = width; value.height = height;
                ui->resolutionComboBox->insertItem(index, r, QVariant::fromValue(value));
                index++;
            }
        }
    }

    //resolution status init
    int output_width, output_height;
    mate_rr_output_info_get_geometry (monitor.current_output, NULL, NULL, &output_width, &output_height);
    QString current = QString::number(output_width) + "*" + QString::number(output_height);
    ui->resolutionComboBox->setCurrentText(current);
}

void DisplaySet::rebuild_refresh_combo(){
    ui->refreshComboBox->clear();

    MateRRMode ** modes;
    QList<int> rateList;

    if (!monitor.current_output || !(modes = _get_current_modes()) || !mate_rr_output_info_is_active(monitor.current_output)){
        ui->refreshComboBox->setEnabled(false);
    }
    else{
        ui->refreshComboBox->setEnabled(true);
        int output_width, output_height;
        mate_rr_output_info_get_geometry(monitor.current_output, NULL, NULL, &output_width, &output_height);
        int index = 0;
        for (int i = 0; modes[i] != NULL; i++){
            MateRRMode * mode = modes[i];
            int width, height, rate;
            width = mate_rr_mode_get_width(mode);
            height = mate_rr_mode_get_height(mode);
            rate = mate_rr_mode_get_freq(mode);
            if (width == output_width && height == output_height){
                if (!rateList.contains(rate)){
                    rateList.append(rate);
                    ui->refreshComboBox->insertItem(index, QString("%1 Hz").arg(QString::number(rate)), QVariant(rate));
                    index++;
                }
            }
        }
        ui->refreshComboBox->setCurrentText(QString("%1 Hz").arg(mate_rr_output_info_get_refresh_rate(monitor.current_output)));
    }
}

void DisplaySet::rebuild_rotation_combo(){
    ui->rotationComboBox->clear();

    //rotation init
    QString selection = NULL;
    typedef struct{
        MateRRRotation	rotation;
        QString	name;
    } RotationInfo;
    static const RotationInfo rotations[] = {
    { MATE_RR_ROTATION_0, tr("Normal") },
    { MATE_RR_ROTATION_90, tr("Left") },
    { MATE_RR_ROTATION_270, tr("Right") },
    { MATE_RR_ROTATION_180, tr("Upside-down") },
    };


    ui->rotationComboBox->setEnabled(monitor.current_output && mate_rr_output_info_is_active (monitor.current_output));
    if (monitor.current_output){
        MateRRRotation current;
        current = mate_rr_output_info_get_rotation (monitor.current_output);
        int index = 0;
        for (unsigned long i = 0; i < G_N_ELEMENTS (rotations); ++i){
            const RotationInfo *info = &(rotations[i]);
            mate_rr_output_info_set_rotation (monitor.current_output, info->rotation);

            if (mate_rr_config_applicable (monitor.current_configuration, monitor.screen, NULL)){
                ui->rotationComboBox->insertItem(index, info->name, QVariant::fromValue(info->rotation));
//                ui->rotationComboBox->addItem(info->name);
                index++;

                if (info->rotation == current)
                    selection = info->name;
            }
        }
        mate_rr_output_info_set_rotation(monitor.current_output, current);
        ui->rotationComboBox->setCurrentText(selection);
    }

}

void DisplaySet::component_init(){
    rebuild_monitor_combo();
    rebuild_resolution_combo();
    rebuild_refresh_combo();
    rebuild_rotation_combo();
}

void DisplaySet::status_init(){

    //brightness init
    double value;
    value = g_settings_get_double(brightnessgsettings, BRIGHTNESS_AC_KEY);
    ui->brightnessHSlider->setValue((int)value);
    ui->brightnessLabel->setText(QString("%1%").arg((int)value));

    connect(ui->brightnessHSlider, SIGNAL(valueChanged(int)), this, SLOT(brightness_value_changed_slot(int)));
    connect(ui->rotationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(refresh_changed_slot(int)));
    connect(ui->rotationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(rotation_changed_slot(int)));
    connect(ui->resolutionComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(resolution_changed_slot(int)));
    connect(ui->monitorComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(monitor_changed_slot(int)));
    connect(ui->ApplyBtn, SIGNAL(clicked(bool)), this, SLOT(apply_btn_clicked_slot()));
}

void DisplaySet::brightness_value_changed_slot(int value){
    ui->brightnessLabel->setText(QString("%1%").arg(value));
    g_settings_set_double(brightnessgsettings, BRIGHTNESS_AC_KEY, (double)value);
}

void DisplaySet::refresh_changed_slot(int index){
    if (monitor.current_output){
        int rate = ui->refreshComboBox->itemData(index).toInt();
        mate_rr_output_info_set_refresh_rate(monitor.current_output, rate);
    }
}

void DisplaySet::rotation_changed_slot(int index){
    if (monitor.current_output){
        MateRRRotation rotation;
        rotation = (ui->rotationComboBox->itemData(index)).value<MateRRRotation>();
        mate_rr_output_info_set_rotation(monitor.current_output, rotation);
    }
}

void DisplaySet::_realign_output_after_resolution_changed(MateRROutputInfo *output_changed, int oldwidth, int oldheight){
    int x, y, width, height;
    int dx, dy;
    int old_right_edge, old_bottom_edge;
    MateRROutputInfo ** outputs;

    if (monitor.current_configuration != NULL){
        mate_rr_output_info_get_geometry(output_changed, &x, &y, &width, &height);
        if (width == oldwidth && height == oldheight)
            return;
        old_right_edge = x + oldwidth; old_bottom_edge = y + oldheight;

        dx = width - oldwidth; dy = height - oldheight;

        outputs = mate_rr_config_get_outputs(monitor.current_configuration);

        for (int i = 0; outputs[i] != NULL; i++){
            int output_x, output_y;
            int output_width, output_height;
            if (outputs[i] == output_changed || mate_rr_output_info_is_connected(outputs[i]))
                continue;
            mate_rr_output_info_get_geometry(outputs[i], &output_x, &output_y, &output_width, &output_height);

            if (output_x >= old_right_edge)
                output_x += dx;
            else if (output_x + output_width == old_right_edge){
                output_x = x + width - output_width;
            }

            if (output_y >= old_bottom_edge)
                output_y += dy;
            else if (output_y + output_height == old_bottom_edge)
                output_y = y + height - output_height;

            mate_rr_output_info_set_geometry(outputs[i], output_x, output_y, output_width, output_height);
        }

    }
}

void DisplaySet::resolution_changed_slot(int index){
    if (monitor.current_output){
        int old_width, old_height;
        int x, y;

        mate_rr_output_info_get_geometry(monitor.current_output, &x, &y, &old_width, &old_height);

        ResolutionValue setValue;
        setValue = ui->resolutionComboBox->itemData(index).value<ResolutionValue>();

        mate_rr_output_info_set_geometry(monitor.current_output, x, y, setValue.width, setValue.height);

        if (setValue.width == 0 || setValue.height == 0)
            mate_rr_output_info_set_active(monitor.current_output, FALSE);
        else
            mate_rr_output_info_set_active(monitor.current_output, TRUE);

        _realign_output_after_resolution_changed(monitor.current_output, old_width, old_height);

        rebuild_refresh_combo();
        rebuild_rotation_combo();
    }
}

void DisplaySet::monitor_changed_slot(int index){
    if (monitor.current_output){
        MateRROutputInfo * output = ui->monitorComboBox->itemData(index).value<MateRROutputInfo *>();
        if (output){
            monitor.current_output = output;
            rebuild_resolution_combo();
            rebuild_refresh_combo();
            rebuild_rotation_combo();
        }
    }
}

void DisplaySet::_ensure_current_configuration_is_saved(){
    MateRRScreen * rr_screen;
    MateRRConfig * rr_config;

    /* Normally, mate_rr_config_save() creates a backup file based on the
     * old monitors.xml.  However, if *that* file didn't exist, there is
     * nothing from which to create a backup.  So, here we'll save the
     * current/unchanged configuration and then let our caller call
     * mate_rr_config_save() again with the new/changed configuration, so
     * that there *will* be a backup file in the end.
     */

    rr_screen = mate_rr_screen_new(gdk_screen_get_default(), NULL);

    if (!rr_screen)
        return;

    rr_config = mate_rr_config_new_current(rr_screen, NULL);
    mate_rr_config_save(rr_config, NULL);

    g_object_unref(rr_config);
    g_object_unref(rr_screen);
}

gboolean DisplaySet::_sanitize_save_configuration(){
    GError * error;
    mate_rr_config_sanitize(monitor.current_configuration);

    _ensure_current_configuration_is_saved();

    error = NULL;
    if(!mate_rr_config_save(monitor.current_configuration, &error)){
        g_warning("Could not save the monitor configuration \n%s", error->message);
        g_error_free(error);
        return FALSE;
    }
    return TRUE;

}

void DisplaySet::apply_btn_clicked_slot(){

    time_t rawtime;
    rawtime = time(NULL);
    if (abs(bak_timestamp - rawtime) >= 7){ //7s内连击无效
        bak_timestamp = rawtime;
        qDebug() << rawtime;
        monitor.ApplyBtnClickTimeStamp = GDK_CURRENT_TIME;
    }

    GError * error = NULL;

    if (!_sanitize_save_configuration())
        return;
    monitor.connection = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
    if (monitor.connection == NULL){
        g_warning("Could not get session bus while applying display configuration");
        g_error_free(error);
        return;
    }

    _begin_version2_apply_configuration();
}

void DisplaySet::_begin_version2_apply_configuration(){
//    XID parent_window_xid;
//    parent_window_xid = GDK_WINDOW_XID(gtk_widget_get_window(monitor.window));

    monitor.proxy = dbus_g_proxy_new_for_name(monitor.connection,
                                              "org.ukui.SettingsDaemon",
                                              "/org/ukui/SettingsDaemon/XRANDR",
                                              "org.ukui.SettingsDaemon.XRANDR_2");

    if (monitor.proxy == NULL)
        return;
    g_warning("aaaaaaaaaaaaaaa");

}
