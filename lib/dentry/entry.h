#ifndef __DENTRY_H__

#include <glib.h>
#include <gio/gio.h>
#include <jsextension.h>

typedef void Entry;
#define DEEPIN_RICH_DIR ".deepin_rich_dir_"
#define DEEPIN_RICH_DIR_LEN 17

gboolean dentry_launch(Entry* e, const ArrayContainer fs);
Entry* dentry_create_by_path(const char* path);

gboolean dentry_is_fileroller_exist();
double dentry_files_compressibility(const ArrayContainer fs);
void dentry_compress_files(const ArrayContainer fs);
void dentry_decompress_files(const ArrayContainer fs);
void dentry_decompress_files_here(const ArrayContainer fs);

gboolean dentry_set_name(Entry* e, const char* name);
char* calc_id(const char* uri);
char* dentry_get_id(Entry* e);
ArrayContainer dentry_list_files(GFile* f);
char* dentry_get_icon(Entry* e);
char* dentry_get_icon_path(Entry* e);
gboolean dentry_move(ArrayContainer fs, GFile* dest, gboolean prompt);

void ArrayContainer_free(ArrayContainer array);
void ArrayContainer_free0(ArrayContainer array);
void g_message_boolean(gboolean b);

gboolean dentry_is_gapp(Entry* e);
ArrayContainer dentry_get_templates_files(void);
gboolean dentry_rename_move(GFile* src,char* new_name,GFile* dest,gboolean prompt);
GFile* dentry_create_templates(GFile* src, char* name_add_before);
char* dentry_get_rich_dir_group_name(ArrayContainer const _fs);
void do_dereference_symlink_copy(GFile* src, GFile* dest, GFileCopyFlags copy_flag);
#endif
