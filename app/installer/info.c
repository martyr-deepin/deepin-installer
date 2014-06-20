#include <glib.h>
void write_bootloader_info(const char* uuid, gboolean uefi)
{
    char* content = g_strdup_printf("{\"bootloader\": {\n"
	    "\t\"type\": \"%s\"\n"
	    "\t\"uuid\": \"%s\"\n"
	    "\t}\n"
	    "}\n", 
	    uefi ? "uefi" : "bios",
	    uuid);
    printf("%s\n", content);
}
