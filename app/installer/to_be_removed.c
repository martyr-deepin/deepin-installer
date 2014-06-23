#include "jsextension.h"
#include <glib.h>

#include "misc.h"
#include "part_util.h"
#include "hooks.h"

JS_EXPORT_API 
void installer_update_bootloader (const gchar *uuid, gboolean uefi)
{
    //TODO: move to corret place
    
    run_hooks_in_chroot();
}
