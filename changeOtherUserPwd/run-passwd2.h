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

#ifndef RUNPASSWD_H
#define RUNPASSWD_H


struct PasswdHandler;

typedef struct PasswdHandler PasswdHandler;

typedef void (*PasswdCallback) (PasswdHandler * passwd_handler, GError * error, const gpointer user_data);

/* Error codes */
typedef enum {
    PASSWD_ERROR_REJECTED,		/* New password is not secure enough */
    PASSWD_ERROR_AUTH_FAILED,	/* Wrong old password, or PAM failure */
    PASSWD_ERROR_REAUTH_FAILED,	/* Password has changed since first authentication */
    PASSWD_ERROR_BACKEND,		/* Backend error */
    PASSWD_ERROR_UNKNOWN		/* General error */
} PasswdError;

PasswdHandler *passwd_init                ();

void           passwd_destroy             (PasswdHandler *passwd_handler);

void           passwd_authenticate        (PasswdHandler *passwd_handler,
                                           const char    *current_password,
                                           PasswdCallback cb,
                                           gpointer       user_data);

gboolean       passwd_change_password     (PasswdHandler *passwd_handler,
                                           const char    *user_name,
                                           const char    *new_password,
                                           PasswdCallback cb,
                                           const gpointer user_data);

#endif // RUNPASSWD_H
