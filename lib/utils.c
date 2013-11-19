/**
 * Copyright (c) 2011 ~ 2012 Deepin, Inc.
 *               2011 ~ 2012 snyh
 *
 * Author:      snyh <snyh@snyh.org>
 * Maintainer:  snyh <snyh@snyh.org>
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
#include "utils.h"
#include "jsextension.h"
#include "dentry/entry.h"
#include "dcore.h"
#include <glib.h>
#include <glib/gprintf.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>

int binding(int server_sockfd, const char* path)
{
    socklen_t server_len;
    struct sockaddr_un server_addr;

    server_addr.sun_path[0] = '\0'; //make it be an name unix socket
    int path_size = g_sprintf (server_addr.sun_path+1, "%s%d", path, getuid());
    server_addr.sun_family = AF_UNIX;
    server_len = 1 + path_size + offsetof(struct sockaddr_un, sun_path);

    const int reuse = 1;
    socklen_t val_len = sizeof reuse;
    setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&reuse, val_len);

    // force quit
    /* const struct linger linger_val = {1, 0}; */
    /* val_len = sizeof linger_val; */
    /* setsockopt(server_sockfd, SOL_SOCKET, SO_LINGER, (const void*)&linger_val, val_len); */

    return bind(server_sockfd, (struct sockaddr *)&server_addr, server_len);
}


gboolean is_application_running(const char* path)
{
    int server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (0 == binding(server_sockfd, path)) {
        close(server_sockfd);
        return FALSE;
    } else {
        return TRUE;
    }
}

void singleton(const char* name)
{
    static int sd = 0;
    if (sd != 0)
        return;

    sd = socket(AF_UNIX, SOCK_STREAM, 0);
    while (0 != binding(sd, name))
        g_debug("binding failed");
}

char* shell_escape(const char* source)
{
    const unsigned char *p;
    char *dest;
    char *q;

    g_return_val_if_fail (source != NULL, NULL);

    p = (unsigned char *) source;
    q = dest = g_malloc (strlen (source) * 4 + 1);

    while (*p)
    {
        switch (*p)
        {
            case '\'':
                *q++ = '\\';
                *q++ = '\'';
                break;
            case '\\':
                *q++ = '\\';
                *q++ = '\\';
                break;
            case ' ':
                *q++ = '\\';
                *q++ = ' ';
                break;

            default:
                *q++ = *p;
        }
        p++;
    }
    *q = 0;
    return dest;
}

void log_to_file(const gchar* log_domain, GLogLevelFlags log_level, const gchar* message, char* app_name)
{
    char* log_file_path = g_strdup_printf("/tmp/%s.log", app_name);
    FILE *logfile = fopen(log_file_path, "a");
    g_free(log_file_path);
    if (logfile != NULL) {
        fprintf(logfile, "%s\n", message);
        fclose(logfile);
    }
    g_log_default_handler(log_domain, log_level, message, NULL);
}

JS_EXPORT_API
char* dcore_gen_id(const char* seed)
{
    return g_compute_checksum_for_string(G_CHECKSUM_MD5, seed, strlen(seed));
}

void run_command(const char* cmd)
{
    g_spawn_command_line_async(cmd, NULL);
}
void run_command1(const char* cmd, const char* p1)
{
    char* e_p = shell_escape(p1);
    char* e_cmd = g_strdup_printf("%s %s\n", cmd, e_p);
    g_free(e_p);

    g_spawn_command_line_async(e_cmd, NULL);
    g_free(e_cmd);
}
void run_command2(const char* cmd, const char* p1, const char* p2)
{
    char* e_p1 = shell_escape(p1);
    char* e_p2 = shell_escape(p2);
    char* e_cmd = g_strdup_printf("%s %s %s\n", cmd, e_p1, e_p2);
    g_free(e_p1);
    g_free(e_p2);

    g_spawn_command_line_async(e_cmd, NULL);
    g_free(e_cmd);
}

#include "i18n.h"
void init_i18n()
{
    setlocale(LC_MESSAGES, "");
    textdomain("DDE");
}

JS_EXPORT_API
const char* dcore_gettext(const char* c)
{
    return gettext(c);
}

JS_EXPORT_API
const char* dcore_dgettext(char const* domain, char const* s)
{
    return dgettext(domain, s);
}

JS_EXPORT_API
void dcore_bindtextdomain(char const* domain, char const* mo_file)
{
    bindtextdomain(domain, mo_file);
}


#include <unistd.h>
#include <fcntl.h>
char* get_name_by_pid(int pid)
{
#define LEN 1024
    char content[LEN];

    char* path = g_strdup_printf("/proc/%d/cmdline", pid);
    int fd = open(path, O_RDONLY);
    g_free(path);

    if (fd == -1) {
        return NULL;
    } else {
        int dump = read(fd, content, LEN);
        close(fd);
    }
    for (int i=0; i<LEN; i++) {
        if (content[i] == ' ') {
            content[i] = '\0';
            break;
        }
    }


    return g_path_get_basename(content);
}


GKeyFile* load_app_config(const char* name)
{
    char* path = g_build_filename(g_get_user_config_dir(), name, NULL);
    GKeyFile* key = g_key_file_new();
    g_key_file_load_from_file(key, path, G_KEY_FILE_NONE, NULL);
    g_free(path);
    /* no need to test file exitstly */
    return key;
}

void save_key_file(GKeyFile* key, const char* path)
{
    gsize size;
    gchar* content = g_key_file_to_data(key, &size, NULL);
    write_to_file(path, content, size);
    g_free(content);
}

void save_app_config(GKeyFile* key, const char* name)
{
    char* path = g_build_filename(g_get_user_config_dir(), name, NULL);
    save_key_file(key, path);
    g_free(path);
}

gboolean write_to_file(const char* path, const char* content, size_t size/* if 0 will use strlen(content)*/)
{
    char* dir = g_path_get_dirname(path);
    if (g_file_test(dir, G_FILE_TEST_IS_REGULAR)) {
        g_free(dir);
        g_warning("write content to %s, but %s is not directory!!\n",
                path, dir);
        return FALSE;
    } else if (!g_file_test(dir, G_FILE_TEST_EXISTS)) {
        if (g_mkdir_with_parents(dir, 0755) == -1) {
            g_warning("write content to %s, but create %s is failed!!\n",
                    path, dir);
            return FALSE;
        }
    }
    g_free(dir);

    if (size == 0) {
        size = strlen(content);
    }
    FILE* f = fopen(path, "w");
    if (f != NULL) {
        fwrite(content, sizeof(char), size, f);
        fclose(f);
        return TRUE;
    } else {
        return FALSE;
    }
}
// reparent to init process.
int reparent_to_init ()
{
    switch (fork())
    {
	case -1:
	    return EXIT_FAILURE;
	case 0:
	    return EXIT_SUCCESS;
	default:
	    _exit(EXIT_SUCCESS);
    }
}
static void _consolidate_cmd_line (int subargc, char*** subargv_ptr)
{
    //recursively consolidate
}
void parse_cmd_line (int* argc_ptr, char*** argv_ptr)
{
    char*** subargv_ptr = argv_ptr;
    int     subargc     = (*argc_ptr);

    gboolean should_reparent = TRUE;
    gboolean enable_debug = FALSE;
    int i=0;
    for (;i<(*argc_ptr);i++)
    {
	if(!g_strcmp0 ((*argv_ptr)[i], "-f"))
	{
	    should_reparent=FALSE;
            //(*argv_ptr)[i]=NULL;
	    //(*argc_ptr)--;
            continue;
	}
	if(!g_strcmp0 ((*argv_ptr)[i], "-d"))
	{
            enable_debug = TRUE;
	    should_reparent=FALSE;
            //(*argv_ptr)[i]=NULL;
	    //(*argc_ptr)--;
            continue;
	}
    }
    //uncomment previous comments
    //consolidate *argv, remove NULL slots.
    _consolidate_cmd_line(subargc, subargv_ptr);

    if (should_reparent)
    {
	//close stdin, stdout, stderr
	//redirect them to /dev/null
	int fd;
	close(STDIN_FILENO);
	fd=open("/dev/null", O_RDWR);
	//FIXME: shall we exit?
	if(fd!=STDIN_FILENO)
	    return;
	if(dup2(STDIN_FILENO, STDOUT_FILENO)!=STDOUT_FILENO)
	    return;
	if(dup2(STDIN_FILENO, STDERR_FILENO)!=STDERR_FILENO)
	    return;
	reparent_to_init();
    }
    if (enable_debug)
    {
	g_setenv("G_MESSAGES_DEBUG", "all", FALSE);
    }
}

char* to_lower_inplace(char* str)
{
    g_assert(str != NULL);
    for (size_t i=0; i<strlen(str); i++)
        str[i] = g_ascii_tolower(str[i]);
    return str;
}

gboolean file_filter(const char *file_name)
{
    if((file_name[0] == '.' && !g_str_has_prefix(file_name, DEEPIN_RICH_DIR)) || g_str_has_suffix(file_name, "~"))
        return TRUE;
    else
        return FALSE;
}

char* get_desktop_file_basename(GDesktopAppInfo* file)
{
    const char* filename = g_desktop_app_info_get_filename(file);
    return g_path_get_basename(filename);
}

GDesktopAppInfo* guess_desktop_file(char const* app_id)
{
    char* basename = g_strconcat(app_id, ".desktop", NULL);
    GDesktopAppInfo* desktop_file = g_desktop_app_info_new(basename);
    g_free(basename);
    return desktop_file;
}


char* get_basename_without_extend_name(char const* path)
{
    g_assert(path!= NULL);
    char* basename = g_path_get_basename(path);
    char* ext_sep = strrchr(basename, '.');
    if (ext_sep != NULL) {
        char* basename_without_ext = g_strndup(basename, ext_sep - basename);
        g_free(basename);
        return basename_without_ext;
    }

    return basename;
}


gboolean is_deepin_icon(char const* icon_path)
{
    return g_str_has_prefix(icon_path, "/usr/share/icons/Deepin/");
}


static char* _check(char const* app_id)
{
    char* icon = NULL;
    char* temp_icon_name_holder = dcore_get_theme_icon(app_id, 48);

    if (temp_icon_name_holder != NULL) {
        if (!g_str_has_prefix(temp_icon_name_holder, "data:image"))
            icon = temp_icon_name_holder;
        else
            g_free(temp_icon_name_holder);
    }

    return icon;
}


char* check_absolute_path_icon(char const* app_id, char const* icon_path)
{
    char* icon = NULL;
    if ((icon = _check(app_id)) == NULL) {
        char* basename = get_basename_without_extend_name(icon_path);
        if (basename != NULL) {
            if (g_strcmp0(app_id, basename) == 0
                || (icon = _check(basename)) == NULL)
                icon = g_strdup(icon_path);
            g_free(basename);
        }
    }

    return icon;
}


gboolean is_chrome_app(char const* name)
{
    return g_str_has_prefix(name, "chrome-");
}

