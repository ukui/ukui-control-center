#include <dbus/dbus-glib.h>
#include <glib/gi18n.h>
#define UPOWER_ENABLE_DEPRECATED
#include <libupower-glib/upower.h>

#include "kpm-prefs-core.h"
#include "kpm-common.h"
#include "egg-debug.h"
#include "egg-console-kit.h"
#include "kpm-brightness.h"

#include "../../shell/mainwindow.h"

static void kpm_prefs_finalize (GObject *object);

#define KPM_PREFS_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), KPM_TYPE_PREFS, KpmPrefsPrivate))

struct KpmPrefsPrivate
{
        UpClient                *client;
        GtkBuilder              *builder;
        gboolean                 has_batteries;
        gboolean                 has_lcd;
        gboolean                 has_ups;
        gboolean                 has_button_lid;
        gboolean                 has_button_suspend;
        gboolean                 can_shutdown;
        gboolean                 can_suspend;
        gboolean                 can_hibernate;
        GSettings               *settings;
        EggConsoleKit           *console;
};

G_DEFINE_TYPE (KpmPrefs, kpm_prefs, G_TYPE_OBJECT)

/**
 * kpm_prefs_class_init:
 * @klass: This prefs class instance
 **/
static void
kpm_prefs_class_init (KpmPrefsClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        object_class->finalize = kpm_prefs_finalize;
        g_type_class_add_private (klass, sizeof (KpmPrefsPrivate));
}

/**
 * kpm_prefs_finalize:
 * @object: This prefs class instance
 **/
static void
kpm_prefs_finalize (GObject *object)
{
        KpmPrefs *prefs;
        g_return_if_fail (object != NULL);
        g_return_if_fail (KPM_IS_PREFS (object));

        prefs = KPM_PREFS (object);
        prefs->priv = KPM_PREFS_GET_PRIVATE (prefs);

        g_object_unref (prefs->priv->settings);
        g_object_unref (prefs->priv->client);
        g_object_unref (prefs->priv->console);

        G_OBJECT_CLASS (kpm_prefs_parent_class)->finalize (object);
}

/**
 * kpm_prefs_format_percentage_cb:
 * @scale: The GtkScale object
 * @value: The value in %.
 **/
static gchar *
kpm_prefs_format_percentage_cb (GtkScale *scale, gdouble value)
{
        return g_strdup_printf ("%.0f%%", value);
}

/**
 * kpm_prefs_set_combo_simple_text:
 **/
static void
kpm_prefs_set_combo_simple_text (GtkWidget *combo_box)
{
        #if !GTK_CHECK_VERSION (2, 24, 0)
                GtkCellRenderer *cell;
                GtkListStore *store;

                store = gtk_list_store_new (1, G_TYPE_STRING);
                gtk_combo_box_set_model (GTK_COMBO_BOX (combo_box), GTK_TREE_MODEL (store));
                g_object_unref (store);

                cell = gtk_cell_renderer_text_new ();
                gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box), cell, TRUE);
                gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box), cell,
                                                "text", 0,
                                                NULL);
        #else
                // nothing to do with GTK_COMBO_BOX_TEXT
        #endif
}


/**
 * kpm_prefs_action_time_changed_cb:
 **/
static void
kpm_prefs_action_time_changed_cb (GtkWidget *widget, KpmPrefs *prefs)
{
        guint value;
        const gint *values;
        const gchar *kpm_pref_key;
        guint active;

        values = (const gint *) g_object_get_data (G_OBJECT (widget), "values");
        kpm_pref_key = (const gchar *) g_object_get_data (G_OBJECT (widget), "settings_key");

        active = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
        value = values[active];

        egg_debug ("Changing %s to %i", kpm_pref_key, value);
        g_settings_set_int (prefs->priv->settings, kpm_pref_key, value);
}


/**
 * kpm_prefs_setup_time_combo:
 * @prefs: This prefs class instance
 * @widget_name: The GtkWidget name
 * @kpm_pref_key: The settings key for this preference setting.
 * @actions: The actions to associate in an array.
 **/
static void
kpm_prefs_setup_time_combo (KpmPrefs *prefs, const gchar *widget_name,
                                const gchar *kpm_pref_key, const gint *values)
{
        guint value;
        gchar *text;
        guint i;
        gboolean is_writable;
        GtkWidget *widget;

    	widget = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, widget_name));
        kpm_prefs_set_combo_simple_text (widget);

        value = g_settings_get_int (prefs->priv->settings, kpm_pref_key);
        is_writable = g_settings_is_writable (prefs->priv->settings, kpm_pref_key);
        gtk_widget_set_sensitive (widget, is_writable);

        g_object_set_data (G_OBJECT (widget), "settings_key", (gpointer) kpm_pref_key);
        g_object_set_data (G_OBJECT (widget), "values", (gpointer) values);

        /* add each time */
        for (i=0; values[i] != -1; i++) {

                /* get translation for number of seconds */
                if (values[i] != 0) {
                        text = kpm_get_timestring (values[i]);
                        #if GTK_CHECK_VERSION (2, 24, 0)
                                gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (widget), text);
                        #else
                                gtk_combo_box_append_text (GTK_COMBO_BOX (widget), text);
                        #endif
                        g_free (text);
                } else {
                        #if GTK_CHECK_VERSION (2, 24, 0)
                                gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (widget), _("Never"));
                        #else
                                gtk_combo_box_append_text (GTK_COMBO_BOX (widget), _("Never"));
                        #endif
                }

                /* matches, so set default */
                if (value == values[i])
                         gtk_combo_box_set_active (GTK_COMBO_BOX (widget), i);
        }

        /* connect after set */
        g_signal_connect (G_OBJECT (widget), "changed",
                          G_CALLBACK (kpm_prefs_action_time_changed_cb), prefs);
}

/**
 * kpm_prefs_actions_destroy_cb:
 **/
static void
kpm_prefs_actions_destroy_cb (KpmActionPolicy *array)
{
        g_free (array);
}

/**
 * kpm_prefs_action_combo_changed_cb:
 **/
static void
kpm_prefs_action_combo_changed_cb (GtkWidget *widget, KpmPrefs *prefs)
{
        KpmActionPolicy policy;
        const KpmActionPolicy *actions;
        const gchar *kpm_pref_key;
        guint active;

        actions = (const KpmActionPolicy *) g_object_get_data (G_OBJECT (widget), "actions");
        kpm_pref_key = (const gchar *) g_object_get_data (G_OBJECT (widget), "settings_key");

        active = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
        policy = actions[active];
        g_settings_set_enum (prefs->priv->settings, kpm_pref_key, policy);
}


/**
 * kpm_prefs_setup_action_combo:
 * @prefs: This prefs class instance
 * @widget_name: The GtkWidget name
 * @kpm_pref_key: The settings key for this preference setting.
 * @actions: The actions to associate in an array.
 **/
static void
kpm_prefs_setup_action_combo (KpmPrefs *prefs, const gchar *widget_name,
                                  const gchar *kpm_pref_key, const KpmActionPolicy *actions)
{
        gint i;
        gboolean is_writable;
        GtkWidget *widget;
        KpmActionPolicy policy;
        KpmActionPolicy value;
        GPtrArray *array;
        KpmActionPolicy *actions_added;

        widget = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, widget_name));
        kpm_prefs_set_combo_simple_text (widget);

        value = g_settings_get_enum (prefs->priv->settings, kpm_pref_key);
        is_writable = g_settings_is_writable (prefs->priv->settings, kpm_pref_key);

        //gtk_widget_set_sensitive (widget, is_writable);

        array = g_ptr_array_new ();
        g_object_set_data (G_OBJECT (widget), "settings_key", (gpointer) kpm_pref_key);
        g_signal_connect (G_OBJECT (widget), "changed",
                          G_CALLBACK (kpm_prefs_action_combo_changed_cb), prefs);
        for (i=0; actions[i] != -1; i++) {
                policy = actions[i];
                if (policy == KPM_ACTION_POLICY_SHUTDOWN && !prefs->priv->can_shutdown) {
					//只是为了在交流电情况下显示出来
					gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (widget), _("Shutdown"));
                                g_ptr_array_add(array, GINT_TO_POINTER (policy));
                        egg_debug ("Cannot add option, as cannot shutdown.");
                } else if (policy == KPM_ACTION_POLICY_SHUTDOWN && prefs->priv->can_shutdown) {
                        #if GTK_CHECK_VERSION (2, 24, 0)
                                gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (widget), _("Shutdown"));
                                g_ptr_array_add(array, GINT_TO_POINTER (policy));
                        #else
                                gtk_combo_box_append_text (GTK_COMBO_BOX (widget), _("Shutdown"));
                                g_ptr_array_add (array, GINT_TO_POINTER (policy));
                        #endif
                } else if (policy == KPM_ACTION_POLICY_SUSPEND && !prefs->priv->can_suspend) {
					gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (widget), _("Hang"));
                                g_ptr_array_add (array, GINT_TO_POINTER (policy));
                        egg_debug ("Cannot add option, as cannot suspend.");
                } else if (policy == KPM_ACTION_POLICY_HIBERNATE && !prefs->priv->can_hibernate) {
					gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (widget), _("Sleep"));
                                g_ptr_array_add(array, GINT_TO_POINTER (policy));
                        egg_debug ("Cannot add option, as cannot hibernate.");
                } else if (policy == KPM_ACTION_POLICY_SUSPEND && prefs->priv->can_suspend) {
                        #if GTK_CHECK_VERSION (2, 24, 0)
                                gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (widget), _("Hang"));
                                g_ptr_array_add (array, GINT_TO_POINTER (policy));
                        #else
                                gtk_combo_box_append_text (GTK_COMBO_BOX (widget), _("Hang"));
                                g_ptr_array_add (array, GINT_TO_POINTER (policy));
                        #endif
                } else if (policy == KPM_ACTION_POLICY_HIBERNATE && prefs->priv->can_hibernate) {
                        #if GTK_CHECK_VERSION (2, 24, 0)
                                gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (widget), _("Sleep"));
                                g_ptr_array_add(array, GINT_TO_POINTER (policy));
                        #else
                                gtk_combo_box_append_text (GTK_COMBO_BOX (widget), _("Sleep"));
                                g_ptr_array_add (array, GINT_TO_POINTER (policy));
                        #endif
                } else if (policy == KPM_ACTION_POLICY_BLANK) {
                        #if GTK_CHECK_VERSION (2, 24, 0)
                                gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (widget), _("Blank screen"));
                                g_ptr_array_add (array, GINT_TO_POINTER (policy));
                        #else
                                gtk_combo_box_append_text (GTK_COMBO_BOX (widget), _("Blank screen"));
                                g_ptr_array_add (array, GINT_TO_POINTER (policy));
                        #endif
                } else if (policy == KPM_ACTION_POLICY_INTERACTIVE) {
                        #if GTK_CHECK_VERSION (2, 24, 0)
                                gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (widget), _("Ask"));
                                g_ptr_array_add(array, GINT_TO_POINTER (policy));
                        #else
                                gtk_combo_box_append_text (GTK_COMBO_BOX (widget), _("Ask"));
                                g_ptr_array_add (array, GINT_TO_POINTER (policy));
                        #endif
                } else if (policy == KPM_ACTION_POLICY_NOTHING) {
                        #if GTK_CHECK_VERSION (2, 24, 0)
                                gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (widget), _("Do not perform operations"));
                                g_ptr_array_add(array, GINT_TO_POINTER (policy));
                        #else
                                gtk_combo_box_append_text (GTK_COMBO_BOX (widget), _("Do not perform operations"));
                                g_ptr_array_add (array, GINT_TO_POINTER (policy));
                        #endif
                } else {
                        egg_warning ("Unknown action read from settings: %i", policy);
                }
        }

        /* save as array _only_ the actions we could add */
        actions_added = g_new0 (KpmActionPolicy, array->len+1);
        for (i=0; i<array->len; i++)
                actions_added[i] = GPOINTER_TO_INT (g_ptr_array_index (array, i));
        actions_added[i] = -1;

        g_object_set_data_full (G_OBJECT (widget), "actions", (gpointer) actions_added, (GDestroyNotify) kpm_prefs_actions_destroy_cb);

        /* set what we have in the settings */
        for (i=0; actions_added[i] != -1; i++) {
                policy = actions_added[i];
                if (value == policy)
                         gtk_combo_box_set_active (GTK_COMBO_BOX (widget), i);
        }

        g_ptr_array_unref (array);
}

static void init_picture(KpmPrefs *prefs)
{
    GtkImage *ac_image = GTK_IMAGE(gtk_builder_get_object (prefs->priv->builder, "image1_ac"));
	gtk_image_set_from_file(ac_image, "/usr/share/ukui-control-center/icons/AC.png");
    GtkImage *battery_image = GTK_IMAGE(gtk_builder_get_object (prefs->priv->builder, "image2_battery"));
	gtk_image_set_from_file(battery_image, "/usr/share/ukui-control-center/icons/battery.png");
}

static void set_hide(KpmPrefs *prefs, gchar *widget_name){
	GtkWidget *widget;
	widget = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, widget_name));
	gtk_widget_hide(widget);
}

//如果是arm架构，隐藏掉有问题的选项
static void hide_arch(KpmPrefs *prefs)
{
	FILE *stream;
	char buffer[20];
	//arch命令获取体系架构
	stream = popen("arch", "r");
	if(!stream)
		return;
	fgets(buffer, sizeof(buffer), stream);
	//获取到的buffer会多一个换行符
	char *arch = strtok(buffer,"\n");
	if(!strcmp(arch, "aarch64"))
	{
        set_hide (prefs, "layout7_action");
        set_hide (prefs, "label27_sleep");
        set_hide (prefs, "label65_laptop");
		set_hide (prefs, "combobox_ac_computer");
		set_hide (prefs, "combobox_ac_lid");
		set_hide (prefs, "combobox_battery_computer");
		set_hide (prefs, "combobox_battery_lid");
        GtkWidget *layout_power = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, "layout5_power"));
        GtkWidget *layout_icon = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, "layout9_icon"));
		gtk_layout_move(GTK_LAYOUT(layout_power), layout_icon, 0, 150);
	}
	pclose(stream);
}

static void
prefs_setup_ac (KpmPrefs *prefs)
{

		init_picture(prefs);
		hide_arch(prefs);
        GtkWidget *widget;
        const KpmActionPolicy button_lid_actions[] =
                                {KPM_ACTION_POLICY_NOTHING,
                                 KPM_ACTION_POLICY_BLANK,
                                 KPM_ACTION_POLICY_SUSPEND,
                                 KPM_ACTION_POLICY_HIBERNATE,
                                 KPM_ACTION_POLICY_SHUTDOWN,
                                 -1};

        static const gint computer_times[] =
                {10*60,
                 30*60,
                 1*60*60,
                 2*60*60,
                 0, /* never */
                 -1};
        static const gint display_times[] =
                {1*60,
                 5*60,
                 10*60,
                 30*60,
                 1*60*60,
                 0, /* never */
                 -1};

        kpm_prefs_setup_time_combo (prefs, "combobox_ac_computer",
                                        KPM_SETTINGS_SLEEP_COMPUTER_AC,
                                        computer_times);
        kpm_prefs_setup_time_combo (prefs, "combobox_ac_display",
                                        KPM_SETTINGS_SLEEP_DISPLAY_AC,
                                        display_times);
        kpm_prefs_setup_action_combo (prefs, "combobox_ac_lid",
                                          KPM_SETTINGS_BUTTON_LID_AC,
                                          button_lid_actions);

        //对电源界面的标签进行处理
        GtkWidget *label27 = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, "label27_sleep"));
        GtkWidget *label28 = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, "label28_sleep"));
        GtkWidget *label65 = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, "label65_laptop"));
        GtkWidget *label34 = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, "label34_press"));
        GtkWidget *label37 = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, "label37_suspend"));
        gtk_label_set_xalign(GTK_LABEL(label27), 0.0);
        gtk_label_set_xalign(GTK_LABEL(label28), 0.0);
        gtk_label_set_xalign(GTK_LABEL(label65), 0.0);
        gtk_label_set_xalign(GTK_LABEL(label34), 0.0);
        gtk_label_set_xalign(GTK_LABEL(label37), 0.0);


        /* setup brightness slider */
//        widget = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, "hscale_ac_brightness"));
//		g_settings_bind (prefs->priv->settings, KPM_SETTINGS_BRIGHTNESS_AC,
//                         gtk_range_get_adjustment (GTK_RANGE (widget)), "value",
//                         G_SETTINGS_BIND_DEFAULT);
//        g_signal_connect (G_OBJECT (widget), "format-value",
//                          G_CALLBACK (kpm_prefs_format_percentage_cb), NULL);

        /* set up the checkboxes */
//        widget = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, "checkbutton_ac_display_dim"));
//        g_settings_bind (prefs->priv->settings, KPM_SETTINGS_IDLE_DIM_AC,
//                         widget, "active",
//                         G_SETTINGS_BIND_DEFAULT);
//        widget = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, "checkbutton_ac_spindown"));
//        g_settings_bind (prefs->priv->settings, KPM_SETTINGS_SPINDOWN_ENABLE_AC,
//                         widget, "active",
//                         G_SETTINGS_BIND_DEFAULT);

		/* 不是笔记本 */
        if (prefs->priv->has_button_lid == FALSE) {
                widget = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, "combobox_ac_lid"));
                #if GTK_CHECK_VERSION (2, 24, 0)
                        gtk_widget_hide(widget);
                #else
                        gtk_widget_hide_all (widget);
                #endif
        }
		/* 关于显示器的设置选项暂时去掉 */
        if (prefs->priv->has_lcd == FALSE) {
			/*
                widget = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, "viewport_ac_brightness"));

                #if GTK_CHECK_VERSION (2, 24, 0)
                        gtk_widget_hide(widget);
                #else
                        gtk_widget_hide_all (widget);
                #endif

                widget = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, "viewport_ac_dim"));

                #if GTK_CHECK_VERSION (2, 24, 0)
                        gtk_widget_hide(widget);
                #else
                        gtk_widget_hide_all (widget);
                #endif
			*/
        }
}

static void set_insensitive(KpmPrefs *prefs, gchar *widget_name){
	GtkWidget *widget;
	widget = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, widget_name));
	gtk_widget_set_sensitive(widget, FALSE);
}

static void
prefs_setup_battery (KpmPrefs *prefs)
{
        GtkWidget *widget;
        GtkNotebook *notebook;
        gint page;

        const KpmActionPolicy button_lid_actions[] =
                                {KPM_ACTION_POLICY_NOTHING,
                                 KPM_ACTION_POLICY_BLANK,
                                 KPM_ACTION_POLICY_SUSPEND,
                                 KPM_ACTION_POLICY_HIBERNATE,
                                 KPM_ACTION_POLICY_SHUTDOWN,
                                 -1};
        const KpmActionPolicy battery_critical_actions[] =
                                {KPM_ACTION_POLICY_NOTHING,
                                 KPM_ACTION_POLICY_SUSPEND,
                                 KPM_ACTION_POLICY_HIBERNATE,
                                 KPM_ACTION_POLICY_SHUTDOWN,
                                 -1};

        static const gint computer_times[] =
                {10*60,
                 30*60,
                 1*60*60,
                 2*60*60,
                 0, /* never */
                 -1};
        static const gint display_times[] =
                {1*60,
                 5*60,
                 10*60,
                 30*60,
                 1*60*60,
                 0, /* never */
                 -1};

        kpm_prefs_setup_time_combo (prefs, "combobox_battery_computer",
                                        KPM_SETTINGS_SLEEP_COMPUTER_BATT,
                                        computer_times);
        kpm_prefs_setup_time_combo (prefs, "combobox_battery_display",
                                        KPM_SETTINGS_SLEEP_DISPLAY_BATT,
                                        display_times);

        if (prefs->priv->has_batteries == FALSE) {
                //notebook = GTK_NOTEBOOK (gtk_builder_get_object (prefs->priv->builder, "notebook_power_manager"));
                //widget = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, "vbox_battery"));
                //page = gtk_notebook_page_num (notebook, GTK_WIDGET (widget));
                //gtk_notebook_remove_page (notebook, page);
                //return;
				set_insensitive(prefs, "combobox_battery_computer");
				set_insensitive(prefs, "combobox_battery_display");
				set_insensitive(prefs, "combobox_general_power");
				set_insensitive(prefs, "combobox_general_suspend");
				//set_insensitive(prefs, "combobox_ac_lid");
				set_insensitive(prefs, "combobox_battery_lid");
        }


        kpm_prefs_setup_action_combo (prefs, "combobox_battery_lid",
                                          KPM_SETTINGS_BUTTON_LID_BATT,
                                          button_lid_actions);
		/*
		kpm_prefs_setup_action_combo (prefs, "combobox_battery_critical",
                                          KPM_SETTINGS_ACTION_CRITICAL_BATT,
                                          battery_critical_actions);
		*/

        /* set up the checkboxes */
		/*
        widget = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, "checkbutton_battery_display_reduce"));
        g_settings_bind (prefs->priv->settings, KPM_SETTINGS_BACKLIGHT_BATTERY_REDUCE,
                         widget, "active",
                         G_SETTINGS_BIND_DEFAULT);
        widget = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, "checkbutton_battery_display_dim"));
        g_settings_bind (prefs->priv->settings, KPM_SETTINGS_IDLE_DIM_BATT,
                         widget, "active",
                         G_SETTINGS_BIND_DEFAULT);
        widget = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, "checkbutton_battery_spindown"));
        g_settings_bind (prefs->priv->settings, KPM_SETTINGS_SPINDOWN_ENABLE_BATT,
                         widget, "active",
                         G_SETTINGS_BIND_DEFAULT);
		*/

		//如果不是笔记本，隐藏有关笔记本盖子的相关选项，并调整布局
        if (prefs->priv->has_button_lid == FALSE) {
                widget = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, "combobox_battery_lid"));
                GtkWidget *label = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, "label65_laptop"));
                GtkWidget *layout_action = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, "layout7_action"));
                GtkWidget *layout_power = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, "layout5_power"));
				gtk_layout_move(GTK_LAYOUT(layout_power), layout_action, 0, 195);
                #if GTK_CHECK_VERSION(2, 24, 0)
                        gtk_widget_hide(widget);
						gtk_widget_hide(label);
                #else
                        gtk_widget_hide_all (widget);
                        gtk_widget_hide(label);
                #endif
        }
        if (prefs->priv->has_lcd == FALSE) {
                //widget = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, "viewport_battery_dim"));
                #if GTK_CHECK_VERSION(2, 24, 0)
                //      gtk_widget_hide(widget);
                #else
                //      gtk_widget_hide_all (widget);
                #endif
        }
}

static void
prefs_setup_general (KpmPrefs *prefs)
{
        GtkWidget *widget;
        const KpmActionPolicy power_button_actions[] =
                                {KPM_ACTION_POLICY_INTERACTIVE,
                                 KPM_ACTION_POLICY_SUSPEND,
                                 KPM_ACTION_POLICY_HIBERNATE,
                                 KPM_ACTION_POLICY_SHUTDOWN,
                                 -1};
        const KpmActionPolicy suspend_button_actions[] =
                                {KPM_ACTION_POLICY_NOTHING,
                                 KPM_ACTION_POLICY_SUSPEND,
                                 KPM_ACTION_POLICY_HIBERNATE,
                                 -1};

        kpm_prefs_setup_action_combo (prefs, "combobox_general_power",
                                          KPM_SETTINGS_BUTTON_POWER,
                                          power_button_actions);
        kpm_prefs_setup_action_combo (prefs, "combobox_general_suspend",
                                          KPM_SETTINGS_BUTTON_SUSPEND,
                                          suspend_button_actions);

		/*如果没有挂起按钮则要隐藏挂起的选项*/
        if (prefs->priv->has_button_suspend == FALSE) {
                widget = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, "combobox_general_suspend"));
                GtkWidget *widget_label = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, "label37_suspend"));

                #if GTK_CHECK_VERSION (2, 24, 0)
                        gtk_widget_hide (widget);
						gtk_widget_hide (widget_label);
                #else
                        gtk_widget_hide_all (widget);
						gtk_widget_hide_all (widget_label);
                #endif
        }
}

/**
 * kpm_prefs_icon_radio_cb:
 * @widget: The GtkWidget object
 **/
static void
kpm_prefs_icon_radio_cb (GtkWidget *widget, KpmPrefs *prefs)
{
        gint policy;

        policy = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget), "policy"));
        g_settings_set_enum (prefs->priv->settings, KPM_SETTINGS_ICON_POLICY, policy);
}

/** setup the notification page */
static void
prefs_setup_notification (KpmPrefs *prefs)
{
        gint icon_policy;
        GtkWidget *radiobutton_icon_always;
        GtkWidget *radiobutton_icon_present;
        GtkWidget *radiobutton_icon_charge;
        GtkWidget *radiobutton_icon_low;
        GtkWidget *radiobutton_icon_never;
        gboolean is_writable;

        icon_policy = g_settings_get_enum (prefs->priv->settings, KPM_SETTINGS_ICON_POLICY);

        radiobutton_icon_always = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder,
                                                  "radiobutton_notification_always"));
        radiobutton_icon_present = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder,
                                                   "radiobutton_notification_present"));
        radiobutton_icon_charge = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder,
                                                  "radiobutton_notification_charge"));
        radiobutton_icon_low = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder,
                                           "radiobutton_notification_low"));
        radiobutton_icon_never = GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder,
                                                 "radiobutton_notification_never"));

        is_writable = g_settings_is_writable (prefs->priv->settings, KPM_SETTINGS_ICON_POLICY);
        gtk_widget_set_sensitive (radiobutton_icon_always, is_writable);
        gtk_widget_set_sensitive (radiobutton_icon_present, is_writable);
        //gtk_widget_set_sensitive (radiobutton_icon_charge, is_writable);
        //gtk_widget_set_sensitive (radiobutton_icon_low, is_writable);
        //gtk_widget_set_sensitive (radiobutton_icon_never, is_writable);

        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton_icon_always),
                                          icon_policy == KPM_ICON_POLICY_ALWAYS);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton_icon_present),
                                          icon_policy == KPM_ICON_POLICY_PRESENT);
		/*
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton_icon_charge),
                                          icon_policy == KPM_ICON_POLICY_CHARGE);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton_icon_low),
                                          icon_policy == KPM_ICON_POLICY_LOW);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton_icon_never),
                                          icon_policy == KPM_ICON_POLICY_NEVER);
		*/

        g_object_set_data (G_OBJECT (radiobutton_icon_always), "policy",
                           GINT_TO_POINTER (KPM_ICON_POLICY_ALWAYS));
        g_object_set_data (G_OBJECT (radiobutton_icon_present), "policy",
                           GINT_TO_POINTER (KPM_ICON_POLICY_PRESENT));
		/*
        g_object_set_data (G_OBJECT (radiobutton_icon_charge), "policy",
                           GINT_TO_POINTER (KPM_ICON_POLICY_CHARGE));
        g_object_set_data (G_OBJECT (radiobutton_icon_low), "policy",
                           GINT_TO_POINTER (KPM_ICON_POLICY_LOW));
        g_object_set_data (G_OBJECT (radiobutton_icon_never), "policy",
                           GINT_TO_POINTER (KPM_ICON_POLICY_NEVER));
		*/

        /* only connect the callbacks after we set the value, else the settings
         * keys gets written to (for a split second), and the icon flickers. */
        g_signal_connect (radiobutton_icon_always, "clicked",
                          G_CALLBACK (kpm_prefs_icon_radio_cb), prefs);
        g_signal_connect (radiobutton_icon_present, "clicked",
                          G_CALLBACK (kpm_prefs_icon_radio_cb), prefs);
		/*
        g_signal_connect (radiobutton_icon_charge, "clicked",
                          G_CALLBACK (kpm_prefs_icon_radio_cb), prefs);
        g_signal_connect (radiobutton_icon_low, "clicked",
                          G_CALLBACK (kpm_prefs_icon_radio_cb), prefs);
        g_signal_connect (radiobutton_icon_never, "clicked",
                          G_CALLBACK (kpm_prefs_icon_radio_cb), prefs);
		*/
}


static void
kpm_prefs_init (KpmPrefs *prefs)
{
        GError *error = NULL;
        GPtrArray *devices = NULL;
        UpDevice *device;
        UpDeviceKind kind;
        KpmBrightness *brightness;
#if !UP_CHECK_VERSION(0, 99, 0)
        gboolean ret;
#endif
        guint i;

        GDBusProxy *proxy;
        GVariant *res, *inner;
        gchar * r;

        prefs->priv = KPM_PREFS_GET_PRIVATE (prefs);
	
	prefs->priv->client = up_client_new ();
        prefs->priv->console = egg_console_kit_new ();
        prefs->priv->settings = g_settings_new (KPM_SETTINGS_SCHEMA);

        prefs->priv->can_shutdown = FALSE;
        prefs->priv->can_suspend = FALSE;
        prefs->priv->can_hibernate = FALSE;

	prefs->priv->builder = builder;


	if (LOGIND_RUNNING()) {
                /* get values from logind */

                proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
                                                       G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
                                                       NULL,
                                                       "org.freedesktop.login1",
                                                       "/org/freedesktop/login1",
                                                       "org.freedesktop.login1.Manager",
                                                       NULL,
                                                       &error );
                if (proxy == NULL) {
                        egg_error("Error connecting to dbus - %s", error->message);
                        g_error_free (error);
                        return;
                }

                res = g_dbus_proxy_call_sync (proxy, "CanPowerOff",
                                              NULL,
                                              G_DBUS_CALL_FLAGS_NONE,
                                              -1,
                                              NULL,
                                              &error
                                              );
                if (error == NULL && res != NULL) {
                        g_variant_get(res,"(s)", &r);
                        prefs->priv->can_shutdown = g_strcmp0(r,"yes")==0?TRUE:FALSE;
                        g_variant_unref (res);
                } else if (error != NULL ) {
                        egg_error ("Error in dbus - %s", error->message);
                        g_error_free (error);
                }

                res = g_dbus_proxy_call_sync (proxy, "CanSuspend",
                                              NULL,
                                              G_DBUS_CALL_FLAGS_NONE,
                                              -1,
                                              NULL,
                                              &error
                                              );
                if (error == NULL && res != NULL) {
                        g_variant_get(res,"(s)", &r);
                        prefs->priv->can_suspend = g_strcmp0(r,"yes")==0?TRUE:FALSE;
                        g_variant_unref (res);
                } else if (error != NULL ) {
                        egg_error ("Error in dbus - %s", error->message);
                        g_error_free (error);
                }

                res = g_dbus_proxy_call_sync (proxy, "CanHibernate",
                                              NULL,
                                              G_DBUS_CALL_FLAGS_NONE,
                                              -1,
                                              NULL,
                                              &error
                                              );
                if (error == NULL && res != NULL) {
                        g_variant_get(res,"(s)", &r);
                        prefs->priv->can_hibernate = g_strcmp0(r,"yes")==0?TRUE:FALSE;
                        g_variant_unref (res);
                } else if (error != NULL ) {
                        egg_error ("Error in dbus - %s", error->message);
                        g_error_free (error);
                }
                g_object_unref(proxy);
        }
        else {
                /* are we allowed to shutdown? */
                egg_console_kit_can_stop (prefs->priv->console, &prefs->priv->can_shutdown, NULL);
#if !UP_CHECK_VERSION(0, 99, 0)
                /* get values from UpClient */
                prefs->priv->can_suspend = up_client_get_can_suspend (prefs->priv->client);
                prefs->priv->can_hibernate = up_client_get_can_hibernate (prefs->priv->client);
#endif
        }

	if (LOGIND_RUNNING()) {
                proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
                                                       G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
                                                       NULL,
                                                       "org.freedesktop.UPower",
                                                       "/org/freedesktop/UPower",
                                                       "org.freedesktop.DBus.Properties",
                                                       NULL,
                                                       &error );
                if (proxy == NULL) {
                        egg_error("Error connecting to dbus - %s", error->message);
                        g_error_free (error);
                        return;
                }

                res = g_dbus_proxy_call_sync (proxy, "Get",
                                              g_variant_new( "(ss)",
                                                             "org.freedesktop.UPower",
                                                             "LidIsPresent"),
                                              G_DBUS_CALL_FLAGS_NONE,
                                              -1,
                                              NULL,
                                              &error
                                              );
                if (error == NULL && res != NULL) {
                        g_variant_get(res, "(v)", &inner );
                        prefs->priv->has_button_lid = g_variant_get_boolean(inner);
                        g_variant_unref (inner);
                        g_variant_unref (res);
                } else if (error != NULL ) {
                        egg_error ("Error in dbus - %s", error->message);
                        g_error_free (error);
                }
                g_object_unref(proxy);
        }
	else {
#if UP_CHECK_VERSION(0,9,2)
                prefs->priv->has_button_lid = up_client_get_lid_is_present (prefs->priv->client);
#else
                g_object_get (prefs->priv->client,
                              "lid-is-present", &prefs->priv->has_button_lid,
                              NULL);
#endif
        }

        prefs->priv->has_button_suspend = TRUE;

	 /* find if we have brightness hardware */
        brightness = kpm_brightness_new ();
        prefs->priv->has_lcd = kpm_brightness_has_hw (brightness);
        g_object_unref (brightness);
#if !UP_CHECK_VERSION(0, 99, 0)
        /* get device list */
        ret = up_client_enumerate_devices_sync (prefs->priv->client, NULL, &error);
        if (!ret) {
                egg_warning ("failed to get device list: %s", error->message);
                g_error_free (error);
        }
#endif
        devices = up_client_get_devices (prefs->priv->client);
        for (i=0; i<devices->len; i++) {
                device = g_ptr_array_index (devices, i);
                g_object_get (device,
                              "kind", &kind,
                              NULL);
                if (kind == UP_DEVICE_KIND_BATTERY)
                        prefs->priv->has_batteries = TRUE;
                if (kind == UP_DEVICE_KIND_UPS)
                        prefs->priv->has_ups = TRUE;
        }
        g_ptr_array_unref (devices);



	prefs_setup_ac (prefs);
    //prefs_setup_screensaver(prefs);
	prefs_setup_battery (prefs);
	prefs_setup_general (prefs);
#if     defined(__aarch64__)
	/*
        GtkWidget *w =  GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, "label_battery"));
        gtk_widget_hide(w);
        GtkWidget *w2 =  GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, "vbox_battery"));
        gtk_widget_hide(w2);
		GtkWidget *w1 =  GTK_WIDGET (gtk_builder_get_object (prefs->priv->builder, "vbox25"));
		gtk_widget_hide(w1);
	*/
#else
    //prefs_setup_battery (prefs);
    //prefs_setup_general (prefs);
#endif
	prefs_setup_notification (prefs);
}

/**
 * kpm_prefs_new:
 * Return value: new KpmPrefs instance.
 **/
KpmPrefs *
kpm_prefs_new (void)
{
        KpmPrefs *prefs;
        prefs = g_object_new (KPM_TYPE_PREFS, NULL);
        return KPM_PREFS (prefs);
}


void init_power()
{
	g_warning("power");
	screensaver_init(builder);
	kpm_prefs_new();
	
}
