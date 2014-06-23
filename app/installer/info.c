#include "info.h"
#include <glib.h>
#include "jsextension.h"
#include "part_util.h"

static struct _InstallerConf {
    char* bootloader;
    gboolean uefi;

    char* user_name;
    char* password;
    char* host_name;

    GHashTable* mount_points;
	
} InstallerConf;

char* find_path_by_uuid(const char* uuid)
{
    if (g_str_has_prefix (uuid, "disk")) {
	return installer_get_disk_path (uuid);
    } else if (g_str_has_prefix (uuid, "part")) {
	return installer_get_partition_path (uuid);
    } else {
	g_error("update grub:invalid uuid %s\n", uuid);
	return NULL;
    }
}


char* installer_conf_to_string()
{
    GString* mp = g_string_new(NULL);

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, InstallerConf.mount_points);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
	g_string_append_printf(mp, "%s=%s;", (gchar*)key, (gchar*)value);
    }

    return g_strdup_printf("\n"
	    "GRUB_TARGET=\"%s\"\n"
	    "DI_UEFI=\"%s\"\n"
	    "DI_USER=\"%s\"\n"
	    "DI_PASSWORD=\"%s\"\n"
	    "DI_HOSTNAME=\"%s\"\n"
	    "DI_MOUNTPOINTS=\"%s\"\n",
	    InstallerConf.bootloader,
	    InstallerConf.uefi ? "true" : "false",
	    InstallerConf.user_name,
	    InstallerConf.password,
	    InstallerConf.host_name,
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
	g_warning("Write_bootloader_info(to %s) failed:%s\n", path, error->message);
	g_error_free(error);
    }
}

void write_bootloader_info(const char* target, gboolean uefi)
{
    write_installer_conf("/etc/deepin-installer.conf");
}

JS_EXPORT_API
void installer_record_mountpoint_info(const char* part, const char* mountpoint)
{
    if (InstallerConf.mount_points == NULL) {
	InstallerConf.mount_points = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    }
    g_hash_table_insert(InstallerConf.mount_points, g_strdup(part), g_strdup(mountpoint));
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

    InstallerConf.bootloader = g_strdup(uuid);
    InstallerConf.uefi = uefi;
}