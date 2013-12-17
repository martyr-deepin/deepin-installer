/**
 * Copyright (c) 2011 ~ 2012 Deepin, Inc.
 *               2011 ~ 2012 snyh
 *               2011 ~ 2012 hooke
 *
 * Author:      snyh <snyh@snyh.org>
 *              hooke
 * Maintainer:  snyh <snyh@snyh.org>
 *              hooke
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
#include <sys/stat.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "entry.h"
#include <glib.h>
#include <glib/gstdio.h>
#include "jsextension.h"
#include "dcore.h"
#include <string.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
#include "utils.h"
#include "xdg_misc.h"
#include "fileops.h"
#include "fileops_clipboard.h"
#include "fileops_trash.h"
#include "fileops_delete.h"
#include "thumbnails.h"
#include "mime_actions.h"
#include "fileops_error_reporting.h"
#include "category.h"

ArrayContainer EMPTY_CONTAINER = {0, 0};

static GFile* _get_gfile_from_gapp(GDesktopAppInfo* info);
static ArrayContainer _normalize_array_container(ArrayContainer pfs);
static gboolean _file_is_archive (GFile *file);
static void _commandline_exec(const char *commandline, GList *list);


#define TEST_GFILE(e, f) if (G_IS_FILE(e)) { \
    GFile* f = e;

#define TEST_GAPP(e, app) } else if (G_IS_APP_INFO(e)) { \
    GAppInfo* app = e;

#define TEST_END } else { g_warn_if_reached();}

#define FILES_COMPRESSIBLE_NONE 0
#define FILES_COMPRESSIBLE      1
#define FILES_DECOMPRESSIBLE    2
#define FILES_COMPRESSIBLE_ALL  3


JS_EXPORT_API
Entry* dentry_get_desktop()
{
    return dentry_create_by_path(DESKTOP_DIR());
}

JS_EXPORT_API
gboolean dentry_should_move(Entry* e)
{
    GFile* file = e;
    if (G_IS_APP_INFO(e)) {
        file = _get_gfile_from_gapp(e);
    }
    if (!g_file_is_native(file)) {
        if (file != e)
            g_object_unref(file);
        return FALSE;
    }
    GFileInfo* info = g_file_query_info (file,
            "access::can-delete",
            G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
            NULL,
            NULL);
    if (file != e)
        g_object_unref(file);

    gboolean can_delete = g_file_info_get_attribute_boolean(info, "access::can-delete");
    g_object_unref(info);
    return can_delete;
}

JS_EXPORT_API
gboolean dentry_is_native(Entry* e)
{

    if (G_IS_FILE(e)) {
        return g_file_is_native (G_FILE(e));
    }
    return TRUE;
}

JS_EXPORT_API
gboolean dentry_is_gapp(Entry* e)
{
    if(G_IS_APP_INFO(e))
        return TRUE;
    else return FALSE;
}

JS_EXPORT_API
double dentry_get_type(Entry* e)
{
    TEST_GFILE(e, f)
        switch (g_file_query_file_type(f, G_FILE_QUERY_INFO_NONE, NULL)) {
            case G_FILE_TYPE_REGULAR:
                return 1;
            case G_FILE_TYPE_DIRECTORY:
                {
                    char* path = g_file_get_basename(f);
                    if (g_str_has_prefix(path, DEEPIN_RICH_DIR)) {
                        g_free(path);
                        return 3;
                    } else {
                        g_free(path);
                        return 2;
                    }
                }
        case G_FILE_TYPE_SYMBOLIC_LINK:
        {
            char* src = g_file_get_path(f);
            char* target = g_file_read_link (src, NULL);
            g_free (src);
            if (target != NULL||g_file_test(target, G_FILE_TEST_EXISTS))
            {
            GFile* target_gfile = g_file_new_for_commandline_arg(target);
            g_free(target);
            double retval = dentry_get_type(target_gfile);
            g_object_unref(target_gfile);
            if (retval == -1)
                retval = 4;
            return retval;
            }
            return 4;
        }
        //the remaining file type values.
        case G_FILE_TYPE_SPECIAL:
        case G_FILE_TYPE_MOUNTABLE:
            default:
                {
                    char* path = g_file_get_path(f);
                    g_warning("Did't know file type %s", path);
                    g_free(path);
                    return -1;
                }
        }
    TEST_GAPP(e, app)
        return 0;
    TEST_END
}

//TODO:
JS_EXPORT_API
JSObjectRef dentry_get_flags (Entry* e)
{
    JSObjectRef json = json_array_create();
    GFile* f;
    if (!G_IS_FILE(e)) {
        return json;
    }
    f = e;

    GFileInfo* info = g_file_query_info (f,
            "standard::*,access::*",
            G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
            NULL,
            NULL);

    if (info != NULL)
    {
        gboolean is_read_only = FALSE;
        gboolean is_symlink = FALSE;
        gboolean is_unreadable = FALSE;
        is_unreadable = !g_file_info_get_attribute_boolean(info, "access::can-read");
        is_read_only = !g_file_info_get_attribute_boolean(info, "access::can-write");
        is_symlink = g_file_info_get_is_symlink(info);
        g_object_unref(info);
        json_append_number(json, "read_only", is_read_only);
        json_append_number(json, "symbolic_link", is_symlink);
        json_append_number(json, "unreadable", is_unreadable);
    }

    return json;
}
JS_EXPORT_API
char* dentry_get_name(Entry* e)
{
    TEST_GFILE(e, f)
        return g_file_get_basename(f);
    TEST_GAPP(e, app)
        return g_strdup(g_app_info_get_name(app));
    TEST_END
}

JS_EXPORT_API
char* dentry_get_appid(Entry* e)
{
    char* basename = g_path_get_basename(g_desktop_app_info_get_filename(e));
    char* ext_spe = strchr(basename, '.');
    if (ext_spe == NULL) {
        return basename;
    } else {
        char* app_id = g_strndup(basename, ext_spe - basename);
        g_free(basename);
        return app_id;
    }
}


JS_EXPORT_API
char* dentry_get_uri(Entry* e)
{
    TEST_GFILE(e, f)
        char* uri = g_file_get_uri(f);
        /*
         * some characters like ')', '(' may not be escaped
         *
         * ':' for scheme, like file:///...., the : cannot be escaped
         * '/' for path separator, should be escaped
         * '%' for the characters escaped, should be escaped
         */
        char* escaped_uri = g_uri_escape_string(uri, ":/%", FALSE);
        g_free(uri);
        return escaped_uri;
    TEST_GAPP(e, app)
        char* encode = g_uri_escape_string(g_desktop_app_info_get_filename(G_DESKTOP_APP_INFO(app)),
                    "/", FALSE);
        char* uri = g_strdup_printf("file://%s", encode);
        g_free(encode);
        return uri;
    g_message("uri:---%s---",uri);
    TEST_END
}

JS_EXPORT_API
char* dentry_get_icon(Entry* e)
{
    char* ret = NULL;
    TEST_GFILE(e, f)
        GFileInfo *info = g_file_query_info(f, "standard::icon", G_FILE_QUERY_INFO_NONE, NULL, NULL);
        if (info != NULL) {
            GIcon* icon = g_file_info_get_icon(info);
            ret = lookup_icon_by_gicon(icon);
        }
        g_object_unref(info);
    TEST_GAPP(e, app)
        GIcon *icon = g_app_info_get_icon(app);

        if (icon != NULL) {
            char* icon_str = g_icon_to_string(icon);

            if (icon_str && g_path_is_absolute(icon_str) && !is_deepin_icon(icon_str)) {
                char* app_id =
                    get_basename_without_extend_name(g_desktop_app_info_get_filename(G_DESKTOP_APP_INFO(app)));
                char* temp_icon_name_holder = icon_str;
                icon_str = check_absolute_path_icon(app_id, icon_str);
                g_free(temp_icon_name_holder);
            }

            ret = icon_name_to_path_with_check_xpm(icon_str, 48);
            g_free(icon_str);
        }
    TEST_END

    return ret;
}
/*
 *      this differs dentry_get_icon:
 *      dentry_get_icon can return data_uri instead of actual paths
 */
char* dentry_get_icon_path(Entry* e)
{
    char* ret = NULL;
    TEST_GFILE(e, f)
        GFileInfo *info = g_file_query_info(f, "standard::icon", G_FILE_QUERY_INFO_NONE, NULL, NULL);
        if (info != NULL) {
            GIcon* icon = g_file_info_get_icon(info);
            ret = lookup_icon_by_gicon(icon);
        }
        g_object_unref(info);
    TEST_GAPP(e, app)
        GIcon *icon = g_app_info_get_icon(app);
        if (icon != NULL) {
            char* icon_str = g_icon_to_string(icon);
            ret = icon_name_to_path (icon_str, 48);
            if (ret == NULL)
            {
                g_warning("richdir dentry  get_icon is null use invalid-dock_app.png instead");
                const char * invalid_app = "invalid-dock_app";
                ret = dcore_get_theme_icon(invalid_app, 48);
            }

            g_free(icon_str);
        }
        else{
            g_debug("g_app_info_get_icon return NULL ");
            return NULL;
        }
    TEST_END
    return ret;
}
JS_EXPORT_API
gboolean dentry_can_thumbnail(Entry* e)
{
    TEST_GFILE(e, f)
        return gfile_can_thumbnail (f);
    TEST_GAPP(e, app)
        return FALSE;
    TEST_END
}

JS_EXPORT_API
char* dentry_get_thumbnail(Entry* e)
{
    g_assert (G_IS_FILE(e));
    char* ret = NULL;
    GFile* f = e;
    //use thumbnail if possible.
    ret = gfile_lookup_thumbnail (f);
    return ret;
}

char* calc_id(const char* uri)
{
    char* name = g_path_get_basename(uri);
    char* id = g_compute_checksum_for_string(G_CHECKSUM_MD5, name, strlen(name));
    g_free(name);
    return id;
}

JS_EXPORT_API
char* dentry_get_id(Entry* e)
{
    char* uri = dentry_get_uri(e);
    char* id = calc_id(uri);
    g_free(uri);
    return id;
}

JS_EXPORT_API
gboolean dentry_launch(Entry* e, const ArrayContainer fs)
{
    TEST_GFILE(e, f)
        gboolean launch_res = TRUE;
        GFileInfo* info = g_file_query_info(f, "standard::content-type,access::can-execute", G_FILE_QUERY_INFO_NONE, NULL, NULL);
        if (info != NULL) {
            const char* content_type = g_file_info_get_attribute_string(info, G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE);
            gboolean is_executable = g_file_info_get_attribute_boolean(info, "access::can-execute");
            //ugly hack here. we just read the first GFile*.
            GFile* _file_arg = NULL;
            ArrayContainer _fs;
            if (fs.num != 0)
            {
                _fs = _normalize_array_container(fs);
                GFile** files = _fs.data;
                _file_arg = files[0];
            }

            launch_res = activate_file (f, content_type, is_executable, _file_arg);
            /* launch_res =TRUE; */

            if (fs.num != 0)
            {
                for (size_t i=0; i<_fs.num; i++) {
                     g_object_unref(((GObject**)_fs.data)[i]);
                }
                g_free(_fs.data);
            }

            g_object_unref(info);
        } else {
            g_message("GFileInfo is NULL");
            char* path = g_file_get_path(f);
            run_command1("gvfs-open", path);
            g_free(path);
            return TRUE;
        }
        return launch_res;
    TEST_GAPP(e, app)
        char const* startup_wm_class = g_desktop_app_info_get_startup_wm_class(G_DESKTOP_APP_INFO(app));
        if (startup_wm_class != NULL) {
            GKeyFile* f = load_app_config("dock/filter.ini");
            const char* path = g_desktop_app_info_get_filename(G_DESKTOP_APP_INFO(app));
            char* appid = get_basename_without_extend_name(path);
            g_strdelimit(appid, "_", '-');
            /* g_warning("[%s] save %s", __func__, appid); */
            g_key_file_set_string(f, startup_wm_class, "appid", appid);
            g_key_file_set_string(f, startup_wm_class, "path", path);
            g_free(appid);
            save_app_config(f, "dock/filter.ini");
            g_key_file_unref(f);
        }
        ArrayContainer _fs = _normalize_array_container(fs);

        GFile** files = _fs.data;
        GList* list = NULL;
        for (size_t i=0; i<fs.num; i++) {
            list = g_list_append(list, files[i]);
        }
        GdkAppLaunchContext* launch_context = gdk_display_get_app_launch_context(gdk_display_get_default());
        gdk_app_launch_context_set_screen(launch_context, gdk_screen_get_default());
        gdk_app_launch_context_set_icon(launch_context, g_app_info_get_icon(app));
        gboolean ret = g_app_info_launch(app, list, (GAppLaunchContext*)launch_context, NULL);
        g_object_unref(launch_context);
        g_list_free(list);

        for (size_t i=0; i<_fs.num; i++) {
            g_object_unref(((GObject**)_fs.data)[i]);
        }
        g_free(_fs.data);
        return ret;

    TEST_END
     return FALSE;
}



JS_EXPORT_API
ArrayContainer dentry_list_files(GFile* f)
{
    g_return_val_if_fail(g_file_query_file_type(f, G_FILE_QUERY_INFO_NONE, NULL) == G_FILE_TYPE_DIRECTORY, EMPTY_CONTAINER);
    char* dir_path = g_file_get_path(f);
    GDir* dir = g_dir_open(dir_path, 0, NULL);
    const char* child_name = NULL;
    GPtrArray* array = g_ptr_array_sized_new(1024);

    for (int i=0; NULL != (child_name = g_dir_read_name(dir)); i++) {
        char* path = g_build_filename(dir_path, child_name, NULL);
        g_ptr_array_add(array, dentry_create_by_path(path));
        g_free(path);
    }

    g_dir_close(dir);
    g_free(dir_path);

    ArrayContainer ac;
    ac.num = array->len;
    ac.data = array->pdata;
    g_ptr_array_free(array, FALSE);

    return ac;
}

JS_EXPORT_API
Entry* dentry_create_by_path(const char* path)
{
    if (g_str_has_suffix(path, ".desktop")) {
        Entry* e = g_desktop_app_info_new_from_filename(path);
        if (e != NULL) return e;
    }
    return g_file_new_for_commandline_arg(path);
}

JS_EXPORT_API
gboolean dentry_is_fileroller_exist()
{
    gchar *path = g_find_program_in_path("file-roller");

    if(NULL != path)
    {
        g_free(path);
        return TRUE;
    }

    return FALSE;
}

JS_EXPORT_API
double dentry_files_compressibility(ArrayContainer fs)
{
    ArrayContainer _fs;
    GFile** files = NULL;

    if (fs.num != 0)
    {
        _fs = _normalize_array_container(fs);
        files = _fs.data;
    }

    if(1 == fs.num)
    {
        GFile *f = files[0];
        if(_file_is_archive(f))
        {
            g_free(_fs.data);
            return FILES_DECOMPRESSIBLE;
        }
        char *filename = g_file_get_basename(f);
        if(NULL!=filename && g_str_has_suffix(filename, ".desktop"))
        {
            g_free(_fs.data);
            g_free(filename);
            return FILES_COMPRESSIBLE_NONE;
        }
    }
    else if(1 < fs.num)
    {
        gboolean all_compressed = TRUE;
        for(guint i=0; i<fs.num; i++)
        {
            GFile *f = files[i];
            if(NULL == f)
            {
                g_free(_fs.data);
                return FILES_COMPRESSIBLE_NONE;
            }
            if(!_file_is_archive(f))
            {
                all_compressed = FALSE;
                if(!g_file_get_path(f))
                {
                    g_free(_fs.data);
                    return FILES_COMPRESSIBLE_NONE;
                }
            }

            char *filename = g_file_get_basename(f);
            if(NULL!=filename && g_str_has_suffix(filename, ".desktop"))
            {
                g_free(_fs.data);
                g_free(filename);
                return FILES_COMPRESSIBLE_NONE;
            }
        }

        if(all_compressed)
        {
            g_free(_fs.data);
            return FILES_COMPRESSIBLE_ALL;
        }
    }

    if(_fs.data != NULL)
    {
        g_free(_fs.data);
    }
    return FILES_COMPRESSIBLE;
}

static gboolean
_file_is_archive (GFile *file)
{
    char *mime_type = NULL;
    guint i;
    static const char * archive_mime_types[] = { "application/x-gtar",
        "application/x-zip",
        "application/x-zip-compressed",
        "application/zip",
        "application/x-zip",
        "application/x-tar",
        "application/x-7z-compressed",
        "application/x-rar",
        "application/x-rar-compressed",
        "application/x-jar",
        "application/x-java-archive",
        "application/x-war",
        "application/x-ear",
        "application/x-arj",
        "application/x-gzip",
        "application/gzip",
        "application/x-bzip-compressed-tar",
        "application/x-compressed-tar",
        "application/x-archive",
        "application/x-xz-compressed-tar",
        "application/x-bzip",
        "application/x-cbz",
        "application/x-xz",
        "application/x-lzma-compressed-tar",
        "application/x-ms-dos-executable",
        "application/x-lzma",
        "application/x-cd-image",
        "application/x-deb",
        "application/x-rpm",
        "application/x-stuffit",
        "application/x-tzo",
        "application/x-tarz",
        "application/x-tzo",
        "application/x-msdownload",
        "application/x-lha",
        "application/x-zoo"};

    g_return_val_if_fail (file != NULL, FALSE);

    GFileInfo* info = g_file_query_info(file, "standard::content-type", G_FILE_QUERY_INFO_NONE, NULL, NULL);
    if (info != NULL) {
        mime_type = (char*)g_file_info_get_attribute_string(info, G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE);
        char* path = g_file_get_path(file);
        g_debug("[%s] \"%s\" MINE type is: \"%s\"", __func__, path, mime_type);
        g_free(path);

        for (i = 0; i < G_N_ELEMENTS (archive_mime_types); i++) {
            if (!strcmp (mime_type, archive_mime_types[i])) {
                return TRUE;
            }
        }
        g_object_unref(info);
    }
    return FALSE;
}

JS_EXPORT_API
void dentry_compress_files(ArrayContainer fs)
{
    ArrayContainer _fs;
    GFile** files = NULL;

    if(fs.num != 0)
    {
        _fs = _normalize_array_container(fs);
        files = _fs.data;

        GList *list = NULL;
        for (size_t i=0; i<_fs.num; i++)
        {
            GFile *file = files[i];
            list = g_list_append(list, file);
        }
        _commandline_exec("file-roller -d %U ", list);

        g_list_free(list);
        for (size_t i=0; i<_fs.num; i++) {
             g_object_unref(((GObject**)_fs.data)[i]);
        }
        g_free(_fs.data);
    }
}

JS_EXPORT_API
void dentry_decompress_files(ArrayContainer fs)
{
    ArrayContainer _fs;
    GFile** files = NULL;

    if(fs.num != 0)
    {
        _fs = _normalize_array_container(fs);
        files = _fs.data;

        for (size_t i=0; i<_fs.num; i++)
        {
            GList *list = NULL;
            GFile *file = files[i];
            list = g_list_append(list, file);
            _commandline_exec("file-roller -f ", list);
            g_list_free(list);
        }

        for (size_t i=0; i<_fs.num; i++) {
             g_object_unref(((GObject**)_fs.data)[i]);
        }
        g_free(_fs.data);
    }
}

JS_EXPORT_API
void dentry_decompress_files_here(ArrayContainer fs)
{
    ArrayContainer _fs;
    GFile** files = NULL;

    if(fs.num != 0)
    {
        _fs = _normalize_array_container(fs);
        files = _fs.data;

        GList *list = NULL;
        for (size_t i=0; i<_fs.num; i++)
        {
            GFile *file = files[i];
            list = g_list_append(list, file);
        }
        _commandline_exec("file-roller -h ", list);

        g_list_free(list);
        for (size_t i=0; i<_fs.num; i++) {
             g_object_unref(((GObject**)_fs.data)[i]);
        }
        g_free(_fs.data);
    }
}

static void
_commandline_exec(const char *commandline, GList *list)
{
    GAppInfo *app_info = g_app_info_create_from_commandline(commandline, NULL, G_APP_INFO_CREATE_SUPPORTS_STARTUP_NOTIFICATION, NULL);
    g_app_info_launch(app_info, list, NULL, NULL);

    g_object_unref(app_info);
}

static
GFile* _get_gfile_from_gapp(GDesktopAppInfo* info)
{
    return g_file_new_for_commandline_arg(g_desktop_app_info_get_filename(info));
}

JS_EXPORT_API
double dentry_get_mtime(Entry* e)
{
    GFile* file = NULL;
    TEST_GFILE(e, f)
        file = g_file_dup(f);
    TEST_GAPP(e, app)
        if (G_IS_DESKTOP_APP_INFO(app))
            file = _get_gfile_from_gapp((GDesktopAppInfo*)app);
    TEST_END

    guint64 time = 0;
    if (file != NULL) {
        GFileInfo* info = g_file_query_info(file, G_FILE_ATTRIBUTE_TIME_CHANGED, G_FILE_QUERY_INFO_NONE, NULL, NULL);
        time = g_file_info_get_attribute_uint64(info, G_FILE_ATTRIBUTE_TIME_CHANGED);
        g_object_unref(info);
        g_object_unref(file);
    }
    return time;
}
static void show_rename_error_dialog (const char* name, gboolean is_app)
{
    GtkWidget* dialog;
    dialog = gtk_message_dialog_new (NULL,
                         GTK_DIALOG_MODAL,
                         GTK_MESSAGE_WARNING,
                         GTK_BUTTONS_OK,
                         NULL);
    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
    char* secondary_text;
    if (is_app)
    {
       secondary_text = g_strdup_printf(_("This *.desktop file cannot be changed to the name \"%s\"."
                                       "You may not have such permission"),
                                       name);
    }
    else
    {
       secondary_text = g_strdup_printf(_("The name \"%s\" is already used in this "
                                        "folder. Please use a different name."),
                                        name);
    }

    g_object_set (dialog,
                  "text", _("The Item could not be renamed"),
                  "secondary-text", secondary_text,
                  NULL);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
    g_free(secondary_text);
}
JS_EXPORT_API
gboolean dentry_set_name(Entry* e, const char* name)
{
    TEST_GFILE(e, f)
        GError* err = NULL;
        GFile* new_file = g_file_set_display_name(f, name, NULL, &err);
        if (err) {
            show_rename_error_dialog (name, FALSE);
            g_error_free(err);
            return FALSE;
        } else {
            g_object_unref(new_file);
            return TRUE;
        }
    TEST_GAPP(e, app)
        const char* path = g_desktop_app_info_get_filename(G_DESKTOP_APP_INFO(app));
        if (!change_desktop_entry_name(path, name))
        {
            show_rename_error_dialog (name, TRUE);
            return FALSE;
        }
        else
        {
            return TRUE;
        }
    TEST_END
}

static ArrayContainer _normalize_array_container(ArrayContainer pfs)
{
    GPtrArray* array = g_ptr_array_new();

    GFile** _array = pfs.data;
    for(size_t i=0; i<pfs.num; i++) {
        if (G_IS_DESKTOP_APP_INFO(_array[i])) {
            g_ptr_array_add(array, _get_gfile_from_gapp(((GDesktopAppInfo*)_array[i])));
        } else {
            g_ptr_array_add(array, g_object_ref(_array[i]));
        }
    }

    ArrayContainer ret;
    ret.num = pfs.num;
    ret.data = g_ptr_array_free(array, FALSE);
    return ret;
}

JS_EXPORT_API
gboolean dentry_move(ArrayContainer fs, GFile* dest, gboolean prompt)
{
    gboolean retval = TRUE;
    ArrayContainer _fs = _normalize_array_container(fs);
    retval = fileops_move(_fs.data, _fs.num, dest, prompt);
    for (size_t i=0; i<_fs.num; i++) {
        g_object_unref(((GObject**)_fs.data)[i]);
    }
    g_free(_fs.data);
    return retval;
}

void do_dereference_symlink_copy(GFile* src, GFile* dest, GFileCopyFlags copy_flag)
{
    GError* error = NULL;
    if (!g_file_copy(src, dest, copy_flag, NULL, NULL, NULL, &error)) {
        g_warning("[%s] error message: %s, error code: %d\n", __func__,
                  error->message, error->code);
        FileOpsResponse* response;
        response = fileops_move_copy_error_show_dialog(_("copy"), error, src, dest, NULL);

        if (response != NULL) {
            switch (response->response_id)
            {
            case GTK_RESPONSE_CANCEL:
                //cancel all operations
                g_debug ("response : Cancel");
                break;

            case CONFLICT_RESPONSE_SKIP:
                //skip, imediately return.
                g_debug ("response : Skip");
                break;
            case CONFLICT_RESPONSE_RENAME:
                //rename, redo operations
                g_warning ("response : Rename to %s", response->file_name);

                GFile* dest_parent = g_file_get_parent(dest);
                GFile* new_dest = g_file_get_child (dest_parent, response->file_name);
                g_object_unref(dest_parent);

                do_dereference_symlink_copy(src, new_dest, copy_flag);
                g_object_unref(new_dest);

                break;
            case CONFLICT_RESPONSE_REPLACE:
                {
                    g_file_copy(src, dest, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, NULL);

                    g_debug ("response : Replace");
                    break;
                }
            default:
                break;
            }

            fileops_response_free (response);
            g_error_free(error);
        }
    }
}

JS_EXPORT_API
void dentry_copy_dereference_symlink(ArrayContainer fs, GFile* dest_dir)
{
    ArrayContainer _fs = _normalize_array_container(fs);
    GFile** _srcs = (GFile**)_fs.data;
    for (size_t i = 0; i < _fs.num; ++i) {
        char* src_basename = g_file_get_basename(_srcs[i]);
        GFile* dest = g_file_get_child(dest_dir, src_basename);
        g_free(src_basename);

        do_dereference_symlink_copy(_srcs[i], dest, G_FILE_COPY_NONE);
        char* path = g_file_get_path(dest);
        g_chmod(path, S_IRWXU | S_IROTH | S_IRGRP);
        g_free(path);

        g_object_unref(dest);
    }

    for (size_t i=0; i<_fs.num; i++) {
        g_object_unref(((GObject**)_fs.data)[i]);
    }
    g_free(_fs.data);
}

JS_EXPORT_API
void dentry_copy (ArrayContainer fs, GFile* dest)
{
    ArrayContainer _fs = _normalize_array_container(fs);
    /*fileops_copy (_fs.data, _fs.num, dest);*/
    files_copy_via_dbus (_fs.data, _fs.num, dest);
    for (size_t i=0; i<_fs.num; i++) {
        g_object_unref(((GObject**)_fs.data)[i]);
    }
    g_free(_fs.data);
}


JS_EXPORT_API
gboolean dentry_rename_move(GFile* src,char* new_name,GFile* dest,gboolean prompt)
{
    gboolean result = FALSE;
    if(dentry_set_name(src,new_name))
    {
        result =  fileops_move((gpointer)src,1,dest,prompt);
    }
    else
    {
        g_warning("rename failed!");
    }
    return result;
}


JS_EXPORT_API
void dentry_delete_files(ArrayContainer fs, gboolean show_dialog)
{
    ArrayContainer _fs = _normalize_array_container(fs);
    fileops_confirm_delete(_fs.data, _fs.num, show_dialog);
    for (size_t i=0; i<_fs.num; i++) {
        g_object_unref(((GObject**)_fs.data)[i]);
    }
    g_free(_fs.data);
}
JS_EXPORT_API
void dentry_trash(ArrayContainer fs)
{
    ArrayContainer _fs = _normalize_array_container(fs);
    fileops_trash (_fs.data, _fs.num);
    for (size_t i=0; i<_fs.num; i++) {
        g_object_unref(((GObject**)_fs.data)[i]);
    }
    g_free(_fs.data);
}


JS_EXPORT_API
void dentry_clipboard_copy(ArrayContainer fs)
{
    ArrayContainer _fs = _normalize_array_container(fs);
    init_fileops_clipboard (_fs.data, _fs.num, FALSE);
    for (size_t i=0; i<_fs.num; i++) {
        g_object_unref(((GObject**)_fs.data)[i]);
    }
    g_free(_fs.data);
}

JS_EXPORT_API
void dentry_clipboard_cut(ArrayContainer fs)
{
    ArrayContainer _fs = _normalize_array_container(fs);
    init_fileops_clipboard (_fs.data, _fs.num, TRUE);
    for (size_t i=0; i<_fs.num; i++) {
        g_object_unref(((GObject**)_fs.data)[i]);
    }
    g_free(_fs.data);
}

JS_EXPORT_API
void dentry_clipboard_paste(GFile* dest_dir)
{
    fileops_paste (dest_dir);
}

JS_EXPORT_API
gboolean dentry_can_paste ()
{
    return ! is_clipboard_empty();
}

JS_EXPORT_API
void dentry_confirm_trash()
{
    fileops_confirm_trash();
}

JS_EXPORT_API
GFile* dentry_get_trash_entry()
{
    return fileops_get_trash_entry ();
}

JS_EXPORT_API
double dentry_get_trash_count()
{
    return fileops_get_trash_count ();
}


void ArrayContainer_free0(ArrayContainer array)
{
    for(size_t i = 0 ; i < array.num ; i++)
    {
        g_object_unref(((GObject**)array.data)[i]);
    }
}
void ArrayContainer_free(ArrayContainer array)
{
    for(size_t i = 0 ; i < array.num ; i++)
    {
        g_object_unref(((GObject**)array.data)[i]);
    }
    g_free(array.data);
}
void g_message_boolean(gboolean b)
{
    (b == TRUE)?(g_message("TRUE")):(g_message("FALSE"));
}


JS_EXPORT_API
gboolean dentry_internal()
{
    char* dde_debug = g_build_filename(g_get_home_dir(), ".dde_debug", NULL);
    gboolean is_exist = g_file_test(dde_debug, G_FILE_TEST_EXISTS);
    g_free(dde_debug);

    return is_exist;
}


JS_EXPORT_API
char* dentry_get_basename_without_ext(char const* path)
{
    char* name = get_basename_without_extend_name(path);
    return name;
}


static
char* _get_svg_icon_aux(char const* icon)
{
    char* icon_name = get_basename_without_extend_name(icon);

    if (icon_name != NULL) {
        char* svg_path =icon_name_to_path_with_check_xpm(icon_name, -1);
        g_free(icon_name);

        if (svg_path != NULL && g_str_has_suffix(svg_path, ".svg")) {
            return svg_path;
        } else {
            g_free(svg_path);
            return NULL;
        }
    }

    return NULL;
}


static
char* _get_svg_icon(Entry* entry)
{
    char* svg_path = NULL;

    char* icon = dentry_get_icon(entry);

    if (icon[0] == '/') {
        svg_path = _get_svg_icon_aux(icon);

        if (svg_path == NULL) {
            char const* filename = g_desktop_app_info_get_filename(entry);
            svg_path = _get_svg_icon_aux(filename);
        }
    }

    g_free(icon);

    return svg_path;
}


static
void _bad_icon_copy(gpointer data, gpointer bad_icon_dir)
{
    GFile* file = (GFile*)data;
    char* dest_basename = g_file_get_basename(file);
    GFile* dest = g_file_get_child(bad_icon_dir, dest_basename);
    g_free(dest_basename);

    do_dereference_symlink_copy(data, dest, G_FILE_COPY_OVERWRITE);

    g_object_unref(dest);
}


JS_EXPORT_API
void dentry_report_bad_icon(Entry* entry)
{
    if (!dentry_internal())
        return;

    GPtrArray* srcs = g_ptr_array_new_with_free_func(g_object_unref);

    GFile* e = entry;
    if (G_IS_APP_INFO(entry))
        e = _get_gfile_from_gapp(entry);
    g_ptr_array_add(srcs, e);

    char* icon_path = dentry_get_icon_path(entry);
    g_ptr_array_add(srcs, g_file_new_for_path(icon_path));
    g_free(icon_path);


    char* svg_path = _get_svg_icon(entry);
    if (svg_path != NULL) {
        GFile* svg_file = g_file_new_for_path(svg_path);
        g_ptr_array_add(srcs, svg_file);
        g_free(svg_path);
    }

    char const* desktop_path = g_get_user_special_dir(G_USER_DIRECTORY_DESKTOP);
    char* bad_icon_dir_path = g_build_filename(desktop_path, "bad_icons", NULL);
    GFile* bad_icon_dir = g_file_new_for_path(bad_icon_dir_path);
    g_free(bad_icon_dir_path);

    if (!g_file_query_exists(bad_icon_dir, NULL))
        g_file_make_directory(bad_icon_dir, NULL, NULL);

    g_ptr_array_foreach(srcs, _bad_icon_copy, (gpointer)bad_icon_dir);

    g_ptr_array_free(srcs, TRUE);
    g_object_unref(bad_icon_dir);
}



JS_EXPORT_API
ArrayContainer dentry_get_templates_files(void)
{
    ArrayContainer ac;
    g_debug("templates dir:--%s--",TEMPLATES_DIR());
    gboolean is_exist = g_file_test(TEMPLATES_DIR(),G_FILE_TEST_EXISTS);
    if(is_exist)
    {
        if(0 == g_strcmp0(TEMPLATES_DIR(),HOME_DIR()))
        {
            g_debug("the templates directory is HOME_DIR,it isnt TEMPLATES_DIR");
            ac.data = NULL;
            ac.num = 0;
        }
        else
        {
            g_debug("the templates directory is exist!");
            GFile* f = g_file_new_for_path(TEMPLATES_DIR());
            ac = dentry_list_files(f);
            g_object_unref(f);
        }
    }
    else{
        g_debug("the templates directory isnot exist!");
        ac.data = NULL;
        ac.num = 0;
    }
    return ac ;
}

JS_EXPORT_API
ArrayContainer dentry_get_templates_filter(ArrayContainer fs)
{
    ArrayContainer _fs;
    GFile** files = NULL;
    GPtrArray* array = g_ptr_array_sized_new(1024);

    _fs = _normalize_array_container(fs);
    files = _fs.data;
    for(guint i=0; i<fs.num; i++)
    {
        GFile *f = files[i];
        GFileType type = g_file_query_file_type (f,G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL);
        if (type != G_FILE_TYPE_DIRECTORY){
            g_ptr_array_add(array, f);
        }
     }
    g_free(_fs.data);
    ArrayContainer ac;
    ac.num = array->len;
    ac.data = array->pdata;
    g_ptr_array_free(array, FALSE);

    return ac;
}

JS_EXPORT_API
GFile* dentry_create_templates(GFile* src, char* name_add_before)
{
    gboolean result = FALSE;
    char* basename = dentry_get_name(src);
    g_debug("choose templates name :---%s---",basename);

    GFile* dir = g_file_new_for_path(DESKTOP_DIR());


    char* name = g_strdup(basename);
    GFile* child = g_file_get_child(dir, name);
    for (int i=0; g_file_query_exists(child, NULL) && (i<500); i++) {
        g_object_unref(child);
        g_free(name);
        name = g_strdup_printf("%s(%d)%s",name_add_before, i,basename);
        child = g_file_get_child(dir, name);
    }

    g_debug("choose templates new name :---%s---",name);

    //we should check @src type  to use diff method.
    GFileType type = g_file_query_file_type (src, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL);
    if (type == G_FILE_TYPE_DIRECTORY)
    {
        result = g_file_make_directory (child, NULL, NULL);
        g_debug ("create_templates: new directory : g_file_make_directory : %s", name);
        ArrayContainer ac;
        ac = dentry_list_files(src);
        dentry_copy(ac,child);
        ArrayContainer_free(ac);
        result = TRUE;
    }
    else
    {
        result = g_file_copy(src, child, G_FILE_COPY_NONE, NULL, NULL, NULL, NULL);
        char* uri = dentry_get_uri(child);
        g_debug ("create_templates: new file : %s", uri);
        g_free(uri);
    }
    g_object_unref(dir);

    return child;
}


PRIVATE
gboolean _is_valid_category(char const* category)
{
#define CATEGORY_FILTER DATA_DIR"/category_filter.ini"
    GKeyFile* filter_file = g_key_file_new();
    GError* err = NULL;
    if (!g_key_file_load_from_file(filter_file, CATEGORY_FILTER, G_KEY_FILE_NONE, &err)) {
        g_warning("[Error] read file %s failed: %s", CATEGORY_FILTER, err->message);
        g_error_free(err);
        return TRUE;
    }

    gboolean is_valid = TRUE;
    gsize size = 0;
    char** filter = g_key_file_get_string_list(filter_file, "Main", "filter", &size, NULL);

    for (guint i = 0; i < size; ++i) {
        char* lowcase_filter = g_utf8_casefold(filter[i], -1);
        char* lowcase_category = g_utf8_casefold(category, -1);

        g_debug("compare #%s# with #%s#: %d", lowcase_category, lowcase_filter, 0 == g_strcmp0(lowcase_category, lowcase_filter));

        if (0 == g_strcmp0(lowcase_category, lowcase_filter)) {
            is_valid = FALSE;
            g_free(lowcase_category);
            g_free(lowcase_filter);
            break;
        }

        g_free(lowcase_category);
        g_free(lowcase_filter);
    }

    g_strfreev(filter);
    g_key_file_unref(filter_file);
#undef CATEGORY_FILTER

    return is_valid;
}


PRIVATE
gboolean _is_generic_category(char const* category)
{
#define CATEGORY_FILTER DATA_DIR"/category_filter.ini"
    GKeyFile* filter_file = g_key_file_new();
    GError* err = NULL;
    if (!g_key_file_load_from_file(filter_file, CATEGORY_FILTER, G_KEY_FILE_NONE, &err)) {
        g_warning("[Error] read file %s failed: %s", CATEGORY_FILTER, err->message);
        g_error_free(err);
        return TRUE;
    }

    gboolean is_generic = FALSE;
    gsize size = 0;
    char** filter = g_key_file_get_string_list(filter_file, "Aux", "filter", &size, NULL);
    for (int i = 0; filter[i] != NULL; ++i) {
        char* lowcase_category = g_utf8_casefold(category, -1);
        char* lowcase_filter = g_utf8_casefold(filter[i], -1);

        g_debug("compare #%s# with #%s#: %d", lowcase_filter, lowcase_category, 0 == g_strcmp0(lowcase_category, lowcase_filter));
        if (0 == g_strcmp0(lowcase_category, lowcase_filter)) {
            is_generic = TRUE;
            g_free(lowcase_category);
            g_free(lowcase_filter);
            break;
        }

        g_free(lowcase_category);
        g_free(lowcase_filter);
    }

    g_strfreev(filter);
    g_key_file_unref(filter_file);
#undef CATEGORY_FILTER

    return is_generic;
}


PRIVATE
char** _get_desktop_file_category(GDesktopAppInfo* f)
{
    const char* origin_categories = g_desktop_app_info_get_categories(f);

    if (origin_categories == NULL)
        return NULL;

    if (origin_categories[0] == '\0')
        return NULL;

    return g_strsplit(origin_categories, ";", 0);
}


PRIVATE
GHashTable* _count_categories(ArrayContainer const fs)
{
    GHashTable* categories_count = g_hash_table_new_full(g_str_hash,
                                                         g_str_equal,
                                                         g_free, NULL);

    for (guint i = 0; i < fs.num; ++i) {
        GHashTable* set = g_hash_table_new_full(g_str_hash, g_str_equal,
                                                g_free, NULL);
        char** categories =
            _get_desktop_file_category(((GDesktopAppInfo**)fs.data)[i]);

        g_debug("[%s] app: %s", __func__,
                g_desktop_app_info_get_filename(((GDesktopAppInfo**)fs.data)[i]));

        if (categories == NULL) {
            g_hash_table_unref(set);
            g_hash_table_unref(categories_count);
            return NULL;
        }

        for (int j = 0; categories[j] != NULL && categories[j][0] != '\0'; ++j) {
            char* low_case_category = g_utf8_casefold(categories[j], -1);

            if (g_hash_table_contains(set, low_case_category)) {
                g_free(low_case_category);
                continue;
            }

            // Do not free low_case_category explicitly, use g_hash_table_unref
            // to free low_case_category, so that do not duplicate it here.
            g_hash_table_add(set, low_case_category);

            if (_is_valid_category(low_case_category)) {
                g_debug("[%s] === insert category: %s (lowcase: %s)", __func__,
                        categories[j], low_case_category);

                int value =
                    GPOINTER_TO_INT(g_hash_table_lookup(categories_count,
                                                        low_case_category));
                g_hash_table_insert(categories_count,
                                    g_strdup(low_case_category),
                                    GINT_TO_POINTER(value + 1));
            }
        }

        g_strfreev(categories);
        g_hash_table_unref(set);
        g_debug("[%s] ===================", __func__);
    }

    return categories_count;
}


PRIVATE
void _get_cadidate_categories(gpointer key, gpointer value, gpointer user_data)
{
    ArrayContainer* fs = ((void**)user_data)[0];
    GPtrArray* candidate_categories = ((void**)user_data)[1];

    if ((guint)GPOINTER_TO_INT(value) == fs->num) {
        if (candidate_categories->len > 1) {
            if (!_is_generic_category(key))
                g_ptr_array_add(candidate_categories, g_strdup(key));
        } else {
            g_ptr_array_add(candidate_categories, g_strdup(key));
        }
    }
}


PRIVATE
char* _get_group_name_from_category_field(ArrayContainer const fs)
{
    char* group_name = g_strdup(_("App Group"));

    GHashTable* categories_count = _count_categories(fs);

    if (categories_count == NULL)
        return group_name;

    GPtrArray* candidate_categories = g_ptr_array_new_with_free_func(g_free);
    candidate_categories->len = 0;

    void* user_data[] = { (ArrayContainer*)(&fs), candidate_categories };
    g_hash_table_foreach(categories_count, _get_cadidate_categories, user_data);

    if (candidate_categories->len == 0)
        goto out;

    char* candidate = NULL;
    for (guint i = 0; candidate_categories->len > 1
         && i < candidate_categories->len; ++i) {
        if (!_is_generic_category(g_ptr_array_index(candidate_categories, i))) {
            g_free(candidate);
            candidate = g_strdup(g_ptr_array_index(candidate_categories, i));
        }
    }

    g_free(group_name);
    if (candidate == NULL)  // all candidate categories is generic category
        group_name = g_strdup(g_ptr_array_index(candidate_categories, 0));
    else
        group_name = candidate;

out:
    g_ptr_array_unref(candidate_categories);
    g_hash_table_unref(categories_count);

    return group_name;
}


PRIVATE
int _get_category_name(void* _basename, int argc, char** argv, char** columnname)
{
    char** basename = (char**)_basename;
    if (argv[0][0] != '\0')
        *basename = g_strdup(argv[0]);
    return 0;
}


PRIVATE
char* _lookup(char const* basename)
{
    char* category_name = NULL;

    GString* sql = g_string_new("select first_category_name from desktop where desktop_name = \"");
    g_string_append(sql, basename);
    g_string_append(sql, "\";");

    search_database(get_category_name_db_path(), sql->str, _get_category_name,
                    &category_name);

    g_string_free(sql, TRUE);

    return category_name;
}


PRIVATE
char* _get_category(GDesktopAppInfo* app)
{
    char* basename = get_basename_without_extend_name(g_desktop_app_info_get_filename(app));
    char* category = _lookup(basename);
    g_free(basename);

    if (category == NULL || category[0] == '\0')
        return NULL;

    return category;
}


PRIVATE
char* _get_group_name_from_software_center(ArrayContainer const fs)
{
    g_assert(fs.num > 1);

    GDesktopAppInfo** datas = (GDesktopAppInfo**)fs.data;
    char* category = _get_category(datas[0]);
    char* another_category = NULL;

    if (category == NULL)
        goto errorout;

    for (guint i = 1; i < fs.num; ++i) {
        g_free(another_category);
        another_category = _get_category(datas[i]);

        if (another_category == NULL)
            goto errorout;

        if (0 != g_strcmp0(category, another_category))
            goto errorout;
    }

    g_free(another_category);
    return category;

errorout:
    g_free(category);
    g_free(another_category);
    return NULL;
}


char* dentry_get_rich_dir_group_name(ArrayContainer const fs)
{
    char* group_name = _get_group_name_from_software_center(fs);

    if (group_name == NULL)
        group_name = _get_group_name_from_category_field(fs);

    return group_name;
}

JS_EXPORT_API
char* dentry_get_default_audio_player_name()
{
   GAppInfo* gappinfo = g_app_info_get_default_for_type("audio/mpeg",FALSE);
   const char* name = g_app_info_get_name(gappinfo);
   g_object_unref(gappinfo);
   return name;
}


JS_EXPORT_API
char* dentry_get_default_audio_player_icon()
{
   GAppInfo* gappinfo = g_app_info_get_default_for_type("audio/mpeg",FALSE);
   GIcon* icon = g_app_info_get_icon(gappinfo);
   gchar* icon_url = g_icon_to_string(icon);
   g_object_unref(icon);
   g_object_unref(gappinfo);
   return icon_url;
}
