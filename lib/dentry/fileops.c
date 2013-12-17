/*
 *      standard GIO implementation doesn't directly support
 *      traversing the filesystem hierachy.
 *      so we need to implement one.
 */
#include <glib/gstdio.h>
#include <glib/gi18n.h>

#include "fileops.h"
#include "fileops_error_reporting.h"

#define DBUS_NAUTILUS_NAME  "org.gnome.Nautilus"
#define DBUS_NAUTILUS_PATH  "/org/gnome/Nautilus"
#define DBUS_NAUTILUS_INFACE    "org.gnome.Nautilus.FileOperations"

#define DBUS_COPY_METHOD    "CopyURIs"


static gboolean _dummy_func             (GFile* file, gpointer data);

static gboolean _delete_files_async     (GFile* file, gpointer data);
static gboolean _trash_files_async      (GFile* file, gpointer data);
static gboolean _move_files_async       (GFile* file, gpointer data);
static gboolean _copy_files_async       (GFile* file, gpointer data);

static void dbus_call_method_cb (GObject *source_object,
        GAsyncResult *res, gpointer user_data);
static void call_method_via_dbus (const GVariantBuilder *builder,
        const gchar *dest_uri);


/*
 *      @dir    : file or directory to traverse
 *      @pre_hook: pre-processing function, this used in move and copy
 *      @post_hook: post-processing function, this used in delete and trash.
 *      @data   : data passed to callback function.
 *                currently we only use this as GFile* which is the fileops destination
 *                corresponding to @dir. for each recursive level, we should update
 *                data to ensure that @dir and @data are consistent.
 *
 *      NOTE: 1.if dir is a file, applying callback and return.
 *              if dir is a directory, traversing the directory tree
 *            2.we don't follow symbol links.
 *            3.there's a race condition in checking @dir type before
 *              enumerating @dir. so we don't check @dir type.
 *              if @dir is a file, we handle it in G_IO_ERROR_NOT_DIRECTORY.
 *            4. (move, copy) and (delete, trash) behave differently.
 *               (move, copy) first create the directory then create files in the directory
 *               (delete, trash) first delete files in the directory then delete the directory
 *               so we need a pre_hook and post_hook separately.
 *
 *
 *      TODO: change "standard::*" to the attributes we actually needed.
 */
//data used by Traverse Directory (TD)
typedef struct _TDData TDData;
struct _TDData
{
    GFile*       dest_file;
    GCancellable* cancellable;
};
//deep copy
static TDData* new_td_data ()
{
    TDData* new_tddata = NULL;
    new_tddata = g_malloc0 (sizeof (TDData));
    return new_tddata;
}
static void free_td_data (TDData* tddata)
{
    g_free (tddata);
}

// src ---> data->dest
// we make use of 'goto' to minimize duplicated 'g_free*' statement
gboolean
traverse_directory (GFile* src, GFileProcessingFunc pre_hook, GFileProcessingFunc post_hook, gpointer data)
{
    gboolean retval = TRUE;

    GError* error = NULL;
    GFileEnumerator* src_enumerator = NULL;

    src_enumerator = g_file_enumerate_children (src,
                                               "standard::name",
                                               G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                               NULL,
                                               &error);
    if (error != NULL)
    {
        //src_enumerator must be NULL, nothing to free.
        switch (error->code)
        {
            case G_IO_ERROR_NOT_FOUND:
                //TODO: showup a message box and quit.
                if (g_file_query_file_type (src, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL) == G_FILE_TYPE_REGULAR)
                {
                    //for smb files, previous enumeration can fail and return G_IO_ERROR_NOT_FOUND.
                    //pass to G_IO_ERROR_NOT_DIRECTORY
                }
                else
                {
                    g_debug("G_IO_ERROR_NOT_FOUND");
                    break;
                }
            case G_IO_ERROR_NOT_DIRECTORY:
                //TODO:we're using a file.
                // g_debug("G_IO_ERROR_NOT_DIRECTORY");

                if (pre_hook (src, data) == FALSE ||
                    post_hook (src, data) == FALSE)
                {
                    g_error_free (error);
                    return FALSE;
                }
                else
                {
                    g_error_free (error);
                    return TRUE;
                }
            default:
                g_debug("src found and is directory");
                break;
        }
        // g_warning ("traverse_directory 1: %s", error->message);
        g_error_free (error);

        return TRUE;
    }

    //here, we must be in a directory.
    //check if it's a symbolic link
    if (pre_hook (src, data) == FALSE) //src_enumerator must be freed
    {
        retval = FALSE;
        goto post_processing;
    }
#if 1
    char* src_uri = NULL;

    src_uri = g_file_get_uri (src);
    g_debug ("traverse_directory: chdir to : %s", src_uri);
#endif

    //begin g_file_enumerator_next_file ,we must check if the file type is symbolic_link then goto post_processing:
    GFileType type = g_file_query_file_type (src, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL);
    if (type == G_FILE_TYPE_SYMBOLIC_LINK)
    {
        //TODO: symbolic links
        g_debug ("-------src type is symbolic_link-------");
        goto post_processing;
    }

    GFileInfo* file_info = NULL;
    while ((file_info = g_file_enumerator_next_file (src_enumerator, NULL, &error)) != NULL)
    {
        //this should not be freed with g_free(). it'll be freed when we call g_object_unref
        //on file_info
#if 1
        const char* src_child_name = g_file_info_get_name (file_info);
        g_debug ("traverse_directory: %s", src_child_name);
#endif

        TDData* tddata = NULL;
        GFile* src_child_file = NULL;
        GFile* dest_dir = NULL;   //corresponding to src
        GFile* dest_child_file = NULL;  //corresponding to src_child_file.

        tddata = new_td_data ();
        src_child_file = g_file_get_child (src, src_child_name);
        dest_dir = ((TDData*)data)->dest_file;

        if (dest_dir != NULL)
        {
            dest_child_file = g_file_get_child (dest_dir, src_child_name);
#if 1
            char* dest_child_file_uri = g_file_get_uri (dest_child_file);
            g_debug ("dest_child_file_uri: %s", dest_child_file_uri);
            g_free (dest_child_file_uri);
#endif
        }

        tddata->dest_file = dest_child_file;
        tddata->cancellable = ((TDData*)data)->cancellable;
        //TODO:
        //get out the loop recursively when operation is cancelled.
        retval = traverse_directory (src_child_file, pre_hook, post_hook, tddata);

        g_object_unref (src_child_file);
        free_td_data (tddata);

        g_object_unref (file_info);
        file_info = NULL;

        if (retval == FALSE)
            goto post_processing;
    }
    //checking errors
    if (error != NULL)
    {
        g_warning ("traverse_directory 2: %s", error->message);
        g_error_free (error);
    }

#if 1
    //change to parent directory.
    g_debug ("traverse_directory: come out: %s", src_uri);
    g_free (src_uri);
#endif

post_processing:
    //close enumerator.
    g_file_enumerator_close (src_enumerator, NULL, &error);
    g_object_unref (src_enumerator);
    //checking errors
    if (error != NULL)
    {
        g_warning ("traverse_directory 3: %s", error->message);
        g_error_free (error);
    }

    //after processing child node. processing this directory.
    if (post_hook (src, data) == FALSE)
        return FALSE;

    return retval;
}

/*
 *      @file_list : files(or directories) to delete.
 *      @num       : number of files(or directories) in file_list
 *      pre_hook =NULL
 *      post_hook = _delete_files_async
 */
void
fileops_delete (GFile* file_list[], guint num)
{
    g_debug ("fileops_delete: Begin deleting files");

    GCancellable* delete_cancellable = g_cancellable_new ();
    TDData* data = g_malloc0 (sizeof (TDData));
    data->dest_file = NULL;
    data->cancellable = delete_cancellable;

    for (guint i = 0; i < num; i++)
    {
        GFile* src = file_list[i];
#if 1
        char* src_uri = g_file_get_uri (src);
        g_debug ("fileops_delete: file %d: %s", i, src_uri);
        g_free (src_uri);
#endif

        traverse_directory (src, _dummy_func, _delete_files_async, data);
    }
    g_object_unref (data->cancellable);
    g_free (data);
    g_debug ("fileops_delete: End deleting files");
}
/*
 *      @file_list : files(or directories) to trash.
 *      @num       : number of files(or directories) in file_list
 *      NOTE: trashing is special because we don't need to
 *            traverse_directory. the default implementation can
 *            recursively trash files.
 */
void
fileops_trash (GFile* file_list[], guint num)
{
    g_debug ("fileops_trash: Begin trashing files");

    GCancellable* trash_cancellable = g_cancellable_new ();
    TDData* data = g_malloc0 (sizeof (TDData));
    data->dest_file = NULL;
    data->cancellable = trash_cancellable;

    for (guint i = 0; i < num; i++)
    {
        GFile* src = file_list[i];
    #if 1
        char* src_uri = g_file_get_uri (src);
        g_debug ("fileops_trash: file %d: %s", i, src_uri);
        g_free (src_uri);
    #endif

        _trash_files_async (src, data);
        // traverse_dirsectory (src, _dummy_func, _trash_files_async, data);
    }
    g_object_unref (data->cancellable);
    g_free (data);
    g_debug ("fileops_trash: End trashing files");
}
/*
 *      @file_list : files(or directories) to move.
 *      @num       : number of files(or directories) in file_list
 *      @dest      : destination directory.
 *
 *      NOTE: moving is special because we don't need to
 *            traverse_directory. the default implementation can
 *            recursively trash files.
 */
static gboolean g_prompt = FALSE; //add a global to retain _move_files_async signature
static FileOpsResponse* g_move_response = NULL; //keep track of user-specified action (applied to all).
gboolean
fileops_move (GFile* file_list[], guint num, GFile* dest_dir, gboolean prompt)
{
    g_prompt = prompt;
    g_move_response = NULL;

    gboolean retval = TRUE;
    g_debug ("fileops_move: Begin moving files");

    GCancellable* move_cancellable = g_cancellable_new ();
    TDData* data = g_malloc0 (sizeof (TDData));
    data->cancellable = move_cancellable;

    for (guint i = 0; i < num; i++)
    {
        GFile* src = file_list[i];
    #if 1
        char* src_uri = g_file_get_uri (src);
        char* dest_dir_uri = g_file_get_uri (dest_dir);
        g_debug ("fileops_move: file %d: %s to dest: %s", i, src_uri, dest_dir_uri);

        g_free (src_uri);
        g_free (dest_dir_uri);
    #endif
        //make sure dest_dir is a directory before proceeding.
        GFileType type = g_file_query_file_type (dest_dir, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL);
        if (type != G_FILE_TYPE_DIRECTORY)
        {
            //TODO: symbolic links
            g_debug ("dest type is not directory");

            return FALSE;
        }
        char* src_basename= g_file_get_basename (src);
        GFile* move_dest_file = g_file_get_child (dest_dir, src_basename);
        g_free (src_basename);

        data->dest_file = move_dest_file;

        //retval &= _move_files_async (src, data);
        //traverse_directory (dir, _move_files_async, _dummy_func, move_dest_gfile);
        retval &= traverse_directory (src, _move_files_async, _dummy_func, data);
        // here i must check out if dest has the same file or directory ,if true , fileops_delete,else, nothing do
        if (retval)
            fileops_delete (&src, 1);//ensure original file is removed.

        g_object_unref (move_dest_file);
    }
    g_object_unref (data->cancellable);
    g_free (data);

    fileops_response_free (g_move_response);
    g_debug ("fileops_move: End moving files");

    return retval;
}
/*
 *      @file_list : files(or directories) to trash.
 *      @num       : number of files(or directories) in file_list
 *      pre_hook = _copy_files_async
 *      post_hook = NULL
 */

static FileOpsResponse* g_copy_response = NULL; //keep track of user-specified action (applied to all).
void
fileops_copy (GFile* file_list[], guint num, GFile* dest_dir)
{
    g_copy_response = NULL;

    g_debug ("fileops_copy: Begin copying files");
    GCancellable* copy_cancellable = g_cancellable_new ();
    TDData* data = g_malloc0 (sizeof (TDData));
    data->cancellable = copy_cancellable;

    guint i;
    for (i = 0; i < num; i++)
    {
        GFile* src = file_list[i];
#if 1
        char* src_uri= g_file_get_uri (src);
        char* dest_dir_uri = g_file_get_uri (dest_dir);
        g_debug ("fileops_copy: file %d: %s to dest_dir: %s", i, src_uri, dest_dir_uri);
        g_free (src_uri);
#endif

        //make sure dest_dir is a directory before proceeding.
        GFileType type = g_file_query_file_type (dest_dir, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL);
        if (type != G_FILE_TYPE_DIRECTORY)
        {
            //TODO: how to handle symbolic links
            return;
        }

        // here ,we should first check the src directory is the same as the dest directory
        // if is same , we should change the copy_dest_file by changing src_basename
         char* src_basename = g_file_get_basename (src);
         char* name = g_strdup(src_basename);
         GFile* child = g_file_get_child(dest_dir, name);
         const char* name_add_before = _("Untitled");

         GFile* parent = g_file_get_parent(src);
         char* parent_uri = g_file_get_uri(parent);
         g_object_unref(parent);
         if(0 == g_strcmp0(parent_uri,dest_dir_uri))
         {
             for (int i=0; g_file_query_exists(child, NULL) && (i<500); i++) {
                g_object_unref(child);
                g_free(name);
                name = g_strdup_printf("%s(%d)%s",name_add_before, i,src_basename);
                child = g_file_get_child(dest_dir, name);
             }
         }

        g_free (dest_dir_uri);
        g_free (src_basename);
        g_free(name);
        g_free(parent_uri);

        data->dest_file = child;
        traverse_directory (src, _copy_files_async, _dummy_func, data);

        g_object_unref (data->dest_file);
    }
    g_object_unref (data->cancellable);
    g_free (data);

    fileops_response_free (g_copy_response);
    g_debug ("fileops_copy: End copying files");
}
// internal functions
// TODO : setup a dialog, support Cancelling and show progress bar.
//
// hook function return value:
// TRUE: continue operation
//       CONFLICT_RESPONSE_SKIP
//       CONFLICT_RESPONSE_RENAME
//       CONFLICT_RESPONSE_REPLACE
// FALSE: get out of traverse_directory.
//        GTK_RESPONSE_CANCEL

static gboolean
_dummy_func (GFile* file, gpointer data)
{
    return TRUE;
}
//NOTE: src: source file
//      dest: destination file (not destination directory)
static gboolean
_cmp_files (GFile* src, GFile* dest)
{
    char* src_uri = g_file_get_uri (src);
    char* dest_uri = g_file_get_uri (dest);
    gboolean retval = g_strcmp0 (src_uri, dest_uri);
    g_free (src_uri);
    g_free (dest_uri);

    return retval;
}

static gboolean
_delete_files_async (GFile* file, gpointer data)
{
    gboolean retval = TRUE;

    TDData* _data = (TDData*) data;

    GError* error = NULL;
    GCancellable* _delete_cancellable = NULL;

    _delete_cancellable = _data->cancellable;
    g_file_delete (file, _delete_cancellable, &error);

    if (error != NULL)
    {
        //show error dialog
        g_cancellable_cancel (_delete_cancellable);
        g_warning ("_delete_files_async: %s", error->message);
        //fileops_delete_trash_error_show_dialog ("delete", error, file, NULL);
        g_error_free (error);
        g_cancellable_reset (_delete_cancellable);
    }
#if 1
    char* file_uri = g_file_get_uri (file);
    g_debug ("_delete_files_async: delete : %s", file_uri);
    g_free (file_uri);
#endif

    return retval;
}

static gboolean
_trash_files_async (GFile* file, gpointer data)
{
    gboolean retval = TRUE;

    TDData* _data = (TDData*) data;

    GError* error = NULL;
    GCancellable* _trash_cancellable = NULL;

    _trash_cancellable = _data->cancellable;
    g_file_trash (file, _trash_cancellable, &error);//By test this function , ensure that the GLIB function-org g_file_trash has bug in too times to trash

    if (error != NULL)
    {
        g_cancellable_cancel (_trash_cancellable);
        g_warning ("_trash_files_async: %s", error->message);
        g_error_free (error);
        g_cancellable_reset (_trash_cancellable);
    }
#if 1
    char* file_uri = g_file_get_uri (file);
    g_debug ("_trash_files_async: trash : %s", file_uri);
    g_free (file_uri);
#endif

    return retval;
}

/*
 * NOTE: the retval has been hacked to please frontend.
 *             it's not consistent with other hook functions.
 *             use with care.
 */
static gboolean
_move_files_async (GFile* src, gpointer data)
{
    g_debug ("begin _move_files_async");
    gboolean retval = TRUE;

    TDData* _data = (TDData*) data;

    GError* error = NULL;
    GCancellable* _move_cancellable = NULL;
    GFile* dest = NULL;

    _move_cancellable = _data->cancellable;
    dest = _data->dest_file;
    if (!_cmp_files (src, dest)) //src==dest
        return FALSE;
    g_file_move (src, dest,
                 G_FILE_COPY_NOFOLLOW_SYMLINKS,
                 _move_cancellable,
                 NULL,
                 NULL,
                 &error);
    GFileType type = g_file_query_file_type (src, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL);
    if (error != NULL)
    {
//      g_cancellable_cancel (_move_cancellable);
        g_warning ("_move_files_async: %s", error->message);
        //TEST:
        if (g_prompt == TRUE)
        {
            FileOpsResponse* response = NULL;
            if (g_move_response != NULL && g_move_response->apply_to_all)
            {
                response = fileops_response_dup (g_move_response); //FIXME:reduce dup calls
            }
            else
            {
                response = fileops_move_copy_error_show_dialog (_("move"), error, src, dest, NULL);
                if (response != NULL && response->apply_to_all)
                    g_move_response = fileops_response_dup (response);
            }

            if(response != NULL)
            {
                switch (response->response_id)
                {
                case GTK_RESPONSE_CANCEL:
                    //cancel all operations
                    g_debug ("response : Cancel");
                    retval = FALSE;
                    break;

                case CONFLICT_RESPONSE_SKIP:
                    //skip, imediately return.
                    g_debug ("response : Skip");
                    retval = FALSE;
                    break;
                case CONFLICT_RESPONSE_RENAME:
                    //rename, redo operations
                    g_debug ("response : Rename");

                    GFile* dest_parent = g_file_get_parent (dest);
                    GFile* new_dest = g_file_get_child (dest_parent, response->file_name);
                    g_object_unref (dest_parent);

                    g_object_unref (dest);
                    _data->dest_file = new_dest;

                    retval = _move_files_async (src, _data);
                    break;
                case CONFLICT_RESPONSE_REPLACE:
                    if (type == G_FILE_TYPE_DIRECTORY)
                    {
                        //Merge:
                        g_debug ("response : Merge");
                        retval = TRUE;
                    }
                    else
                    {
                        //replace
                        g_debug ("response : Replace");
                        retval = _delete_files_async (dest, _data);
                        if (retval == TRUE)
                        {
                            retval = _move_files_async (src, _data);
                        }
                    }

                    retval = TRUE;
                    break;
                default:
                    retval = FALSE;
                    break;
                }

                fileops_response_free (response);
            }
        }
        else  // g_prompt == FALSE
        {
            retval = FALSE;
        }
        g_error_free (error);
        g_debug ("move_async: error handling end");
    }
#if 1
    else
    {
        char* src_uri = g_file_get_uri (src);
        char* dest_uri = g_file_get_uri (dest);
        g_debug ("_move_files_async: move %s to %s", src_uri, dest_uri);
        g_free (src_uri);
        g_free (dest_uri);
    }
#endif

    return retval;
}

/*file copy async var global*/
gboolean COPY_ASYNC_FINISH = TRUE;
GFile* dest_pb = NULL;
GCancellable* _copy_cancellable = NULL;

static void g_file_copy_progress_handler(goffset current_num_bytes,
            goffset total_num_bytes, gpointer user_data)
{
    GtkProgressBar *progress_bar = GTK_PROGRESS_BAR(user_data);
    gchar buf[1024] = { 0 };

    g_sprintf(buf, "%ld KB / %ld KB", current_num_bytes / 1024,
                total_num_bytes / 1024);
    gtk_progress_bar_set_show_text(progress_bar,TRUE);
    gtk_progress_bar_set_text(progress_bar, buf);
    gtk_progress_bar_set_fraction(progress_bar, (gdouble)current_num_bytes / (gdouble)total_num_bytes);
}

static void g_file_copy_async_finish_handler(GObject *source_object,
            GAsyncResult *res, gpointer user_data)
{
    GtkProgressBar *progress_bar = GTK_PROGRESS_BAR(user_data);
    COPY_ASYNC_FINISH =  g_file_copy_finish(G_FILE(source_object), res, NULL);

    gtk_progress_bar_set_show_text(progress_bar,TRUE);
    gtk_progress_bar_set_text(progress_bar, "Finished");
    gtk_progress_bar_set_fraction(progress_bar, 1.0);
    g_debug("_copy_files_async_true Finished");

    GtkWidget *parent = gtk_widget_get_parent((GtkWidget *)progress_bar);
    gtk_widget_destroy((GtkWidget *)progress_bar);
    gtk_widget_destroy(parent);
}

void progress_bar_delete_event(GtkWidget *progress_bar, GdkEvent *event, gpointer data)
{
    g_message("progress_bar_delete_event");
    g_cancellable_cancel(_copy_cancellable);
    GtkWidget *parent = gtk_widget_get_parent((GtkWidget *)progress_bar);
    gtk_widget_destroy((GtkWidget *)progress_bar);
    gtk_widget_destroy(parent);
    g_file_delete (dest_pb,NULL, NULL);
    g_object_unref(dest_pb);
}

static void  _copy_files_async_true(GFile *src,gpointer data)
{
    g_debug("_copy_files_async_true start");
    TDData* _data = (TDData*) data;
    dest_pb = _data->dest_file;
    _copy_cancellable = _data->cancellable;

    GtkWidget *parent = NULL;
    GtkWidget *progress_bar = NULL;
    const char* basename = g_file_get_basename(src);

    parent = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_deletable(GTK_WINDOW(parent),FALSE);
    gtk_window_set_position(GTK_WINDOW(parent),GTK_WIN_POS_CENTER);
    gtk_widget_set_size_request(parent, 400, 30);
    gtk_window_set_title(GTK_WINDOW(parent),basename);
    gtk_window_set_resizable(GTK_WINDOW(parent),FALSE);
    g_signal_connect (G_OBJECT (parent), "delete_event", G_CALLBACK (progress_bar_delete_event), NULL);
    gtk_widget_show(parent);

    progress_bar = gtk_progress_bar_new();
    gtk_container_add(GTK_CONTAINER(parent),progress_bar);
    gtk_widget_show(progress_bar);


#if DEBUG
    char* src_uri = g_file_get_uri (src);
    char* dest_uri = g_file_get_uri (dest_pb);
    g_debug ("_copy_files_async: copy %s to %s", src_uri, dest_uri);
    g_free (src_uri);
    g_free (dest_uri);
#endif
    g_file_copy_async(src, dest_pb, G_FILE_COPY_NOFOLLOW_SYMLINKS,
                G_PRIORITY_DEFAULT, _copy_cancellable, g_file_copy_progress_handler,
                progress_bar, g_file_copy_async_finish_handler, progress_bar);
}
/*
 *
 */
static gboolean
_copy_files_async (GFile* src, gpointer data)
{
    gboolean retval = TRUE;

    TDData* _data = (TDData*) data;

    GError* error = NULL;
    /*GCancellable* _copy_cancellable = NULL;*/
    GFile* dest = NULL;

    _copy_cancellable = _data->cancellable;
    dest = _data->dest_file;

    //because @dest doesn't exist, we should check @src instead.
    GFileType type = g_file_query_file_type (src, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL);
    if (type == G_FILE_TYPE_DIRECTORY)
    {
        //TODO: change permissions
        g_file_make_directory (dest, NULL, &error);
#if 1
        char* dest_uri = g_file_get_uri (dest);
        g_debug ("_copy_files_async: mkdir : %s", dest_uri);
        g_free (dest_uri);
#endif
    }
    else
    {
        if (!_cmp_files (src, dest)) //src==dest
            {
                //rename destination name
                char* tmp = g_file_get_uri (dest);
                char* ext_name = strrchr (tmp, '.');
                if (ext_name != NULL)
                {
                    *ext_name = '\0';
                    ext_name ++;
                }
                char* stem_name = tmp;
                char* tmp_dest = g_strconcat (stem_name,
                                              " (", _("Copy"), ")", ".",
                                              ext_name,
                                              NULL);
                g_free (tmp);

                g_object_unref (dest);
                dest = g_file_new_for_uri (tmp_dest);
                g_free (tmp_dest);
                _data->dest_file = dest;
            }

        gboolean  ASYNC = TRUE;
        if (ASYNC)
        {

            // g_debug("check the g_file_copy_async error first!");

            char* dest_path = g_file_get_path (dest);
            gboolean is_exist = g_file_test(dest_path,G_FILE_TEST_EXISTS);
            g_free (dest_path);
            if (is_exist)
            {
                error = g_error_new(G_FILE_TEST_EXISTS,G_IO_ERROR_EXISTS,"file already exist!");
                //    g_cancellable_cancel (_copy_cancellable);
                g_warning ("_copy_files_async: %s, code = %d", error->message, error->code);
                //TEST:
                FileOpsResponse* response = NULL;
                if (g_copy_response != NULL && g_copy_response->apply_to_all)
                {
                    response = fileops_response_dup (g_copy_response); //FIXME:reduce dup calls
                }
                else
                {
                    response = fileops_move_copy_error_show_dialog (_("copy"), error, src, dest, NULL);
                    if (response->apply_to_all)
                            g_copy_response = fileops_response_dup (response);
                }

                if(response != NULL)
                {
                switch (response->response_id)
                {
                    case GTK_RESPONSE_CANCEL:
                        //cancel all operations
                        g_debug ("response : Cancel");
                        retval = FALSE;
                        break;

                    case CONFLICT_RESPONSE_SKIP:
                        //skip, imediately return.
                            g_debug ("response : Skip");
                        retval = TRUE;
                        break;
                    case CONFLICT_RESPONSE_RENAME:
                        //rename, redo operations
                        g_debug ("response : Rename to %s", response->file_name);

                        GFile* dest_parent = g_file_get_parent (dest);
                        GFile* new_dest = g_file_get_child (dest_parent, response->file_name);
                            g_object_unref (dest_parent);

                        g_object_unref (dest);
                        _data->dest_file = new_dest;

                        // retval = _copy_files_async (src, _data);
                        _copy_files_async_true(src,_data);
                        retval = COPY_ASYNC_FINISH;
                        break;
                    case CONFLICT_RESPONSE_REPLACE:
                        if (type == G_FILE_TYPE_DIRECTORY)
                        {
                            //Merge:
                            g_debug("Merge");
                            retval = TRUE;
                        }
                        else
                        {
                            //replace
                            retval = _delete_files_async (dest, _data);
                            if (retval == TRUE)
                            {
                            // retval = _copy_files_async (src, _data);
                                _copy_files_async_true(src,_data);
                            }
                        }
                        g_debug ("response : Replace");
                        break;
                    default:
                        retval = FALSE;
                        break;
                }

                fileops_response_free (response);
                }
                g_error_free (error);
            }
            else
            {
                g_debug("file not exist in dest");
                _copy_files_async_true(src,_data);
                retval == TRUE;
            }
        }

    }

    COPY_ASYNC_FINISH = retval;
    return COPY_ASYNC_FINISH;
}

void
files_copy_via_dbus (GFile *file_list[], guint num, GFile *dest_dir)
{
    g_debug ("files copy start ...\n");
    guint i = 0;
    GVariantBuilder builder;

    g_variant_builder_init (&builder, G_VARIANT_TYPE("as"));
    for ( ; i < num; i++ ) {
        gchar *src_uri = g_file_get_uri (file_list[i]);
        g_variant_builder_add (&builder, "s", src_uri);
        g_free (src_uri);
    }
    gchar *dest_uri = g_file_get_uri (dest_dir);
    call_method_via_dbus (&builder, dest_uri);
    g_variant_builder_clear (&builder);
    g_free (dest_uri);
}

static void
call_method_via_dbus (const GVariantBuilder *builder, const gchar *dest_uri)
{
    GDBusProxy *proxy = NULL;
    GError *error = NULL;

    proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
            G_DBUS_PROXY_FLAGS_NONE, NULL,
            DBUS_NAUTILUS_NAME, DBUS_NAUTILUS_PATH,
            DBUS_NAUTILUS_INFACE, NULL, &error);
    if ( error ) {
        g_warning ("get new proxy failed: %s", error->message);
        g_error_free (error);
        error = NULL;
        return ;
    }

    g_dbus_proxy_call (proxy, DBUS_COPY_METHOD,
            g_variant_new ("(ass)", builder, dest_uri),
            G_DBUS_CALL_FLAGS_NONE,
            -1, NULL, (GAsyncReadyCallback)dbus_call_method_cb,
            NULL);
    g_object_unref (proxy);
}

static void
dbus_call_method_cb (GObject *source_object,
        GAsyncResult *res, gpointer user_data)
{
    GError *error = NULL;
    GVariant *retval = NULL;
    GDBusProxy *proxy = (GDBusProxy*)source_object;

    retval = g_dbus_proxy_call_finish (proxy, res, &error);
    if ( error ) {
        g_warning ("call method failed: %s", error->message);
        g_error_free (error);
        error = NULL;
        return ;
    }
    if ( retval ) {
        g_variant_unref (retval);
    }
    g_debug ("call method success!\n");
}

