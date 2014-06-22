#include "hooks.h"

#include <glib.h>

#define HOOKS_PATH    RESOURCE_DIR"hooks"

void execute_hook(const gchar *hookname);

void run_hooks_before_chroot()
{
    //TODO:
    //extract /cdrom/casper/filesystem.squashfs
    //extract /cdrom/casper/overlay-deepin-${lang_pack}.squashfs
}

void run_hooks_in_chroot()
{
    //TODO:
    //setup accounts
    //setup locale
    //setup zoneinfo
    
    //DONE: fix pxe network
    //DONE: update-grub
    //DONE: remove-unused-packages
    execute_hook("install-bottom");
}

void run_hooks_after_chroot()
{
}

void execute_hook(const gchar *hookname)
{
    GError *error = NULL;
    const gchar *cmd = g_strdup_printf ("%s/%s", HOOKS_PATH,hookname);
    g_message("excute_scripts:cmd :%s.",cmd);
    g_spawn_command_line_sync (cmd, NULL, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("excute_scripts:excute failed:%s\n", error->message);
        g_error_free (error);
        error = NULL;
    }
}


