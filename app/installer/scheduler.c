#include "scheduler.h"
#include "jsextension.h"
#include "info.h"
#include "fs_util.h"
#include "esp.h"

#include <glib.h>
#include <gio/gio.h>


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
	g_warning("--------------------------------INSTALL progress small previous PROGRESS: %d%% <= %d%%\n", v, current_per);
	return;
    }
    current_per=  v;
    js_post_message("install_progress", jsvalue_from_number(get_global_context(), v));
}


void enter_next_stage()
{
    static int current_stage = STAGE_START_INSTALL;

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

static GHashTable* mkfs_list = NULL;

void mkfs_latter(const char* path, const char* fs)
{
    g_return_if_fail(path != NULL);
    g_return_if_fail(fs != NULL);
    if (mkfs_list == NULL) {
	mkfs_list = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    }
    g_hash_table_insert(mkfs_list, g_strdup(path), g_strdup(fs));
}

static void do_mkfs()
{
    g_assert(mkfs_list != NULL);
    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, mkfs_list);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
	mkfs((char*)key, (char*)value);
    }
    g_hash_table_destroy(mkfs_list);
}
static void start_run_installer()
{
    ped_device_free_all();

    enter_next_stage();
}
static void start_prepare_conf()
{
    if (InstallerConf.simple_mode && InstallerConf.uefi) {
	auto_handle_esp();
    }
    write_installer_conf("/etc/deepin-installer.conf");

    start_run_installer();
}
JS_EXPORT_API
void installer_start_install()
{
    GTask* task = g_task_new(NULL, NULL, start_prepare_conf, NULL);
    g_task_run_in_thread(task, do_mkfs);
}

