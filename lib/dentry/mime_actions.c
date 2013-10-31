#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include "xdg_misc.h"

/*#define TERMINAL_SCHEMA_ID "com.deepin.desktop.default-applications.terminal"*/
/*#define TERMINAL_KEY_EXEC  "exec"*/
/*#define TERMINAL_KEY_EXEC_ARG "exec-arg"*/

/*static GSettings* terminal_gsettings = NULL;*/

GAppInfo *gen_app_info (const char* executable);
gboolean exec_app_info (const char *executable);

//used by dentry/mime_actions.c

GAppInfo *gen_app_info (const char* executable)
{
    GAppInfo *appinfo = NULL;
    GError* error = NULL;
    char* cmd_line;

    if (executable == NULL)
    {
        char* tmp1 = g_shell_quote(DESKTOP_DIR());
        char* tmp2 = g_strdup_printf("cd %s && exec $SHELL", tmp1);
        g_free(tmp1);
        tmp1 = g_shell_quote(tmp2);
        g_free(tmp2);

        cmd_line = g_strconcat("sh -c ", tmp1, NULL);
        g_free(tmp1);
    }
    else
    {
        cmd_line = g_strdup(executable);
    }

    appinfo = g_app_info_create_from_commandline(cmd_line, NULL,
            G_APP_INFO_CREATE_NEEDS_TERMINAL, &error);
    g_free(cmd_line);
    if (error!=NULL)
    {
        g_debug("gen_app_info error: %s", error->message);
        g_error_free(error);
        return NULL;
    }
    error = NULL;

    return appinfo;
}

gboolean exec_app_info (const char *executable)
{
    GAppInfo *appinfo = NULL;
    GError *error = NULL;
    gboolean is_ok = FALSE;

    appinfo = gen_app_info (executable);
    if ( appinfo == NULL ) {
        g_debug ("gen app info failed!");
        return FALSE;
    }

    GdkAppLaunchContext* ctx = gdk_display_get_app_launch_context(gdk_display_get_default());
    gdk_app_launch_context_set_screen(ctx, gdk_screen_get_default()); //must set this otherwise termiator will not work properly
    is_ok = g_app_info_launch (appinfo, NULL, (GAppLaunchContext*)ctx, &error);
    g_object_unref(ctx);
    if (error!=NULL)
    {
        g_debug("exec app info error: %s", error->message);
        g_error_free(error);
        g_object_unref (appinfo);

        return FALSE;
    }

    g_object_unref (appinfo);
    return TRUE;
}

void desktop_run_in_terminal(char* executable)
{
    gboolean is_ok = FALSE;

    is_ok = exec_app_info (executable);
    if ( !is_ok ) {
        g_debug ("exec app info failed!");
        /*exec_app_info (executable);*/
    }

    return ;
}

#define RESPONSE_RUN 1000
#define RESPONSE_DISPLAY 1001
#define RESPONSE_RUN_IN_TERMINAL 1002

static void
run_file  (GFile* file, GFile* _file_arg)
{
    char* cmd_line;
    char* file_path;


    //here we should check the file type
    // if the src is symbolink we should set the file_path as the path for it's value
    GFileType type = g_file_query_file_type (file, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL);
    if (type == G_FILE_TYPE_SYMBOLIC_LINK)
    {
        //TODO: symbolic links
        g_debug("the src file type is  G_FILE_TYPE_SYMBOLIC_LINK");
        GFileInfo* info = g_file_query_info(file, "standard::*", G_FILE_QUERY_INFO_NONE, NULL, NULL);
        if (info != NULL) {
            const char * link_target_path = g_file_info_get_symlink_target(info);
            g_debug("symbolic link target is :%s",link_target_path);
            file_path = g_strdup(link_target_path);
        }
        g_object_unref(info);
    }
    else{
        file_path = g_file_get_path (file);
    }
    g_debug("run file_path :%s",file_path);

    if (_file_arg != NULL)
    {
        char* _file_arg_uri = g_file_get_uri (_file_arg);
        cmd_line = g_strdup_printf ("%s %s", file_path, _file_arg_uri);
	g_free (_file_arg_uri);
    }
    else
    {
        cmd_line = g_strdup (file_path);
    }
    g_free (file_path);
    if(cmd_line != NULL)
    {
        g_spawn_command_line_async (cmd_line, NULL);
    }
    else
    {
        g_warning("run file_path is null");
    }
    g_free (cmd_line);
}

void desktop_run_in_terminal(char* executable);
static void
run_file_in_terminal (GFile* file)
{
    char* executable = g_file_get_path (file);
    desktop_run_in_terminal (executable);
    g_free (executable);
}

static gboolean
display_file (GFile* file, const char* content_type)
{
    gboolean res = TRUE;
    GAppInfo *app  = g_app_info_get_default_for_type(content_type, FALSE);
    GList* list = g_list_append(NULL, file);
    res = g_app_info_launch(app, list, NULL, NULL);
    g_list_free(list);
    g_object_unref(app);

    return res;
}

gboolean
activate_file (GFile* file, const char* content_type,
               gboolean is_executable, GFile* _file_arg)
{
    char* file_name = g_file_get_basename (file);
    gboolean is_bin = g_str_has_suffix(file_name, ".bin");
    gboolean result = TRUE;

    g_debug ("activate_file: %s", file_name);
    g_free (file_name);
    g_debug ("content_type: %s", content_type);

    if (is_executable &&
        (g_content_type_can_be_executable (content_type) || is_bin))
    {
        g_debug("is_executable && g_content_type_can_be_executable || is_bin");
        //1. an executable text file. or an shell script
        if (g_content_type_is_a (content_type, "text/plain"))
        {
            g_debug("g_content_type_is_a");
            GtkWidget* dialog;
            int response;
            char* file_name;
            char* prompt;
            char* detail;

            file_name = g_file_get_basename (file);
            prompt = g_strdup_printf (_("Do you want to run \"%s\", or display its contents?"), file_name);
            detail = g_strdup_printf (_("\"%s\" is an executable text file."), file_name);
            g_free (file_name);
            //create prompt dialog
            dialog = gtk_message_dialog_new (NULL,
                                             GTK_DIALOG_MODAL,
                                             GTK_MESSAGE_QUESTION,
                                             GTK_BUTTONS_NONE,
                                             NULL);
            g_object_set (dialog, "text", prompt, "secondary-text", detail, NULL);
            g_free (prompt);
            g_free (detail);
            gtk_dialog_add_button (GTK_DIALOG(dialog), _("Run in _Terminal"), RESPONSE_RUN_IN_TERMINAL);
            gtk_dialog_add_button (GTK_DIALOG(dialog), _("_Display"), RESPONSE_DISPLAY);
            gtk_dialog_add_button (GTK_DIALOG(dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
            gtk_dialog_add_button (GTK_DIALOG(dialog), _("_Run"), RESPONSE_RUN);
            gtk_dialog_set_default_response (GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL);

            gtk_widget_show (GTK_WIDGET (dialog));

            response = gtk_dialog_run (GTK_DIALOG(dialog));
            gtk_widget_destroy (GTK_WIDGET (dialog));

            g_message("response:%d",response);
            switch (response)
            {
            case RESPONSE_RUN_IN_TERMINAL:
                run_file_in_terminal (file);
                break;
            case RESPONSE_DISPLAY:
                result = display_file (file, content_type);
                break;
            case RESPONSE_RUN:
                run_file (file, NULL);
                break;
            case GTK_RESPONSE_CANCEL:
                break;
            default:
                break;
            }
        }
        //2. an executable binary file
        else
        {
            g_debug("run_file");
            run_file (file, _file_arg);
        }
    }
    //for non-executables just open it.
    else
    {
        g_debug("for non-executables just open it.");
        result = display_file (file, content_type);
    }

    return result;
}

