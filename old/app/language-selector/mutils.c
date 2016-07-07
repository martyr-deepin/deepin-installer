/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "mutils.h"

gboolean 
app_is_running(const char* path)
{
    int server_sockfd;
    socklen_t server_len;
    struct sockaddr_un server_addr;

    server_addr.sun_path[0] = '\0'; //make it be an name unix socket
    int path_size = g_sprintf (server_addr.sun_path+1, "%s", path);
    server_addr.sun_family = AF_UNIX;
    server_len = 1 + path_size + offsetof (struct sockaddr_un, sun_path);

    server_sockfd = socket (AF_UNIX, SOCK_STREAM, 0);

    if (0 == bind(server_sockfd, (struct sockaddr *)&server_addr, server_len)) {
        return FALSE;
    } else {
        return TRUE;
    }
}

gboolean 
is_capslock_on ()
{
    gboolean capslock_flag = FALSE;

    Display *d = XOpenDisplay ((gchar*)0);
    guint n;

    if (d) {
        XkbGetIndicatorState (d, XkbUseCoreKbd, &n);

        if ((n & 1)) {
            capslock_flag = TRUE;
        }
    }
    XCloseDisplay (d);

    return capslock_flag;
}

gchar *
get_date_string ()
{
    gchar *ret = NULL;

    char outstr[200];
    time_t t;
    struct tm *tmp;

    setlocale (LC_ALL, "");

    t = time (NULL);
    tmp = localtime (&t);

    if (tmp == NULL) {
        g_warning("[%S] call localtime() failed\n", __func__);
    }

    if (strftime (outstr, sizeof(outstr), _("%a, %b %d, %Y"), tmp) == 0) {
        g_error("[%s] strftime returns 0\n", __func__);
    }

    ret = g_strdup (outstr);

    return ret;
}

void turn_numlock_on ()
{
    g_spawn_command_line_async ("numlockx on", NULL);
}
