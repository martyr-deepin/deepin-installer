#include "info.h"
#include <glib.h>

#define JS_EXPORT_API

static struct _InstallerConf {
    char* bootloader;
    gboolean uefi;

    char* user_name;
    char* password;
    char* host_name;

    char* locale;

    char* layout;
    char* layout_variant;

    char* timezone;

    GHashTable* mount_points;
	
} InstallerConf;

char* find_path_by_uuid(const char* uuid)
{
    return "hehe";
    //TODO:
    /*if (g_str_has_prefix (uuid, "disk")) {*/
	/*return installer_get_disk_path (uuid);*/
    /*} else if (g_str_has_prefix (uuid, "part")) {*/
	/*return installer_get_partition_path (uuid);*/
    /*} else {*/
	/*g_error("update grub:invalid uuid %s\n", uuid);*/
	/*return NULL;*/
    /*}*/
}


char* installer_conf_to_string()
{
    GString* mp = g_string_new(NULL);

    GHashTableIter iter;
    gpointer key, value;
    if (InstallerConf.mount_points != NULL) {
	g_hash_table_iter_init(&iter, InstallerConf.mount_points);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
	    g_string_append_printf(mp, "%s=%s;", (gchar*)key, (gchar*)value);
	}
    }

    return g_strdup_printf("\n"
	    "GRUB_TARGET=\"%s\"\n"
	    "DI_UEFI=\"%s\"\n"
	    "DI_USER=\"%s\"\n"
	    "DI_PASSWORD=\"%s\"\n"
	    "DI_HOSTNAME=\"%s\"\n"
	    "DI_TIMEZONE=\"%s\"\n"
	    "DI_LOCALE=\"%s\"\n"
	    "DI_LAYOUT=\"%s\"\n"
	    "DI_LAYOUT_VARIANT=\"%s\"\n"
	    "DI_MOUNTPOINTS=\"%s\"\n",
	    InstallerConf.bootloader,
	    InstallerConf.uefi ? "true" : "false",
	    InstallerConf.user_name,
	    InstallerConf.password,
	    InstallerConf.host_name,
	    InstallerConf.timezone ? : "",
	    InstallerConf.locale ? : "",
	    InstallerConf.layout ? : "",
	    InstallerConf.layout_variant ? : "",
	    g_string_free(mp, FALSE)
	    );
}

void write_installer_conf(const char* path)
{
    char* content = installer_conf_to_string();
    GError* error = NULL;
    g_file_set_contents(path, content, -1, &error);
    g_free(content);

    if (error != NULL) {
	g_warning("write deepin-installer.conf(to %s) failed:%s\n", path, error->message);
	g_error_free(error);
    }
}

JS_EXPORT_API
void installer_record_mountpoint_info(const char* part, const char* mountpoint)
{
    if (InstallerConf.mount_points == NULL) {
	InstallerConf.mount_points = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    }
    g_hash_table_insert(InstallerConf.mount_points, find_path_by_uuid(part), g_strdup(mountpoint));
}

JS_EXPORT_API
void installer_record_accounts_info(const char* name, const char* hostname, const char* password)
{
    if(InstallerConf.user_name)
	g_free(InstallerConf.user_name);
    if(InstallerConf.password)
	g_free(InstallerConf.password);
    if(InstallerConf.host_name)
	g_free(InstallerConf.host_name);

    InstallerConf.user_name = g_strdup(name);
    InstallerConf.password = g_strdup(password);
    InstallerConf.host_name = g_strdup(hostname);
}

JS_EXPORT_API
void installer_record_bootloader_info(const char* uuid, gboolean uefi)
{
    if (InstallerConf.bootloader)
	g_free(InstallerConf.bootloader);

    InstallerConf.bootloader = find_path_by_uuid(uuid);
    InstallerConf.uefi = uefi;
}

JS_EXPORT_API
void installer_record_locale_info(const char* locale)
{
    if (InstallerConf.locale)
	g_free(InstallerConf.locale);

    InstallerConf.locale = g_strdup(locale);
}

JS_EXPORT_API
void installer_record_timezone_info(const char* timezone)
{
    if (InstallerConf.timezone)
	g_free(InstallerConf.timezone);

    InstallerConf.timezone = g_strdup(timezone);
}

JS_EXPORT_API
void installer_record_keyboard_layout_info(const char* layout, const char* variant)
{
    if (InstallerConf.layout)
	g_free(InstallerConf.layout);
    if (InstallerConf.layout_variant)
	g_free(InstallerConf.layout_variant);

    InstallerConf.layout = g_strdup(layout);
    InstallerConf.layout_variant = g_strdup(variant);
}
