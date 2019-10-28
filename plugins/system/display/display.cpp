/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include <QtWidgets>

#include "display.h"
#include "ui_display.h"

#include <QDebug>

#define MARGIN 15
#define SPACE 15

static void screen_changed();

time_t bak_timestamp = 0;

typedef struct _ResolutionValue{
    int width;
    int height;
}ResolutionValue;

Q_DECLARE_METATYPE(MateRROutputInfo *)
Q_DECLARE_METATYPE(MateRRRotation)
Q_DECLARE_METATYPE(ResolutionValue)

QList<Edge> edges;
QList<Snap> snaps;
QList<Edge> new_edges;

DisplaySet::DisplaySet(){
    ui = new Ui::DisplayWindow();
    pluginWidget = new CustomWidget;
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

CustomWidget *DisplaySet::get_plugin_ui(){
    return pluginWidget;
}

QString DisplaySet::get_plugin_name(){
    return pluginName;
}

int DisplaySet::get_plugin_type(){
    return pluginType;
}

void DisplaySet::plugin_delay_control(){

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

    g_signal_connect(monitor.screen, "changed", G_CALLBACK(screen_changed), NULL);
}

static void screen_changed(){
    qDebug() << "----------666----->";
}

int DisplaySet::_connect_output_count(){
    int monitor_num = 0;
    MateRROutputInfo **outputs;

    outputs = mate_rr_config_get_outputs (monitor.current_configuration);
    for (int i = 0; outputs[i] != NULL; ++i){
        MateRROutputInfo *output = outputs[i];
        if (mate_rr_output_info_is_connected(output)){
            monitor_num++; //获取当前活动的显示器个数
        }
    }
    return monitor_num;
}

int DisplaySet::_active_output_count(){
    int monitor_num = 0;
    MateRROutputInfo **outputs;

    outputs = mate_rr_config_get_outputs (monitor.current_configuration);
    for (int i = 0; outputs[i] != NULL; ++i){
        MateRROutputInfo *output = outputs[i];
        if (mate_rr_output_info_is_active(output)){
            monitor_num++; //获取当前活动的显示器个数
        }
    }
    return monitor_num;
}

void DisplaySet::rebuild_ui(){
    int monitor_num = _connect_output_count();

    if (monitor_num > 1){
        ui->multipleWidget->show();
        ui->primaryBtn->show();

        rebuild_monitor_switchbtn();
        rebuild_mirror_monitor();

        //设置主屏按钮敏感性
        ui->primaryBtn->setEnabled(monitor.current_output && !mate_rr_output_info_get_primary(monitor.current_output));

    }
    else{
        ui->multipleWidget->hide();
        ui->primaryBtn->hide();
    }

    rebuild_monitor_combo();
    rebuild_resolution_combo();
    rebuild_refresh_combo();
    rebuild_rotation_combo();

    rebuild_view();
}

void DisplaySet::rebuild_view(){
    scene->clear();

    int total_w, total_h;
    double scale = monitor_item_scale();
    _connect_output_total_resolution(&total_w, &total_h);

    MateRROutputInfo **outputs;
    outputs = mate_rr_config_get_outputs (monitor.current_configuration);

    for (int i = 0; outputs[i] != NULL; i++){

        if (mate_rr_output_info_is_connected(outputs[i])){
            int offset_x, offset_y, output_w, output_h;
            double x, y;
            QString monitorname, monitortype;
            MateRROutputInfo * output = outputs[i];
            mate_rr_output_info_get_geometry(output, &offset_x, &offset_y, &output_w, &output_h);
            monitorname = QString(mate_rr_output_info_get_display_name(output));
            monitortype = QString(mate_rr_output_info_get_name(output));

            QPointF offset;
            offset.setX(offset_x); offset.setY(offset_y);

            //计算坐标
            x = offset_x * scale + (scene->width() - total_w * scale)/2.0;
            y = offset_y * scale + (scene->height() - total_h * scale)/2.0;

//            qDebug() << monitorname << monitortype;
//            qDebug() << "----------->" << x << y;
//            qDebug() << "---->" << output_w * scale << output_h * scale;
            GraphicsItem * item = new GraphicsItem();
            item->setRectF(output_w * scale, output_h * scale);
            item->setData(Qt::UserRole, QVariant::fromValue(offset));
            item->setMonitorInfo(monitorname, monitortype);
            item->setPos(x + (output_w * scale)/2, y + (output_h * scale)/2);
//            qDebug() << "first pos" <<  x + (output_w * scale)/2 << y + (output_h * scale)/2;
            connect(item, SIGNAL(dragOverSignal()), this, SLOT(output_drabed_slot()));
            scene->addItem(item);

            if (_connect_output_count() == 1) //单屏
                item->setitemMoveFlags(false);
            else
                item->setitemMoveFlags(true);

        }

    }

}

void DisplaySet::rebuild_monitor_switchbtn(){
    bool sensitive;
    bool active;

    activemonitorBtn->blockSignals(true);

    sensitive = false;
    active = false;

    if (!mate_rr_config_get_clone (monitor.current_configuration) && monitor.current_output){
//        if (_active_output_count() > 1 || !mate_rr_output_info_is_active (monitor.current_output))
//            sensitive = true;
//        else
//            sensitive = false;

        active = mate_rr_output_info_is_active (monitor.current_output);
    }

    activemonitorBtn->setChecked(active);

//    activemonitorBtn->setEnabled(sensitive);

    activemonitorBtn->blockSignals(false);
}

double DisplaySet::monitor_item_scale(){
    int available_w, available_h;
    int total_w, total_h;
    int monitors;

    //计算屏幕总大小
    monitors = _connect_output_count();
    _connect_output_total_resolution(&total_w, &total_h);

    available_w = scene->width() - 2 * MARGIN - (monitors - 1) * SPACE;
    available_h = scene->height() - 2 * MARGIN - (monitors - 1) * SPACE;

    return qMin(static_cast<double>(available_w) / total_w, static_cast<double>(available_h) / total_h);
}

void DisplaySet::_connect_output_total_resolution(int *total_w, int *total_h){
    MateRROutputInfo ** outputs;

    *total_w = 0; *total_h = 0;

    outputs = mate_rr_config_get_outputs(monitor.current_configuration);
    for (int i = 0; outputs[i] != NULL; i++){
        if (mate_rr_output_info_is_connected(outputs[i])){
            int w, h;
            _get_geometry(outputs[i], &w, &h);

            *total_w += w;
            *total_h += h;
        }
    }
}

void DisplaySet::_get_geometry(MateRROutputInfo *output, int *w, int *h){
    MateRRRotation rotation;

    if (mate_rr_output_info_is_active(output)){
        mate_rr_output_info_get_geometry(output, NULL, NULL, w, h);
    }
    else{
        *w = mate_rr_output_info_get_preferred_width(output);
        *h = mate_rr_output_info_get_preferred_height(output);
    }

    rotation = mate_rr_output_info_get_rotation(output);
    if ((rotation & MATE_RR_ROTATION_90) || (rotation & MATE_RR_ROTATION_270)){
        int tmp;
        tmp = *h;
        *h = *w;
        *w = tmp;
    }
}

gboolean DisplaySet::_get_clone_size(int *width, int *height){

    MateRRMode **modes = mate_rr_screen_list_clone_modes (monitor.screen);
    int best_w, best_h;

    best_w = 0;
    best_h = 0;

    for (int i = 0; modes[i] != NULL; ++i) {
            MateRRMode *mode = modes[i];
            int w, h;

            w = mate_rr_mode_get_width (mode);
            h = mate_rr_mode_get_height (mode);

            if (w * h > best_w * best_h) {
                    best_w = w;
                    best_h = h;
            }
    }

    if (best_w > 0 && best_h > 0) {
            if (width)
                    *width = best_w;
            if (height)
                    *height = best_h;

            return TRUE;
    }

    return FALSE;
}

gboolean DisplaySet::_output_info_supports_mode(MateRROutputInfo *info, int width, int height){
    MateRROutput *output;
    MateRRMode **modes;

    if (!mate_rr_output_info_is_connected (info))
        return FALSE;

    output = mate_rr_screen_get_output_by_name (monitor.screen, mate_rr_output_info_get_name (info));
    if (!output)
        return FALSE;

    modes = mate_rr_output_list_modes (output);

    for (int i = 0; modes[i]; i++) {
        int tmp_width, tmp_height;
        tmp_width = mate_rr_mode_get_width(modes[i]); tmp_height = mate_rr_mode_get_width(modes[i]);
        if (tmp_width == width && tmp_height == height)
            return TRUE;
    }

    return FALSE;
}

/* Computes whether "Mirror Screens" (clone mode) is supported based on these criteria:
 *
 * 1. There is an available size for cloning.
 *
 * 2. There are 2 or more connected outputs that support that size.
 */
gboolean DisplaySet::_mirror_screen_is_supported(){
    int clone_width, clone_height;
    gboolean have_clone_size;
    gboolean mirror_is_supported;

    mirror_is_supported = FALSE;

    have_clone_size = _get_clone_size(&clone_width, &clone_height);

    if (have_clone_size) {
        int num_outputs_with_clone_size;
        MateRROutputInfo **outputs = mate_rr_config_get_outputs (monitor.current_configuration);

        num_outputs_with_clone_size = 0;

        for (int i = 0; outputs[i] != NULL; i++)
        {
            /* We count the connected outputs that support the clone size.  It
         * doesn't matter if those outputs aren't actually On currently; we
         * will turn them on in on_clone_changed().
         */
            if (mate_rr_output_info_is_connected (outputs[i]) && _output_info_supports_mode (outputs[i], clone_width, clone_height))
                num_outputs_with_clone_size++;
        }

        if (num_outputs_with_clone_size >= 2)
            mirror_is_supported = TRUE;
    }

    return mirror_is_supported;
}

void DisplaySet::rebuild_mirror_monitor(){
    gboolean is_active;
    gboolean is_supported;

    mirrormonitorBtn->blockSignals(true);

    is_active = monitor.current_configuration && mate_rr_config_get_clone(monitor.current_configuration);

    if (is_active)
        mirrormonitorBtn->setChecked(TRUE);
    else
        mirrormonitorBtn->setChecked(FALSE);

    is_supported = is_active || _mirror_screen_is_supported();

    if (is_supported)
        mirrormonitorBtn->setEnabled(true);
    else
        mirrormonitorBtn->setEnabled(false);

    mirrormonitorBtn->blockSignals(false);
}

void DisplaySet::rebuild_monitor_combo(){
    //monitor init
    int monitor_num = 0;
    MateRROutputInfo **outputs;

    ui->monitorComboBox->blockSignals(true); //阻塞

    ui->monitorComboBox->clear();

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

    ui->monitorComboBox->blockSignals(false);
}

void DisplaySet::rebuild_resolution_combo(){
    //resolution init
    QStringList resolutionStringList;

    MateRRMode ** modes;

    ui->resolutionComboBox->blockSignals(true);

    ui->resolutionComboBox->clear();

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

    ui->resolutionComboBox->blockSignals(false);
}

void DisplaySet::rebuild_refresh_combo(){
    MateRRMode ** modes;
    QList<int> rateList;

    ui->refreshComboBox->blockSignals(true);

    ui->refreshComboBox->clear();

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

    ui->refreshComboBox->blockSignals(false);
}

void DisplaySet::rebuild_rotation_combo(){
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

    ui->rotationComboBox->clear();

    ui->rotationComboBox->blockSignals(true);

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

    ui->rotationComboBox->blockSignals(false);

}

void DisplaySet::component_init(){
    activemonitorBtn = new SwitchButton(pluginWidget);
    ui->enableHLayout->addWidget(activemonitorBtn);
    ui->enableHLayout->addStretch();

    mirrormonitorBtn = new SwitchButton(pluginWidget);
    ui->mirrorHLayout->addWidget(mirrormonitorBtn);
    ui->mirrorHLayout->addStretch();

    //设置Scene
    scene = new QGraphicsScene;
    //    scene->setSceneRect(-ui->graphicsView->width()/2, -ui->graphicsView->height()/2, ui->graphicsView->width() - 2, ui->graphicsView->height() - 2);
    scene->setSceneRect(0, 0, ui->graphicsView->width() - 2, ui->graphicsView->height() - 2);

    ui->graphicsView->setScene(scene);

    rebuild_ui();

    ui->brightnessWidget->setVisible(support_brightness());
}

void DisplaySet::status_init(){

    //brightness init
    double value;
    value = g_settings_get_double(brightnessgsettings, BRIGHTNESS_AC_KEY);
    ui->brightnessHSlider->setValue((int)value);
    ui->brightnessLabel->setText(QString("%1%").arg((int)value));

    connect(scene, SIGNAL(selectionChanged()), this, SLOT(selected_item_changed_slot()));

    connect(activemonitorBtn, SIGNAL(checkedChanged(bool)), this, SLOT(monitor_active_changed_slot()));
    connect(ui->primaryBtn, SIGNAL(clicked(bool)), this, SLOT(set_primary_clicked_slot()));

    connect(ui->brightnessHSlider, SIGNAL(valueChanged(int)), this, SLOT(brightness_value_changed_slot(int)));
    connect(ui->rotationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(refresh_changed_slot(int)));
    connect(ui->rotationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(rotation_changed_slot(int)));
    connect(ui->resolutionComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(resolution_changed_slot(int)));
    connect(ui->monitorComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(monitor_changed_slot(int)));
    connect(ui->ApplyBtn, SIGNAL(clicked(bool)), this, SLOT(apply_btn_clicked_slot()));
}

bool DisplaySet::brightness_setup_display(){
    int major, minor;
    Display * dpy = GDK_DISPLAY_XDISPLAY(gdk_display_get_default());
    if (!dpy){
        g_warning("Cannot Open Dispaly");
        return false;
    }
    if(!XRRQueryExtension(dpy, &major, &minor)){
        g_warning("RandR extension missing");
        return false;
    }
    if (major < 1 || (major == 1 && minor < 2)){
        g_warning("RandR version %d.%d  too old", major, minor);
        return false;
    }

    Atom backlight = XInternAtom(dpy, "BACKLIGHT", True);
    if (backlight == None){
        g_warning("No outputs have backlight property");
        return false;
    }
    return true;
}

bool DisplaySet::isNumber(QString str){
    QByteArray ba = str.toLatin1();
    const char *s = ba.data();
    bool flag = true;
    while(*s){
        if (*s >= '0' && *s <= '9'){

        }
        else{
            flag = false;
        }
        s++;
    }
    return flag;
}

bool DisplaySet::support_brightness(){
    bool hasExtension;

    hasExtension = brightness_setup_display();
    if (hasExtension)
        return true;

    QString cmd = "/usr/sbin/ukui-power-backlight-helper";
    QStringList args = {"--get-max-brightness"};
    QProcess process;
    process.start(cmd, args);
    process.waitForFinished();

    QString output = QString(process.readAllStandardOutput()).simplified();
    if (isNumber(output) && output.toInt() > 0) //如果output不全是数字返回false则不会执行output.toInt()
        return true;
    return false;
}

gboolean DisplaySet::_output_overlaps(MateRROutputInfo *output){
    GdkRectangle output_rect;
    MateRROutputInfo **outputs;

    mate_rr_output_info_get_geometry (output, &output_rect.x, &output_rect.y, &output_rect.width, &output_rect.height);

    outputs = mate_rr_config_get_outputs (monitor.current_configuration);
    for (int i = 0; outputs[i]; ++i){
        if (outputs[i] != output && mate_rr_output_info_is_connected (outputs[i])){

            GdkRectangle other_rect;

            mate_rr_output_info_get_geometry(outputs[i], &other_rect.x, &other_rect.y, &other_rect.width, &other_rect.height);
            if (gdk_rectangle_intersect (&output_rect, &other_rect, NULL))
                return TRUE;
        }
    }
    return FALSE;
}

void DisplaySet::layout_outputs_horizontally(){
    int x;
    MateRROutputInfo **outputs;

    /* Lay out all the monitors horizontally when "mirror screens" is turned
     * off, to avoid having all of them overlapped initially.  We put the
     * outputs turned off on the right-hand side.
     */

    x = 0;

    /* First pass, all "on" outputs */
    outputs = mate_rr_config_get_outputs (monitor.current_configuration);

    for (int i = 0; outputs[i]; ++i){
        int width, height;

        if (mate_rr_output_info_is_connected (outputs[i]) &&mate_rr_output_info_is_active (outputs[i])){
            mate_rr_output_info_get_geometry (outputs[i], NULL, NULL, &width, &height);
            mate_rr_output_info_set_geometry (outputs[i], x, 0, width, height);
            x += width;
        }
    }

    /* Second pass, all the black screens */

    for (int i = 0; outputs[i]; ++i){
        int width, height;

        if (!(mate_rr_output_info_is_connected (outputs[i]) && mate_rr_output_info_is_active (outputs[i]))){
            mate_rr_output_info_get_geometry (outputs[i], NULL, NULL, &width, &height);
            mate_rr_output_info_set_geometry (outputs[i], x, 0, width, height);
            x += width;
        }
    }
}

void DisplaySet::_add_edge(MateRROutputInfo *output, int x1, int y1, int x2, int y2, bool diff_edges){
    Edge e;
    e.x1 = x1;
    e.x2 = x2;
    e.y1 = y1;
    e.y2 = y2;
    e.output = output;

    if (!diff_edges)
        edges.append(e);
    else
        new_edges.append(e);
}

void DisplaySet::_list_edges(bool diff_edges){
    MateRROutputInfo ** outputs = mate_rr_config_get_outputs(monitor.current_configuration);

    for (int i = 0; outputs[i]; i++){
        if (mate_rr_output_info_is_connected(outputs[i])){
            int x, y, w, h;
            mate_rr_output_info_get_geometry(outputs[i], &x, &y, &w, &h);
            _add_edge(outputs[i], x, y, x + w, y, diff_edges);
            _add_edge(outputs[i], x, y + h, x + w, y + h, diff_edges);
            _add_edge(outputs[i], x, y, x, y + h, diff_edges);
            _add_edge(outputs[i], x + w, y, x + w, y + h, diff_edges);
        }
    }
}

bool DisplaySet::_overlap(int s1, int e1, int s2, int e2){
    return (!(e1 < s2 || s1 >= e2));
}

bool DisplaySet::_horizontal_overlap(Edge *snapper, Edge *snappee){
    if (snapper->y1 != snapper->y2 || snappee->y1 != snappee->y2)
        return false;
    return _overlap(snapper->x1, snapper->x2, snappee->x1, snappee->x2);
}

bool DisplaySet::_vertical_overlap(Edge *snapper, Edge *snappee){
    if (snapper->x1 != snapper->x2 || snappee->x1 != snappee->x2)
        return false;
    return _overlap(snapper->y1, snapper->y2, snappee->y1, snappee->y2);
}

void DisplaySet::_add_snap(Snap snap){
    if (ABS(snap.dx) <= 200 || ABS(snap.dy) <= 200)
        snaps.append(snap);
}

void DisplaySet::_add_edge_snaps(Edge *snapper, Edge *snappee){
    Snap snap;
    snap.snapper = snapper;
    snap.snappee = snappee;

    if (_horizontal_overlap(snapper, snappee)){
        snap.dx = 0;
        snap.dy = snappee->y1 - snapper->y1;

        _add_snap(snap);
    }
    else if (_vertical_overlap(snapper, snappee)){
        snap.dy = 0;
        snap.dx = snappee->x1 - snapper->x1;

        _add_snap(snap);
    }

    /* 1->1? */
    snap.dx = snappee->x1 - snapper->x1;
    snap.dy = snappee->y1 - snapper->y1;
    _add_snap(snap);

    /* 1->2? */
    snap.dx = snappee->x2 - snapper->x1;
    snap.dy = snappee->y2 - snapper->y1;
    _add_snap(snap);

    /* 2->2? */
    snap.dx = snappee->x2 - snapper->x2;
    snap.dy = snappee->y2 - snapper->y2;
    _add_snap(snap);

    /* 2->1? */
    snap.dx = snappee->x1 - snapper->x2;
    snap.dy = snappee->y1 - snapper->y2;
    _add_snap(snap);
}

void DisplaySet::_list_snaps(){
    for (int i = 0; i < edges.length(); i++){
        Edge * output_edge = const_cast<Edge *>(&(edges.at(i)));
        if (edges.at(i).output == monitor.current_output){
            for (int j = 0; j < edges.length(); j++){
                Edge * edge = const_cast<Edge *>(&(edges.at(j)));
                if (edges.at(j).output != monitor.current_output)

                    _add_edge_snaps(output_edge, edge);
            }
        }
    }
}

bool DisplaySet::_corner_on_edge(int x, int y, Edge *e){
    if (x == e->x1 && x == e->x2 && y >= e->y1 && y <= e->y2)
        return true;

    if (y == e->y1 && y == e->y2 && x >= e->x1 && x <= e->x2)
        return true;
    return false;
}

bool DisplaySet::_edges_align(Edge *e1, Edge *e2){
    if (_corner_on_edge(e1->x1, e1->y1, e2))
        return true;

    if (_corner_on_edge(e2->x1, e2->y1, e1))
        return true;
    return false;
}

bool DisplaySet::_output_is_aligned(MateRROutputInfo *output){

    for (int i = 0; i < new_edges.length(); i++){
        Edge * output_edge = const_cast<Edge *>(&(new_edges.at(i)));

        if (output_edge->output == output){
            for (int j = 0; j < new_edges.length(); j++){
                Edge * edge = const_cast<Edge *>(&(new_edges.at(j)));

                if (edge->output != output_edge->output){
                    if (_edges_align(output_edge, edge))
                        return true;
                }
            }
        }
    }
    return false;
}

bool DisplaySet::_config_is_aligned(){

    MateRROutputInfo ** outputs = mate_rr_config_get_outputs(monitor.current_configuration);

    for (int i = 0; outputs[i]; i++){
        if (mate_rr_output_info_is_connected(outputs[i])){
            if (!_output_is_aligned(outputs[i]))
                return false;
            if (_output_overlaps(outputs[i]))
                return true;
        }
    }
    return true;
}

void DisplaySet::output_drabed_slot(){

    QObject * object = QObject::sender();
    GraphicsItem * item = dynamic_cast<GraphicsItem *>(object);

    int width, height;
    int new_x, new_y;

    double scale = monitor_item_scale();

    mate_rr_output_info_get_geometry(monitor.current_output, NULL, NULL, &width, &height);

    QPointF originoffset;
    originoffset = item->data(Qt::UserRole).toPointF();


    new_x = originoffset.x() + (item->pos().x() - item->getLastPos().x()) / scale;
    new_y = originoffset.y() + (item->pos().y() - item->getLastPos().y()) / scale;

    edges.clear(); snaps.clear();

    _list_edges(false);
    _list_snaps();

    //g_array_sort();

    mate_rr_output_info_set_geometry(monitor.current_output, new_x, new_y, width, height);

    for (int i = 0; i < snaps.length(); i++){
        Snap * snap = const_cast<Snap *>(&(snaps.at(i)));
        mate_rr_output_info_set_geometry(monitor.current_output, new_x + snap->dx, new_y + snap->dy, width, height);

        new_edges.clear();
        _list_edges(true);

        if (_config_is_aligned()){
            QPointF offset;
            offset.setX(new_x + snap->dx); offset.setY(new_y + snap->dy);
            item->setData(Qt::UserRole, QVariant::fromValue(offset));
            break;
        }
        else{
            mate_rr_output_info_set_geometry(monitor.current_output, originoffset.x(), originoffset.y(), width, height);
        }
    }
}


void DisplaySet::selected_item_changed_slot(){
    qDebug() << "Selected" << scene->selectedItems().length();
}

void DisplaySet::mirror_monitor_changed_slot(){
    mate_rr_config_set_clone (monitor.current_configuration, mirrormonitorBtn->isChecked());

    if (mate_rr_config_get_clone (monitor.current_configuration)){
        int width, height;
        MateRROutputInfo **outputs = mate_rr_config_get_outputs (monitor.current_configuration);

        for (int i = 0; outputs[i]; ++i){
            if (mate_rr_output_info_is_connected(outputs[i]))
            {
                monitor.current_output = outputs[i];
                break;
            }
        }

        /* Turn on all the connected screens that support the best clone mode.
         * The user may hit "Mirror Screens", but he shouldn't have to turn on
         * all the required outputs as well.
         */

        _get_clone_size (&width, &height);

        for (int i = 0; outputs[i]; i++) {
            int x, y;
            if (_output_info_supports_mode (outputs[i], width, height)) {
                mate_rr_output_info_set_active (outputs[i], TRUE);
                mate_rr_output_info_get_geometry (outputs[i], &x, &y, NULL, NULL);
                mate_rr_output_info_set_geometry (outputs[i], x, y, width, height);
            }
        }

    }
    else{ //关闭镜像模式
//        if (output_overlaps (app->current_output, app->current_configuration))
//            lay_out_outputs_horizontally (app);
        if (_output_overlaps(monitor.current_output))
            layout_outputs_horizontally();
    }

    rebuild_ui ();
}

void DisplaySet::monitor_active_changed_slot(){
    if (!monitor.current_output)
        return;

    if (activemonitorBtn->isChecked()){
        mate_rr_output_info_set_active(monitor.current_output, TRUE);

        _select_resolution_for_current_output(); //重新设置最佳分辨率

    }
    else
        mate_rr_output_info_set_active(monitor.current_output, FALSE);

    rebuild_ui();
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

void DisplaySet::_select_resolution_for_current_output(){
    MateRRMode **modes;
    int width, height;
    int x, y;

    mate_rr_output_info_get_geometry (monitor.current_output, &x, &y, NULL, NULL);

    width = mate_rr_output_info_get_preferred_width (monitor.current_output);
    height = mate_rr_output_info_get_preferred_height (monitor.current_output);

    if (width != 0 && height != 0){
        mate_rr_output_info_set_geometry (monitor.current_output, x, y, width, height);
        return;
    }

    modes = _get_current_modes ();
    if (!modes)
        return;

    //find best
    for (int i = 0; modes[i] != NULL; i++){
        int w, h;

        w = mate_rr_mode_get_width (modes[i]);
        h = mate_rr_mode_get_height (modes[i]);

        if (w * h > width * height){
            width = w;
            height = h;
        }
    }

    mate_rr_output_info_set_geometry (monitor.current_output, x, y, width, height);
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

void DisplaySet::set_primary_clicked_slot(){
    MateRROutputInfo **outputs;

    if (!monitor.current_output)
        return;

    outputs = mate_rr_config_get_outputs (monitor.current_configuration);
    for (int i=0; outputs[i]!=NULL; i++) {
        mate_rr_output_info_set_primary (outputs[i], outputs[i] == monitor.current_output);
    }

    ui->primaryBtn->setEnabled(!mate_rr_output_info_get_primary(monitor.current_output));
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
//        qDebug() << rawtime;
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

    XID window_xid;
//    parent_window_xid = GDK_WINDOW_XID(gtk_widget_get_window(monitor.window));
    window_xid = GDK_WINDOW_XID(gdk_screen_get_root_window (gdk_screen_get_default()));

    monitor.proxy = dbus_g_proxy_new_for_name(monitor.connection,
                                              "org.ukui.SettingsDaemon",
                                              "/org/ukui/SettingsDaemon/XRANDR",
                                              "org.ukui.SettingsDaemon.XRANDR_2");

    if (monitor.proxy == NULL)
        return;

    monitor.proxy_call = dbus_g_proxy_begin_call(monitor.proxy, "ApplyConfiguration",
                                                 NULL, NULL, NULL,
                                                 G_TYPE_INT64, (gint64) window_xid,
                                                 G_TYPE_INT64, (gint64) monitor.ApplyBtnClickTimeStamp,
                                                 G_TYPE_INVALID,
                                                 G_TYPE_INVALID);

}
