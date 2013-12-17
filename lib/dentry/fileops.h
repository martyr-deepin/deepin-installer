#ifndef _FILEOPS_H_
#define _FILEOPS_H_

#include <glib.h>
#include <gio/gio.h>

typedef gboolean (*GFileProcessingFunc) (GFile* file, gpointer data);

void fileops_delete	(GFile* file_list[], guint num);
void fileops_trash	(GFile* file_list[], guint num);

// this paste operation is a wrapper around fileops_move and fileops_copy, but read its 
// parameters from clipboard. see fileops_clipboard.c/h
void fileops_paste	(GFile* dest_dir);

//@dest is a directory
gboolean fileops_move		(GFile* file_list[], guint num, GFile* dest_dir, gboolean prompt);
void fileops_copy		(GFile* file_list[], guint num, GFile* dest_dir);
void files_copy_via_dbus (GFile *file_list[], guint num, GFile *dest_dir);

#endif
