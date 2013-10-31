
#ifndef _ENUMS_H_
#define _ENUMS_H_

//fileops.h
typedef enum _FileOpsFlags	FileOpsFlags;
enum _FileOpsFlags
{
    FILE_OPS_NONE,
    FILE_OPS_DELETE,
    FILE_OPS_TRASH,
    FILE_OPS_MOVE,
    FILE_OPS_COPY,
};
// fileops.c
// status returned by file operations
typedef enum _FileOpsStatus	FileOpsStatus;
//the following definition is in contradiction to Unix tradition
enum _FileOpsStatus
{
    FILE_OPS_STATUS_CANCELLED = 0,
    FILE_OPS_STATUS_SUCCESS   = 1
};
// fileops_clipboard.c
typedef enum _FileOpsClipboard	FileOpsClipboard;
enum _FileOpsClipboard
{
    FILE_OPS_CLIPBOARD_CUT =0, 
    FILE_OPS_CLIPBOARD_COPY = 1
};
//fileops_error_dialog.c
//NOTE:
//we need to handle file replace and
//directory merge in CONFLICT_RESPONSE_REPLACE
enum
{
    CONFLICT_RESPONSE_SKIP = 1,
    CONFLICT_RESPONSE_REPLACE = 2,
    CONFLICT_RESPONSE_RENAME = 3,
};
#endif
