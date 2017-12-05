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
#ifndef CHECKPASSWD_H
#define CHECKPASSWD_H

#include <gtk/gtk.h>

#define CO_RETRY_TIMES      1
#define CO_DIFF_OK          5
#define CO_MIN_LENGTH       9
#define CO_MIN_LENGTH_BASE  5
#define CO_DIG_CREDIT       1
#define CO_UP_CREDIT        1
#define CO_LOW_CREDIT       1
#define CO_OTH_CREDIT       1
#define CO_MIN_WORD_LENGTH  4

struct cracklib_options {
    int retry_times;
    int diff_ok;
    int min_length;
    int dig_credit;
    int up_credit;
    int low_credit;
    int oth_credit;
    int min_class;
    int max_repeat;
    int max_sequence;
    int max_class_repeat;
    int reject_user;
    int gecos_check;
    int enforce_for_root;
};

const char *passwd_check(const char *old, const char *new, const char *user);
GPtrArray *get_passwd_configuration();

#endif // CHECKPASSWD_H
