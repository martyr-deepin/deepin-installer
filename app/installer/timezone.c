/**
 * Copyright (c) 2011 ~ 2014 Deepin, Inc.
 *               2013 ~ 2014 jouyouyun
 *
 * Author:      jouyouyun <jouyouwen717@gmail.com>
 * Maintainer:  jouyouyun <jouyouwen717@gmail.com>
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

#include <time.h>
#include <glib.h>
#include "jsextension.h"
#include "utils.h"

int get_offset(const char *timezone )
{
    g_debug("[%s] timezone: %s\n", __func__, timezone);
    if (timezone == NULL) {
        return 0;
    }
    const char *tz = g_getenv("TZ");
    g_setenv("TZ", timezone, TRUE);
    time_t t = time(NULL);
    struct tm* tp = localtime(&t);
    if (tz != NULL){
        g_setenv("TZ", tz, TRUE);
        tz = NULL;
    }
# if defined __USE_MISC
    return tp->tm_gmtoff;
# else
    return tp->__tm_gmtoff;
#endif
}

JS_EXPORT_API
char* installer_get_timezone_utc (const char* timezone)
{
    int offset = get_offset(timezone);
    int offset_hour = offset / 3600;
    int offset_min =  60 * ( (float)offset / 3600 - offset_hour );
    const char * sign = offset_hour >= 0 ? "+" : "";
    char* utc = g_strdup_printf("UTC%s%02d:%02d" , sign , offset_hour,
                                offset_min );
    g_debug("[%s] offset: %d, utc: %s;" , __func__, offset, utc);
    return utc;
}

JS_EXPORT_API
char* installer_get_timezone_local()
{
    return get_timezone_local();
}
