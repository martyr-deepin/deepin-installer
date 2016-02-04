/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include <stdio.h>
#include <glib.h>
#include <unistd.h>

char* output_file = NULL;

void update_progress(int v)
{
    if (output_file != NULL) {
        char* p = g_strdup_printf("%d\n", v);
        GError* error = NULL;
        g_file_set_contents(output_file, p, -1, &error);
        g_free(p);
        if (error != NULL) {
            g_warning("[%s] can't write unsquashfs progress(%d) to %s : %s\n",
                      __func__, v, output_file, error->message);
            g_error_free(error);
        }
    } else {
        g_message("[%s]: %d\n", __func__, v);
    }
}

int main(int argc, char* argv[])
{
    if (argc == 2) {
        output_file = g_strdup(argv[1]);
        char* dir = g_path_get_dirname(output_file);
        g_mkdir_with_parents(dir, 0755);
        g_free(dir);
    } else {
        g_warning("[%s] Hasn't set progress output file, use stdout\n",
                  __func__);
    }
    GRegex* regex = g_regex_new("\\d{1,3}%", 0, 0, NULL);

    GError* error = NULL;
    char* content = NULL;
    GIOChannel* channel = g_io_channel_unix_new(STDIN_FILENO);
    GMatchInfo* match_info = NULL;
    int current_per = -1;
    while (G_IO_STATUS_NORMAL ==
           g_io_channel_read_line(channel, &content, NULL, NULL, &error)) {

        g_regex_match(regex, content, 0, &match_info);
        {
            char* per = g_match_info_fetch(match_info, 0);
            if (per !=  NULL) {
                gchar* endptr = NULL;
                int tmp_per = g_strtod(per, &endptr);
                if (endptr != NULL && tmp_per != current_per) {
                    current_per =  tmp_per;
                    update_progress(current_per);
                }
            }
        }
        g_message("[%s] content: %s\n", __func__, content);
        fflush(stdout);

        g_match_info_free(match_info);
        g_free(content);
    }

    g_io_channel_unref(channel);
    g_regex_unref(regex);
}
