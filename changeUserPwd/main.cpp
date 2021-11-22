/*
 * Copyright 2021 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <QCoreApplication>

#include <glib.h>
#include "run-passwd.h"

PasswdHandler *passwd_handler = NULL;

static void auth_cb (PasswdHandler *passwd_handler, GError *error, gpointer user_data);
static void chpasswd_cb (PasswdHandler *passwd_handler, GError *error, gpointer user_data);

int main(int argc, char *argv[])
{

    if (argc != 3)
    {
        return -1;
    }

    QCoreApplication a(argc, argv);

    passwd_handler = passwd_init ();

    passwd_authenticate (passwd_handler, argv[1], auth_cb, argv[2]);

    return a.exec();
}

static void
auth_cb (PasswdHandler *passwd_handler,
         GError        *error,
         gpointer       user_data)
{
    char *secondary_text;
    char * pwd = (char*) user_data;

    if (error){
        secondary_text = error->message;
        printf("%s\n", secondary_text);
        qApp->exit(1);
    } else {
        passwd_change_password (passwd_handler, pwd, chpasswd_cb, NULL);
    }

}

/**
 * @brief chpasswd_cb
 * @param passwd_handler
 * @param error
 * @param user_data
 */
static void
chpasswd_cb (PasswdHandler *passwd_handler,
             GError        *error,
             gpointer       user_data)
{
//    char *primary_text;
    char *secondary_text;

    if (!error) {
        //finish_password_change (TRUE);
//        primary_text = "Success";
        secondary_text = "";

        printf("%s\n", secondary_text);

        qApp->exit(0);
    } else {
//        primary_text = "Failed";
        secondary_text = error->message;

        char ** lines = g_strsplit(secondary_text, "\n", -1);

        printf("%s\n", lines[0]);

        passwd_destroy (passwd_handler);

        qApp->exit(1);
    }

}
