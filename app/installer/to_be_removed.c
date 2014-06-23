#include "jsextension.h"
#include <glib.h>

#include "misc.h"

JS_EXPORT_API 
void installer_update_bootloader (const gchar *uuid, gboolean uefi)
{
    finish_install_cleanup ();
    emit_progress ("bootloader", "finish");
}
