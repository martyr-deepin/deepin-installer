

#ifndef _FILEOPS_ERROR_REPORTING_H_
#define _FILEOPS_ERROR_REPORTING_H_

#include <gtk/gtk.h>
#include "enums.h"

typedef struct _FileOpsResponse FileOpsResponse;
struct _FileOpsResponse
{
    gint   response_id;

    char*  file_name;  //this is the renamed name
    gboolean apply_to_all;
};

FileOpsResponse* fileops_response_dup (FileOpsResponse* response);
void             fileops_response_free (FileOpsResponse* response);

//FileOpsResponse fileops_error_show_dialog (GError* error);
//users should free FileOpsResponse
FileOpsResponse* fileops_delete_trash_error_show_dialog (const char* fileops_str, GError* error, 
							GFile* file, GtkWindow* parent);
FileOpsResponse* fileops_move_copy_error_show_dialog (const char* fileops_str, GError* error, 
	                                             GFile* src, GFile* dest, GtkWindow* parent);

#endif
