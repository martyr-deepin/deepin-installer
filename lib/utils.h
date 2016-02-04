/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef __UTILS_H__
#define __UTILS_H__

#include <glib.h>
#include <stdlib.h>
#include <gio/gdesktopappinfo.h>

#define GET_HTML_PATH(name) "file://"RESOURCE_DIR"/"name"/index.html"
#define STR_EXP(__A) #__A
#define STR(A) STR_EXP(A)
#define BG_BLUR_PICT_CACHE_DIR "gaussian-background"

#ifndef M_PI
#define M_PI 3.141592653589793
#endif

int binding(int server_sockfd, const char* path);
char* shell_escape(const char* source);
int is_application_running(const char* path);
void singleton(const char* name);
void log_to_file(const gchar* log_domain, GLogLevelFlags log_level, const gchar* message, char* app_name);

char* dcore_gen_id(const char* seed);
void run_command(const char* cmd);
void run_command2(const char* cmd, const char* p1, const char* p2);
void run_command1(const char* cmd, const char* p1);

char* get_name_by_pid(int pid);

gboolean write_to_file(const char* path, const char* content, size_t size/* if 0 will use strlen(content)*/);

GKeyFile* load_app_config(const char* name);

void save_key_file(GKeyFile*, const char* path); /*careful, this function didn't free the key file*/
void save_app_config(GKeyFile*, const char* name); /*careful, this function didn't free the key file*/

int reparent_to_init();
void parse_cmd_line (int* argc, char*** argv);
char* to_lower_inplace(char* str);
gboolean file_filter(const char *file_name);
char* get_desktop_file_basename(GDesktopAppInfo* file);  // g_free the return value
GDesktopAppInfo* guess_desktop_file(char const* app_id);

char* get_basename_without_extend_name(char const* path);
gboolean is_deepin_icon(char const* icon_path);
char* check_absolute_path_icon(char const* app_id, char const* icon_path);
gboolean is_chrome_app(char const* name);
char* bg_blur_pict_get_dest_path (const char* src_uri);
gboolean is_livecd();
gboolean is_virtual_pc();
gchar* get_timezone_local ();
gboolean spawn_command_sync (const char* command,gboolean sync);

#endif

