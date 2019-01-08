/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2016 Tianjin KYLIN Information Technology Co., Ltd.
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
#include <crypt.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <glib/gi18n.h>
#include <unistd.h>
#define MATE_DESKTOP_USE_UNSTABLE_API
#include <libmate-desktop/mate-desktop-thumbnail.h>

#include "run-passwd.h"
#include "user-accounts.h"
#include "check-passwd.h"

GList *userlist = NULL;
GtkDialog *dialog;
GtkBuilder *ui = NULL;

#define ROW_SPAN 6

#define FACES_PATH "/usr/share/pixmaps/faces/"
enum
{
    COL_PIXBUF,
    NUM_COLS
};

static void file_icon_selected (GtkMenuItem   *menuitem, UserInfo *user);

void find_all_face_file(GtkListStore *list_store, GtkTreeIter iter)
{
    if (!g_file_test(FACES_PATH, G_FILE_TEST_IS_DIR))
    {
        g_warning("dir is not exists");
        exit(0);
    }
    GError *error = NULL;
    GError *err = NULL;
    GFile *directory;
    GFileEnumerator *enumer;
    GFileInfo *info;
    GdkPixbuf *pixbuf;
    GFileType type;
    const gchar *target;
    directory = g_file_new_for_path(FACES_PATH);
    enumer = g_file_enumerate_children(directory, 
		    G_FILE_ATTRIBUTE_STANDARD_NAME ","
		    G_FILE_ATTRIBUTE_STANDARD_TYPE ","
		    G_FILE_ATTRIBUTE_STANDARD_IS_SYMLINK ","
		    G_FILE_ATTRIBUTE_STANDARD_SYMLINK_TARGET, 
		    G_FILE_QUERY_INFO_NONE, NULL, &error);
    if (error != NULL)
    {
        g_warning("enumer is wrong");
        g_error_free(error);
        exit(0);
    }
    while ((info = g_file_enumerator_next_file(enumer, NULL, NULL)))
    {
	char *fullpath;
	type = g_file_info_get_file_type(info);
	if(type != G_FILE_TYPE_REGULAR &&
	   type != G_FILE_TYPE_SYMBOLIC_LINK){
		g_object_unref(info);
		continue;
	}

	target = g_file_info_get_symlink_target(info);
	if(target != NULL && g_str_has_prefix(target, "legacy/")){
		g_object_unref(info);
		continue;
	}

	const char *filename = g_file_info_get_name (info);
	fullpath = g_build_filename(FACES_PATH, filename, NULL, NULL);
        pixbuf = gdk_pixbuf_new_from_file(fullpath, &err);
        pixbuf = gdk_pixbuf_scale_simple(pixbuf, 64, 64, GDK_INTERP_BILINEAR);
        gtk_list_store_append(list_store, &iter);
        gtk_list_store_set(list_store, &iter, COL_PIXBUF, pixbuf, -1);
        g_object_set_data_full (G_OBJECT (pixbuf), "filename", g_strdup (fullpath), (GDestroyNotify) g_free);
        if (err)
            g_error_free(err);
        g_object_unref(info);
        g_free(fullpath);
    }
    g_file_enumerator_close(enumer, NULL, NULL);
    g_object_unref(directory);
}

void user_bt_clicked(GtkWidget *widget, gpointer userdata)
{
    GList *it = NULL;
    for (it = userlist; it; it = it->next)
    {
        UserInfo *user = (UserInfo *)it->data;
        GtkNotebook *notebook = GTK_NOTEBOOK(user->notebook);
        gtk_notebook_set_show_border(notebook, FALSE);
        gtk_container_set_border_width(GTK_CONTAINER(notebook), 0);
        gtk_notebook_set_show_tabs(GTK_NOTEBOOK(userdata), FALSE);
        gtk_notebook_set_current_page(notebook, 1);
    }
    gtk_notebook_set_show_border(GTK_NOTEBOOK(userdata), TRUE);
    gtk_container_set_border_width(GTK_CONTAINER(userdata), 1);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(userdata), 0);
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(userdata), FALSE);
    gtk_widget_grab_focus(widget);
}

void modify_font_color(GtkWidget *button, char *textcolor)
{
    GtkWidget *label;
    label = gtk_bin_get_child((GtkBin*)button);
    GdkColor color;
    gdk_color_parse(textcolor, &color);
    gtk_widget_modify_fg(GTK_WIDGET(label), GTK_STATE_NORMAL, &color);
}

static gchar
salt_char (GRand *rand)
{
    gchar salt[] = "ABCDEFGHIJKLMNOPQRSTUVXYZ"
                   "abcdefghijklmnopqrstuvxyz"
                   "./0123456789";

    return salt[g_rand_int_range (rand, 0, G_N_ELEMENTS (salt))];
}

static gchar *
make_crypted (const gchar *plain)
{
    GString *salt;
    gchar *result;
    GRand *rand;
    gint i;

    rand = g_rand_new ();
    salt = g_string_sized_new (21);

    /* SHA 256 */
    g_string_append (salt, "$6$");
    for (i = 0; i < 16; i++) {
        g_string_append_c (salt, salt_char (rand));
    }
    g_string_append_c (salt, '$');

    result = g_strdup ((const gchar *)crypt(plain, salt->str));

    g_string_free (salt, TRUE);
    g_rand_free (rand);

    return result;
}

static gboolean
textChanged(GtkWidget *widget, gpointer userdata)
{	
    GtkWidget *buttoncreate = GTK_WIDGET(gtk_builder_get_object (ui, "buttoncreate"));
    GtkWidget *entryname = GTK_WIDGET(gtk_builder_get_object (ui, "entryname"));
    GtkWidget *labelname = GTK_WIDGET(gtk_builder_get_object (ui, "labelname"));
    GtkWidget *labelpwd = GTK_WIDGET(gtk_builder_get_object (ui, "labelpwd"));
    GtkWidget *labelensurepwd = GTK_WIDGET(gtk_builder_get_object (ui, "labelensurepwd"));
    gtk_label_set_xalign(GTK_LABEL(labelname), 0.0);
    gtk_label_set_xalign(GTK_LABEL(labelpwd), 0.0);
    gtk_label_set_xalign(GTK_LABEL(labelensurepwd), 0.0);
    if (widget == entryname)
    {
        const char *name = gtk_entry_get_text(GTK_ENTRY(widget));
        if (strlen(name) < 1)
        {
            gtk_label_set_text(GTK_LABEL(labelname), _("User name cannot be empty!"));
            gtk_widget_set_sensitive(buttoncreate, FALSE);
            return FALSE;
        }
        else
            gtk_label_set_text(GTK_LABEL(labelname), "");
    }
    GtkWidget *entrypwd = GTK_WIDGET(gtk_builder_get_object (ui, "entrypwd"));
    const char *pwd = gtk_entry_get_text(GTK_ENTRY(entrypwd));

    char *msg = NULL;
    //判断是否配置了密码复杂度模块，没有配置的话就不进行密码复杂度检验
    GPtrArray *tmp_array = get_passwd_configuration();
    if(tmp_array->len != 0)
        //因为这只是创建用户时的密码复杂度检查，所以旧密码为空
        msg = passwd_check("", pwd, g_get_user_name());
    g_ptr_array_free(tmp_array, TRUE);

    if (widget == entrypwd)
    {
        GtkWidget *entryensurepwd = GTK_WIDGET(gtk_builder_get_object (ui, "entryensurepwd"));
        const char *ensurepwd = gtk_entry_get_text(GTK_ENTRY(entryensurepwd));
        GtkWidget *labelpwd = GTK_WIDGET(gtk_builder_get_object (ui, "labelpwd"));
        GtkWidget *labelensurepwd = GTK_WIDGET(gtk_builder_get_object (ui, "labelensurepwd"));
        if (strlen(pwd) < 6)
        {
            gtk_label_set_text(GTK_LABEL(labelpwd), _("Password length needs to more than 5 digits!"));
            gtk_label_set_text(GTK_LABEL(labelensurepwd), "");
            gtk_widget_set_sensitive(buttoncreate, FALSE);
            return FALSE;
        }
        else if (strlen(pwd) > 63)
        {
            gtk_label_set_text(GTK_LABEL(labelpwd), _("Password length needs to less than 64 digits!"));
            gtk_widget_set_sensitive(buttoncreate, FALSE);
            return FALSE;
        }
        else if(msg)
        {
            gtk_label_set_text(GTK_LABEL(labelpwd), msg);
            gtk_widget_set_sensitive(buttoncreate, FALSE);
            return FALSE;
        }
        else
            gtk_label_set_text(GTK_LABEL(labelpwd), "");
        if (strcmp(pwd, ensurepwd) != 0 && strcmp(ensurepwd, _("Please confirm the new password")) != 0)
        {
            gtk_label_set_text(GTK_LABEL(labelensurepwd), _("enter the password twice inconsistencies!"));
            gtk_widget_set_sensitive(buttoncreate, FALSE);
            return FALSE;
        }
        else
        {
            gtk_label_set_text(GTK_LABEL(labelensurepwd), "");
        }

    }
    GtkWidget *entryensurepwd = GTK_WIDGET(gtk_builder_get_object (ui, "entryensurepwd"));
    if (widget == entryensurepwd)
    {
        const char *pwd = gtk_entry_get_text(GTK_ENTRY(entrypwd));
        const char *ensurepwd = gtk_entry_get_text(GTK_ENTRY(widget));
        GtkWidget *labelensurepwd = GTK_WIDGET(gtk_builder_get_object (ui, "labelensurepwd"));
        if (strcmp(pwd, ensurepwd) != 0)
        {
            gtk_label_set_text(GTK_LABEL(labelensurepwd), _("enter the password twice inconsistencies!"));
            gtk_widget_set_sensitive(buttoncreate, FALSE);
            return FALSE;
        }
        else
            gtk_label_set_text(GTK_LABEL(labelensurepwd), "");

    }

    char *username = (char *)gtk_entry_get_text(GTK_ENTRY(entryname));
    int i;
    for (i = 0; *(username + i) != 0; i++)
    {
        if (*(username) == 95)
        {
            gtk_label_set_text(GTK_LABEL(labelname), _("The first character cannot be underlined!"));
            gtk_widget_set_sensitive(buttoncreate, FALSE);
            return FALSE;
        }
        if (isupper(*(username + i)))
        {
            gtk_label_set_text(GTK_LABEL(labelname), _("User name can not contain capital letters!"));
            gtk_widget_set_sensitive(buttoncreate, FALSE);
            return FALSE;
        }
        if ((*(username + i) >=48 && *(username + i) <= 57 ) || (*(username + i) >= 97 && *(username + i) <= 122 ) || ( *(username + i) == 95))
            continue;
        else
        {
            gtk_label_set_text(GTK_LABEL(labelname), _("The user name can only be composed of letters, numbers and underline!"));
            gtk_widget_set_sensitive(buttoncreate, FALSE);
            return FALSE;
        }
    }
    if (isdigit(*username))
    {
        gtk_label_set_text(GTK_LABEL(labelname), _("The first character cannot be numeric!"));
        gtk_widget_set_sensitive(buttoncreate, FALSE);
        return FALSE;
    }
    char *password = (char *)gtk_entry_get_text(GTK_ENTRY(entrypwd));
    char *ensurepassword = (char *)gtk_entry_get_text(GTK_ENTRY(entryensurepwd));

    if (password && username && (strcmp(password, ensurepassword) == 0)
            && (strlen(password) > 5)
            && (strlen(password) < 64)
            && (strlen(username) >= 1)
            && (strcmp(username, _("Please enter the username")) != 0)
            && (strcmp(password, _("Please enter the password ")) != 0)
            && !msg)
        gtk_widget_set_sensitive(buttoncreate, TRUE);
    else
        gtk_widget_set_sensitive(buttoncreate, FALSE);

    GList *list;
    for(list = userlist; list; list = list->next)
    {
        UserInfo *info = (UserInfo *)list->data;
        if (strcmp(info->username, username) == 0)
        {
            gtk_label_set_text(GTK_LABEL(labelname), _("The user name is already in use, please use a different one."));
            gtk_widget_set_sensitive(buttoncreate, FALSE);
            return FALSE;
        }
        else
            gtk_label_set_text(GTK_LABEL(labelname), " ");
    }

    if (strlen(username) > 0 && strlen(username) < 32)
    {
        char cmd[50];
        sprintf(cmd, "getent group %s", username);
        pid_t status;
        status = system(cmd);
        if (WIFEXITED(status))
        {
            if (0 == WEXITSTATUS(status))
            {
                printf("run shell script successfully.\n");
                //gtk_widget_set_size_request(GTK_WIDGET(labelname), -1, 18);
                gtk_label_set_text(GTK_LABEL(labelname),_("The user name corresponds to the group already exists,please use a different user name"));
                gtk_widget_set_sensitive(buttoncreate, FALSE);
                return FALSE;
            }
            else
                gtk_widget_set_size_request(GTK_WIDGET(labelname), -1, 8);
        }
        gtk_label_set_text(GTK_LABEL(labelname), " ");
    }
    else
    {
        //gtk_widget_set_size_request(GTK_WIDGET(labelname), -1, 32);
        gtk_label_set_text(GTK_LABEL(labelname),_("username length need to less than 32!"));
        gtk_widget_set_sensitive(buttoncreate, FALSE);
        return FALSE;
    }

    return FALSE;
}

static gboolean
focusIn(GtkWidget *widget, gpointer userdata)
{
    const char *text = gtk_entry_get_text(GTK_ENTRY(widget));
    if (strcmp(text, _("Please enter the username")) == 0 || strcmp(text, _("Please enter the new username")) == 0)
        gtk_entry_set_text(GTK_ENTRY(widget), "");
    if( strcmp(text, _("Please enter the password")) == 0 || strcmp(text, _("Please enter new password")) == 0 ||
            strcmp(text, _("Please enter the current password")) == 0 || strcmp(text, _("Please enter the new password")) == 0 || strcmp(text, _("Please confirm the new password")) == 0)
    {
        gtk_entry_set_visibility(GTK_ENTRY(widget), FALSE);
        gtk_entry_set_text(GTK_ENTRY(widget), "");
    }
    GdkColor color;
    gdk_color_parse("#000000", &color);
    gtk_widget_modify_text(widget, GTK_STATE_NORMAL, &color);
    return FALSE;
}

static void
pwdTextChanged(GtkWidget *widget, gpointer userdata)
{
    const char *pwd1, *pwd2, *pwd3, *msg;

    GtkWidget *entry1 = GTK_WIDGET(gtk_builder_get_object (ui, "entry1"));
    GtkWidget *entry2 = GTK_WIDGET(gtk_builder_get_object (ui, "entry2"));
    GtkWidget *entry3 = GTK_WIDGET(gtk_builder_get_object (ui, "entry3"));
    GtkWidget *label2 = GTK_WIDGET(gtk_builder_get_object (ui, "label2"));
    GtkWidget *label4 = GTK_WIDGET(gtk_builder_get_object (ui, "label4"));
    guint16 length2 = gtk_entry_get_text_length(GTK_ENTRY(entry2));
    guint16 length3 = gtk_entry_get_text_length(GTK_ENTRY(entry3));
    pwd1 = gtk_entry_get_text (GTK_ENTRY(entry1));
    pwd2 = gtk_entry_get_text (GTK_ENTRY(entry2));
    pwd3 = gtk_entry_get_text (GTK_ENTRY(entry3));
    msg = passwd_check(pwd1, pwd2, g_get_user_name());
    GtkWidget *buttonok = GTK_WIDGET(gtk_builder_get_object (ui, "buttonok"));
    gboolean visible = gtk_widget_get_visible(entry1);
    //	gtk_label_set_text(GTK_LABEL(label2), "");
    gtk_label_set_text(GTK_LABEL(label4), "");
    gtk_label_set_xalign(GTK_LABEL(label2), 0.0);
    gtk_label_set_xalign(GTK_LABEL(label4), 0.0);
    if (visible)
    {
        if(strlen(pwd1) > 0 && strlen(pwd2) > 0 && strlen(pwd3) > 0 &&
                strcmp(pwd1, _("Please enter the current password")) != 0 &&
                strcmp(pwd2, _("Please enter the new password")) != 0 &&
                strcmp(pwd3, _("Please confirm the new password")) != 0
                )
            gtk_widget_set_sensitive(buttonok, TRUE);
        else
            gtk_widget_set_sensitive(buttonok, FALSE);
    }
    if(strlen(pwd2) > 0 && strlen(pwd3) > 0 &&
            strcmp(pwd2, _("Please enter the new password")) != 0 &&
            strcmp(pwd3, _("Please confirm the new password")) != 0 &&
            length2 <= 63 &&
            length3 <= 63 &&
            !msg
            )
        gtk_widget_set_sensitive(buttonok, TRUE);
    else if(length2 > 63 || length3 >63)
    {
        gtk_label_set_text(GTK_LABEL(label2), _("Password length needs to less than 64 digits!"));
        gtk_widget_set_sensitive(buttonok, FALSE);
    }
    else
        gtk_widget_set_sensitive(buttonok, FALSE);
    if(strcmp(pwd2, pwd3) != 0 &&
            strcmp(pwd2, _("Please enter the new password")) != 0 &&
            strcmp(pwd3, _("Please confirm the new password")) != 0
            )
    {
        gtk_label_set_text(GTK_LABEL(label4), _("enter the password twice inconsistencies!"));
        gtk_widget_set_sensitive(buttonok, FALSE);
    }
}

void dialog_quit(GtkWidget *widget, gpointer userdata)
{
    gtk_widget_destroy(GTK_WIDGET(dialog));
    g_object_unref(ui);
}

static void
chpasswd_cb (PasswdHandler *passwd_handler,
             GError        *error,
             gpointer       user_data)
{
    GtkWidget *user_passwd_dialog;
    GtkWidget *dialog;
    GtkWidget *entry;
    char *primary_text;
    char *secondary_text;

    user_passwd_dialog = GTK_WIDGET(gtk_builder_get_object (ui, "changepwd"));

    /* Restore dialog sensitivity and reset cursor */
    gtk_widget_set_sensitive (GTK_WIDGET (user_passwd_dialog), TRUE);
    gdk_window_set_cursor (gtk_widget_get_window (user_passwd_dialog), NULL);


    if (!error) {
        //finish_password_change (TRUE);
        gtk_widget_destroy(user_passwd_dialog);
        g_object_unref(ui);
        passwd_destroy (passwd_handler);
        return;
    }

    if (error->code == PASSWD_ERROR_REJECTED) {
        primary_text = error->message;
        secondary_text = _("Please choose another password.");

        entry = GTK_WIDGET(gtk_builder_get_object(ui, "entry1"));
        gtk_entry_set_text (GTK_ENTRY (entry), "");
        gtk_widget_grab_focus (entry);

        entry = GTK_WIDGET(gtk_builder_get_object (ui, "entry2"));
        gtk_entry_set_text (GTK_ENTRY (entry), "");

        entry = GTK_WIDGET(gtk_builder_get_object (ui, "entry3"));
        gtk_entry_set_text (GTK_ENTRY (entry), "");
    }
    else if (error->code == PASSWD_ERROR_AUTH_FAILED) {
        primary_text = error->message;
        secondary_text = _("Please reenter the current password.");

        entry = GTK_WIDGET(gtk_builder_get_object(ui, "entry1"));
        gtk_widget_grab_focus (entry);
        gtk_entry_set_text (GTK_ENTRY (entry), "");
    }
    else {
        primary_text = _("Password can not be modified.");
        secondary_text = error->message;
    }

    GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object(ui, "changepwd"));
    dialog = gtk_message_dialog_new (GTK_WINDOW (widget),
                                     GTK_DIALOG_MODAL,
                                     GTK_MESSAGE_ERROR,
                                     GTK_BUTTONS_CLOSE,
                                     "%s", primary_text);
    gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                              "%s", secondary_text);

    gtk_widget_set_name(GTK_WIDGET(dialog), "ukuicc");
    gtk_dialog_run (GTK_DIALOG (dialog));

    gtk_widget_destroy (dialog);
}


void change_pwd(GtkWidget *widget, gpointer userdata)
{
    GtkWidget *user_passwd_dialog;
    PasswdHandler *passwd_handler = NULL;
    const char *passwd;
    UserInfo *user = (UserInfo *)userdata;
    user_passwd_dialog = GTK_WIDGET(gtk_builder_get_object (ui, "changepwd"));
    GtkWidget *user_passwd_current = GTK_WIDGET(gtk_builder_get_object (ui, "entry1"));
    GtkWidget *new_passwd_entry = GTK_WIDGET(gtk_builder_get_object (ui, "entry2"));

    passwd_handler = g_object_get_data (G_OBJECT (user_passwd_current), "passwd_handler");

    //要修改的新密码
    passwd = gtk_entry_get_text (GTK_ENTRY (new_passwd_entry));

    if(!passwd)
        return;

    //如果是修改root的密码，通过chpasswd修改以避免交互
    if(!getuid() && user->currentuser)
    {
        char buffer[256];
        sprintf(buffer, "echo 'root:%s' | chpasswd", passwd);

        int sysback = system(buffer);
        if(sysback == -1)
            g_warning("Change the root password failed!");

        gtk_widget_destroy(user_passwd_dialog);
        g_object_unref(ui);
        return;
    }

    if (user->currentuser) {
        passwd_change_password (passwd_handler, passwd, chpasswd_cb, NULL);
        /* chpasswd_cb() will run finish_passwd_change() when done */

        gtk_widget_set_sensitive (GTK_WIDGET (user_passwd_dialog), FALSE);
        GdkDisplay *display = gtk_widget_get_display (GTK_WIDGET (user_passwd_current));
        GdkCursor *cursor;
        cursor = gdk_cursor_new_for_display (display, GDK_WATCH);
        gdk_window_set_cursor (gtk_widget_get_window (GTK_WIDGET (user_passwd_current)),
                               cursor);
        gdk_display_flush (display);
        g_object_unref (cursor);
    }
    /* For other users, set password via the dbus */
    else {
        char *crypted = make_crypted(passwd);
        g_dbus_proxy_call(user->proxy, "SetPassword",
                          g_variant_new("(ss)", crypted, ""),
                          G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);
        if (passwd_handler)
            passwd_destroy (passwd_handler);

        gtk_widget_destroy(user_passwd_dialog);
        g_object_unref(ui);
    }
}

static void
auth_cb (PasswdHandler *passwd_handler,
         GError        *error,
         gpointer       user_data)
{
    GtkWidget *entry = GTK_WIDGET (user_data);
    GdkColor color;

    gdk_color_parse ("red", &color);
    gtk_widget_modify_base (entry, GTK_STATE_NORMAL, error ? &color : NULL);

    GtkWidget *label1 = GTK_WIDGET(gtk_builder_get_object (ui, "label1"));
    gtk_label_set_xalign(GTK_LABEL(label1), 0.0);
    gtk_label_set_text(GTK_LABEL(label1), error ? _("Password input error, please re-enter!"): "");
    gtk_widget_modify_fg(label1, GTK_STATE_NORMAL, error ? &color : NULL);
}

gboolean
on_user_passwd_focus_out (GtkWidget     *entry,
                          GdkEventFocus *event,
                          gpointer       user_data)
{
    PasswdHandler *passwd_handler;
    const char *password;
    GdkColor color;
    gdk_color_parse("#999999", &color);

    GtkWidget *entry1 = GTK_WIDGET(gtk_builder_get_object (ui, "entry1"));
    GtkWidget *entry2 = GTK_WIDGET(gtk_builder_get_object (ui, "entry2"));
    GtkWidget *entry3 = GTK_WIDGET(gtk_builder_get_object (ui, "entry3"));
    GtkWidget *label2 = GTK_WIDGET(gtk_builder_get_object (ui, "label2"));
    const char *current_passwd = gtk_entry_get_text (GTK_ENTRY (entry1));

    if(entry == entry1)
    {
        password = gtk_entry_get_text (GTK_ENTRY (entry));

        if (strlen (password) > 0) {
            passwd_handler = g_object_get_data (G_OBJECT (entry), "passwd_handler");
            /*
             * 通过spawn_backend()在后台启动passwd
             * 验证当前输入的密码是否正确，
             * 是将输入的当前密码先运行一次passwd，如果passwd通过，就说明输入的当前密码是正确的
             */
            passwd_authenticate (passwd_handler, password, auth_cb, entry);
        }
        else {
            gtk_entry_set_visibility(GTK_ENTRY(entry), TRUE);
            gtk_entry_set_text(GTK_ENTRY(entry), _("Please enter the current password"));
            gtk_widget_modify_text(entry, GTK_STATE_NORMAL, &color);
            GtkWidget *label1 = GTK_WIDGET(gtk_builder_get_object (ui, "label1"));
            gtk_label_set_text(GTK_LABEL(label1), "");
            gdk_color_parse("white", &color);
            gtk_widget_modify_base (entry, GTK_STATE_NORMAL, &color);
        }
    }
    if(entry == entry2)
    {
        password = gtk_entry_get_text (GTK_ENTRY (entry2));

        const char *msg = NULL;
        msg = passwd_check(current_passwd, password, g_get_user_name());
        if(msg){
            gtk_label_set_text(GTK_LABEL(label2), msg);
        }
        else
            gtk_label_set_text(GTK_LABEL(label2), "");

        if (strlen (password) < 1) {
            gtk_entry_set_visibility(GTK_ENTRY(entry2), TRUE);
            gtk_entry_set_text(GTK_ENTRY(entry2), _("Please enter new password"));
            gtk_widget_modify_text(entry2, GTK_STATE_NORMAL, &color);
        }
    }
    if(entry == entry3)
    {
        password = gtk_entry_get_text (GTK_ENTRY (entry3));
        if (strlen (password) < 1) {
            gtk_entry_set_visibility(GTK_ENTRY(entry3), TRUE);
            gtk_entry_set_text(GTK_ENTRY(entry3), _("Please confirm the new password"));
            gtk_widget_modify_text(entry3, GTK_STATE_NORMAL, &color);
        }
    }
    return FALSE;
}

void show_change_pwd_dialog(GtkButton *button, gpointer user_data)
{
    GError *err = NULL;
    PasswdHandler *passwd_handler = NULL;

    UserInfo *user = (UserInfo *)user_data;
    ui = gtk_builder_new();
    gtk_builder_add_from_file(ui, UIDIR "/change-pwd.ui", &err);
    if (err)
    {
        g_warning("Could not load user interface file: %s", err->message);
        g_error_free(err);
        g_object_unref(ui);
        return;
    }

    dialog = GTK_DIALOG(gtk_builder_get_object (ui, "changepwd"));
    gtk_window_set_icon_from_file (GTK_WINDOW(dialog), "/usr/share/ukui-control-center/icons/用户账号.png", NULL);
    GtkWidget *image = GTK_WIDGET(gtk_builder_get_object (ui, "image1"));
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(user->iconfile, NULL);
    if (!pixbuf)
        pixbuf = gdk_pixbuf_new_from_file("/usr/share/pixmaps/faces/stock_person.png", NULL);
    if (pixbuf){
        GdkPixbuf *face = gdk_pixbuf_scale_simple(pixbuf, 88, 88, GDK_INTERP_BILINEAR);
        gtk_image_set_from_pixbuf (GTK_IMAGE(image), face);
        g_object_unref(face);
    }
    g_object_unref(pixbuf);

    GtkWidget *labelname = GTK_WIDGET(gtk_builder_get_object (ui, "labelname"));
    gtk_label_set_xalign(GTK_LABEL(labelname), 0.0);
    gtk_label_set_text(GTK_LABEL(labelname), user->username);
    char *markup = g_markup_printf_escaped ("<span weight='bold' font_desc='11'>%s</span>", user->username);
    gtk_label_set_markup(GTK_LABEL(labelname), markup);

    GtkWidget *labeltype = GTK_WIDGET(gtk_builder_get_object (ui, "labeltype"));
    gtk_label_set_xalign(GTK_LABEL(labeltype), 0.0);
    if (user->accounttype == ADMINISTRATOR)
        gtk_label_set_text(GTK_LABEL(labeltype), _("Administrators"));
    else if (user->accounttype == STANDARDUSER)
        gtk_label_set_text(GTK_LABEL(labeltype), _("Standard user"));

    GtkWidget *label3 = GTK_WIDGET(gtk_builder_get_object (ui, "label3"));
    gtk_label_set_xalign(GTK_LABEL(label3), 0.0);
    if (user->currentuser)
    {
        gtk_label_set_text(GTK_LABEL(label3), _("Logged(Current User)"));
        passwd_handler = passwd_init ();
    }
    else if (user->logined && !user->currentuser)
    {
        gtk_label_set_text(GTK_LABEL(label3), _("Logged(Other Users)"));
    }
    else
    {
        gtk_label_set_text(GTK_LABEL(label3), _("Un-login(Other Users)"));
    }

    GtkWidget *user_passwd_current = GTK_WIDGET(gtk_builder_get_object (ui, "entry1"));
    //root用户修改密码时，不需要输入当前密码
    if(!getuid() && user->currentuser)
        gtk_widget_hide(user_passwd_current);
    //只是将passwd_handler这个字符串key转换成一个PasswdHandler的类型
    g_object_set_data (G_OBJECT (user_passwd_current), "passwd_handler", passwd_handler);
    if (user->currentuser)
    {
        gtk_entry_set_text(GTK_ENTRY(user_passwd_current), _("Please enter the current password"));
        GdkColor color;
        gdk_color_parse("#999999", &color);
        gtk_widget_modify_text(user_passwd_current, GTK_STATE_NORMAL, &color);
        g_signal_connect(user_passwd_current, "focus-in-event", G_CALLBACK(focusIn), NULL);
        g_signal_connect(user_passwd_current, "focus-out-event", G_CALLBACK(on_user_passwd_focus_out), NULL);
        g_signal_connect(user_passwd_current, "changed", G_CALLBACK(pwdTextChanged), user);
    }
    else
    {
        gtk_widget_hide(GTK_WIDGET(user_passwd_current));
    }

    GtkWidget *entry2 = GTK_WIDGET(gtk_builder_get_object (ui, "entry2"));
    gtk_entry_set_text(GTK_ENTRY(entry2), _("Please enter new password"));
    GdkColor color;
    gdk_color_parse("#999999", &color);
    gtk_widget_modify_text(entry2, GTK_STATE_NORMAL, &color);
    g_signal_connect(entry2, "focus-in-event", G_CALLBACK(focusIn), NULL);
    g_signal_connect(entry2, "focus-out-event", G_CALLBACK(on_user_passwd_focus_out), NULL);
    g_signal_connect(entry2, "changed", G_CALLBACK(pwdTextChanged), user);

    GtkWidget *entry3 = GTK_WIDGET(gtk_builder_get_object (ui, "entry3"));
    gtk_entry_set_text(GTK_ENTRY(entry3), _("Please confirm the new password"));
    gtk_widget_modify_text(entry3, GTK_STATE_NORMAL, &color);
    g_signal_connect(entry3, "focus-in-event", G_CALLBACK(focusIn), NULL);
    g_signal_connect(entry3, "focus-out-event", G_CALLBACK(on_user_passwd_focus_out), NULL);
    g_signal_connect(entry3, "changed", G_CALLBACK(pwdTextChanged), user);


    GtkWidget *buttoncancel = GTK_WIDGET(gtk_builder_get_object (ui, "buttoncancel"));
    g_signal_connect(buttoncancel, "clicked", G_CALLBACK(dialog_quit), NULL);
    gtk_widget_grab_focus(buttoncancel);

    GtkWidget *buttonok = GTK_WIDGET(gtk_builder_get_object (ui, "buttonok"));
    g_signal_connect(buttonok, "clicked", G_CALLBACK(change_pwd), user);
    gtk_widget_set_sensitive(buttonok, FALSE);
    gtk_widget_set_name(GTK_WIDGET(dialog), "ukuicc");
    gtk_widget_show(GTK_WIDGET(dialog));
}

void usernameChanged(GtkWidget *widget, gpointer userdata)
{
    char *primary_text;

    const char *username = gtk_entry_get_text(GTK_ENTRY(widget));
    GtkWidget *label1 = GTK_WIDGET(gtk_builder_get_object (ui, "label1"));
    gtk_label_set_xalign(GTK_LABEL(label1), 0.0);
    GtkWidget *buttonok = GTK_WIDGET(gtk_builder_get_object (ui, "buttonok"));
    guint16 length = gtk_entry_get_text_length(GTK_ENTRY(widget));

    if (strlen(username) > 0)
    {
        int i;
        for (i = 0; *(username + i) != 0; i++)
        {
            if (*(username) == 95)
            {
                gtk_label_set_text(GTK_LABEL(label1), _("The first character cannot be underlined!"));
                gtk_widget_set_sensitive(buttonok, FALSE);
                return;
            }
            if (isupper(*(username + i)))
            {
                gtk_label_set_text(GTK_LABEL(label1), _("User name can not contain capital letters!"));
                gtk_widget_set_sensitive(buttonok, FALSE);
                return;
            }
            if ((*(username + i) >=48 && *(username + i) <= 57 ) || (*(username + i) >= 97 && *(username + i) <= 122 ) || ( *(username + i) == 95))
                continue;
            else
            {
                gtk_label_set_text(GTK_LABEL(label1), _("The user name can only be composed of letters, numbers and underline!"));
                gtk_widget_set_sensitive(buttonok, FALSE);
                return;
            }
        }
        if (isdigit(*username))
        {
            gtk_label_set_text(GTK_LABEL(label1), _("User name cannot start with number!"));
            gtk_widget_set_sensitive(buttonok, FALSE);
            return;
        }
        if (length > 31)
        {
            gtk_label_set_text(GTK_LABEL(label1), _("Username length need to less than 32!"));
            gtk_widget_set_sensitive(buttonok, FALSE);
            return;
        }


        GList *list;
        for(list = userlist; list; list = list->next)
        {
            UserInfo *info = (UserInfo *)list->data;
            if (strcmp(info->username, username) == 0)
            {
                primary_text = _("The user name has been used, please replace with another one!");
                gtk_widget_set_sensitive(buttonok, FALSE);
                gtk_label_set_text(GTK_LABEL(label1), primary_text);
                return;
            }
        }
        gtk_widget_set_sensitive(buttonok, TRUE);
        gtk_label_set_text(GTK_LABEL(label1), "");
    }
    else
        gtk_widget_set_sensitive(buttonok, FALSE);

}

void set_username_callback(GObject *object, GAsyncResult *res, gpointer user_data)
{
    GError * error = NULL;
    GVariant *result;

    UserInfo *user = (UserInfo *)user_data;

    result = g_dbus_proxy_call_finish(G_DBUS_PROXY(object), res, &error);
    if (result == NULL)
    {
        g_warning("Callback Result is null");
        return;
    }
    if (error != NULL)
    {
        g_warning("DBUS error:%s", error->message);
        g_error_free(error);
        char *primary_text = _("Modify username failed!");
        GtkWidget *w = GTK_WIDGET(gtk_builder_get_object(ui, "changename"));
        GtkWidget *d = gtk_message_dialog_new (GTK_WINDOW (w),
                                               GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_CLOSE,
                                               "%s", primary_text);
        gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (d),
                                                  "%s", error->message);

        gtk_widget_set_name(GTK_WIDGET(d), "ukuicc");
        gtk_dialog_run (GTK_DIALOG (d));

        gtk_widget_destroy (d);
        g_object_unref(ui);
        return;
    }
    GtkWidget *entry1 = GTK_WIDGET(gtk_builder_get_object (ui, "entry1"));
    const char *username = gtk_entry_get_text(GTK_ENTRY(entry1));
    strcpy(user->username, username);

    char *markup = g_markup_printf_escaped ("<span weight='bold' font_desc='11'>%s</span>", user->username);
    gtk_label_set_markup(GTK_LABEL(user->labelname0), markup);
    gtk_label_set_markup(GTK_LABEL(user->labelname1), markup);
    gtk_widget_destroy(GTK_WIDGET(dialog));
    g_object_unref(ui);
}

void change_username(GtkWidget *widget, gpointer userdata)
{
    GtkWidget *entry1 = GTK_WIDGET(gtk_builder_get_object (ui, "entry1"));
    const char *username = gtk_entry_get_text(GTK_ENTRY(entry1));

    UserInfo *user = (UserInfo *)userdata;

    if(user->logined)
    {
        char *primary_text = _("Modify username failed!");
        GtkWidget *w = GTK_WIDGET(gtk_builder_get_object(ui, "changename"));
        GtkWidget *d = gtk_message_dialog_new (GTK_WINDOW (w),
                                               GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_CLOSE,
                                               "%s", primary_text);

        char *secondary_text = _("the user has logged in, please log out and modify!");
        gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (d),
                                                  "%s", secondary_text);
        gtk_widget_set_name(GTK_WIDGET(d), "ukuicc");
        gtk_dialog_run (GTK_DIALOG (d));

        gtk_widget_destroy (d);
        //g_object_unref(ui);
        return;
    }
    g_dbus_proxy_call(user->proxy, "SetUserName",
                      g_variant_new("(s)", username), G_DBUS_CALL_FLAGS_NONE, -1,
                      NULL, set_username_callback, user);

}

void show_change_name_dialog(GtkButton *button, gpointer user_data)
{
    GError *err = NULL;

    UserInfo *user = (UserInfo *)user_data;
    ui = gtk_builder_new();
    gtk_builder_add_from_file(ui, UIDIR "/change-name.ui", &err);
    //gtk_builder_add_from_file(ui, "../panels/user-accounts/change-name.ui", &err);
    if (err)
    {
        g_warning("Could not load user interface file: %s", err->message);
        g_error_free(err);
        g_object_unref(ui);
        return;
    }

    dialog = GTK_DIALOG(gtk_builder_get_object (ui, "changename"));
    gtk_window_set_icon_from_file (GTK_WINDOW(dialog), "/usr/share/ukui-control-center/icons/用户账号.png", NULL);
    GtkWidget *image = GTK_WIDGET(gtk_builder_get_object (ui, "image1"));
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(user->iconfile, NULL);
    if (!pixbuf){
        pixbuf = gdk_pixbuf_new_from_file("/usr/share/pixmaps/faces/stock_person.png", NULL);
        gtk_image_set_from_pixbuf (GTK_IMAGE(image), pixbuf);
    }
    else{
        GdkPixbuf *face = gdk_pixbuf_scale_simple(pixbuf, 88, 88, GDK_INTERP_BILINEAR);
        gtk_image_set_from_pixbuf (GTK_IMAGE(image), face);
        g_object_unref(face);
    }
    g_object_unref(pixbuf);

    GtkWidget *labelname = GTK_WIDGET(gtk_builder_get_object (ui, "labelname"));
    gtk_label_set_xalign(GTK_LABEL(labelname), 0.03);
    gtk_label_set_yalign(GTK_LABEL(labelname), 1.0);
    gtk_label_set_text(GTK_LABEL(labelname), user->username);
    char *markup = g_markup_printf_escaped ("<span weight='bold' font_desc='11'>%s</span>", user->username);
    gtk_label_set_markup(GTK_LABEL(labelname), markup);
    GtkWidget *labeltype = GTK_WIDGET(gtk_builder_get_object (ui, "labeltype"));
    gtk_label_set_xalign(GTK_LABEL(labeltype), 0.03);
    if (user->accounttype == ADMINISTRATOR)
        gtk_label_set_text(GTK_LABEL(labeltype), _("Administrators"));
    else if (user->accounttype == STANDARDUSER)
        gtk_label_set_text(GTK_LABEL(labeltype), _("Standard user"));

    GtkWidget *label3 = GTK_WIDGET(gtk_builder_get_object (ui, "label3"));
    gtk_label_set_xalign(GTK_LABEL(label3), 0.03);
    gtk_label_set_yalign(GTK_LABEL(label3), 0.0);
    if (user->currentuser)
        gtk_label_set_text(GTK_LABEL(label3), _("Logged(Current User)"));
    else if (user->logined && !user->currentuser)
        gtk_label_set_text(GTK_LABEL(label3), _("Logged(Other Users)"));
    else
        gtk_label_set_text(GTK_LABEL(label3), _("Un-login(Other Users)"));

    GtkWidget *entry1 = GTK_WIDGET(gtk_builder_get_object (ui, "entry1"));
    gtk_entry_set_text(GTK_ENTRY(entry1), _("Please enter the new username"));
    GdkColor color;
    gdk_color_parse("#999999", &color);
    gtk_widget_modify_text(entry1, GTK_STATE_NORMAL, &color);
    g_signal_connect(entry1, "focus-in-event", G_CALLBACK(focusIn), NULL);
    g_signal_connect(entry1, "changed", G_CALLBACK(usernameChanged), user);


    GtkWidget *buttoncancel = GTK_WIDGET(gtk_builder_get_object (ui, "buttoncancel"));
    g_signal_connect(buttoncancel, "clicked", G_CALLBACK(dialog_quit), NULL);
    gtk_widget_grab_focus(buttoncancel);

    GtkWidget *buttonok = GTK_WIDGET(gtk_builder_get_object (ui, "buttonok"));
    g_signal_connect(buttonok, "clicked", G_CALLBACK(change_username), user);
    gtk_widget_set_sensitive(buttonok, FALSE);
    gtk_widget_set_name(GTK_WIDGET(dialog), "ukuicc");
    gtk_widget_show(GTK_WIDGET(dialog));
}

void change_face_callback(GObject *object, GAsyncResult *res, gpointer user_data)
{
    GError * error = NULL;
    GVariant *result;

    UserInfo *user = (UserInfo *)user_data;

    result = g_dbus_proxy_call_finish(G_DBUS_PROXY(object), res, &error);
    if (result == NULL)
    {
        g_warning("Callback Result is null");
        return;
    }
    if (error != NULL)
    {
        g_warning("DBUS error:%s", error->message);
        g_error_free(error);
        return;
    }

    if (g_strrstr(user->iconfile, "stock_person"))
        user->iconfile = "/usr/share/pixmaps/faces/stock_person.png";
    GdkPixbuf *buf = gdk_pixbuf_new_from_file(user->iconfile, NULL);
    buf = gdk_pixbuf_scale_simple(buf, FACEHEIGHT, FACEWIDTH, GDK_INTERP_BILINEAR);
    gtk_image_set_from_pixbuf(GTK_IMAGE(user->image0), buf);
    gtk_image_set_from_pixbuf(GTK_IMAGE(user->image1), buf);

    g_object_unref(buf);

    system("gsettings set org.ukui.ukui-menu user-icon-changed true");
}

void change_face(GtkWidget *widget, gpointer userdata)
{
    UserInfo *user = (UserInfo *)userdata;

    GtkWidget *image = GTK_WIDGET(gtk_builder_get_object (ui, "imageuser"));
    GdkPixbuf *pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(image));
    const char *filename = g_object_get_data (G_OBJECT (pixbuf), "filename");

    if (filename){
        user->iconfile = (gchar *)filename;
        if (g_strrstr(user->iconfile, "stock_person"))
            user->iconfile = "/usr/share/pixmaps/faces/stock_person_nobg.png";
    }

    g_dbus_proxy_call(user->proxy, "SetIconFile",
                      g_variant_new("(s)", user->iconfile),
                      G_DBUS_CALL_FLAGS_NONE, -1, NULL, change_face_callback, user);

    g_object_unref(pixbuf);
    gtk_widget_destroy(GTK_WIDGET(dialog));
    g_object_unref(ui);
}

GtkTreeModel *init_model(void)
{
    GtkListStore *list_store;
    GtkTreeIter iter;

    list_store = gtk_list_store_new(NUM_COLS, GDK_TYPE_PIXBUF);
    find_all_face_file(list_store, iter);

    return GTK_TREE_MODEL(list_store);
}

void itemSelected(GtkWidget *widget, gpointer userdata)
{
    GList *selected;
    GdkPixbuf *pixbuf;
    GtkTreeModel *smodel;
    selected = gtk_icon_view_get_selected_items(GTK_ICON_VIEW(widget));
    smodel = gtk_icon_view_get_model(GTK_ICON_VIEW(widget));
    if (selected != NULL){
        GtkTreeIter sel_iter;
        gtk_tree_model_get_iter(smodel, &sel_iter, selected->data);
        gtk_tree_model_get(smodel, &sel_iter, COL_PIXBUF, &pixbuf, -1);
        GtkWidget *image = GTK_WIDGET(gtk_builder_get_object (ui, "imageuser"));
        gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);
    }
}

//如果已经设置了自动登陆用户，提示用户是否覆盖原先的设置
static
void confirm_dialog(GtkWidget *widget, gpointer user_data)
{
    UserInfo *user = (UserInfo *)user_data;
    if(!user->autologin){
        GList *it = NULL;
        for (it = userlist; it; it = it->next)
        {
            UserInfo *system_user = (UserInfo *)it->data;
            if(system_user->autologin == TRUE)
            {
                if(0 == strcmp(user->username,system_user->username) || !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
                    continue;
                GtkWidget *label;
                //GtkWidget *hbox;
                GtkWidget *dialog = gtk_dialog_new_with_buttons(
                            _("auto login"),
                            window,
                            GTK_DIALOG_MODAL,
                            _("_Cancel"),
                            GTK_RESPONSE_REJECT,
                            _("_OK"),
                            GTK_RESPONSE_ACCEPT,
                            NULL);
                label = gtk_label_new(_("\tAlready have other users set to automatically log in,\t\n \tclick OK will overwrite the existing settings!"));
                gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), label, TRUE, TRUE, 30);
                gtk_widget_show_all(dialog);
                gint result = gtk_dialog_run(GTK_DIALOG(dialog));
                if(result == GTK_RESPONSE_ACCEPT)
                    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
                else
                    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), FALSE);
                gtk_widget_destroy(dialog);
            }
        }
    }//end-if
}

void show_change_face_dialog(GtkButton *button, gpointer user_data)
{
    GError *err = NULL;

    UserInfo *user = (UserInfo *)user_data;
    ui = gtk_builder_new();
    gtk_builder_add_from_file(ui, UIDIR "/change-face.ui", &err);
    //gtk_builder_add_from_file(ui, "../panels/user-accounts/change-face.ui", &err);
    if (err)
    {
        g_warning("Could not load user interface file: %s", err->message);
        g_error_free(err);
        g_object_unref(ui);
        return;
    }

    dialog = GTK_DIALOG(gtk_builder_get_object (ui, "changeface"));
    gtk_window_set_icon_from_file (GTK_WINDOW(dialog), "/usr/share/ukui-control-center/icons/用户账号.png", NULL);
    GtkWidget *image = GTK_WIDGET(gtk_builder_get_object (ui, "imageuser"));
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(user->iconfile, NULL);
    if (!pixbuf)
        pixbuf = gdk_pixbuf_new_from_file("/usr/share/pixmaps/faces/stock_person.png", NULL);
    GdkPixbuf *face = gdk_pixbuf_scale_simple(pixbuf, 88, 88, GDK_INTERP_BILINEAR);
    gtk_image_set_from_pixbuf (GTK_IMAGE(image), face);
    g_object_unref(pixbuf);
    g_object_unref(face);

    GtkWidget *labelname = GTK_WIDGET(gtk_builder_get_object (ui, "labelname"));
    gtk_label_set_xalign(GTK_LABEL(labelname), 0.0);
    gtk_label_set_text(GTK_LABEL(labelname), user->username);
    char *markup = g_markup_printf_escaped ("<span weight='bold' font_desc='11'>%s</span>", user->username);
    gtk_label_set_markup(GTK_LABEL(labelname), markup);
    GtkWidget *labeltype = GTK_WIDGET(gtk_builder_get_object (ui, "labeltype"));
    gtk_label_set_xalign(GTK_LABEL(labeltype), 0.0);
    if (user->accounttype == ADMINISTRATOR)
        gtk_label_set_text(GTK_LABEL(labeltype), _("Administrators"));
    else if (user->accounttype == STANDARDUSER)
        gtk_label_set_text(GTK_LABEL(labeltype), _("Standard user"));

    GtkWidget *label3 = GTK_WIDGET(gtk_builder_get_object (ui, "label3"));
    gtk_label_set_xalign(GTK_LABEL(label3), 0.0);
    if (user->currentuser)
        gtk_label_set_text(GTK_LABEL(label3), _("Logged(Current User)"));
    else if (user->logined && !user->currentuser)
        gtk_label_set_text(GTK_LABEL(label3), _("Logged(Other Users)"));
    else
        gtk_label_set_text(GTK_LABEL(label3), _("Un-login(Other Users)"));

    GtkWidget *facecontainer = GTK_WIDGET(gtk_builder_get_object (ui, "scrolledwindow1"));
    GtkWidget *icon_view = gtk_icon_view_new_with_model(init_model());
    gtk_container_add(GTK_CONTAINER(facecontainer), icon_view);
    gtk_widget_show_all(icon_view);
    g_signal_connect(icon_view, "selection-changed", G_CALLBACK(itemSelected), NULL);

    gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(icon_view), COL_PIXBUF);
    gtk_icon_view_set_selection_mode(GTK_ICON_VIEW(icon_view), GTK_SELECTION_MULTIPLE);
    gtk_icon_view_set_item_padding (GTK_ICON_VIEW(icon_view), 3);
    gtk_icon_view_set_spacing (GTK_ICON_VIEW(icon_view), 1);

    GtkWidget *bt_add = GTK_WIDGET(gtk_builder_get_object (ui, "bt_add"));
    g_signal_connect(bt_add, "clicked", G_CALLBACK(file_icon_selected), user);

    GtkWidget *buttoncancel = GTK_WIDGET(gtk_builder_get_object (ui, "buttoncancel"));
    g_signal_connect(buttoncancel, "clicked", G_CALLBACK(dialog_quit), NULL);
    gtk_widget_grab_focus(buttoncancel);

    GtkWidget *buttonok = GTK_WIDGET(gtk_builder_get_object (ui, "buttonok"));
    g_signal_connect(buttonok, "clicked", G_CALLBACK(change_face), user);
    gtk_widget_set_name(GTK_WIDGET(dialog), "ukuicc");
    gtk_widget_show(GTK_WIDGET(dialog));
}

void set_accounttype_callback(GObject *object, GAsyncResult *res, gpointer user_data)
{
    GError * error = NULL;
    GError * err = NULL;
    GVariant *result;

    UserInfo *user = (UserInfo *)user_data;

    ui = gtk_builder_new();
    gtk_builder_add_from_file(ui, UIDIR "/change-type.ui", &err);
    if (err)
    {
        g_warning("Could not load user interface file: %s", err->message);
        g_error_free(err);
        g_object_unref(ui);
        return;
    }
    GtkWidget *radio_admin = GTK_WIDGET (gtk_builder_get_object (ui, "radiobutton2"));
    GtkWidget *radio_standard_user = GTK_WIDGET (gtk_builder_get_object (ui, "radiobutton1"));
    GVariant *value = NULL;

    result = g_dbus_proxy_call_finish(G_DBUS_PROXY(object), res, &error);
    //调用未成功或取消了调用
    if (result == NULL)
    {
        //若取消调用,radiobutton的状态应不改变，这时不能直接使用user->accounttype,应该获取dbus的accounttype属性的值
        g_warning("Callback Result is null.\n");
        value = g_dbus_proxy_get_cached_property(user->proxy, "AccountType");
        user->accounttype = (gint)g_variant_get_int32(value);
        if(user->accounttype == 1)
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_admin), TRUE);
        else
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_standard_user), TRUE);
        return;
    }
    if (error != NULL)
    {
        g_warning("DBUS error:%s", error->message);
        g_error_free(error);
        return;
    }
    g_warning("----------%s\n",user->username);
    if(user->autologin == TRUE)  //确保自动登录用户只有最新设置的生效
    {
        g_warning("------1----%s\n",user->username);
        GList *it = NULL;
        for (it = userlist; it; it = it->next)
        {
            UserInfo *system_user = (UserInfo *)it->data;
            if(system_user->autologin == TRUE)
            {
                if(0 == strcmp(user->username,system_user->username))
                    continue;
                system_user->autologin = FALSE;
                g_dbus_proxy_call(user->proxy, "SetAutomaticLogin",
                                  g_variant_new("(b)", system_user->autologin),
                                  G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);
            }
        }
    }
    if (error != NULL)
    {
        g_warning("DBUS error:%s", error->message);
        g_error_free(error);
        return;
    }

    g_dbus_proxy_call(user->proxy, "SetAutomaticLogin",
                      g_variant_new("(b)", user->autologin),
                      G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);
    if (user->accounttype)
    {
        gtk_label_set_text(GTK_LABEL(user->labeltype0), _("Administrators"));
        gtk_label_set_text(GTK_LABEL(user->labeltype1), _("Administrators"));
    }
    else
    {
        gtk_label_set_text(GTK_LABEL(user->labeltype0), _("Standard user"));
        gtk_label_set_text(GTK_LABEL(user->labeltype1), _("Standard user"));
    }
}

void change_accounttype(GtkWidget *widget, gpointer userdata)
{
    UserInfo *user = (UserInfo *)userdata;
    GtkWidget *admin = GTK_WIDGET (gtk_builder_get_object (ui, "radiobutton2"));
    gboolean state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(admin));
    if (state)
        user->accounttype = ADMINISTRATOR;
    else
        user->accounttype = STANDARDUSER;

    g_dbus_proxy_call(user->proxy, "SetAccountType",
                      g_variant_new("(i)", user->accounttype),
                      G_DBUS_CALL_FLAGS_NONE, -1, NULL, set_accounttype_callback, user);

    GtkWidget *btautologin = GTK_WIDGET(gtk_builder_get_object (ui, "btautologin"));
    state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btautologin));
    if (state)
        user->autologin = TRUE;
    else
        user->autologin = FALSE;
    gtk_widget_destroy(GTK_WIDGET(dialog));
    g_object_unref(ui);
}

//获取系统的管理员数量,判断sudo组有多少个成员,其实还须判断admin组
int get_adm_count()
{
    int account_count = 0;
    FILE *file;
    char buffer[128];
    file = fopen("/etc/group", "r");
    if(!file)
        g_warning("/etc/group not exist!");
    while(fgets(buffer, 128, file)){
        gchar **tmp = g_strsplit(buffer, ":", 4);
        tmp[0] = g_strstrip(tmp[0]);
        tmp[3] = g_strstrip(tmp[3]);
        if(!strcmp(tmp[0], "sudo"))
        {
            if(!tmp[3]){
                g_strfreev(tmp);
                fclose(file);
                return 0;
            }
            else{
                int i = 0;
                while(tmp[3][i] != '\0'){
                    if(tmp[3][i] == ',')
                        ++account_count;
                    ++i;
                }
                if(!account_count){
                    g_strfreev(tmp);
                    fclose(file);
                    return 1;
                }
            }
            break;
        }
    }
    fclose(file);
    return account_count + 1;
}

void show_change_accounttype_dialog(GtkButton *button, gpointer user_data)
{
    GError *err = NULL;

    UserInfo *user = (UserInfo *)user_data;
    ui = gtk_builder_new();
    gtk_builder_add_from_file(ui, UIDIR "/change-type.ui", &err);
    if (err)
    {
        g_warning("Could not load user interface file: %s", err->message);
        g_error_free(err);
        g_object_unref(ui);
        return;
    }

    dialog = GTK_DIALOG(gtk_builder_get_object (ui, "changetype"));
    gtk_window_set_icon_from_file (GTK_WINDOW(dialog), "/usr/share/ukui-control-center/icons/用户账号.png", NULL);
    GtkWidget *image = GTK_WIDGET(gtk_builder_get_object (ui, "image1"));
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(user->iconfile, NULL);
    if (!pixbuf)
        pixbuf = gdk_pixbuf_new_from_file("/usr/share/pixmaps/faces/stock_person.png", NULL);
    GdkPixbuf *face = gdk_pixbuf_scale_simple(pixbuf, 88, 88, GDK_INTERP_BILINEAR);
    gtk_image_set_from_pixbuf (GTK_IMAGE(image), face);
    g_object_unref(pixbuf);
    g_object_unref(face);

    GtkWidget *label6 = GTK_WIDGET(gtk_builder_get_object (ui, "label6"));
    gtk_label_set_xalign(GTK_LABEL(label6), 0.0);
    GtkWidget *labelname = GTK_WIDGET(gtk_builder_get_object (ui, "labelname"));
    gtk_label_set_xalign(GTK_LABEL(labelname), 0.05);
    gtk_label_set_text(GTK_LABEL(labelname), user->username);
    char *markup = g_markup_printf_escaped ("<span weight='bold' font_desc='11'>%s</span>", user->username);
    gtk_label_set_markup(GTK_LABEL(labelname), markup);
    GtkWidget *stard = GTK_WIDGET (gtk_builder_get_object (ui, "radiobutton1"));
    GtkWidget *stard_label = GTK_WIDGET (gtk_builder_get_object (ui, "label2"));
    GtkWidget *admin = GTK_WIDGET (gtk_builder_get_object (ui, "radiobutton2"));
    GtkWidget *labeltype = GTK_WIDGET(gtk_builder_get_object (ui, "labeltype"));
    gtk_label_set_xalign(GTK_LABEL(labeltype), 0.05);
    if (user->accounttype == ADMINISTRATOR)
    {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(admin), TRUE);
        gtk_label_set_text(GTK_LABEL(labeltype), _("Administrators"));
    }
    else if (user->accounttype == STANDARDUSER)
    {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(stard), TRUE);
        gtk_label_set_text(GTK_LABEL(labeltype), _("Standard user"));
    }

    int adm_count = get_adm_count();
    //确保系统至少有一个管理员
    if(adm_count == 1 && user->accounttype)
    {
        gtk_widget_set_sensitive(stard, FALSE);
        gtk_widget_set_sensitive(stard_label, FALSE);
    }

    GtkWidget *label3 = GTK_WIDGET(gtk_builder_get_object (ui, "label3"));
    gtk_label_set_xalign(GTK_LABEL(label3), 0.05);
    if (user->currentuser)
        gtk_label_set_text(GTK_LABEL(label3), _("Logged(Current User)"));
    else if (user->logined && !user->currentuser)
        gtk_label_set_text(GTK_LABEL(label3), _("Logged(Other Users)"));
    else
        gtk_label_set_text(GTK_LABEL(label3), _("Un-login(Other Users)"));

    GtkWidget *btautologin = GTK_WIDGET(gtk_builder_get_object (ui, "btautologin"));
    if (user->autologin)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(btautologin), user->autologin);
    g_signal_connect(btautologin, "clicked", G_CALLBACK(confirm_dialog), user);

    GtkWidget *buttoncancel = GTK_WIDGET(gtk_builder_get_object (ui, "buttoncancel"));
    g_signal_connect(buttoncancel, "clicked", G_CALLBACK(dialog_quit), NULL);
    gtk_widget_grab_focus(buttoncancel);

    GtkWidget *buttonok = GTK_WIDGET(gtk_builder_get_object (ui, "buttonok"));
    g_signal_connect(buttonok, "clicked", G_CALLBACK(change_accounttype), user);
    gtk_widget_set_name(GTK_WIDGET(dialog), "ukuicc");
    gtk_widget_show(GTK_WIDGET(dialog));
}

static void
deleteUserDone(GObject *object, GAsyncResult *res, gpointer user_data)
{
    GError * error = NULL;
    GVariant *result;

    result = g_dbus_proxy_call_finish(G_DBUS_PROXY(object), res, &error);
    if (result == NULL)
    {
        g_warning("Callback Result is null");
        return;
    }
    if (error != NULL)
    {
        g_warning("DBUS error:%s", error->message);
        g_error_free(error);
        if (result)
            g_variant_unref(result);
        return;
    }
    UserInfo *user = (UserInfo *)user_data;
    GtkWidget *box = GTK_WIDGET (gtk_builder_get_object (builder, "other_users"));
    GtkWidget *other_hbox = GTK_WIDGET (gtk_builder_get_object (builder, "hbox6"));
    gtk_container_remove(GTK_CONTAINER(box), GTK_WIDGET(user->notebook));
    gtk_widget_show_all(box);
    userlist = g_list_remove(userlist, user);
    if (g_list_length(userlist) == 1)
        gtk_widget_hide(other_hbox);
    free(user);

    if (result)
        g_variant_unref(result);
}

void storeFiles(GtkWidget *widget, gpointer userdata)
{
    GError *error = NULL;
    GDBusProxy *account_proxy;

    account_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,
                                                  NULL, "org.freedesktop.Accounts",
                                                  "/org/freedesktop/Accounts",
                                                  "org.freedesktop.Accounts", NULL, &error);
    if (error != NULL)
    {
        g_error("Could not connect to org.freedesktop.Accounts:%s\n",error->message);
        if (account_proxy)
            g_object_unref(account_proxy);
        return;
    }

    UserInfo *user = (UserInfo *)userdata;

    g_dbus_proxy_call(account_proxy, "DeleteUser",
                      g_variant_new("(xb)", user->uid, FALSE),
                      G_DBUS_CALL_FLAGS_NONE, -1, NULL, deleteUserDone, user);
    if (account_proxy)
        g_object_unref(account_proxy);
}

void deleteFiles(GtkWidget *widget, gpointer userdata)
{
    GError *error = NULL;
    GDBusProxy *account_proxy;

    account_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,
                                                  NULL, "org.freedesktop.Accounts",
                                                  "/org/freedesktop/Accounts",
                                                  "org.freedesktop.Accounts", NULL, &error);
    if (error != NULL)
    {
        g_error("Could not connect to org.freedesktop.Accounts:%s\n",error->message);
        if (account_proxy)
            g_object_unref(account_proxy);
        return;
    }

    UserInfo *user = (UserInfo *)userdata;

    g_dbus_proxy_call(account_proxy, "DeleteUser",
                      g_variant_new("(xb)", user->uid, TRUE),
                      G_DBUS_CALL_FLAGS_NONE, -1, NULL, deleteUserDone, user);
    if (account_proxy)
        g_object_unref(account_proxy);
}

void delete_user(GtkWidget *widget, gpointer userdata)
{
    GError *err = NULL;

    UserInfo *user = (UserInfo *)userdata;

    if (user->logined)
    {
        char *primary_text = _("The user can not be deleted!");
        GtkWidget *d = gtk_message_dialog_new (NULL,
                                               GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_CLOSE,
                                               "%s", primary_text);

        gtk_window_set_icon_from_file (GTK_WINDOW(d), "/usr/share/ukui-control-center/icons/用户账号.png", NULL);
        char *secondary_text = _("The user has logged in, please perform the delete operation after logging out!");
        gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (d),
                                                  "%s", secondary_text);
        gtk_widget_set_name(GTK_WIDGET(d), "ukuicc");
        gtk_dialog_run (GTK_DIALOG (d));

        gtk_widget_destroy (d);
        return;
    }

    ui = gtk_builder_new();
    gtk_builder_add_from_file(ui, UIDIR "/delete-user.ui", &err);
    //gtk_builder_add_from_file(ui, "../panels/user-accounts/delete-user.ui", &err);
    if (err)
    {
        g_warning("Could not load user interface file: %s", err->message);
        g_error_free(err);
        g_object_unref(ui);
        return;
    }

    dialog = GTK_DIALOG(gtk_builder_get_object (ui, "deleteuser"));
    gtk_window_set_icon_from_file (GTK_WINDOW(dialog), "/usr/share/ukui-control-center/icons/用户账号.png", NULL);
    GtkWidget *label1 = GTK_WIDGET(gtk_builder_get_object (ui, "label1"));
    gtk_label_set_xalign(GTK_LABEL(label1), 0.0);
    char *markup = g_markup_printf_escaped (_("<span weight='bold' font_desc='11'>do you confirm to delete all the files of %s?</span>"), user->username);
    gtk_label_set_markup(GTK_LABEL(label1), markup);

    GtkWidget *label2 = GTK_WIDGET(gtk_builder_get_object (ui, "label2"));
    gtk_label_set_xalign(GTK_LABEL(label2), 0.0);
    gtk_label_set_yalign(GTK_LABEL(label2), 0.1);
    char *message = g_strdup_printf(_("if you want to delete the %s user, belonging to the user's \ndesktop, documents, favorites, music, pictures and video \nfolder will be deleted!"), user->username);
    gtk_label_set_text(GTK_LABEL(label2), message);

    GtkWidget *dialog_action_area1 = GTK_WIDGET(gtk_builder_get_object (ui, "dialog-action_area1"));
    GdkColor color;
    gdk_color_parse("red", &color);
    gtk_widget_modify_fg(dialog_action_area1, GTK_STATE_NORMAL, &color);

    GtkWidget *buttonstore = GTK_WIDGET(gtk_builder_get_object (ui, "buttonstore"));
    g_signal_connect(buttonstore, "clicked", G_CALLBACK(storeFiles), user);
    GtkWidget *buttondelete = GTK_WIDGET(gtk_builder_get_object (ui, "buttondelete"));
    g_signal_connect(buttondelete, "clicked", G_CALLBACK(deleteFiles), user);

    gtk_widget_set_name(GTK_WIDGET(dialog), "ukuicc");
    gtk_dialog_run(dialog);

    gtk_widget_destroy(GTK_WIDGET(dialog));
    g_object_unref(ui);
}

void init_notebook(UserInfo *userinfo, gint page)
{
    GtkWidget *label1, *label2, *label3, *hbox1, *bt_ch_name, *bt_ch_pwd, *bt_ch_face, *bt_ch_accounttype, *bt_del_user, *sep1, *sep2, *sep3, *sep4;
    char *markup;

    if (page == 0)
    {
        GtkWidget *hbox;
        hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        GtkWidget *image;
        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(userinfo->iconfile, NULL);
        if (!pixbuf)
            pixbuf = gdk_pixbuf_new_from_file("/usr/share/pixmaps/faces/stock_person.png", NULL);
        GdkPixbuf *face = gdk_pixbuf_scale_simple(pixbuf, FACEHEIGHT, FACEWIDTH, GDK_INTERP_BILINEAR);
        image = gtk_image_new_from_pixbuf (face);
        userinfo->image0 = image;
        g_object_unref(pixbuf);
        g_object_unref(face);
        gtk_widget_show(image);
        gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, TRUE, 27);
        GtkWidget *vbox;
        vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_widget_set_size_request(vbox, -1, 90);
        gtk_widget_show(vbox);
        gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
        label1 = gtk_label_new(userinfo->username);
        markup = g_markup_printf_escaped ("<span weight='bold' font_desc='11'>%s</span>", userinfo->username);
        gtk_label_set_markup(GTK_LABEL(label1), markup);
        gtk_label_set_xalign(GTK_LABEL(label1), 0.0);
        hbox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        label2 = gtk_label_new("");
        userinfo->labeltype0 = label2;
        if (userinfo->accounttype == ADMINISTRATOR)
            gtk_label_set_text(GTK_LABEL(label2), _("Administrators"));
        else if (userinfo->accounttype == STANDARDUSER)
            gtk_label_set_text(GTK_LABEL(label2), _("Standard user"));
        else
            gtk_label_set_text(GTK_LABEL(label2), _("Super user"));
        gtk_label_set_xalign(GTK_LABEL(label2), 0.0);
        bt_ch_name = gtk_button_new_with_label(_("Rename"));
        modify_font_color(bt_ch_name, "#074ca6");
        gtk_button_set_relief(GTK_BUTTON(bt_ch_name), GTK_RELIEF_NONE);
        userinfo->labelname0 = label1;
        g_signal_connect (G_OBJECT (bt_ch_name), "clicked", G_CALLBACK (show_change_name_dialog), userinfo);
        sep1 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
        bt_ch_pwd = gtk_button_new_with_label(_("Change PWD"));
        modify_font_color(bt_ch_pwd, "#074ca6");
        gtk_button_set_relief(GTK_BUTTON(bt_ch_pwd), GTK_RELIEF_NONE);
        g_signal_connect (G_OBJECT (bt_ch_pwd), "clicked", G_CALLBACK (show_change_pwd_dialog), userinfo);
        sep2 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
        bt_ch_face = gtk_button_new_with_label(_("Change Face"));
        modify_font_color(bt_ch_face, "#074ca6");
        gtk_button_set_relief(GTK_BUTTON(bt_ch_face), GTK_RELIEF_NONE);
        g_signal_connect (G_OBJECT (bt_ch_face), "clicked", G_CALLBACK (show_change_face_dialog), userinfo);
        sep3 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
        bt_ch_accounttype = gtk_button_new_with_label(_("Change Type"));
        modify_font_color(bt_ch_accounttype, "#074ca6");
        gtk_button_set_relief(GTK_BUTTON(bt_ch_accounttype), GTK_RELIEF_NONE);
        g_signal_connect (G_OBJECT (bt_ch_accounttype), "clicked", G_CALLBACK (show_change_accounttype_dialog), userinfo);
        sep4 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
        bt_del_user = gtk_button_new_with_label(_("Delete"));
        modify_font_color(bt_del_user, "#074ca6");
        gtk_button_set_relief(GTK_BUTTON(bt_del_user), GTK_RELIEF_NONE);
        g_signal_connect (G_OBJECT (bt_del_user), "clicked", G_CALLBACK (delete_user), userinfo);
        label3 = gtk_label_new("");
        if (userinfo->currentuser)
        {
            gtk_label_set_text(GTK_LABEL(label3), _("Logged(Current User)"));
        }
        else if (userinfo->logined && !userinfo->currentuser)
        {
            gtk_label_set_text(GTK_LABEL(label3), _("Logged(Other Users)"));
        }
        else
        {
            gtk_label_set_text(GTK_LABEL(label3), _("Un-login(Other Users)"));
        }
        gtk_label_set_xalign(GTK_LABEL(label3), 0.0);
        gtk_widget_show(label1);
        gtk_widget_show(label2);
        gtk_widget_show(label3);
        gtk_box_pack_start(GTK_BOX(vbox), label1, TRUE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), hbox1, TRUE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(hbox1), label2, TRUE, TRUE, 0);
        //label_space1和label_space2只是为了调整按钮位置的空白label
        GtkWidget *label_space1 = gtk_label_new("");
        gtk_box_pack_start(GTK_BOX(hbox1), label_space1, TRUE, TRUE, 30);
        gtk_box_pack_start(GTK_BOX(hbox1), bt_ch_name, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(hbox1), sep1, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(hbox1), bt_ch_pwd, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(hbox1), sep2, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(hbox1), bt_ch_face, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(hbox1), sep3, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(hbox1), bt_ch_accounttype, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(hbox1), sep4, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(hbox1), bt_del_user, TRUE, TRUE, 0);
        //------------------同上---------------------
        GtkWidget *label_space2 = gtk_label_new("");
        gtk_box_pack_start(GTK_BOX(hbox1), label_space2, TRUE, TRUE, 30);
        gtk_box_pack_start(GTK_BOX(vbox), label3, TRUE, FALSE, 0);
        gtk_container_add(GTK_CONTAINER(userinfo->notebook), hbox);
        //屏蔽掉修改用户名按钮，因为修改了一个假的用户名
        gtk_widget_set_no_show_all(bt_ch_name, TRUE);
        gtk_widget_set_no_show_all(sep1, TRUE);
        gtk_widget_hide(bt_ch_name);
        gtk_widget_hide(sep1);
        //kycc -u时要显示当前用户的所有控件
        gtk_widget_show_all(GTK_WIDGET(userinfo->notebook));
    }
    else if (page == 1)
    {
        GtkWidget *button;
        button = gtk_button_new();
        gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
        GtkWidget *hbox;
        hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_show(hbox);
        gtk_container_add(GTK_CONTAINER(button), hbox);
        GtkWidget *image;
        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(userinfo->iconfile, NULL);
        if (!pixbuf)
            pixbuf = gdk_pixbuf_new_from_file("/usr/share/pixmaps/faces/stock_person.png", NULL);
        GdkPixbuf *face = gdk_pixbuf_scale_simple(pixbuf, FACEHEIGHT, FACEWIDTH, GDK_INTERP_BILINEAR);
        image = gtk_image_new_from_pixbuf (face);
        userinfo->image1 = image;
        g_object_unref(pixbuf);
        g_object_unref(face);
        gtk_widget_show(image);
        gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, TRUE, 20);
        GtkWidget *vbox;
        vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
//        gtk_box_set_spacing(GTK_BOX(vbox), 4);
        gtk_widget_set_size_request(vbox, -1, 90);
        gtk_widget_show(vbox);
        gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 7);
        label1 = gtk_label_new(userinfo->username);
        markup = g_markup_printf_escaped ("<span weight='bold' font_desc='11'>%s</span>", userinfo->username);
        gtk_label_set_markup(GTK_LABEL(label1), markup);
        gtk_label_set_xalign(GTK_LABEL(label1), 0.0);
        userinfo->labelname1 = label1;
        label2 = gtk_label_new("");
        userinfo->labeltype1 = label2;
        if (userinfo->accounttype == ADMINISTRATOR)
            gtk_label_set_text(GTK_LABEL(label2), _("Administrators"));
        else if (userinfo->accounttype == STANDARDUSER)
            gtk_label_set_text(GTK_LABEL(label2), _("Standard user"));
        else
            gtk_label_set_text(GTK_LABEL(label2), _("Super user"));
//        gtk_misc_set_alignment(GTK_MISC(label2), 0, 0.5);
        gtk_label_set_xalign(GTK_LABEL(label2), 0.0);
        label3 = gtk_label_new("");
        if (userinfo->currentuser)
        {
            gtk_label_set_text(GTK_LABEL(label3), _("Logged(Current User)"));
        }
        else if (userinfo->logined && !userinfo->currentuser)
        {
            gtk_label_set_text(GTK_LABEL(label3), _("Logged(Other Users)"));
        }
        else
        {
            gtk_label_set_text(GTK_LABEL(label3), _("Un-login(Other Users)"));
        }
        gtk_label_set_xalign(GTK_LABEL(label3), 0.0);
        gtk_widget_show(label1);
        gtk_widget_show(label2);
        gtk_widget_show(label3);
        gtk_box_pack_start(GTK_BOX(vbox), label1, TRUE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), label2, TRUE, FALSE, 16);
        gtk_box_pack_start(GTK_BOX(vbox), label3, TRUE, FALSE, 0);

        gtk_widget_show_all(button);
        gtk_container_add(GTK_CONTAINER(userinfo->notebook), button);
        g_signal_connect(G_OBJECT(button), "enter", G_CALLBACK(user_bt_clicked), userinfo->notebook);
    }
}


void init_user_button(GtkBox *box, UserInfo *user)
{
    gtk_notebook_set_show_border(user->notebook, FALSE);
    gtk_notebook_set_show_tabs (user->notebook, FALSE);
    gtk_widget_set_size_request(GTK_WIDGET(user->notebook), -1, 90);
    gtk_notebook_set_current_page(user->notebook, 0);

    init_notebook(user, 0);
    init_notebook(user, 1);
    gtk_notebook_set_current_page(user->notebook, 1);

    gtk_box_pack_start(box, GTK_WIDGET(user->notebook), FALSE, FALSE, 0);
}

void init_user_info(const gchar *object_path)
{
    GError *error = NULL;
    GVariant *value = NULL;
    gsize size;

    UserInfo *user = (UserInfo *)malloc(sizeof(UserInfo));

    user->currentuser = FALSE;
    user->logined = FALSE;
    user->autologin = FALSE;

    user->proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,
                                                NULL, "org.freedesktop.Accounts", object_path,
                                                "org.freedesktop.Accounts.User", NULL, &error);

    value = g_dbus_proxy_get_cached_property(user->proxy, "UserName");
    size = g_variant_get_size(value);
    user->username = (char *)g_variant_get_string(value, &size);
    const char *username = g_get_user_name();
    if (g_strcmp0((const char *)user->username, username) == 0)
    {
        user->currentuser = TRUE;
        user->logined = TRUE;
    }
    else
        user->logined = FALSE;

    //OobsUser *obsuser = oobs_user_new(user->username);
    //user->logined = oobs_user_get_active(obsuser);

    value = g_dbus_proxy_get_cached_property(user->proxy, "AccountType");
    user->accounttype = (gint)g_variant_get_int32(value);

    value = g_dbus_proxy_get_cached_property(user->proxy, "IconFile");
    size = g_variant_get_size(value);
    user->iconfile = (char *)g_variant_get_string(value, &size);
    if (g_strrstr(user->iconfile, "stock_person"))
        user->iconfile = "/usr/share/pixmaps/faces/stock_person.png";

    value = g_dbus_proxy_get_cached_property(user->proxy, "PasswordMode");
    user->passwdtype = g_variant_get_int32(value);

    value = g_dbus_proxy_get_cached_property(user->proxy, "Uid");
    user->uid = (gint)g_variant_get_uint64(value);

    value = g_dbus_proxy_get_cached_property(user->proxy, "AutomaticLogin");
    user->autologin = (gint)g_variant_get_boolean(value);

    user->notebook = GTK_NOTEBOOK(gtk_notebook_new());
    if (user->currentuser)
        userlist = g_list_insert(userlist, user, 0);
    else
        userlist = g_list_append(userlist, user);

    g_variant_unref(value);
}

//写死root用户的一些信息
void init_root_info()
{
    UserInfo *user = (UserInfo *)malloc(sizeof(UserInfo));
    user->accounttype = 2;       //代表超级用户
    user->currentuser = TRUE;
    user->autologin = FALSE;
    user->username = g_get_user_name();
    user->iconfile = "/usr/share/pixmaps/faces/stock_person.png";
    //OobsUser *obsuser = oobs_user_new(user->username);
    //user->logined = oobs_user_get_active(obsuser);
    if (g_strcmp0((const char *)user->username, "root") == 0)
        user->logined = TRUE;
    else
        user->logined = FALSE;
    user->uid = 0;
    user->notebook = GTK_NOTEBOOK(gtk_notebook_new());
    userlist = g_list_insert(userlist, user, 0);
}

void update_user_box(GtkWidget *widget, gpointer data)
{
    GList *list;
    GtkWidget *other_hbox = GTK_WIDGET (gtk_builder_get_object (builder, "hbox6"));
    //init_label(GTK_BOX(widget), TRUE);
    if (g_list_length(userlist) == 1)
        gtk_widget_hide(other_hbox);
    else
        gtk_widget_show(other_hbox);
    GtkWidget *current_user_box = GTK_WIDGET (gtk_builder_get_object (builder, "current_user_box"));
    gtk_widget_show_all(current_user_box);
    for(list = userlist; list; list = list->next)
    {
        UserInfo *info = (UserInfo *)list->data;
        if (!info->currentuser)
            init_user_button (GTK_BOX(widget), info);
        else
            init_user_button (GTK_BOX(current_user_box), info);
    }
    gtk_widget_show_all(widget);
}

void get_all_users_in_callback(GObject *object, GAsyncResult *res, gpointer user_data)
{
    GError * error = NULL;
    const gchar **users_name;
    GVariant *result, *users_name_variant;
    gsize size, number;
    gint i;

    result = g_dbus_proxy_call_finish(G_DBUS_PROXY(object), res, &error);
    if (result == NULL)
    {
        g_warning("Callback Result is null");
        return;
    }
    if (error != NULL)
    {
        g_warning("DBUS error:%s", error->message);
        g_error_free(error);
        return;
    }
    size = g_variant_get_size(result);
    users_name_variant = g_variant_get_child_value(result, 0);
    number = g_variant_n_children(users_name_variant);
    users_name = g_variant_get_objv(users_name_variant, &size);
    for(i =0; i< (gint)number; i++)
    {
        init_user_info(users_name[i]);
    }
    //root用户单独处理
    if(!getuid())
        init_root_info();

    GtkWidget *box = GTK_WIDGET (gtk_builder_get_object (builder, "other_users"));
    update_user_box(box, NULL);
}

void dbus_get_users_in_system()
{
    GError * error=NULL;
    GDBusProxy * account_proxy;

    account_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,G_DBUS_PROXY_FLAGS_NONE,
                                                  NULL, "org.freedesktop.Accounts",
                                                  "/org/freedesktop/Accounts",
                                                  "org.freedesktop.Accounts", NULL, &error);
    if (error != NULL)
    {
        g_error("Could not connect to org.freedesktop.Accounts:%s\n",error->message);
        if (account_proxy)
            g_object_unref(account_proxy);
        return;
    }

    g_dbus_proxy_call(account_proxy, "ListCachedUsers",
                      NULL, G_DBUS_CALL_FLAGS_NONE, -1,
                      NULL, get_all_users_in_callback, "ListCachedUsers");
    if (account_proxy)
        g_object_unref(account_proxy);
}

void update_user(GtkWidget *widget, gpointer data)
{
    GtkWidget *label1;
    label1 = gtk_label_new("");
    gtk_widget_set_size_request(label1, -1, 1);
    gtk_label_set_xalign(GTK_LABEL(label1), 0.0);
    gtk_box_pack_start(GTK_BOX(widget), GTK_WIDGET(label1), FALSE, FALSE, 0);

    gtk_widget_show_all(widget);
}

static void
stock_icon_selected (GtkMenuItem   *menuitem, UserInfo *user)
{
    const char *filename;
    filename = g_object_get_data (G_OBJECT (menuitem), "filename");
    user->iconfile = (char *)filename;
    GtkWidget *image = GTK_WIDGET(gtk_builder_get_object (ui, "imageuser"));
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(user->iconfile, NULL);
    GdkPixbuf *face = gdk_pixbuf_scale_simple(pixbuf, 99, 99, GDK_INTERP_BILINEAR);
    gtk_image_set_from_pixbuf(GTK_IMAGE(image), face);
    GtkWidget *label = GTK_WIDGET(gtk_builder_get_object (ui, "labeluser"));
    gtk_widget_hide(label);
    gtk_widget_show(image);
    GtkWidget *button = GTK_WIDGET(gtk_builder_get_object (ui, "user-icon-button"));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), FALSE);
    g_object_unref(pixbuf);
    g_object_unref(face);
}

static void
none_icon_selected (GtkMenuItem   *menuitem, UserInfo *user)
{
    user->iconfile = "";
}


static GtkWidget *
menu_item_for_filename (UserInfo *user, const char *filename){
    GtkWidget *image, *menuitem;
    GFile *file;
    GIcon *icon;

    file = g_file_new_for_path (filename);
    icon = g_file_icon_new (file);
    g_object_unref (file);
    image = gtk_image_new_from_gicon (icon, GTK_ICON_SIZE_DIALOG);
    g_object_unref (icon);

    menuitem = gtk_menu_item_new ();
    gtk_container_add (GTK_CONTAINER (menuitem), image);
    gtk_widget_show_all (menuitem);

    g_object_set_data_full (G_OBJECT (menuitem), "filename",
                            g_strdup (filename), (GDestroyNotify) g_free);
    g_signal_connect (G_OBJECT (menuitem), "activate",
                      G_CALLBACK (stock_icon_selected), user);

    return menuitem;
}

static void
file_chooser_response (GtkDialog     *chooser,
                       gint           response,
                       UserInfo      *user)
{
    gchar *filename;
    GError *error;
    GdkPixbuf *pixbuf;

    if (response != GTK_RESPONSE_ACCEPT) {
        gtk_widget_destroy (GTK_WIDGET (chooser));
        return;
    }

    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));

    user->iconfile = filename;

    GtkWidget *image = GTK_WIDGET(gtk_builder_get_object (ui, "imageuser"));
    error = NULL;
    pixbuf = gdk_pixbuf_new_from_file (user->iconfile, &error);
    if (pixbuf == NULL) {
        g_warning ("Failed to load %s: %s", filename, error->message);
        g_error_free (error);
    }

    GdkPixbuf *face = gdk_pixbuf_scale_simple(pixbuf, 99, 99, GDK_INTERP_BILINEAR);
    gtk_image_set_from_pixbuf(GTK_IMAGE(image), face);
    GtkWidget *label = GTK_WIDGET(gtk_builder_get_object (ui, "labeluser"));
    if (label)
    {
        gtk_widget_hide(label);
        gtk_widget_show(image);
        GtkWidget *button = GTK_WIDGET(gtk_builder_get_object (ui, "user-icon-button"));
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), FALSE);
    }

    gtk_widget_destroy (GTK_WIDGET (chooser));

    //um_photo_dialog_crop (um, pixbuf);
    g_object_unref (pixbuf);
    g_object_unref (face);
}

static void
update_preview (GtkFileChooser               *chooser,
                MateDesktopThumbnailFactory *thumb_factory)
{
    gchar *uri;

    uri = gtk_file_chooser_get_preview_uri (chooser);

    if (uri) {
        GdkPixbuf *pixbuf = NULL;
        const gchar *mime_type = NULL;
        GFile *file;
        GFileInfo *file_info;
        GtkWidget *preview;

        preview = gtk_file_chooser_get_preview_widget (chooser);

        file = g_file_new_for_uri (uri);
        file_info = g_file_query_info (file,
                                       G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
                                       G_FILE_QUERY_INFO_NONE,
                                       NULL, NULL);
        g_object_unref (file);

        if (file_info != NULL) {
            mime_type = g_file_info_get_content_type (file_info);
            g_object_unref (file_info);
        }

        if (mime_type) {
            pixbuf = mate_desktop_thumbnail_factory_generate_thumbnail (thumb_factory,
                                                                        uri,
                                                                        mime_type);
        }

        gtk_dialog_set_response_sensitive (GTK_DIALOG (chooser),
                                           GTK_RESPONSE_ACCEPT,
                                           (pixbuf != NULL));

        if (pixbuf != NULL) {
            gtk_image_set_from_pixbuf (GTK_IMAGE (preview), pixbuf);
            g_object_unref (pixbuf);
        }
        else {
            gtk_image_set_from_icon_name (GTK_IMAGE (preview),
                                          NULL,
                                          GTK_ICON_SIZE_DIALOG);
        }

        g_free (uri);
    }

    gtk_file_chooser_set_preview_widget_active (chooser, TRUE);
}


static void
user_photo_dialog_select_file (UserInfo *user)
{
    GtkWidget *chooser;
    const gchar *folder;
    GtkWidget *preview;
    chooser = gtk_file_chooser_dialog_new (_("Browse more pictures"),
                                           NULL,
                                           GTK_FILE_CHOOSER_ACTION_OPEN,
                                           _("Cancel"), GTK_RESPONSE_CANCEL,
                                           _("Ok"), GTK_RESPONSE_ACCEPT,
                                           NULL);
    gtk_window_set_modal (GTK_WINDOW (chooser), TRUE);

    MateDesktopThumbnailFactory* thumb_factory = mate_desktop_thumbnail_factory_new (MATE_DESKTOP_THUMBNAIL_SIZE_NORMAL);
    preview = gtk_image_new ();
    gtk_widget_set_size_request (preview, 128, -1);
    gtk_file_chooser_set_preview_widget (GTK_FILE_CHOOSER (chooser), preview);
    gtk_file_chooser_set_use_preview_label (GTK_FILE_CHOOSER (chooser), FALSE);
    gtk_widget_show (preview);
    g_signal_connect (chooser, "update-preview",
                      G_CALLBACK (update_preview), thumb_factory);

    folder = g_get_user_special_dir (G_USER_DIRECTORY_PICTURES);
    if (folder)
        gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (chooser),
                                             folder);
    g_signal_connect (chooser, "response",
                      G_CALLBACK (file_chooser_response), user);
    gtk_widget_set_name(GTK_WIDGET(chooser), "ukuicc");
    gtk_window_present (GTK_WINDOW (chooser));
}


static void
file_icon_selected (GtkMenuItem   *menuitem, UserInfo *user)
{
    user_photo_dialog_select_file (user);
}

GtkWidget *
setup_photo_popup (UserInfo *user)
{
    GtkWidget *menu, *menuitem;
    guint x, y;
    const gchar * const * dirs;
    guint i;
    GDir *dir;
    const char *face;
    gboolean none_item_shown;
    gboolean added_faces;

    menu = gtk_menu_new ();

    x = 0;
    y = 0;
    none_item_shown = added_faces = FALSE;

    dirs = g_get_system_data_dirs ();
    for (i = 0; dirs[i] != NULL; i++) {
        char *path;

        path = g_build_filename (dirs[i], "pixmaps", "faces", NULL);
        dir = g_dir_open (path, 0, NULL);
        if (dir == NULL) {
            g_free (path);
            continue;
        }

        while ((face = g_dir_read_name (dir)) != NULL) {
            char *filename;

            added_faces = TRUE;

            filename = g_build_filename (path, face, NULL);
            menuitem = menu_item_for_filename (user, filename);
            g_free (filename);
            if (menuitem == NULL)
                continue;

            gtk_menu_attach (GTK_MENU (menu), GTK_WIDGET (menuitem),
                             x, x + 1, y, y + 1);
            gtk_widget_show (menuitem);

            x++;
            if (x >= ROW_SPAN - 1) {
                y++;
                x = 0;
            }
        }
        g_dir_close (dir);
        g_free (path);

        if (added_faces)
            break;
    }

    if (!added_faces)
        goto skip_faces;

    /*image = gtk_image_new_from_icon_name ("avatar-default", GTK_ICON_SIZE_DIALOG);
        menuitem = gtk_menu_item_new ();
        gtk_container_add (GTK_CONTAINER (menuitem), image);
        gtk_widget_show_all (menuitem);
        gtk_menu_attach (GTK_MENU (menu), GTK_WIDGET (menuitem),
                         x, x + 1, y, y + 1);
        g_signal_connect (G_OBJECT (menuitem), "activate",
                          G_CALLBACK (none_icon_selected), user);*/
    gtk_widget_set_name(GTK_WIDGET(menuitem), "ukuicc");
    gtk_widget_show (menuitem);
    none_item_shown = TRUE;
    y++;

skip_faces:
    if (!none_item_shown) {
        menuitem = gtk_menu_item_new_with_label ("Disable image");
        gtk_menu_attach (GTK_MENU (menu), GTK_WIDGET (menuitem),
                         0, ROW_SPAN - 1, y, y + 1);
        g_signal_connect (G_OBJECT (menuitem), "activate",
                          G_CALLBACK (none_icon_selected), user);
        gtk_widget_show (menuitem);
        y++;
    }

    /* Separator */
    menuitem = gtk_separator_menu_item_new ();
    gtk_menu_attach (GTK_MENU (menu), GTK_WIDGET (menuitem),
                     0, ROW_SPAN - 1, y, y + 1);
    gtk_widget_show (menuitem);

    y++;

    /*#ifdef HAVE_CHEESE
        um->take_photo_menuitem = gtk_menu_item_new_with_label (_("Take a photo…"));
        gtk_menu_attach (GTK_MENU (menu), GTK_WIDGET (um->take_photo_menuitem),
                         0, ROW_SPAN - 1, y, y + 1);
        g_signal_connect (G_OBJECT (um->take_photo_menuitem), "activate",
                          G_CALLBACK (webcam_icon_selected), um);
        gtk_widget_set_sensitive (um->take_photo_menuitem, FALSE);
        gtk_widget_show (um->take_photo_menuitem);

        um->monitor = cheese_camera_device_monitor_new ();
        g_signal_connect (G_OBJECT (um->monitor), "added",
                          G_CALLBACK (device_added), um);
        g_signal_connect (G_OBJECT (um->monitor), "removed",
                          G_CALLBACK (device_removed), um);
        cheese_camera_device_monitor_coldplug (um->monitor);

        y++;
#endif*/ /* HAVE_CHEESE */

    menuitem = gtk_menu_item_new_with_label (_("Browse more pictures..."));
    gtk_menu_attach (GTK_MENU (menu), GTK_WIDGET (menuitem),
                     0, ROW_SPAN - 1, y, y + 1);
    g_signal_connect (G_OBJECT (menuitem), "activate",
                      G_CALLBACK (file_icon_selected), user);
    gtk_widget_set_name(GTK_WIDGET(menuitem), "ukuicc");
    gtk_widget_show (menuitem);

    return menu;
}

void
popup_menu_below_button (GtkMenu   *menu,
                         gint      *x,
                         gint      *y,
                         gboolean  *push_in,
                         GtkWidget *button)
{
    GtkRequisition menu_req;
    GtkTextDirection direction;
    GtkAllocation allocation;

    gtk_widget_get_preferred_size(GTK_WIDGET (menu), NULL, &menu_req);

    direction = gtk_widget_get_direction (button);

    gdk_window_get_origin (gtk_widget_get_window (button), x, y);
    gtk_widget_get_allocation (button, &allocation);
    *x += allocation.x;
    *y += allocation.y + allocation.height;

    if (direction == GTK_TEXT_DIR_LTR)
        *x += MAX (allocation.width - menu_req.width, 0);
    else if (menu_req.width > allocation.width)
        *x -= menu_req.width - allocation.width;

    *push_in = FALSE;
}


static void
popup_icon_menu (GtkToggleButton *button, GtkWidget *menu)
{
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)) && !gtk_widget_get_visible (menu)) {
        gtk_menu_popup (GTK_MENU (menu),
                        NULL, NULL,
                        (GtkMenuPositionFunc) popup_menu_below_button, button,
                        0, gtk_get_current_event_time ());
    } else {
        gtk_menu_popdown (GTK_MENU(menu));
        //	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (user->popup_button), FALSE);
    }
}

static gboolean
on_popup_button_button_pressed (GtkToggleButton *button,
                                GdkEventButton *event,
                                GtkWidget  *menu)
{
    if (event->button == 1) {
        if (!gtk_widget_get_visible (menu)) {
            popup_icon_menu (button, menu);
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
        } else {
            gtk_menu_popdown (GTK_MENU (menu));
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), FALSE);
        }

        return TRUE;
    }

    return FALSE;
}

static void
popup_button_focus_changed (GObject       *button,
                            GParamSpec    *pspec,
                            UserInfo *user)
{
    gtk_widget_queue_draw (gtk_bin_get_child (GTK_BIN (button)));
}

static void
on_photo_popup_unmap (GtkWidget *popup_menu, GtkWidget *button)
{
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), FALSE);
}


static void
autologin(GtkWidget *widget, gpointer userdata)
{
    UserInfo *user = (UserInfo *)userdata;
    if(!user->autologin){
        GList *it = NULL;
        for (it = userlist; it; it = it->next)
        {
            UserInfo *system_user = (UserInfo *)it->data;
            if(system_user->autologin == TRUE)
            {
                //为什么一个clicked信号，会触发两次这个回调？这里过滤掉了第二次的回调。
                if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
                    continue;
                GtkWidget *label;
                GtkWidget *dialog = gtk_dialog_new_with_buttons(
                            _("auto login"),
                            window,
                            GTK_DIALOG_MODAL,
                            _("_Cancel"),
                            GTK_RESPONSE_REJECT,
                            _("_OK"),
                            GTK_RESPONSE_ACCEPT,
                            NULL);
                label = gtk_label_new(_("\tAlready have other users set to automatically log in,\t\n \tclick OK will overwrite the existing settings!"));
                gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), label, TRUE, TRUE, 30);
                gtk_widget_show_all(dialog);
                gint result = gtk_dialog_run(GTK_DIALOG(dialog));
                if(result == GTK_RESPONSE_ACCEPT)
                    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
                else
                    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), FALSE);
                gtk_widget_destroy(dialog);
            }
        }
    }//end-if
    gboolean toggled = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(widget));
    user->autologin = toggled;
}

static void
toggled(GtkWidget *widget, gpointer userdata)
{
    UserInfo *userinfo = (UserInfo *)userdata;
    gboolean toggled = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(widget));
    GtkWidget *w = GTK_WIDGET (gtk_builder_get_object (ui, "radiobutton1"));
    if (toggled && widget == w)
    {
        userinfo->accounttype = STANDARDUSER;
    }
    else
        userinfo->accounttype = ADMINISTRATOR;
}

gboolean update_user_autologin(gpointer user_date)
{
    UserInfo *user = (UserInfo *)user_date;
    g_warning("--xiaoyi-----autologin = %d",user->autologin);
    g_dbus_proxy_call(user->proxy, "SetAutomaticLogin",
                      g_variant_new("(b)", user->autologin),
                      G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);
    return FALSE;
}

static void
createUserDone(GObject *object, GAsyncResult *res, gpointer user_data)
{
    GError * error = NULL;
    GVariant *result, *value;

    result = g_dbus_proxy_call_finish(G_DBUS_PROXY(object), res, &error);
    if (result == NULL)
    {
        g_warning("Callback Result is null");
        return;
    }
    if (error != NULL)
    {
        g_warning("DBUS error:%s", error->message);
        g_error_free(error);
        return;
    }
    UserInfo *user = (UserInfo *)user_data;
    char *path = (char *)g_variant_get_data(result);
    user->proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,
                                                NULL, "org.freedesktop.Accounts", path,
                                                "org.freedesktop.Accounts.User", NULL, &error);
    if (user->iconfile == NULL)
        user->iconfile = "/usr/share/pixmaps/faces/stock_person_nobg.png";

    g_dbus_proxy_call(user->proxy, "SetIconFile",
                      g_variant_new("(s)", user->iconfile),
                      G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);

    char *crypted = make_crypted(user->password);

    g_dbus_proxy_call(user->proxy, "SetPassword",
                      g_variant_new("(ss)", crypted, ""),
                      G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);

    g_dbus_proxy_call(user->proxy, "SetAccountType",
                      g_variant_new("(i)", user->accounttype),
                      G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);

    // g_warning("--xiaoyi-----autologin = %d",user->autologin);
    if(user->autologin == TRUE)  //确保自动登录用户只有最新设置的生效
    {
        GList *it = NULL;
        for (it = userlist; it; it = it->next)
        {
            UserInfo *system_user = (UserInfo *)it->data;
            if(system_user->autologin == TRUE)
            {
                if(0 == strcmp(user->username,system_user->username))
                    continue;
                system_user->autologin = FALSE;
                g_dbus_proxy_call(user->proxy, "SetAutomaticLogin",
                                  g_variant_new("(b)", system_user->autologin),
                                  G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);
            }
        }
    }
    g_timeout_add(1000,(GSourceFunc)update_user_autologin,user);
    value = g_dbus_proxy_get_cached_property(user->proxy, "Uid");
    user->uid = (gint)g_variant_get_uint64(value);

    user->notebook = GTK_NOTEBOOK(gtk_notebook_new());
    if (user->currentuser)
        userlist = g_list_insert(userlist, user, 0);
    else
        userlist = g_list_append(userlist, user);

    GtkWidget *box = GTK_WIDGET (gtk_builder_get_object (builder, "other_users"));
    if (g_list_length(userlist) == 2)
    {
        GtkWidget *other_hbox = GTK_WIDGET (gtk_builder_get_object (builder, "hbox6"));
        gtk_widget_show(other_hbox);
    }
    init_user_button(GTK_BOX(box), user);
    gtk_widget_show_all(box);

    gtk_widget_destroy(GTK_WIDGET(dialog));
    g_object_unref(ui);
}

static void 
createuser(GtkWidget *widget, gpointer userdata)
{
    UserInfo *user = (UserInfo *)userdata;
    user->username = NULL;
    user->password = NULL;

    GtkWidget *entryname = GTK_WIDGET(gtk_builder_get_object (ui, "entryname"));
    GtkWidget *entrypwd = GTK_WIDGET(gtk_builder_get_object (ui, "entrypwd"));

    char *username = (char *)gtk_entry_get_text(GTK_ENTRY(entryname));
    user->username = (char *)malloc(strlen(username)*sizeof(char));
    strcpy(user->username, username);

    char *password = (char *)gtk_entry_get_text(GTK_ENTRY(entrypwd));
    user->password = (char *)malloc(strlen(password)*sizeof(char));
    strcpy(user->password, password);

    gtk_widget_hide(GTK_WIDGET(dialog));

    GError *error = NULL;
    GDBusProxy *account_proxy;

    account_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,G_DBUS_PROXY_FLAGS_NONE,
                                                  NULL, "org.freedesktop.Accounts",
                                                  "/org/freedesktop/Accounts",
                                                  "org.freedesktop.Accounts", NULL, &error);
    if (error != NULL)
    {
        g_error("Could not connect to org.freedesktop.Accounts:%s\n",error->message);
        if (account_proxy)
            g_object_unref(account_proxy);
        return;
    }

    g_dbus_proxy_call(account_proxy, "CreateUser",
                      g_variant_new("(ssi)", user->username, "", user->accounttype),
                      G_DBUS_CALL_FLAGS_NONE, -1, NULL, createUserDone, user);

    if (account_proxy)
        g_object_unref(account_proxy);
}

static void
createCancel(GtkWidget *widget, gpointer userdata)
{
    if (userdata != NULL)
    {
        UserInfo *user = (UserInfo *)userdata;
        /*if (user->username)
        {
            free(user->username);
        }
        if (user->password)
        {
            free(user->password);
        }*/
        free(user);
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));
    g_object_unref(ui);
}

//用于修改密码的时候
gboolean
createuser_pwd_focus_out(GtkWidget *entry, GdkEventFocus *event, gpointer user_data)
{
    char *msg = NULL;
    GtkWidget *entrypwd = GTK_WIDGET(gtk_builder_get_object (ui, "entrypwd"));
    GtkWidget *labelpwd = GTK_WIDGET(gtk_builder_get_object (ui, "labelpwd"));

    const char *user_passwd = gtk_entry_get_text (GTK_ENTRY (entrypwd));
    msg = passwd_check("", user_passwd, g_get_user_name());
    if(msg)
        gtk_label_set_text(GTK_LABEL(labelpwd), msg);
    else
        gtk_label_set_text(GTK_LABEL(labelpwd), "");
}

void show_create_user_dialog(GtkWidget *widget, gpointer data)
{
    GError *err = NULL;
    ui = gtk_builder_new();
    gtk_builder_add_from_file(ui, UIDIR "/user-create.ui", &err);
    if (err)
    {
        g_warning("Could not load user interface file: %s", err->message);
        g_error_free(err);
        g_object_unref(ui);
        return;
    }

    UserInfo *user = (UserInfo *)malloc(sizeof(UserInfo));
    user->currentuser = FALSE;
    user->logined = FALSE;
    user->autologin = FALSE;
    user->iconfile = NULL;

    dialog = GTK_DIALOG(gtk_builder_get_object (ui, "usercreate"));
    gtk_window_set_icon_from_file (GTK_WINDOW(dialog), "/usr/share/ukui-control-center/icons/用户账号.png", NULL);
    GtkWidget *imageuser = GTK_WIDGET(gtk_builder_get_object (ui, "imageuser"));
    gtk_widget_hide(imageuser);

    GtkWidget *button = GTK_WIDGET(gtk_builder_get_object (ui, "user-icon-button"));
    GtkWidget *menu = setup_photo_popup(user);
    g_signal_connect (button, "toggled", G_CALLBACK (popup_icon_menu), menu);
    g_signal_connect (button, "button-press-event",
                      G_CALLBACK (on_popup_button_button_pressed), menu);
    g_signal_connect (button, "notify::is-focus",
                      G_CALLBACK (popup_button_focus_changed), user);
    /*	g_signal_connect_after (button, "draw",
                                G_CALLBACK (popup_button_draw), user);*/

    g_signal_connect (menu, "unmap",
                      G_CALLBACK (on_photo_popup_unmap), button);

    GtkWidget *entryname = GTK_WIDGET(gtk_builder_get_object (ui, "entryname"));
    gtk_entry_set_text(GTK_ENTRY(entryname), _("Please enter the username"));
    GdkColor color;
    gdk_color_parse("#999999", &color);
    gtk_widget_modify_text(entryname, GTK_STATE_NORMAL, &color);
    g_signal_connect(entryname, "focus-in-event", G_CALLBACK(focusIn), NULL);
    g_signal_connect(entryname, "changed", G_CALLBACK(textChanged), user);
    GtkWidget *entrypwd = GTK_WIDGET(gtk_builder_get_object (ui, "entrypwd"));
    gtk_entry_set_text(GTK_ENTRY(entrypwd), _("Please enter the password"));
    gtk_widget_modify_text(entrypwd, GTK_STATE_NORMAL, &color);
    g_signal_connect(entrypwd, "focus-in-event", G_CALLBACK(focusIn), NULL);
    g_signal_connect(entrypwd, "changed", G_CALLBACK(textChanged), user);
    GtkWidget *entryensurepwd = GTK_WIDGET(gtk_builder_get_object (ui, "entryensurepwd"));
    gtk_entry_set_text(GTK_ENTRY(entryensurepwd), _("Please confirm the new password"));
    gtk_widget_modify_text(entryensurepwd, GTK_STATE_NORMAL, &color);
    g_signal_connect(entryensurepwd, "focus-in-event", G_CALLBACK(focusIn), NULL);
    g_signal_connect(entryensurepwd, "changed", G_CALLBACK(textChanged), user);

    GtkWidget *btautologin = GTK_WIDGET(gtk_builder_get_object (ui, "btautologin"));
    g_signal_connect(btautologin, "clicked", G_CALLBACK(autologin), user);

    GtkWidget *bt_normal = GTK_WIDGET(gtk_builder_get_object (ui, "radiobutton1"));
    gtk_toggle_button_set_active((GtkToggleButton *)bt_normal, TRUE);
    user->accounttype = STANDARDUSER;
    g_signal_connect(bt_normal, "toggled", G_CALLBACK(toggled), user);
    GtkWidget *bt_admin = GTK_WIDGET(gtk_builder_get_object (ui, "radiobutton2"));
    g_signal_connect(bt_admin, "toggled", G_CALLBACK(toggled), user);

    GtkWidget *buttoncreate = GTK_WIDGET(gtk_builder_get_object (ui, "buttoncreate"));
    g_signal_connect(buttoncreate, "clicked", G_CALLBACK(createuser), user);

    GtkWidget *buttoncancel = GTK_WIDGET(gtk_builder_get_object (ui, "buttoncancel"));
    g_signal_connect(buttoncancel, "clicked", G_CALLBACK(createCancel), user);
    gtk_widget_grab_focus(buttoncancel);

    gtk_widget_set_sensitive(buttoncreate, FALSE);
    gtk_widget_set_name(GTK_WIDGET(dialog), "ukuicc");
    //	gtk_dialog_run(dialog);
    gtk_widget_show(GTK_WIDGET(dialog));
}

void init_user_accounts()
{
    g_warning("user accounts");
    GtkWidget *widget;
    GtkWidget *other_users;
    GtkWidget *current_user;
    other_users = GTK_WIDGET (gtk_builder_get_object (builder, "other_users"));
    current_user = GTK_WIDGET (gtk_builder_get_object (builder, "current_user_box"));
    g_signal_connect(G_OBJECT(other_users), "realize", G_CALLBACK(update_user), NULL);
    g_signal_connect(G_OBJECT(current_user), "realize", G_CALLBACK(update_user), NULL);
    dbus_get_users_in_system();

    widget = GTK_WIDGET (gtk_builder_get_object (builder, "bt_new"));
    g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(show_create_user_dialog), NULL);
}

void users_data_destory()
{
    GList *it = NULL;
    for (it = userlist; it; it = it->next)
        g_free(it->data);
}
