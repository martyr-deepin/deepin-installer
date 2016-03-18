#include "scheduler.h"
#include "jsextension.h"
#include "info.h"
#include "fs_util.h"
#include "esp.h"

#include <string.h>
#include <glib.h>
#include <gio/gio.h>

#define CONF_PATH "/etc/deepin-installer.conf"

extern char* auto_conf_path;

void run_hooks_before_chroot();
void run_hooks_in_chroot();
void run_hooks_after_chroot();

enum {
    STAGE_START_INSTALL,
    STAGE_HOOKS_BEFORE_CHROOT,
    STAGE_HOOKS_IN_CHROOT,
    STAGE_HOOKS_AFTER_CHROOT,
    STAGE_INSTALL_FINISH,
};

void update_install_progress(int v)
{
    static int current_per = 0;
    if (v < current_per) {
        g_debug("[%s] INSTALL progress small previous PROGRESS: %d <= %d\n",
                __func__, v, current_per);
        return;
    }
    current_per=  v;
    js_post_message("install_progress",
                    jsvalue_from_number(get_global_context(), v));
}

void installer_terminate()
{
    g_message("[%s]\n", __func__);
    js_post_message("install_terminate", NULL);
}

void enter_next_stage()
{
    static int current_stage = STAGE_START_INSTALL;
    g_message("[%s], current_stage: %d\n", __func__, current_stage);

    switch (current_stage) {
    case STAGE_START_INSTALL:
        current_stage = STAGE_HOOKS_BEFORE_CHROOT;
        run_hooks_before_chroot();
        break;

    case STAGE_HOOKS_BEFORE_CHROOT:
        current_stage = STAGE_HOOKS_IN_CHROOT;
        run_hooks_in_chroot();
        break;

    case STAGE_HOOKS_IN_CHROOT:
        current_stage = STAGE_HOOKS_AFTER_CHROOT;
        run_hooks_after_chroot();
        break;

    case STAGE_HOOKS_AFTER_CHROOT:
        current_stage = STAGE_INSTALL_FINISH;
        update_install_progress(100);
        break;

    default:
        g_assert_not_reached();
    }
}

static GHashTable* mkfs_pending_list = NULL;

void mkfs_latter(const char* path, const char* fs)
{
    g_message("[%s], path: %s, fs: %s\n", __func__, path, fs);
    g_return_if_fail(path != NULL);
    g_return_if_fail(fs != NULL);
    if (mkfs_pending_list == NULL) {
        mkfs_pending_list = g_hash_table_new_full(g_str_hash, g_str_equal,
                                                  g_free, g_free);
    }
    g_hash_table_insert(mkfs_pending_list, g_strdup(path), g_strdup(fs));
}

static void do_mkfs()
{
    g_message("[%s]\n", __func__);
    g_assert(mkfs_pending_list != NULL);
    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, mkfs_pending_list);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        mkfs((char*)key, (char*)value);
    }
    g_hash_table_destroy(mkfs_pending_list);
}

void start_run_installer()
{
    g_message("[%s]\n", __func__);
    ped_device_free_all();
    enter_next_stage();
}

// Read lfs-atm.conf. If failed, returns FALSE.
static gboolean read_lfs_atm_template() {
  g_message("[%s]\n", __func__);
  g_message("auto_conf_path: %s\n", auto_conf_path);
  if (auto_conf_path == NULL) {
    return false;
  }

  GError* error = NULL;
  GKeyFile* key_file = g_key_file_new();

  g_key_file_load_from_file(key_file, auto_conf_path, G_KEY_FILE_NONE, &error);
  if (error != NULL) {
    g_warning("[%s], g_key_file_new() failed: %s\n",
              __func__, error->message);
    g_error_free(error);
    error = NULL;
    return FALSE;
  } else {
    g_message("read key file ok\n");
  }

  const char username_key[] = "DI_USERNAME";
  const char password_key[] = "DI_PASSWORD";
  const char hostname_key[] = "DI_HOSTNAME";
  const char group_name[] = "default";

  char* username = NULL;
  char* hostname = NULL;
  char* password = NULL;
  username = g_key_file_get_value(key_file, group_name,
                                  username_key, &error);
  if (error != NULL) {
    g_warning("[%s], g_key_file_new() failed: %s\n",
              __func__, error->message);
    g_error_free(error);
    error = NULL;
    return 1;
  }
  hostname = g_key_file_get_value(key_file, group_name,
                                  hostname_key, &error);
  if (error != NULL) {
    g_warning("[%s], g_key_file_new() failed: %s\n",
              __func__, error->message);
    g_error_free(error);
    error = NULL;
    return 1;
  }
  password = g_key_file_get_value(key_file, group_name,
                                  password_key, &error);
  if (error != NULL) {
    g_warning("[%s], g_key_file_new() failed: %s\n",
              __func__, error->message);
    g_error_free(error);
    error = NULL;
    return 1;
  }
  InstallerConf.password = g_strdup(password);
  InstallerConf.user_name = g_strdup(username);
  InstallerConf.host_name = g_strdup(hostname);
  g_free(username);
  g_free(hostname);
  g_free(password);

  const char locale_key[] = "DI_LOCALE";
  char* locale = NULL;
  locale  = g_key_file_get_value(key_file, group_name, locale_key, &error);
  if (error != NULL) {
    g_warning("[%s], g_key_file_new() failed: %s\n",
              __func__, error->message);
    g_error_free(error);
    error = NULL;
    return 1;
  }
  InstallerConf.locale = g_strdup(locale);
  g_free(locale);

  const char timezone_key[] = "DI_TIMEZONE";
  char* timezone = NULL;
  timezone = g_key_file_get_value(key_file, group_name,
                                  timezone_key, &error);
  if (error != NULL) {
    g_warning("[%s], g_key_file_new() failed: %s\n",
              __func__, error->message);
    g_error_free(error);
    error = NULL;
    return 1;
  }
  InstallerConf.timezone = g_strdup(timezone);
  g_free(timezone);

  const char layout_key[] = "DI_LAYOUT";
  const char layout_variant_key[] = "DI_LAYOUT_VARIANT";
  gchar* layout = NULL;
  gchar* layout_variant = NULL;
  layout = g_key_file_get_value(key_file, group_name,
                                layout_key, &error);
  if (error != NULL) {
    g_warning("[%s], g_key_file_new() failed: %s\n",
              __func__, error->message);
    g_error_free(error);
    error = NULL;
    return 1;
  }
  layout_variant = g_key_file_get_value(key_file, group_name,
                                        layout_variant_key, &error);
  if (error != NULL) {
    g_warning("[%s], g_key_file_new() failed: %s\n",
              __func__, error->message);
    g_error_free(error);
    error = NULL;
    return 1;
  }
  InstallerConf.layout = g_strdup(layout);
  InstallerConf.layout_variant = g_strdup(layout_variant);
  g_free(layout);
  g_free(layout_variant);

  g_key_file_free(key_file);
  return TRUE;
}

static void start_prepare_conf()
{
    g_message("[%s]\n", __func__);
    if (InstallerConf.simple_mode && InstallerConf.uefi) {
        auto_handle_esp();
    }
    if (!read_lfs_atm_template()) {
       installer_terminate();
    }
    write_installer_conf(CONF_PATH);
    start_run_installer();
}

JS_EXPORT_API
void installer_start_install()
{
    g_message("[%s]\n", __func__);
    if (mkfs_pending_list != NULL) {
        GTask* task = g_task_new(NULL, NULL, start_prepare_conf, NULL);
        g_task_run_in_thread(task, do_mkfs);
    } else {
        start_prepare_conf();
    }
}
