#ifndef _FILEOPS_ERROR_DIALOG_H_
#define _FILEOPS_ERROR_DIALOG_H_
// most of the code are copied from FileOps
#include <glib-object.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include "enums.h"
#include "xdg_misc.h"	// use char* lookup_icon_by_gicon(GIcon* icon);
#include "fileops_error_reporting.h"//for FileOpsResponse


typedef struct _FileOpsFileConflictDialogDetails FileOpsFileConflictDialogDetails;

GtkWidget* fileops_error_conflict_dialog_new (GtkWindow* parent, GFile* src, 
	                                      GFile* dest, FileOpsResponse* response);
#endif
