#include <gtk/gtk.h>
#include <glib/gi18n.h>


#include "enums.h"
#include "fileops_error_dialog.h"
#include "fileops_error_reporting.h"

/*
 *	because we use dialog in applications which manages desktop.
 *	@parent is always NULL.
 */
static FileOpsResponse*  _show_simple_error_message_dialog		(const char *fileops_str,
									 const char *error_message,
									 GFile* file,
									 GtkWindow* parent);
static FileOpsResponse*	_show_skip_cancel_all_dialog			(const char *fileops_str,
									 const char *error_message,
									 GFile *file,
									 GtkWindow* parent);
static FileOpsResponse*	_show_skip_cancel_replace_rename_all_dialog	(const char *fileops_str,
									 const char *error_message,
									 GFile *src, 
									 GFile *dest,
									 GtkWindow* parent);
FileOpsResponse* 
fileops_response_dup (FileOpsResponse* response)
{
    FileOpsResponse* _dup_response;
    _dup_response = g_new0 (FileOpsResponse, 1);

    _dup_response->response_id = response->response_id;
    _dup_response->file_name = g_strdup (response->file_name);
    _dup_response->apply_to_all = response->apply_to_all;

    return _dup_response;
}

void
fileops_response_free (FileOpsResponse* response)
{
    if (response == NULL)
        return;

    g_free (response->file_name);
    g_free (response);
}

/*
 *	delete, trash error need only one GFile* parameters. 
 *	@fileops_str : "delete" or "trash"
 *	@error:
 *	@file: file to delete or trash.
 */
FileOpsResponse*
fileops_delete_trash_error_show_dialog (const char* fileops_str, GError* error, 
					GFile* file, GtkWindow* parent)
{
    FileOpsResponse* ret = NULL;
    switch (error->code)
    {
	case G_IO_ERROR_PERMISSION_DENIED: 
	     ret = _show_skip_cancel_all_dialog (fileops_str, error->message, file, parent);
	     break;
	case G_IO_ERROR_CANCELLED:   
	    /*
	     * TODO: response: this is caused by progress_dialog. 
	     */
	     ret = _show_simple_error_message_dialog (fileops_str, error->message, file, parent);
	    break;

	case G_IO_ERROR_IS_DIRECTORY:  //programming error.
	case G_IO_ERROR_INVALID_ARGUMENT: //programming error
	     ret = _show_simple_error_message_dialog (fileops_str, "Programming error", NULL, parent);
	    break;
	default: //all other errors are not handled.
	     ret = _show_simple_error_message_dialog (fileops_str, "Unexpected error happens!", NULL, parent);
	    break;
    }

    return ret;
}
/*
 *	move, copy needs a src, and dest.
 *	@fileops_str : "move" or "copy"
 *	@src : source file 
 *	@dest: destinatin file.
 */
FileOpsResponse*
fileops_move_copy_error_show_dialog (const char* fileops_str, GError* error, 
	                             GFile* src, GFile* dest, GtkWindow* parent)
{
    FileOpsResponse* ret = NULL;
    switch (error->code)
    {
	case G_IO_ERROR_NOT_FOUND:
	    {
		GtkWidget* dialog;
	        dialog = gtk_message_dialog_new (NULL, 
					     GTK_DIALOG_MODAL,
					     GTK_MESSAGE_WARNING, 
					     GTK_BUTTONS_OK,
					     NULL);
	        gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
		char* primary_text = g_strdup_printf (_("Error while %s files"), fileops_str);
	        char* secondary_text = g_strdup (error->message);

		g_object_set (dialog,
	          "text", primary_text,
		  "secondary-text", secondary_text,
		  NULL);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_free(secondary_text);
		g_free(primary_text);
	    }
	    break;
	case G_IO_ERROR_EXISTS:      //move, copy
	    //TODO: message dialog.
	    //      overwrite, replace, rename, //all overwrite, replace all, rename all.
	    ret = _show_skip_cancel_replace_rename_all_dialog (fileops_str, error->message, src, dest, parent);
	    break;
	case G_IO_ERROR_NOT_DIRECTORY: //move, copy destination
	    /*
	     * TODO: response: FILE_OPS_RESPONSE_CANCEL.
	     */
	    _show_simple_error_message_dialog (fileops_str, error->message, dest, parent);
	    break;
	case G_IO_ERROR_PERMISSION_DENIED: //delete, trash, move, copy
	    /*
	     * TODO: response: skip, cancel, //skip all
	     * NOTE: use @dest instead of @src here.
	     */
	    _show_skip_cancel_all_dialog (fileops_str, error->message, dest, parent);
	    break;
	case G_IO_ERROR_CANCELLED:   //operatin was cancelled
	    /*
	     * TODO: response: this is caused by progress_dialog. 
	     */
	    _show_simple_error_message_dialog (fileops_str, error->message, dest, parent);
	    break;

	case G_IO_ERROR_IS_DIRECTORY:  //programming error.
	case G_IO_ERROR_INVALID_ARGUMENT: //programming error
	    _show_simple_error_message_dialog (fileops_str, "Programming error !", NULL, parent);
	    break;
	default: //all other errors are not handled.
	    _show_simple_error_message_dialog (fileops_str, "Unexpected error happens!", NULL, parent);
	    break;
    }

    return ret;
}

//internal functions
/*
 *	when there're some unrecoverable errors, we use this dialog to prompt users.
 *	after calling this, we stop all operations.
 *	TODO:
 */
static FileOpsResponse*
_show_simple_error_message_dialog (const char* fileops_str, const char *error_message,
				   GFile *file, GtkWindow* parent)
{
    if (file == NULL)
    {
	//just show error_message and return.
    }
    //file != NULL:
    return NULL;
}
/*
 *	permission denied, what we do now?
 *	TODO:
 */
static FileOpsResponse*	
_show_skip_cancel_all_dialog (const char* fileops_str, const char *error_message, 
			      GFile* file, GtkWindow* parent)
{
    GtkWidget* dialog;
    dialog = gtk_message_dialog_new (NULL, 
				     GTK_DIALOG_MODAL,
			             GTK_MESSAGE_WARNING, 
				     GTK_BUTTONS_OK,
			             NULL);
    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
    char* primary_text = g_strdup_printf (_("Error while %s files"), fileops_str);
    char* secondary_text = g_strdup (error_message);

    g_object_set (dialog,
	          "text", primary_text,
		  "secondary-text", secondary_text,
		  NULL);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
    g_free(secondary_text);
    g_free(primary_text);
    return NULL;
}
/*
 *
 */
static FileOpsResponse*
_show_skip_cancel_replace_rename_all_dialog (const char *fileops_str, const char *error_message,
					     GFile *src, GFile *dest, GtkWindow* parent)
{
    GtkWidget* dialog;
    FileOpsResponse* response;

    response = g_malloc0 (sizeof (FileOpsResponse));
    //get file_name and boolean apply_to_all here
    dialog = fileops_error_conflict_dialog_new (parent, src, dest, response);

    response->response_id = gtk_dialog_run (GTK_DIALOG (dialog));

    gtk_widget_destroy (dialog);


    return response;
}
