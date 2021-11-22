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
#include "run-passwd2.h"

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

    passwd_handler = passwd_init();

    passwd_change_password(passwd_handler, argv[1], argv[2], chpasswd_cb, NULL);

    return a.exec();
}

static void
auth_cb (PasswdHandler *passwd_handler, GError *error, gpointer user_data)
{

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


    if (!error) {

        qApp->exit(0);
    } else {

        passwd_destroy (passwd_handler);

        qApp->exit(1);
    }

}
