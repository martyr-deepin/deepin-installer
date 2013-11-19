#ifndef _FILEOPS_CLIPBOARD_H
#define _FILEOPS_CLIPBOARD_H

#include "fileops.h"
// this is mainly used by move, copy to temporarily
// store file paths.

typedef struct _FileOpsClipboardInfo FileOpsClipboardInfo;
struct _FileOpsClipboardInfo
{
	GFile**   file_list;	    // list of GFiles
	guint	  num;
	gboolean  cut;       //TRUE, cut; FALSE, copy
};

void fileops_paste	    (GFile* dest_dir);
void init_fileops_clipboard (GFile* file_list[], guint num, gboolean cut);
//a function to return status
gboolean  is_clipboard_empty ();

#endif
