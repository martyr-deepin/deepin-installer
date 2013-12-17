//get a list of GVolumes
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include "xdg_misc.h"
#include "fileops_trash.h"

static GList *  _get_trash_dirs_for_mount       (GMount *mount);
static gboolean _empty_trash_job                (GIOSchedulerJob *io_job,
                                                 GCancellable* cancellable,
                                                 gpointer user_data);
static gboolean _empty_trash_job_done           (gpointer user_data);
static void     _delete_trash_file              (GFile *file,
                                                 gboolean del_file,
                                                 gboolean del_children);

static GFile* trash_can = NULL;
GFile* fileops_get_trash_entry()
{
    // g_assert(_trash_can != NULL);
    if (trash_can == NULL)
        trash_can = g_file_new_for_uri("trash:///");
    g_object_ref(trash_can);

    return trash_can;
}
double fileops_get_trash_count()
{
    GFile* _trash_can = fileops_get_trash_entry ();
    GFileInfo* info = g_file_query_info(_trash_can, G_FILE_ATTRIBUTE_TRASH_ITEM_COUNT, G_FILE_QUERY_INFO_NONE, NULL, NULL);
    double count = 0;
    if (info != NULL) { // info maybe equal NULL when use xinit run desktop
        count = g_file_info_get_attribute_uint32(info, G_FILE_ATTRIBUTE_TRASH_ITEM_COUNT);
        g_object_unref(info);
    }
    g_object_unref(_trash_can);
    return count;
}

static gboolean
focus_cb (GtkWidget* widget, GtkDirectionType direction, gpointer user_data)
{
    GdkWindow* gdk_dialog = gtk_widget_get_window (widget);
    gdk_window_raise (gdk_dialog);
    return FALSE;
}
void fileops_confirm_trash ()
{
    GtkWidget* dialog;
    int result;

    dialog = gtk_message_dialog_new (NULL,
                                     0,  // use unmodal dialog
                                     GTK_MESSAGE_WARNING,
                                     GTK_BUTTONS_CANCEL,
                                     NULL);
    gtk_window_set_title (GTK_WINDOW (dialog), _("Empty Trash"));
    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
    char* _icon = icon_name_to_path ("user-trash-full", 16);
    if (_icon != NULL)
    {
        gtk_window_set_icon_from_file (GTK_WINDOW (dialog), _icon, NULL);
        g_free (_icon);
    }
    g_object_set (dialog,
                  "text", _("Empty all items from Trash?"),
                  "secondary-text", _("All items in the Trash will be permanently deleted."),
                  NULL);

    gtk_dialog_add_buttons (GTK_DIALOG (dialog), _("Empty _Trash"),
                            GTK_RESPONSE_OK, NULL);

    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

    gtk_widget_add_events (dialog, GDK_FOCUS_CHANGE_MASK);
    g_signal_connect (dialog, "focus-in-event", G_CALLBACK(focus_cb), NULL);

    result = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);

    if (result == GTK_RESPONSE_OK)
        fileops_empty_trash ();


}

void fileops_empty_trash ()
{
    GList* trash_list = NULL;

    GVolumeMonitor* vol_monitor = g_volume_monitor_get ();
    GList* mount_list = g_volume_monitor_get_mounts (vol_monitor);
    g_object_unref (vol_monitor);

    //iterate through all mounts
    GList* l;
    for (l = mount_list; l != NULL; l = l->next)
    {
        trash_list = g_list_concat (trash_list,
                                    _get_trash_dirs_for_mount (l->data));
    }
    g_list_free_full (mount_list, g_object_unref);
    //add 'trash:' prefix
    trash_list = g_list_prepend (trash_list,
                                 g_file_new_for_uri ("trash:"));

    g_io_scheduler_push_job (_empty_trash_job,
                             trash_list,
                             NULL,
                             0,
                             NULL);
}
static GList *
_get_trash_dirs_for_mount (GMount *mount)
{
    GFile *root;
    GFile *trash;
    char *relpath;
    GList *list;

    root = g_mount_get_root (mount);
    if (root == NULL)
        return NULL;

    list = NULL;
    if (g_file_is_native (root))
    {
        relpath = g_strdup_printf (".Trash/%d", getuid ());
        trash = g_file_resolve_relative_path (root, relpath);
        g_free (relpath);

        list = g_list_prepend (list, g_file_get_child (trash, "files"));
        list = g_list_prepend (list, g_file_get_child (trash, "info"));

        g_object_unref (trash);

        relpath = g_strdup_printf (".Trash-%d", getuid ());
        trash = g_file_get_child (root, relpath);
        g_free (relpath);

        list = g_list_prepend (list, g_file_get_child (trash, "files"));
        list = g_list_prepend (list, g_file_get_child (trash, "info"));

        g_object_unref (trash);
    }
    g_object_unref (root);

    return list;
}

static gboolean
_empty_trash_job (GIOSchedulerJob *io_job,
                  GCancellable* cancellable,
                  gpointer user_data)
{
    GList* trash_list = (GList*) user_data;

    GList* l;
    for (l = trash_list; l != NULL; l = l->next)
        _delete_trash_file (l->data, FALSE, TRUE);

    g_io_scheduler_job_send_to_mainloop_async (io_job,
                                               _empty_trash_job_done,
                                               user_data,
                                               NULL);
    return FALSE;
}
static gboolean
_empty_trash_job_done (gpointer user_data)
{
    g_list_free_full (user_data, g_object_unref);
    return FALSE;
}
static void
_delete_trash_file (GFile *file,
                    gboolean del_file,
                    gboolean del_children)
{
    GFileInfo *info;
    GFile *child;
    GFileEnumerator *enumerator;

    if (del_children)
    {
        enumerator = g_file_enumerate_children (file,
                                                G_FILE_ATTRIBUTE_STANDARD_NAME ","
                                                G_FILE_ATTRIBUTE_STANDARD_TYPE,
                                                G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                                NULL, NULL);
        if (enumerator)
        {
            while ((info = g_file_enumerator_next_file (enumerator, NULL, NULL)) != NULL)
            {
                child = g_file_get_child (file, g_file_info_get_name (info));
                _delete_trash_file (child, TRUE,
                                    g_file_info_get_file_type (info) == G_FILE_TYPE_DIRECTORY);
                g_object_unref (child);
                g_object_unref (info);
            }
            g_file_enumerator_close (enumerator, NULL, NULL);
            g_object_unref (enumerator);
        }
    }
    if (del_file)
    {
        g_file_delete (file, NULL, NULL);
    }
}

