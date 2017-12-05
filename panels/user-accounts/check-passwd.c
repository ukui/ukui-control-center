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
#include "check-passwd.h"

#include <stdlib.h>
#include <glib.h>
#include <gtk/gtk.h>

#include <stdio.h>

#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <limits.h>
#include <pwd.h>

#include <libintl.h>
#include <locale.h>
#include <glib/gi18n.h>

#define  x_strdup(s)  ( (s) ? strdup(s):NULL )

#ifdef MIN
#undef MIN
#endif
#define MIN(_a, _b) (((_a) < (_b)) ? (_a) : (_b))

/* Helper functions */

/*
 * can't be a palindrome - like `R A D A R' or `M A D A M'
 */
static int palindrome(const char *new)
{
    int	i, j;

    i = strlen (new);

    for (j = 0;j < i;j++)
        if (new[i - j - 1] != new[j])
            return 0;

    return 1;
}

/*
 * Calculate how different two strings are in terms of the number of
 * character removals, additions, and changes needed to go from one to
 * the other
 */

static int distdifferent(const char *old, const char *new,
             size_t i, size_t j)
{
    char c, d;

    if ((i == 0) || (strlen(old) < i)) {
    c = 0;
    } else {
    c = old[i - 1];
    }
    if ((j == 0) || (strlen(new) < j)) {
    d = 0;
    } else {
    d = new[j - 1];
    }
    return (c != d);
}

static int distcalculate(int **distances, const char *old, const char *new,
             size_t i, size_t j)
{
    int tmp = 0;

    if (distances[i][j] != -1) {
    return distances[i][j];
    }

    tmp =          distcalculate(distances, old, new, i - 1, j - 1);
    tmp = MIN(tmp, distcalculate(distances, old, new,     i, j - 1));
    tmp = MIN(tmp, distcalculate(distances, old, new, i - 1,     j));
    tmp += distdifferent(old, new, i, j);

    distances[i][j] = tmp;

    return tmp;
}

static int distance(const char *old, const char *new)
{
    int **distances = NULL;
    size_t m, n, i, j, r;

    m = strlen(old);
    n = strlen(new);
    distances = malloc(sizeof(int*) * (m + 1));

    for (i = 0; i <= m; i++) {
    distances[i] = malloc(sizeof(int) * (n + 1));
    for(j = 0; j <= n; j++) {
        distances[i][j] = -1;
    }
    }
    for (i = 0; i <= m; i++) {
    distances[i][0] = i;
    }
    for (j = 0; j <= n; j++) {
    distances[0][j] = j;
    }
    distances[0][0] = 0;

    r = distcalculate(distances, old, new, m, n);

    for (i = 0; i <= m; i++) {
    memset(distances[i], 0, sizeof(int) * (n + 1));
    free(distances[i]);
    }
    free(distances);

    return r;
}

static int similar(struct cracklib_options *opt,
           const char *old, const char *new)
{
    if (distance(old, new) >= opt->diff_ok) {
    return 0;
    }

    if (strlen(new) >= (strlen(old) * 2)) {
    return 0;
    }

    /* passwords are too similar */
    return 1;
}

/*
 * enough classes of charecters
 */

static int minclass (struct cracklib_options *opt,
             const char *new)
{
    int digits = 0;
    int uppers = 0;
    int lowers = 0;
    int others = 0;
    int total_class;
    int i;
    int retval;

//    D(( "called" ));
    for (i = 0; new[i]; i++)
       {
     if (isdigit (new[i]))
             digits = 1;
     else if (isupper (new[i]))
             uppers = 1;
     else if (islower (new[i]))
             lowers = 1;
     else
             others = 1;
       }

    total_class = digits + uppers + lowers + others;

//    D (("total class: %d\tmin class: %d", total_class, opt->min_class));

    if (total_class >= opt->min_class)
        retval = 0;
    else
      retval = 1;

    return retval;
}


/*
 * a nice mix of characters.
 */
static int simple(struct cracklib_options *opt, const char *new)
{
    int	digits = 0;
    int	uppers = 0;
    int	lowers = 0;
    int	others = 0;
    int	size;
    int	i;
    enum { NONE, DIGIT, UCASE, LCASE, OTHER } prevclass = NONE;
    int sameclass = 0;

    for (i = 0;new[i];i++) {
    if (isdigit (new[i])) {
        digits++;
            if (prevclass != DIGIT) {
                prevclass = DIGIT;
                sameclass = 1;
            } else
                sameclass++;
        }
    else if (isupper (new[i])) {
        uppers++;
            if (prevclass != UCASE) {
                prevclass = UCASE;
                sameclass = 1;
            } else
                sameclass++;
        }
    else if (islower (new[i])) {
        lowers++;
            if (prevclass != LCASE) {
                prevclass = LCASE;
                sameclass = 1;
            } else
                sameclass++;
        }
    else {
        others++;
            if (prevclass != OTHER) {
                prevclass = OTHER;
                sameclass = 1;
            } else
                sameclass++;
        }
        if (opt->max_class_repeat > 1 && sameclass > opt->max_class_repeat) {
                return 1;
        }
    }

    /*
     * The scam was this - a password of only one character type
     * must be 8 letters long.  Two types, 7, and so on.
     * This is now changed, the base size and the credits or defaults
     * see the docs on the module for info on these parameters, the
     * defaults cause the effect to be the same as before the change
     */

    if ((opt->dig_credit >= 0) && (digits > opt->dig_credit))       //满足密码复杂度的数字条件
    digits = opt->dig_credit;

    if ((opt->up_credit >= 0) && (uppers > opt->up_credit))         //满足大写字母要求
    uppers = opt->up_credit;

    if ((opt->low_credit >= 0) && (lowers > opt->low_credit))       //满足小写字母要求
    lowers = opt->low_credit;

    if ((opt->oth_credit >= 0) && (others > opt->oth_credit))
    others = opt->oth_credit;

    size = opt->min_length;

    if (opt->dig_credit >= 0)
    size -= digits;
    else if (digits < opt->dig_credit * -1)
    return 1;

    if (opt->up_credit >= 0)
    size -= uppers;
    else if (uppers < opt->up_credit * -1)
    return 1;

    if (opt->low_credit >= 0)
    size -= lowers;
    else if (lowers < opt->low_credit * -1)
    return 1;

    if (opt->oth_credit >= 0)
    size -= others;
    else if (others < opt->oth_credit * -1)
    return 1;

    if (size <= i){
        return 0;
    }
    return 1;
}

static int consecutive(struct cracklib_options *opt, const char *new)
{
    char c;
    int i;
    int same;

    if (opt->max_repeat == 0)
    return 0;

    for (i = 0; new[i]; i++) {
    if (i > 0 && new[i] == c) {
        ++same;
        if (same > opt->max_repeat)
        return 1;
    } else {
        c = new[i];
        same = 1;
    }
    }
    return 0;
}

static int sequence(struct cracklib_options *opt, const char *new)
{
    char c;
    int i;
    int sequp = 1;
    int seqdown = 1;

    if (opt->max_sequence == 0)
    return 0;

    if (new[0] == '\0')
        return 0;

    for (i = 1; new[i]; i++) {
        c = new[i-1];
    if (new[i] == c+1) {
        ++sequp;
        if (sequp > opt->max_sequence)
        return 1;
        seqdown = 1;
    } else if (new[i] == c-1) {
        ++seqdown;
        if (seqdown > opt->max_sequence)
        return 1;
        sequp = 1;
    } else {
        sequp = 1;
            seqdown = 1;
        }
    }
    return 0;
}

static int wordcheck(const char *new, char *word)
{
    char *f, *b;

    if (strstr(new, word) != NULL)
    return 1;

    /* now reverse the word, we can do that in place
       as it is strdup-ed */
    f = word;
    b = word+strlen(word)-1;
    while (f < b) {
    char c;

    c = *f;
    *f = *b;
    *b = c;
    --b;
    ++f;
    }

    if (strstr(new, word) != NULL)
    return 1;
    return 0;
}

static int usercheck(struct cracklib_options *opt, const char *new,
             char *user)
{
//    if (!opt->reject_user)
//        return 0;

    return wordcheck(new, user);
}

static char * str_lower(char *string)
{
    char *cp;

    if (!string)
        return NULL;

    for (cp = string; *cp; cp++)
        *cp = tolower(*cp);
    return string;
}

/*
 * 配置common-password文件时，必须保证各个区块之间是以tab键分隔，
 * 最后一个区块的用户密码复杂度要求之间要以空格分隔
 */
GPtrArray *get_passwd_configuration()
{
    GPtrArray *pam_conf = g_ptr_array_new();
    FILE *pam_file;
    char buffer[1024];

    pam_file = fopen("/etc/pam.d/common-password", "r");
    if (!pam_file) {
        g_warning("Could not open common-passwd.\n");
        return NULL;
    }
    while(fgets(buffer, 1024, pam_file)){
        if(*buffer == '#')
            continue;
        if(strstr(buffer, "pam_cracklib.so") == NULL)
            continue;
        if(strstr(buffer, "\t") == NULL)
        {
            g_warning("请检查common-passwd文件是否配置正确,各个区块应以Tab键分隔");
            continue;
        }
        gchar **tmp = g_strsplit(buffer, "\t", 4);
        gchar ***tmp_tmp = g_strsplit(tmp[3], " ", 10);
        for(int i = 0; i != 10; ++i){
            if(tmp_tmp[i] != NULL)
                g_ptr_array_add (pam_conf, (gpointer)g_strdup(tmp_tmp[i]));
            else
                break;
        }
        g_strfreev(tmp_tmp);
        g_strfreev(tmp);
    }
    fclose(pam_file);
    return pam_conf;
}

//初始化密码复杂度的要求
void init_options(GPtrArray *pam_array, struct cracklib_options *opt)
{
    for (int i=0; i != pam_array->len; ++i) {
        char *tmp = g_ptr_array_index(pam_array,i);
        if(!tmp)
            continue;

        char *ep = NULL;

        /* generic options */

        if (!strncmp(tmp,"retry=",6)) {
            opt->retry_times = strtol(tmp+6,&ep,10);
            if (!ep || (opt->retry_times < 1))
                opt->retry_times = CO_RETRY_TIMES;
        } else if (!strncmp(tmp,"difok=",6)) {
            opt->diff_ok = strtol(tmp+6,&ep,10);
            if (!ep || (opt->diff_ok < 0))
                opt->diff_ok = CO_DIFF_OK;
        } else if (!strncmp(tmp,"difignore=",10)) {
            /* just ignore */
        } else if (!strncmp(tmp,"minlen=",7)) {
            opt->min_length = strtol(tmp+7,&ep,10);
            if (!ep || (opt->min_length < CO_MIN_LENGTH_BASE))
                opt->min_length = CO_MIN_LENGTH_BASE;
        } else if (!strncmp(tmp,"dcredit=",8)) {
            opt->dig_credit = strtol(tmp+8,&ep,10);
            if (!ep)
                opt->dig_credit = 0;
        } else if (!strncmp(tmp,"ucredit=",8)) {
            opt->up_credit = strtol(tmp+8,&ep,10);
            if (!ep)
                opt->up_credit = 0;
        } else if (!strncmp(tmp,"lcredit=",8)) {
            opt->low_credit = strtol(tmp+8,&ep,10);
            if (!ep)
                opt->low_credit = 0;
        } else if (!strncmp(tmp,"ocredit=",8)) {
            opt->oth_credit = strtol(tmp+8,&ep,10);
            if (!ep)
                opt->oth_credit = 0;
        } else if (!strncmp(tmp,"minclass=",9)) {
            opt->min_class = strtol(tmp+9,&ep,10);
            if (!ep)
                opt->min_class = 0;
            if (opt->min_class > 4)
                opt->min_class = 4;
        } else if (!strncmp(tmp,"maxrepeat=",10)) {
            opt->max_repeat = strtol(tmp+10,&ep,10);
            if (!ep)
                opt->max_repeat = 0;
        } else if (!strncmp(tmp,"maxsequence=",12)) {
            opt->max_sequence = strtol(tmp+12,&ep,10);
            if (!ep)
                opt->max_sequence = 0;
        } else if (!strncmp(tmp,"maxclassrepeat=",15)) {
            opt->max_class_repeat = strtol(tmp+15,&ep,10);
            if (!ep)
                opt->max_class_repeat = 0;
        } else if (!strncmp(tmp,"reject_username",15)) {
            opt->reject_user = 1;
        } else if (!strncmp(tmp,"gecoscheck",10)) {
            opt->gecos_check = 1;
        } else if (!strncmp(tmp,"enforce_for_root",16)) {
            opt->enforce_for_root = 1;
        } else if (!strncmp(tmp,"authtok_type",12)) {
            /* for pam_get_authtok, ignore */;
        } else if (!strncmp(tmp,"use_authtok",11)) {
            /* for pam_get_authtok, ignore */;
        } else if (!strncmp(tmp,"use_first_pass",14)) {
            /* for pam_get_authtok, ignore */;
        } else if (!strncmp(tmp,"try_first_pass",14)) {
            /* for pam_get_authtok, ignore */;
        }
    }
//查看是否初始化成功
//    g_warning("\n-------diff_ok =             %d-------\n"
//              "-------dig_credit =          %d-------\n"
//              "-------enforce_for_root =    %d-------\n"
//              "-------gecos_check =         %d-------\n"
//              "-------low_credit =          %d-------\n"
//              "-------max_class_repeat =    %d-------\n"
//              "-------max_repeat =          %d-------\n"
//              "-------max_sequence =        %d-------\n"
//              "-------min_class =           %d-------\n"
//              "-------min_length =          %d-------\n"
//              "-------oth_credit =          %d-------\n"
//              "-------reject_user =         %d-------\n"
//              "-------retry_times =         %d-------\n"
//              "-------up_credit =           %d-------\n",
//              opt->diff_ok,
//              opt->dig_credit,
//              opt->enforce_for_root,
//              opt->gecos_check,
//              opt->low_credit,
//              opt->max_class_repeat,
//              opt->max_repeat,
//              opt->max_sequence,
//              opt->min_class,
//              opt->min_length,
//              opt->oth_credit,
//              opt->reject_user,
//              opt->retry_times,
//              opt->up_credit);

//        else if (!strncmp(tmp,"dictpath=",9)) {
//            opt->cracklib_dictpath = tmp+9;
//            if (!*(opt->cracklib_dictpath)) {
//                opt->cracklib_dictpath = CRACKLIB_DICTS;
//            }
//        }
//        else {
//            pam_syslog(pamh,LOG_ERR,"pam_parse: unknown option; %s",tmp);
//        }
//    }
//    return ctrl;
}

const char *passwd_check(const char *old, const char *new, const char *user)
{
    const char *msg = NULL;
    char *oldmono = NULL, *newmono, *wrapped = NULL;
    char *usermono = NULL;
    struct cracklib_options options;
    memset(&options, 0, sizeof(options));
    options.retry_times = CO_RETRY_TIMES;
    options.diff_ok = CO_DIFF_OK;
    options.min_length = CO_MIN_LENGTH;
    options.dig_credit = CO_DIG_CREDIT;
    options.up_credit = CO_UP_CREDIT;
    options.low_credit = CO_LOW_CREDIT;
    options.oth_credit = CO_OTH_CREDIT;

    //从common-passwd文件中读取用户设置的密码复杂度要求
    GPtrArray *pam_array = get_passwd_configuration();
    init_options(pam_array, &options);
    g_ptr_array_free(pam_array, TRUE);

    struct cracklib_options *opt = &options;

    if (old && strcmp(old, "") != 0 && strcmp(new, old) == 0) {
        msg = _("is the same as the old one");
        return msg;
    }

    newmono = str_lower(x_strdup(new));
    if (!newmono)
        msg = _("memory allocation error");

    usermono = str_lower(x_strdup(user));
    if (!usermono)
        msg = _("memory allocation error");

    if (!msg && old) {
        oldmono = str_lower(x_strdup(old));
        if (oldmono)
            wrapped = malloc(strlen(oldmono) * 2 + 1);
        if (wrapped) {
            strcpy (wrapped, oldmono);
            strcat (wrapped, oldmono);
        } else {
            msg = _("memory allocation error");
        }
    }

    if (!msg && strcmp(old, "") != 0 && strcmp(new, "") != 0 && palindrome(newmono))
        msg = _("is a palindrome");

    if (!msg && strcmp(old, "") != 0 && oldmono && strcmp(oldmono, newmono) == 0)
        msg = _("case changes only");

    if (!msg && strcmp(old, "") != 0 && oldmono && similar(opt, oldmono, newmono))
        msg = _("is too similar to the old one");

    if (!msg && simple(opt, new))
        msg = _("is too simple");

    if (!msg && wrapped && strstr(wrapped, newmono))
        msg = _("is rotated");

    if (!msg && minclass (opt, new))
            msg = _("not enough character classes");

    if (!msg && consecutive(opt, new))
            msg = _("contains too many same characters consecutively");

    if (!msg && sequence(opt, new))
            msg = _("contains too long of a monotonic character sequence");

    if (!msg && (usercheck(opt, newmono, usermono))) //|| gecoscheck(pamh, opt, newmono, user)))
            msg = _("contains the user name in some form");

    free(usermono);
    if (newmono) {
        memset(newmono, 0, strlen(newmono));
        free(newmono);
    }
    if (oldmono) {
      memset(oldmono, 0, strlen(oldmono));
      free(oldmono);
    }
    if (wrapped) {
      memset(wrapped, 0, strlen(wrapped));
      free(wrapped);
    }

    return msg;
}
