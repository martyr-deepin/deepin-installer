/**
 * Copyright (c) 2011 ~ 2013 Deepin, Inc.
 *               2011 ~ 2013 Long Wei
 *
 * Author:      Long Wei <yilang2007lw@gmail.com>
 * Maintainer:  Long Wei <yilang2007lw@gamil.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 **/

#include "misc.h"
#include "part_util.h"
#include "fs_util.h"
#include "info.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/types.h>
#include <parted/parted.h>
#include <unistd.h>

#define LOG_FILE_PATH           "/tmp/installer.log"


gboolean enter_chroot()
{
    gboolean ret = FALSE;

    extern int chroot_fd;
    if ((chroot_fd = open (".", O_RDONLY)) < 0) {
        g_warning ("chroot:set chroot fd failed\n");
	return ret;
    }

    extern gboolean in_chroot;
    if (chroot (TARGET) == 0) {
	chdir("/"); //change to an valid directory
        in_chroot = TRUE;
        ret = TRUE;
    } else {
        g_warning ("chroot:chroot to %s falied:%s\n", TARGET, strerror (errno));
    }

    return ret;
}

gboolean break_chroot()
{
    extern gboolean in_chroot;
    extern int chroot_fd;

    if (in_chroot) {

	if (fchdir (chroot_fd) < 0) {
	    g_warning ("finish install:reset to chroot fd dir failed\n");
	} else {
	    int i = 0;
	    for (i = 0; i < 1024; i++) {
		chdir ("..");
	    }
	}
	chroot (".");
	in_chroot = FALSE;
    }
}

JS_EXPORT_API 
void  installer_show_log ()
{
    gchar *cmd = g_strdup_printf ("xdg-open %s\n", LOG_FILE_PATH);
    g_spawn_command_line_async (cmd, NULL);
    g_free (cmd);
}



JS_EXPORT_API
char* installer_get_default_lang_pack()
{
    char* contents = NULL;
    if (!g_file_get_contents("/proc/cmdline", &contents, NULL, NULL)) {
	return g_strdup("en_US");
    }

    gchar* begin = g_strstr_len(contents, -1, "locale=");
    if (begin == NULL) {
	g_free(contents);
	return g_strdup("en_US");
    }
    g_free(contents);

    int end = 0;
    for (; begin[end] != ' ' && begin[end] != '.' && begin[end] != '\0'; end++);
    if (end == 0) {
	return g_strdup("en_US");
    }


    char* new_str = g_strdup(begin);
    new_str[end] = '\0';
    char* ret = g_strdup(new_str+ 7);
    g_free(new_str);
    return ret;
}

JS_EXPORT_API 
JSObjectRef installer_get_system_users()
{
    GRAB_CTX ();
    JSObjectRef array = json_array_create ();

    struct passwd *user;
    gchar *username = NULL;
    int i = 0;

    while ((user = getpwent ()) != NULL){
        if (user->pw_uid >= 1000 || g_strcmp0 ("deepin", user->pw_name) == 0) {
            continue;
        }
        username = g_strdup (user->pw_name);
        json_array_insert (array, i, jsvalue_from_cstr (get_global_context(), username));
        i++;
        g_free (username);
    }
    endpwent ();
    UNGRAB_CTX ();

    return array;
}
