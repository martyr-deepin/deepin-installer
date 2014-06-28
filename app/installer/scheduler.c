#include "scheduler.h"
#include "jsextension.h"
#include "info.h"

#include "esp.h"
#include <glib.h>

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

JS_EXPORT_API
void installer_start_install()
{
    if (InstallerConf.simple_mode && InstallerConf.uefi) {
	auto_handle_esp();
    }
    write_installer_conf("/etc/deepin-installer.conf");
    enter_next_stage();
}


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
