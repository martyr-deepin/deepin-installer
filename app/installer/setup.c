#include "setup.h"

#include <glib.h>


void setup_mount_point()
{
    //bind the root dir to /target/host
    //bind /cdrom to /target/media/cdrom
    
    g_mkdir_with_parents ("/target/host", 0755);
    g_spawn_command_line_sync ("mount --bind / /target/host", NULL, NULL, NULL, NULL);

    g_mkdir_with_parents ("/target/media/cdrom", 0755);
    g_spawn_command_line_sync ("mount --bind /cdrom /target/media/cdrom", NULL, NULL, NULL, NULL);
}

