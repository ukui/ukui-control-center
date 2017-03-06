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
#include <oobs/oobs.h>
#include <sys/wait.h>  
#include <sys/types.h>  
#include <glib/gi18n.h>
#define MATE_DESKTOP_USE_UNSTABLE_API
#include <libmate-desktop/mate-desktop-thumbnail.h>

#include "run-passwd.h"
#include "user-accounts.h"
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
	directory = g_file_new_for_path(FACES_PATH);
	enumer = g_file_enumerate_children(directory, G_FILE_ATTRIBUTE_STANDARD_NAME, G_FILE_QUERY_INFO_NONE, NULL, &error);
	if (error != NULL)
	{
		g_warning("enumer is wrong");
		g_error_free(error);
		exit(0);
	}
	while ((info = g_file_enumerator_next_file(enumer, NULL, NULL)))
	{
		const char *filename = g_file_info_get_name (info);
		char *fullpath = g_build_filename(FACES_PATH, filename, NULL, NULL);
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
		gtk_notebook_set_current_page(notebook, 1);
	}
	gtk_notebook_set_show_border(GTK_NOTEBOOK(userdata), TRUE);
	gtk_container_set_border_width(GTK_CONTAINER(userdata), 1);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(userdata), 0);
	gtk_widget_grab_focus(widget);

//	GdkColor color;
//	GtkStyle *style = gtk_rc_get_style(widget);
    //gtk_style_lookup_color (style, "selected_bg_color", &color);
    //gtk_widget_modify_bg(GTK_WIDGET(userdata), GTK_STATE_NORMAL, &color);
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
/*	UserInfo *user = (UserInfo *)userdata;
	user->username = NULL;
	user->password = NULL;*/
	
	GtkWidget *buttoncreate = GTK_WIDGET(gtk_builder_get_object (ui, "buttoncreate"));
	GtkWidget *entryname = GTK_WIDGET(gtk_builder_get_object (ui, "entryname"));
	GtkWidget *labelname = GTK_WIDGET(gtk_builder_get_object (ui, "labelname"));
	if (widget == entryname)
	{
		const char *name = gtk_entry_get_text(GTK_ENTRY(widget));
		if (strlen(name) < 1)
		{
			gtk_label_set_text(GTK_LABEL(labelname), _("User name cannot be empty!"));
			GdkColor color;
        		gdk_color_parse("red", &color);
        		gtk_widget_modify_fg(labelname, GTK_STATE_NORMAL, &color);
	        	gtk_widget_set_sensitive(buttoncreate, FALSE);
			return FALSE;
		}
		else
			gtk_label_set_text(GTK_LABEL(labelname), "");
	}
	GtkWidget *entrypwd = GTK_WIDGET(gtk_builder_get_object (ui, "entrypwd"));
	if (widget == entrypwd)
	{
		const char *pwd = gtk_entry_get_text(GTK_ENTRY(widget));
		GtkWidget *labelpwd = GTK_WIDGET(gtk_builder_get_object (ui, "labelpwd"));
		if (strlen(pwd) < 6)
		{
			gtk_label_set_text(GTK_LABEL(labelpwd), _("Password length needs to more than 5 digits!"));
			GdkColor color;
			gdk_color_parse("red", &color);
			gtk_widget_modify_fg(labelpwd, GTK_STATE_NORMAL, &color);
			gtk_widget_set_sensitive(buttoncreate, FALSE);
			return FALSE;
		}
		else
			gtk_label_set_text(GTK_LABEL(labelpwd), " ");

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
			GdkColor color;
			gdk_color_parse("red", &color);
			gtk_widget_modify_fg(labelensurepwd, GTK_STATE_NORMAL, &color);
			gtk_widget_set_sensitive(buttoncreate, FALSE);
			return FALSE;
		}
		else
			gtk_label_set_text(GTK_LABEL(labelensurepwd), " ");
		
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
	/*user->username = (char *)malloc(strlen(username)*sizeof(char));
	strcpy(user->username, username);

	user->password = (char *)malloc(strlen(password)*sizeof(char));
	strcpy(user->password, password);*/
	char *password = (char *)gtk_entry_get_text(GTK_ENTRY(entrypwd));
	char *ensurepassword = (char *)gtk_entry_get_text(GTK_ENTRY(entryensurepwd));

	if (password && username && (strcmp(password, ensurepassword) == 0) 
	    && (strlen(password) > 5) && (strlen(username) >= 1 
	    && (strcmp(username, _("Please enter the username")) != 0) 
	    && strcmp(password, _("Please enter the password ")) != 0))
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
                       	GdkColor color;
                       	gdk_color_parse("red", &color);
                       	gtk_widget_modify_fg(labelname, GTK_STATE_NORMAL, &color);
			gtk_widget_set_sensitive(buttoncreate, FALSE);
                       	return FALSE;
               	}
		else
			gtk_label_set_text(GTK_LABEL(labelname), " ");
        }

	if (strlen(username) > 0 && strlen(username) < 33)
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
				GdkColor color;
       				gdk_color_parse("red", &color);
       				gtk_widget_modify_fg(labelname, GTK_STATE_NORMAL, &color);
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
                gtk_label_set_text(GTK_LABEL(labelname),_("username length should not long than 32!"));
                GdkColor color;
                gdk_color_parse("red", &color);
                gtk_widget_modify_fg(labelname, GTK_STATE_NORMAL, &color);
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
	const char *pwd1, *pwd2, *pwd3;	
	
	GtkWidget *entry1 = GTK_WIDGET(gtk_builder_get_object (ui, "entry1"));
	GtkWidget *entry2 = GTK_WIDGET(gtk_builder_get_object (ui, "entry2"));
	GtkWidget *entry3 = GTK_WIDGET(gtk_builder_get_object (ui, "entry3"));
	pwd1 = gtk_entry_get_text (GTK_ENTRY(entry1));
	pwd2 = gtk_entry_get_text (GTK_ENTRY(entry2));
	pwd3 = gtk_entry_get_text (GTK_ENTRY(entry3));
	GtkWidget *buttonok = GTK_WIDGET(gtk_builder_get_object (ui, "buttonok"));
	gboolean visible = gtk_widget_get_visible(entry1);
	if (visible)
	{
		if(strlen(pwd1) > 0 && strlen(pwd2) > 0 && strlen(pwd3) > 0 && 
           	strcmp(pwd1, _("Please enter the current password")) != 0 && strcmp(pwd2, _("Please enter the new password")) != 0 &&
           	strcmp(pwd3, _("Please confirm the new password")) != 0)
			gtk_widget_set_sensitive(buttonok, TRUE);
		else
			gtk_widget_set_sensitive(buttonok, FALSE);
	}
	else
	{
        if(strlen(pwd2) > 0 && strlen(pwd3) > 0 && strcmp(pwd2, _("Please enter the new password")) != 0 && strcmp(pwd3, _("Please confirm the new password")) != 0)
                        gtk_widget_set_sensitive(buttonok, TRUE);
                else
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

static const char*
check_password ()
{
        GtkWidget *dialog;
        GtkWidget *widget;
        const gchar *password, *confirmation;
        char *primary_text = NULL;
        char *secondary_text;
        int len;

        widget = GTK_WIDGET(gtk_builder_get_object (ui, "entry2"));
	password = gtk_entry_get_text (GTK_ENTRY (widget));
        widget = GTK_WIDGET(gtk_builder_get_object (ui, "entry3"));
	confirmation = gtk_entry_get_text (GTK_ENTRY (widget));

        len = strlen (password);

        /* empty password, accept but don't change it */
        if (len == 0) {
                return password;
        }
        else if (len < 6) {
                primary_text = _("Password length is too short!");
                secondary_text = _("Password length needs to more than 5 digits, and composed of letters, \n numbers or special characters.");
        } else if (strcmp (password, confirmation) != 0) {
                primary_text = _("Password error");
                secondary_text = _("Please make sure you enter the password two times.");
        }

        if (primary_text) {
		GtkWidget *wid = GTK_WIDGET(gtk_builder_get_object(ui, "changepwd"));
                dialog = gtk_message_dialog_new (GTK_WINDOW (wid),
                                                 GTK_DIALOG_MODAL,
                                                 GTK_MESSAGE_ERROR,
                                                 GTK_BUTTONS_CLOSE,
                                                 "%s", primary_text);

                gtk_message_dialog_format_secondary_markup (GTK_MESSAGE_DIALOG (dialog),
                                                            "%s", secondary_text);

		gtk_widget_set_name(GTK_WIDGET(dialog), "ukuicc");
                gtk_dialog_run (GTK_DIALOG (dialog));
                gtk_widget_destroy (dialog);
                return NULL;
        }

        return password;
}

void change_pwd(GtkWidget *widget, gpointer userdata)
{
        GtkWidget *user_passwd_dialog;
	PasswdHandler *passwd_handler = NULL;
	const char *passwd;

	UserInfo *user = (UserInfo *)userdata;

        user_passwd_dialog = GTK_WIDGET(gtk_builder_get_object (ui, "changepwd"));
	GtkWidget *user_passwd_current = GTK_WIDGET(gtk_builder_get_object (ui, "entry1"));
	
	passwd_handler = g_object_get_data (G_OBJECT (user_passwd_current), "passwd_handler");

	passwd = check_password();
	if(!passwd)
		return;

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

                gdk_cursor_unref (cursor);
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
	if(entry == entry1)
	{
        	password = gtk_entry_get_text (GTK_ENTRY (entry));

        	if (strlen (password) > 0) {
                	passwd_handler = g_object_get_data (G_OBJECT (entry), "passwd_handler");
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
        //gtk_builder_add_from_file(ui, "../panels/user-accounts/change-pwd.ui", &err);
        if (err)
        {
                g_warning("Could not load user interface file: %s", err->message);
                g_error_free(err);
                g_object_unref(ui);
                return;
        }

	dialog = GTK_DIALOG(gtk_builder_get_object (ui, "changepwd"));
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
	gtk_label_set_text(GTK_LABEL(labelname), user->username); 
	char *markup = g_markup_printf_escaped ("<span weight='bold' font_desc='11'>%s</span>", user->username);
	gtk_label_set_markup(GTK_LABEL(labelname), markup);

	GtkWidget *labeltype = GTK_WIDGET(gtk_builder_get_object (ui, "labeltype"));
	if (user->accounttype == ADMINISTRATOR)
        gtk_label_set_text(GTK_LABEL(labeltype), _("Administrators"));
	else if (user->accounttype == STANDARDUSER)
        gtk_label_set_text(GTK_LABEL(labeltype), _("Standard user"));
 
	GtkWidget *label3 = GTK_WIDGET(gtk_builder_get_object (ui, "label3"));
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
	GtkWidget *buttonok = GTK_WIDGET(gtk_builder_get_object (ui, "buttonok"));

	if (strlen(username) > 0)
	{
                GdkColor color;
                gdk_color_parse("red", &color);
                gtk_widget_modify_fg(label1, GTK_STATE_NORMAL, &color);
		int i;
	        for (i = 0; *(username + i) != 0; i++)
		{
			if (*(username) == 95)
			{
                gtk_label_set_text(GTK_LABEL(label1), _("The first character cannot be underlined!"));
	        		gtk_widget_set_sensitive(buttonok, FALSE);
				return FALSE;
			}
			if (isupper(*(username + i)))
			{
                gtk_label_set_text(GTK_LABEL(label1), _("User name can not contain capital letters!"));
	        		gtk_widget_set_sensitive(buttonok, FALSE);
				return FALSE;
			}
			if ((*(username + i) >=48 && *(username + i) <= 57 ) || (*(username + i) >= 97 && *(username + i) <= 122 ) || ( *(username + i) == 95))
				continue;
			else
			{
                gtk_label_set_text(GTK_LABEL(label1), _("The user name can only be composed of letters, numbers and underline!"));
	        		gtk_widget_set_sensitive(buttonok, FALSE);
				return FALSE;
			}
		}
        	if (isdigit(*username))
        	{
                    gtk_label_set_text(GTK_LABEL(label1), _("User name cannot start with number!"));
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
        gtk_label_set_text(GTK_LABEL(labelname), user->username);
        char *markup = g_markup_printf_escaped ("<span weight='bold' font_desc='11'>%s</span>", user->username);
        gtk_label_set_markup(GTK_LABEL(labelname), markup);
	GtkWidget *labeltype = GTK_WIDGET(gtk_builder_get_object (ui, "labeltype"));
        if (user->accounttype == ADMINISTRATOR)
                gtk_label_set_text(GTK_LABEL(labeltype), _("Administrators"));
        else if (user->accounttype == STANDARDUSER)
                gtk_label_set_text(GTK_LABEL(labeltype), _("Standard user"));

        GtkWidget *label3 = GTK_WIDGET(gtk_builder_get_object (ui, "label3"));
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
	GdkPixbuf *buf = gdk_pixbuf_new_from_file(user->iconfile, NULL);
	buf = gdk_pixbuf_scale_simple(buf, FACEHEIGHT, FACEWIDTH, GDK_INTERP_BILINEAR); 
	gtk_image_set_from_pixbuf(GTK_IMAGE(user->image0), buf);
	gtk_image_set_from_pixbuf(GTK_IMAGE(user->image1), buf);
	
	g_object_unref(buf);

	system("gsettings set org.ubuntu-mate.matemenu.plugins.ukuimenu ifchange true");
}

void change_face(GtkWidget *widget, gpointer userdata)
{
	UserInfo *user = (UserInfo *)userdata;

	GtkWidget *image = GTK_WIDGET(gtk_builder_get_object (ui, "imageuser"));
	GdkPixbuf *pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(image));
	const char *filename = g_object_get_data (G_OBJECT (pixbuf), "filename");

	if (filename)
		user->iconfile = (gchar *)filename;
	
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
        GtkWidget *image = GTK_WIDGET(gtk_builder_get_object (ui, "imageuser"));
        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(user->iconfile, NULL);
        if (!pixbuf)
                pixbuf = gdk_pixbuf_new_from_file("/usr/share/pixmaps/faces/stock_person.png", NULL);
        GdkPixbuf *face = gdk_pixbuf_scale_simple(pixbuf, 88, 88, GDK_INTERP_BILINEAR);
        gtk_image_set_from_pixbuf (GTK_IMAGE(image), face);
        g_object_unref(pixbuf);
        g_object_unref(face);

        GtkWidget *labelname = GTK_WIDGET(gtk_builder_get_object (ui, "labelname"));
        gtk_label_set_text(GTK_LABEL(labelname), user->username);
        char *markup = g_markup_printf_escaped ("<span weight='bold' font_desc='11'>%s</span>", user->username);
        gtk_label_set_markup(GTK_LABEL(labelname), markup);
	GtkWidget *labeltype = GTK_WIDGET(gtk_builder_get_object (ui, "labeltype"));
        if (user->accounttype == ADMINISTRATOR)
                gtk_label_set_text(GTK_LABEL(labeltype), _("Administrators"));
        else if (user->accounttype == STANDARDUSER)
                gtk_label_set_text(GTK_LABEL(labeltype), _("Standard user"));

        GtkWidget *label3 = GTK_WIDGET(gtk_builder_get_object (ui, "label3"));
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

void show_change_accounttype_dialog(GtkButton *button, gpointer user_data)
{
	GError *err = NULL;

        UserInfo *user = (UserInfo *)user_data;
        ui = gtk_builder_new();
        gtk_builder_add_from_file(ui, UIDIR "/change-type.ui", &err);
        //gtk_builder_add_from_file(ui, "../panels/user-accounts/change-type.ui", &err);
        if (err)
        {
                g_warning("Could not load user interface file: %s", err->message);
                g_error_free(err);
                g_object_unref(ui);
                return;
        }

        dialog = GTK_DIALOG(gtk_builder_get_object (ui, "changetype"));
        GtkWidget *image = GTK_WIDGET(gtk_builder_get_object (ui, "image1"));
        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(user->iconfile, NULL);
        if (!pixbuf)
                pixbuf = gdk_pixbuf_new_from_file("/usr/share/pixmaps/faces/stock_person.png", NULL);
        GdkPixbuf *face = gdk_pixbuf_scale_simple(pixbuf, 88, 88, GDK_INTERP_BILINEAR);
        gtk_image_set_from_pixbuf (GTK_IMAGE(image), face);
        g_object_unref(pixbuf);
        g_object_unref(face);

        GtkWidget *labelname = GTK_WIDGET(gtk_builder_get_object (ui, "labelname"));
        gtk_label_set_text(GTK_LABEL(labelname), user->username);
        char *markup = g_markup_printf_escaped ("<span weight='bold' font_desc='11'>%s</span>", user->username);
        gtk_label_set_markup(GTK_LABEL(labelname), markup);
        GtkWidget *stard = GTK_WIDGET (gtk_builder_get_object (ui, "radiobutton1"));
        GtkWidget *admin = GTK_WIDGET (gtk_builder_get_object (ui, "radiobutton2"));
	GtkWidget *labeltype = GTK_WIDGET(gtk_builder_get_object (ui, "labeltype"));
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

        GtkWidget *label3 = GTK_WIDGET(gtk_builder_get_object (ui, "label3"));
        if (user->currentuser)
                gtk_label_set_text(GTK_LABEL(label3), _("Logged(Current User)"));
        else if (user->logined && !user->currentuser)
                gtk_label_set_text(GTK_LABEL(label3), _("Logged(Other Users)"));
        else
                gtk_label_set_text(GTK_LABEL(label3), _("Un-login(Other Users)"));

	GtkWidget *btautologin = GTK_WIDGET(gtk_builder_get_object (ui, "btautologin"));
	if (user->autologin)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(btautologin), user->autologin);

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
	GtkWidget *box = GTK_WIDGET (gtk_builder_get_object (builder, "vbox_users"));
	GtkWidget *label = g_object_get_data (G_OBJECT (box), "label");
	gtk_container_remove(GTK_CONTAINER(box), GTK_WIDGET(user->notebook));
        gtk_widget_show_all(box);
	userlist = g_list_remove(userlist, user);
	if (g_list_length(userlist) == 1)
		gtk_container_remove(GTK_CONTAINER(box), label);
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

	GtkWidget *label1 = GTK_WIDGET(gtk_builder_get_object (ui, "label1"));
    char *markup = g_markup_printf_escaped (_("<span weight='bold' font_desc='11'>do you confirm to delete all the files of %s?</span>"), user->username);
	gtk_label_set_markup(GTK_LABEL(label1), markup);

	GtkWidget *label2 = GTK_WIDGET(gtk_builder_get_object (ui, "label2"));
    char *message = g_strdup_printf(_("if you want to delete the %s user, belonging to the user's desktop, documents, favorites, music, pictures and video folder will be deleted!"), user->username);
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
        	hbox = gtk_hbox_new(FALSE, 0);
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
        	gtk_box_pack_start(GTK_BOX(hbox), image, TRUE, TRUE, 0);
        	GtkWidget *vbox;
        	vbox = gtk_vbox_new(TRUE, 0);
        	gtk_widget_set_size_request(vbox, 480, -1);
        	gtk_widget_show(vbox);
        	gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
        	label1 = gtk_label_new(userinfo->username);
  		markup = g_markup_printf_escaped ("<span weight='bold' font_desc='11'>%s</span>", userinfo->username);
		gtk_label_set_markup(GTK_LABEL(label1), markup);
        	gtk_misc_set_alignment(GTK_MISC(label1), 0, 0.5);
        	hbox1 = gtk_hbox_new(FALSE, 0);
		label2 = gtk_label_new("");
		userinfo->labeltype0 = label2;
		if (userinfo->accounttype == ADMINISTRATOR)
            gtk_label_set_text(GTK_LABEL(label2), _("Administrators"));
		else if (userinfo->accounttype == STANDARDUSER)
            gtk_label_set_text(GTK_LABEL(label2), _("Standard user"));
        gtk_widget_set_size_request(label2, 98, -1);
        	gtk_misc_set_alignment(GTK_MISC(label2), 0, 0.5);
            bt_ch_name = gtk_button_new_with_label(_("Rename"));
		modify_font_color(bt_ch_name, "blue");
		gtk_button_set_relief(GTK_BUTTON(bt_ch_name), GTK_RELIEF_NONE);
		userinfo->labelname0 = label1;
		g_signal_connect (G_OBJECT (bt_ch_name), "clicked", G_CALLBACK (show_change_name_dialog), userinfo);
        	sep1 = gtk_vseparator_new();
            bt_ch_pwd = gtk_button_new_with_label(_("Change PWD"));
		modify_font_color(bt_ch_pwd, "blue");
		gtk_button_set_relief(GTK_BUTTON(bt_ch_pwd), GTK_RELIEF_NONE);
		g_signal_connect (G_OBJECT (bt_ch_pwd), "clicked", G_CALLBACK (show_change_pwd_dialog), userinfo);
        	sep2 = gtk_vseparator_new();
            bt_ch_face = gtk_button_new_with_label(_("Change Face"));
		modify_font_color(bt_ch_face, "blue");
		gtk_button_set_relief(GTK_BUTTON(bt_ch_face), GTK_RELIEF_NONE);
		g_signal_connect (G_OBJECT (bt_ch_face), "clicked", G_CALLBACK (show_change_face_dialog), userinfo);
        	sep3 = gtk_vseparator_new();
            bt_ch_accounttype = gtk_button_new_with_label(_("Change Type"));
		modify_font_color(bt_ch_accounttype, "blue");
		gtk_button_set_relief(GTK_BUTTON(bt_ch_accounttype), GTK_RELIEF_NONE);
		g_signal_connect (G_OBJECT (bt_ch_accounttype), "clicked", G_CALLBACK (show_change_accounttype_dialog), userinfo);
        	sep4 = gtk_vseparator_new();
        bt_del_user = gtk_button_new_with_label(_("Delete"));
		modify_font_color(bt_del_user, "blue");
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
        	gtk_misc_set_alignment(GTK_MISC(label3), 0, 0.5);
        	gtk_widget_show(label1);
        	gtk_widget_show(label2);
        	gtk_widget_show(label3);
		gtk_box_pack_start(GTK_BOX(vbox), label1, TRUE, TRUE, 0);
        	gtk_box_pack_start(GTK_BOX(vbox), hbox1, TRUE, TRUE, 0);
        	gtk_box_pack_start(GTK_BOX(hbox1), label2, TRUE, TRUE, 0);
      		gtk_box_pack_start(GTK_BOX(hbox1), bt_ch_name, TRUE, TRUE, 0);
        	gtk_box_pack_start(GTK_BOX(hbox1), sep1, TRUE, TRUE, 0);
        	gtk_box_pack_start(GTK_BOX(hbox1), bt_ch_pwd, TRUE, TRUE, 0);
        	gtk_box_pack_start(GTK_BOX(hbox1), sep2, TRUE, TRUE, 0);
        	gtk_box_pack_start(GTK_BOX(hbox1), bt_ch_face, TRUE, TRUE, 0);
        	gtk_box_pack_start(GTK_BOX(hbox1), sep3, TRUE, TRUE, 0);
        	gtk_box_pack_start(GTK_BOX(hbox1), bt_ch_accounttype, TRUE, TRUE, 0);
        	gtk_box_pack_start(GTK_BOX(hbox1), sep4, TRUE, TRUE, 0);
        	gtk_box_pack_start(GTK_BOX(hbox1), bt_del_user, TRUE, TRUE, 0);
        	gtk_box_pack_start(GTK_BOX(vbox), label3, TRUE, TRUE, 0);
        	gtk_container_add(GTK_CONTAINER(userinfo->notebook), hbox);
	}
	else if (page == 1)
	{
		GtkWidget *button;
		button = gtk_button_new();
		gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
		gtk_widget_set_size_request(button, -1, 90);
		GtkWidget *hbox;
		hbox = gtk_hbox_new(FALSE, 0);
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
		gtk_box_pack_start(GTK_BOX(hbox), image, TRUE, TRUE, 0);
		GtkWidget *vbox;
		vbox = gtk_vbox_new(TRUE, 0);
		gtk_widget_set_size_request(vbox, 480, -1);
		gtk_widget_show(vbox);
		gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
		label1 = gtk_label_new(userinfo->username);
  		markup = g_markup_printf_escaped ("<span weight='bold' font_desc='11'>%s</span>", userinfo->username);
		gtk_label_set_markup(GTK_LABEL(label1), markup);
		gtk_misc_set_alignment(GTK_MISC(label1), 0, 0.5);
		userinfo->labelname1 = label1;
		label2 = gtk_label_new("");
		userinfo->labeltype1 = label2;
		if (userinfo->accounttype == ADMINISTRATOR)
            gtk_label_set_text(GTK_LABEL(label2), _("Administrators"));
		else if (userinfo->accounttype == STANDARDUSER)
            gtk_label_set_text(GTK_LABEL(label2), _("Standard user"));
		gtk_misc_set_alignment(GTK_MISC(label2), 0, 0.5);
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
		gtk_misc_set_alignment(GTK_MISC(label3), 0, 0.5);
		gtk_widget_show(label1);
		gtk_widget_show(label2);
		gtk_widget_show(label3);
		gtk_box_pack_start(GTK_BOX(vbox), label1, TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(vbox), label2, TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(vbox), label3, TRUE, TRUE, 0);

		gtk_widget_show_all(button);
		gtk_container_add(GTK_CONTAINER(userinfo->notebook), button);
//		g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(user_bt_clicked), userinfo->notebook);
		g_signal_connect(G_OBJECT(button), "enter", G_CALLBACK(user_bt_clicked), userinfo->notebook);
	}
}

void init_label(GtkBox *box, gboolean current)
{
	GtkWidget *label1, *label2;
    label1 = gtk_label_new(_("My Account"));
	gtk_widget_set_size_request(label1, -1, 28);
	gtk_misc_set_alignment(GTK_MISC(label1), 0.01, 0.5);
	if (g_list_length(userlist) > 1)
	{
        label2 = gtk_label_new(_("Other accounts"));
		gtk_widget_set_size_request(label2, -1, 28);
		gtk_misc_set_alignment(GTK_MISC(label2), 0.01, 0.5);
		g_object_set_data (G_OBJECT (box), "label", label2);

		if (current)
			gtk_box_pack_start(box, GTK_WIDGET(label1), FALSE, FALSE, 0);
		else
			gtk_box_pack_start(box, GTK_WIDGET(label2), FALSE, FALSE, 0);
	}
	else if (g_list_length(userlist) == 1)
	{
		if (current)
			gtk_box_pack_start(box, GTK_WIDGET(label1), FALSE, FALSE, 0);
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
	}

	OobsUser *obsuser = oobs_user_new(user->username);
	user->logined = oobs_user_get_active(obsuser);

	value = g_dbus_proxy_get_cached_property(user->proxy, "AccountType");
	user->accounttype = (gint)g_variant_get_int32(value);

	value = g_dbus_proxy_get_cached_property(user->proxy, "IconFile");
	size = g_variant_get_size(value);
	user->iconfile = (char *)g_variant_get_string(value, &size);

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

	GtkWidget *box = GTK_WIDGET (gtk_builder_get_object (builder, "vbox_users"));
	update_user_box(GTK_BOX(box), NULL);
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

void update_user_box(GtkWidget *widget, gpointer data)
{
	GList *list;
	init_label(GTK_BOX(widget), TRUE);
	for(list = userlist; list; list = list->next)
	{
		UserInfo *info = (UserInfo *)list->data;
		init_user_button(GTK_BOX(widget), info);
		if (info->currentuser)
			init_label(GTK_BOX(widget), FALSE);
	}
	gtk_widget_show_all(widget);
}

void update_user(GtkWidget *widget, gpointer data)
{
	GtkWidget *label1;
    label1 = gtk_label_new("");
    gtk_widget_set_size_request(label1, -1, 1);
    gtk_misc_set_alignment(GTK_MISC(label1), 0.01, 0.5);
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
menu_item_for_filename (UserInfo *user, const char *filename)
{
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
        GtkWidget *menu, *menuitem, *image;
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

        gtk_widget_get_child_requisition (GTK_WIDGET (menu), &menu_req);

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
	UserInfo *userinfo = (UserInfo *)userdata;
	gboolean toggled = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(widget));
	userinfo->autologin = toggled;
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
		user->iconfile = "/usr/share/pixmaps/faces/stock_person.png"; 

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

	g_dbus_proxy_call(user->proxy, "SetAutomaticLogin",
                          g_variant_new("(b)", user->autologin),
                          G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);

	value = g_dbus_proxy_get_cached_property(user->proxy, "Uid");
        user->uid = (gint)g_variant_get_uint64(value);
        
	user->notebook = GTK_NOTEBOOK(gtk_notebook_new());
        if (user->currentuser)
                userlist = g_list_insert(userlist, user, 0);
        else
                userlist = g_list_append(userlist, user);

	GtkWidget *box = GTK_WIDGET (gtk_builder_get_object (builder, "vbox_users"));
	if (g_list_length(userlist) == 2)
	{
        GtkWidget *label2 = gtk_label_new(_("Other accounts"));
                gtk_widget_set_size_request(label2, -1, 28);
                gtk_misc_set_alignment(GTK_MISC(label2), 0.01, 0.5);

                gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(label2), FALSE, FALSE, 0);
		g_object_set_data (G_OBJECT (box), "label", label2);
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

void show_create_user_dialog(GtkWidget *widget, gpointer data)
{
	GError *err = NULL;
	ui = gtk_builder_new();
	gtk_builder_add_from_file(ui, UIDIR "/user-create.ui", &err);
	//gtk_builder_add_from_file(ui, "../panels/user-accounts/user-create.ui", &err);
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
	g_signal_connect(btautologin, "toggled", G_CALLBACK(autologin), user);

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
	gtk_widget_show(dialog);
}

void init_user_accounts()
{
	g_warning("user accounts");
	GtkWidget *widget;
	widget = GTK_WIDGET (gtk_builder_get_object (builder, "vbox_users"));
	g_signal_connect(G_OBJECT(widget), "realize", G_CALLBACK(update_user), NULL);
	dbus_get_users_in_system();

	widget = GTK_WIDGET (gtk_builder_get_object (builder, "bt_new"));
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(show_create_user_dialog), NULL);
}
