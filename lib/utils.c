/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "utils.h"
#include "jsextension.h"
#include "dcore.h"
#include "i18n.h"
#include <glib.h>
#include <glib/gprintf.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


char* shell_escape(const char* source)
{
    const unsigned char *p;
    char *dest;
    char *q;

    g_return_val_if_fail (source != NULL, NULL);

    p = (unsigned char *) source;
    q = dest = g_malloc (strlen (source) * 4 + 1);

    while (*p) {
        switch (*p) {
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
        int dump __attribute__((unused)) = read(fd, content, LEN);
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
        g_warning("[%s] write content to %s, but %s is not directory!!\n",
                  __func__, path, dir);
        return FALSE;
    } else if (!g_file_test(dir, G_FILE_TEST_EXISTS)) {
        if (g_mkdir_with_parents(dir, 0755) == -1) {
            g_warning("[%s] write content to %s, but create %s is failed!!\n",
                      __func__, path, dir);
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

int close_std_stream()
{
    //close stdin, stdout, stderr
    //redirect them to /dev/null
    int fd;
    close(STDIN_FILENO);
    fd = open("/dev/null", O_RDWR);
    if (fd != STDIN_FILENO) {
        return 1;
    }
    if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO) {
        return 1;
    }
    if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO) {
        return 1;
    }
    return 0;
}

// reparent to init process.
int reparent_to_init ()
{
    switch (fork()) {
    case -1:
        return EXIT_FAILURE;

    case 0:
        return EXIT_SUCCESS;

    default:
        _exit(EXIT_SUCCESS);
    }
}

static void _consolidate_cmd_line (int subargc G_GNUC_UNUSED,
                                   char*** subargv_ptr G_GNUC_UNUSED)
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
    for (; i<(*argc_ptr); i++) {
        if(!g_strcmp0 ((*argv_ptr)[i], "-f")) {
            should_reparent=FALSE;
            //(*argv_ptr)[i]=NULL;
            //(*argc_ptr)--;
            continue;
        }

        if(!g_strcmp0 ((*argv_ptr)[i], "-d")) {
            enable_debug = TRUE;
            should_reparent = FALSE;
            //(*argv_ptr)[i]=NULL;
            //(*argc_ptr)--;
            continue;
        }
    }
    //uncomment previous comments
    //consolidate *argv, remove NULL slots.
    _consolidate_cmd_line(subargc, subargv_ptr);

    if (should_reparent) {
        //FIXME: shall we exit?
        if (close_std_stream()) {
            return;
        }
        reparent_to_init();
    }

    if (enable_debug) {
        g_setenv("G_MESSAGES_DEBUG", "all", FALSE);
    }
}

char* to_lower_inplace(char* str)
{
    g_assert(str != NULL);
    for (size_t i=0; i<strlen(str); i++) {
        str[i] = g_ascii_tolower(str[i]);
    }
    return str;
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
        if (!g_str_has_prefix(temp_icon_name_holder, "data:image")) {
            icon = temp_icon_name_holder;
        } else {
            g_free(temp_icon_name_holder);
        }
    }

    return icon;
}

char* check_absolute_path_icon(char const* app_id, char const* icon_path)
{
    char* icon = NULL;
    if ((icon = _check(app_id)) == NULL) {
        char* basename = get_basename_without_extend_name(icon_path);
        if (basename != NULL) {
            if (g_strcmp0(app_id, basename) == 0 ||
                (icon = _check(basename)) == NULL) {
                icon = g_strdup(icon_path);
            }
            g_free(basename);
        }
    }

    return icon;
}

gboolean is_chrome_app(char const* name)
{
    return g_str_has_prefix(name, "chrome-");
}

char* bg_blur_pict_get_dest_path (const char* src_uri)
{
    g_debug("[%s] bg_blur_pict_get_dest_path: src_uri: %s\n",
            __func__, src_uri);
    g_return_val_if_fail (src_uri != NULL, NULL);

    //1. calculate original picture md5
    GChecksum* checksum;
    checksum = g_checksum_new (G_CHECKSUM_MD5);
    g_checksum_update (checksum, (const guchar *) src_uri, strlen (src_uri));

    guint8 digest[16];
    gsize digest_len = sizeof (digest);
    g_checksum_get_digest (checksum, digest, &digest_len);
    g_assert (digest_len == 16);

    //2. build blurred picture path
    char* file;
    file = g_strconcat (g_checksum_get_string (checksum), ".png", NULL);
    g_checksum_free (checksum);
    char* path;
    path = g_build_filename(g_get_user_cache_dir (),
                            BG_BLUR_PICT_CACHE_DIR, file, NULL);
    g_free (file);

    return path;
}

gboolean is_livecd ()
{
    const gchar *filename = "/proc/cmdline";
    gchar *contents = NULL;
    gboolean result = FALSE;
    if (g_file_get_contents(filename, &contents, NULL, NULL)) {
        result = (g_strstr_len(contents, -1, "boot=casper") != NULL);
        g_free(contents);
    }
    return result;
}

gboolean is_virtual_pc ()
{
    gboolean result = FALSE;
    GError *error = NULL;
    gchar* output = NULL;
    const gchar* cmd = "dmidecode -s system-product-name";
    g_spawn_command_line_sync (cmd, &output, NULL, NULL, &error);
    if (error != NULL) {
        g_warning("[%s] cmd: %s, failed: %s\n", __func__, cmd, error->message);
        g_clear_error(&error);
    } else {
        if (output != NULL){
            gchar* low = g_utf8_casefold(output, -1);
            result = (g_strstr_len(low, -1, "virtual") != NULL);
            g_free(low);
        }
    }
    g_free(output);
    g_debug("[%s] cmd: %s, output: %s, result: %d\n",
            __func__, cmd, output, result);
    cmd = NULL;
    return result;
}

gchar* get_timezone_local ()
{
    const gchar *filename = "/etc/timezone";
    gchar *contents = NULL;
    if (g_file_get_contents(filename, &contents, NULL, NULL)) {
        return g_strstrip(contents);
    }
    return NULL;
}

gboolean spawn_command_sync (const char* command, gboolean sync)
{
    GError *error = NULL;
    const gchar *cmd = g_strdup_printf ("%s",command);
    g_message("[%s] cmd: %s, sync: %d\n", __func__, cmd, sync);
    if (sync) {
        g_spawn_command_line_sync (cmd, NULL, NULL, NULL, &error);
    } else {
        g_spawn_command_line_async (cmd, &error);
    }
    if (error != NULL) {
        g_warning("[%s] %s failed: %s\n", __func__, cmd, error->message);
        g_clear_error(&error);
        return FALSE;
    }
    return TRUE;
}
