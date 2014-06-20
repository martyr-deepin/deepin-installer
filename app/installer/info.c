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

    extern const gchar *target;
    char* path= g_strdup_printf("%s/host/config.json", target);
    GError* error = NULL;
    g_file_set_contents(path, content, -1, &error);
    g_free(path);

    if (error != NULL) {
	g_warning("Write_bootloader_info failed:%s\n", error->message);
	g_error_free(error);
    }
}
