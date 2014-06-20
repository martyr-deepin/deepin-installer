#include "info.h"
#include <glib.h>
void write_bootloader_info(const char* target, gboolean uefi)
{
    char* content = g_strdup_printf("\n"
	    "GRUB_TARGET=\"%s\"\n",
	    target);

    GError* error = NULL;
    g_file_set_contents("/etc/deepin-installer.conf", content, -1, &error);

    if (error != NULL) {
	g_warning("Write_bootloader_info failed:%s\n", error->message);
	g_error_free(error);
    }
}


