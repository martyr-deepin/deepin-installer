
#include <JavaScriptCore/JSContextRef.h>
#include <JavaScriptCore/JSStringRef.h>
#include "jsextension.h"
#include <glib.h>
#include <glib-object.h>

extern void* get_global_webview();


extern void installer_emit_webview_ok(JSData*);
static JSValueRef __emit_webview_ok__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 0) {
        js_fill_exception(context, exception,
            "the emit_webview_ok except 0 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

         installer_emit_webview_ok (data);
        r = JSValueMakeNull(context);

        g_free(data);

    }
    
    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern void installer_restart_installer(JSData*);
static JSValueRef __restart_installer__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 0) {
        js_fill_exception(context, exception,
            "the restart_installer except 0 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

         installer_restart_installer (data);
        r = JSValueMakeNull(context);

        g_free(data);

    }
    
    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern JSValueRef  installer_get_system_users(JSData*);
static JSValueRef __get_system_users__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 0) {
        js_fill_exception(context, exception,
            "the get_system_users except 0 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        JSValueRef c_return =  installer_get_system_users (data);
        r = c_return;

        g_free(data);

    }
    
    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern void installer_create_user(char * , char * , char * , JSData*);
static JSValueRef __create_user__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 3) {
        js_fill_exception(context, exception,
            "the create_user except 3 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    gchar* p_1 = jsvalue_to_cstr(context, arguments[1]);

    gchar* p_2 = jsvalue_to_cstr(context, arguments[2]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

         installer_create_user (p_0, p_1, p_2, data);
        r = JSValueMakeNull(context);

        g_free(data);

    }
    
    g_free(p_0);

    g_free(p_1);

    g_free(p_2);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern void installer_finish_install(JSData*);
static JSValueRef __finish_install__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 0) {
        js_fill_exception(context, exception,
            "the finish_install except 0 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

         installer_finish_install (data);
        r = JSValueMakeNull(context);

        g_free(data);

    }
    
    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern void installer_finish_reboot(JSData*);
static JSValueRef __finish_reboot__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 0) {
        js_fill_exception(context, exception,
            "the finish_reboot except 0 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

         installer_finish_reboot (data);
        r = JSValueMakeNull(context);

        g_free(data);

    }
    
    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern char *  installer_get_layout_description(char * , JSData*);
static JSValueRef __get_layout_description__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 1) {
        js_fill_exception(context, exception,
            "the get_layout_description except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gchar* c_return =  installer_get_layout_description (p_0, data);
        
        if (c_return != NULL) {
            r = jsvalue_from_cstr(context, c_return);
            g_free(c_return);
        } else {
            r = JSValueMakeNull(context);
        }


        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern JSValueRef  installer_get_keyboard_layouts(JSData*);
static JSValueRef __get_keyboard_layouts__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 0) {
        js_fill_exception(context, exception,
            "the get_keyboard_layouts except 0 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        JSValueRef c_return =  installer_get_keyboard_layouts (data);
        r = c_return;

        g_free(data);

    }
    
    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern JSValueRef  installer_get_layout_variants(char * , JSData*);
static JSValueRef __get_layout_variants__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 1) {
        js_fill_exception(context, exception,
            "the get_layout_variants except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        JSValueRef c_return =  installer_get_layout_variants (p_0, data);
        r = c_return;

        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern JSValueRef  installer_get_current_layout_variant(JSData*);
static JSValueRef __get_current_layout_variant__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 0) {
        js_fill_exception(context, exception,
            "the get_current_layout_variant except 0 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        JSValueRef c_return =  installer_get_current_layout_variant (data);
        r = c_return;

        g_free(data);

    }
    
    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern void installer_set_keyboard_layout_variant(char * , char * , JSData*);
static JSValueRef __set_keyboard_layout_variant__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 2) {
        js_fill_exception(context, exception,
            "the set_keyboard_layout_variant except 2 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    gchar* p_1 = jsvalue_to_cstr(context, arguments[1]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

         installer_set_keyboard_layout_variant (p_0, p_1, data);
        r = JSValueMakeNull(context);

        g_free(data);

    }
    
    g_free(p_0);

    g_free(p_1);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern double  installer_keyboard_detect_read_step(char * , JSData*);
static JSValueRef __keyboard_detect_read_step__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 1) {
        js_fill_exception(context, exception,
            "the keyboard_detect_read_step except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        double c_return =  installer_keyboard_detect_read_step (p_0, data);
        r = JSValueMakeNumber(context, c_return);

        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern JSValueRef  installer_keyboard_detect_get_symbols(JSData*);
static JSValueRef __keyboard_detect_get_symbols__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 0) {
        js_fill_exception(context, exception,
            "the keyboard_detect_get_symbols except 0 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        JSValueRef c_return =  installer_keyboard_detect_get_symbols (data);
        r = c_return;

        g_free(data);

    }
    
    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern char *  installer_keyboard_detect_get_present(JSData*);
static JSValueRef __keyboard_detect_get_present__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 0) {
        js_fill_exception(context, exception,
            "the keyboard_detect_get_present except 0 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gchar* c_return =  installer_keyboard_detect_get_present (data);
        
        if (c_return != NULL) {
            r = jsvalue_from_cstr(context, c_return);
            g_free(c_return);
        } else {
            r = JSValueMakeNull(context);
        }


        g_free(data);

    }
    
    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern char *  installer_keyboard_detect_get_not_present(JSData*);
static JSValueRef __keyboard_detect_get_not_present__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 0) {
        js_fill_exception(context, exception,
            "the keyboard_detect_get_not_present except 0 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gchar* c_return =  installer_keyboard_detect_get_not_present (data);
        
        if (c_return != NULL) {
            r = jsvalue_from_cstr(context, c_return);
            g_free(c_return);
        } else {
            r = JSValueMakeNull(context);
        }


        g_free(data);

    }
    
    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern JSValueRef  installer_keyboard_detect_get_keycodes(JSData*);
static JSValueRef __keyboard_detect_get_keycodes__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 0) {
        js_fill_exception(context, exception,
            "the keyboard_detect_get_keycodes except 0 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        JSValueRef c_return =  installer_keyboard_detect_get_keycodes (data);
        r = c_return;

        g_free(data);

    }
    
    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern char *  installer_keyboard_detect_get_result(JSData*);
static JSValueRef __keyboard_detect_get_result__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 0) {
        js_fill_exception(context, exception,
            "the keyboard_detect_get_result except 0 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gchar* c_return =  installer_keyboard_detect_get_result (data);
        
        if (c_return != NULL) {
            r = jsvalue_from_cstr(context, c_return);
            g_free(c_return);
        } else {
            r = JSValueMakeNull(context);
        }


        g_free(data);

    }
    
    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern JSValueRef  installer_get_timezone_list(JSData*);
static JSValueRef __get_timezone_list__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 0) {
        js_fill_exception(context, exception,
            "the get_timezone_list except 0 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        JSValueRef c_return =  installer_get_timezone_list (data);
        r = c_return;

        g_free(data);

    }
    
    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern void installer_set_timezone(char * , JSData*);
static JSValueRef __set_timezone__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 1) {
        js_fill_exception(context, exception,
            "the set_timezone except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

         installer_set_timezone (p_0, data);
        r = JSValueMakeNull(context);

        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern void installer_extract_iso(JSData*);
static JSValueRef __extract_iso__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 0) {
        js_fill_exception(context, exception,
            "the extract_iso except 0 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

         installer_extract_iso (data);
        r = JSValueMakeNull(context);

        g_free(data);

    }
    
    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern void installer_extract_squashfs(JSData*);
static JSValueRef __extract_squashfs__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 0) {
        js_fill_exception(context, exception,
            "the extract_squashfs except 0 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

         installer_extract_squashfs (data);
        r = JSValueMakeNull(context);

        g_free(data);

    }
    
    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern void installer_extract_intelligent(JSData*);
static JSValueRef __extract_intelligent__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 0) {
        js_fill_exception(context, exception,
            "the extract_intelligent except 0 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

         installer_extract_intelligent (data);
        r = JSValueMakeNull(context);

        g_free(data);

    }
    
    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern gboolean  installer_mount_procfs(JSData*);
static JSValueRef __mount_procfs__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 0) {
        js_fill_exception(context, exception,
            "the mount_procfs except 0 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gboolean c_return =  installer_mount_procfs (data);
        r = JSValueMakeBoolean(context, c_return);

        g_free(data);

    }
    
    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern gboolean  installer_chroot_target(JSData*);
static JSValueRef __chroot_target__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 0) {
        js_fill_exception(context, exception,
            "the chroot_target except 0 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gboolean c_return =  installer_chroot_target (data);
        r = JSValueMakeBoolean(context, c_return);

        g_free(data);

    }
    
    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern void installer_copy_whitelist(JSData*);
static JSValueRef __copy_whitelist__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 0) {
        js_fill_exception(context, exception,
            "the copy_whitelist except 0 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

         installer_copy_whitelist (data);
        r = JSValueMakeNull(context);

        g_free(data);

    }
    
    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern void installer_draw_background(JSValueRef , char * , JSData*);
static JSValueRef __draw_background__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 2) {
        js_fill_exception(context, exception,
            "the draw_background except 2 paramters but passed in %d", argumentCount);
        return NULL;
    }
    JSValueRef p_0 = arguments[0];

    gchar* p_1 = jsvalue_to_cstr(context, arguments[1]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

         installer_draw_background (p_0, p_1, data);
        r = JSValueMakeNull(context);

        g_free(data);

    }
    
    g_free(p_1);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern double  installer_get_memory_size(JSData*);
static JSValueRef __get_memory_size__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 0) {
        js_fill_exception(context, exception,
            "the get_memory_size except 0 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        double c_return =  installer_get_memory_size (data);
        r = JSValueMakeNumber(context, c_return);

        g_free(data);

    }
    
    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern double  installer_get_keycode_from_keysym(double , JSData*);
static JSValueRef __get_keycode_from_keysym__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 1) {
        js_fill_exception(context, exception,
            "the get_keycode_from_keysym except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    double p_0 = JSValueToNumber(context, arguments[0], NULL);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        double c_return =  installer_get_keycode_from_keysym (p_0, data);
        r = JSValueMakeNumber(context, c_return);

        g_free(data);

    }
    
    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern char *  installer_rand_uuid(JSData*);
static JSValueRef __rand_uuid__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 0) {
        js_fill_exception(context, exception,
            "the rand_uuid except 0 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gchar* c_return =  installer_rand_uuid (data);
        
        if (c_return != NULL) {
            r = jsvalue_from_cstr(context, c_return);
            g_free(c_return);
        } else {
            r = JSValueMakeNull(context);
        }


        g_free(data);

    }
    
    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern JSValueRef  installer_list_disks(JSData*);
static JSValueRef __list_disks__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 0) {
        js_fill_exception(context, exception,
            "the list_disks except 0 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        JSValueRef c_return =  installer_list_disks (data);
        r = c_return;

        g_free(data);

    }
    
    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern char *  installer_get_disk_path(char * , JSData*);
static JSValueRef __get_disk_path__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 1) {
        js_fill_exception(context, exception,
            "the get_disk_path except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gchar* c_return =  installer_get_disk_path (p_0, data);
        
        if (c_return != NULL) {
            r = jsvalue_from_cstr(context, c_return);
            g_free(c_return);
        } else {
            r = JSValueMakeNull(context);
        }


        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern char *  installer_get_disk_type(char * , JSData*);
static JSValueRef __get_disk_type__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 1) {
        js_fill_exception(context, exception,
            "the get_disk_type except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gchar* c_return =  installer_get_disk_type (p_0, data);
        
        if (c_return != NULL) {
            r = jsvalue_from_cstr(context, c_return);
            g_free(c_return);
        } else {
            r = JSValueMakeNull(context);
        }


        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern char *  installer_get_disk_model(char * , JSData*);
static JSValueRef __get_disk_model__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 1) {
        js_fill_exception(context, exception,
            "the get_disk_model except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gchar* c_return =  installer_get_disk_model (p_0, data);
        
        if (c_return != NULL) {
            r = jsvalue_from_cstr(context, c_return);
            g_free(c_return);
        } else {
            r = JSValueMakeNull(context);
        }


        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern double  installer_get_disk_max_primary_count(char * , JSData*);
static JSValueRef __get_disk_max_primary_count__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 1) {
        js_fill_exception(context, exception,
            "the get_disk_max_primary_count except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        double c_return =  installer_get_disk_max_primary_count (p_0, data);
        r = JSValueMakeNumber(context, c_return);

        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern double  installer_get_disk_length(char * , JSData*);
static JSValueRef __get_disk_length__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 1) {
        js_fill_exception(context, exception,
            "the get_disk_length except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        double c_return =  installer_get_disk_length (p_0, data);
        r = JSValueMakeNumber(context, c_return);

        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern double  installer_get_disk_sector_size(char * , JSData*);
static JSValueRef __get_disk_sector_size__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 1) {
        js_fill_exception(context, exception,
            "the get_disk_sector_size except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        double c_return =  installer_get_disk_sector_size (p_0, data);
        r = JSValueMakeNumber(context, c_return);

        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern JSValueRef  installer_get_disk_partitions(char * , JSData*);
static JSValueRef __get_disk_partitions__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 1) {
        js_fill_exception(context, exception,
            "the get_disk_partitions except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        JSValueRef c_return =  installer_get_disk_partitions (p_0, data);
        r = c_return;

        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern gboolean  installer_is_device_slow(char * , JSData*);
static JSValueRef __is_device_slow__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 1) {
        js_fill_exception(context, exception,
            "the is_device_slow except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gboolean c_return =  installer_is_device_slow (p_0, data);
        r = JSValueMakeBoolean(context, c_return);

        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern gboolean  installer_disk_support_efi(char * , JSData*);
static JSValueRef __disk_support_efi__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 1) {
        js_fill_exception(context, exception,
            "the disk_support_efi except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gboolean c_return =  installer_disk_support_efi (p_0, data);
        r = JSValueMakeBoolean(context, c_return);

        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern char *  installer_get_partition_type(char * , JSData*);
static JSValueRef __get_partition_type__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 1) {
        js_fill_exception(context, exception,
            "the get_partition_type except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gchar* c_return =  installer_get_partition_type (p_0, data);
        
        if (c_return != NULL) {
            r = jsvalue_from_cstr(context, c_return);
            g_free(c_return);
        } else {
            r = JSValueMakeNull(context);
        }


        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern char *  installer_get_partition_name(char * , JSData*);
static JSValueRef __get_partition_name__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 1) {
        js_fill_exception(context, exception,
            "the get_partition_name except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gchar* c_return =  installer_get_partition_name (p_0, data);
        
        if (c_return != NULL) {
            r = jsvalue_from_cstr(context, c_return);
            g_free(c_return);
        } else {
            r = JSValueMakeNull(context);
        }


        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern char *  installer_get_partition_path(char * , JSData*);
static JSValueRef __get_partition_path__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 1) {
        js_fill_exception(context, exception,
            "the get_partition_path except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gchar* c_return =  installer_get_partition_path (p_0, data);
        
        if (c_return != NULL) {
            r = jsvalue_from_cstr(context, c_return);
            g_free(c_return);
        } else {
            r = JSValueMakeNull(context);
        }


        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern char *  installer_get_partition_mp(char * , JSData*);
static JSValueRef __get_partition_mp__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 1) {
        js_fill_exception(context, exception,
            "the get_partition_mp except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gchar* c_return =  installer_get_partition_mp (p_0, data);
        
        if (c_return != NULL) {
            r = jsvalue_from_cstr(context, c_return);
            g_free(c_return);
        } else {
            r = JSValueMakeNull(context);
        }


        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern void installer_unmount_partition(char * , JSData*);
static JSValueRef __unmount_partition__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 1) {
        js_fill_exception(context, exception,
            "the unmount_partition except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

         installer_unmount_partition (p_0, data);
        r = JSValueMakeNull(context);

        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern double  installer_get_partition_start(char * , JSData*);
static JSValueRef __get_partition_start__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 1) {
        js_fill_exception(context, exception,
            "the get_partition_start except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        double c_return =  installer_get_partition_start (p_0, data);
        r = JSValueMakeNumber(context, c_return);

        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern double  installer_get_partition_length(char * , JSData*);
static JSValueRef __get_partition_length__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 1) {
        js_fill_exception(context, exception,
            "the get_partition_length except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        double c_return =  installer_get_partition_length (p_0, data);
        r = JSValueMakeNumber(context, c_return);

        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern double  installer_get_partition_end(char * , JSData*);
static JSValueRef __get_partition_end__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 1) {
        js_fill_exception(context, exception,
            "the get_partition_end except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        double c_return =  installer_get_partition_end (p_0, data);
        r = JSValueMakeNumber(context, c_return);

        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern char *  installer_get_partition_fs(char * , JSData*);
static JSValueRef __get_partition_fs__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 1) {
        js_fill_exception(context, exception,
            "the get_partition_fs except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gchar* c_return =  installer_get_partition_fs (p_0, data);
        
        if (c_return != NULL) {
            r = jsvalue_from_cstr(context, c_return);
            g_free(c_return);
        } else {
            r = JSValueMakeNull(context);
        }


        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern char *  installer_get_partition_label(char * , JSData*);
static JSValueRef __get_partition_label__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 1) {
        js_fill_exception(context, exception,
            "the get_partition_label except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gchar* c_return =  installer_get_partition_label (p_0, data);
        
        if (c_return != NULL) {
            r = jsvalue_from_cstr(context, c_return);
            g_free(c_return);
        } else {
            r = JSValueMakeNull(context);
        }


        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern gboolean  installer_get_partition_flag(char * , char * , JSData*);
static JSValueRef __get_partition_flag__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 2) {
        js_fill_exception(context, exception,
            "the get_partition_flag except 2 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    gchar* p_1 = jsvalue_to_cstr(context, arguments[1]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gboolean c_return =  installer_get_partition_flag (p_0, p_1, data);
        r = JSValueMakeBoolean(context, c_return);

        g_free(data);

    }
    
    g_free(p_0);

    g_free(p_1);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern gboolean  installer_get_partition_busy(char * , JSData*);
static JSValueRef __get_partition_busy__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 1) {
        js_fill_exception(context, exception,
            "the get_partition_busy except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gboolean c_return =  installer_get_partition_busy (p_0, data);
        r = JSValueMakeBoolean(context, c_return);

        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern void installer_get_partition_free(char * , JSData*);
static JSValueRef __get_partition_free__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 1) {
        js_fill_exception(context, exception,
            "the get_partition_free except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

         installer_get_partition_free (p_0, data);
        r = JSValueMakeNull(context);

        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern char *  installer_get_partition_os(char * , JSData*);
static JSValueRef __get_partition_os__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 1) {
        js_fill_exception(context, exception,
            "the get_partition_os except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gchar* c_return =  installer_get_partition_os (p_0, data);
        
        if (c_return != NULL) {
            r = jsvalue_from_cstr(context, c_return);
            g_free(c_return);
        } else {
            r = JSValueMakeNull(context);
        }


        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern gboolean  installer_new_disk_partition(char * , char * , char * , char * , double , double , JSData*);
static JSValueRef __new_disk_partition__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 6) {
        js_fill_exception(context, exception,
            "the new_disk_partition except 6 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    gchar* p_1 = jsvalue_to_cstr(context, arguments[1]);

    gchar* p_2 = jsvalue_to_cstr(context, arguments[2]);

    gchar* p_3 = jsvalue_to_cstr(context, arguments[3]);

    double p_4 = JSValueToNumber(context, arguments[4], NULL);

    double p_5 = JSValueToNumber(context, arguments[5], NULL);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gboolean c_return =  installer_new_disk_partition (p_0, p_1, p_2, p_3, p_4, p_5, data);
        r = JSValueMakeBoolean(context, c_return);

        g_free(data);

    }
    
    g_free(p_0);

    g_free(p_1);

    g_free(p_2);

    g_free(p_3);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern gboolean  installer_delete_disk_partition(char * , char * , JSData*);
static JSValueRef __delete_disk_partition__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 2) {
        js_fill_exception(context, exception,
            "the delete_disk_partition except 2 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    gchar* p_1 = jsvalue_to_cstr(context, arguments[1]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gboolean c_return =  installer_delete_disk_partition (p_0, p_1, data);
        r = JSValueMakeBoolean(context, c_return);

        g_free(data);

    }
    
    g_free(p_0);

    g_free(p_1);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern gboolean  installer_update_partition_geometry(char * , double , double , JSData*);
static JSValueRef __update_partition_geometry__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 3) {
        js_fill_exception(context, exception,
            "the update_partition_geometry except 3 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    double p_1 = JSValueToNumber(context, arguments[1], NULL);

    double p_2 = JSValueToNumber(context, arguments[2], NULL);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gboolean c_return =  installer_update_partition_geometry (p_0, p_1, p_2, data);
        r = JSValueMakeBoolean(context, c_return);

        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern gboolean  installer_update_partition_fs(char * , char * , JSData*);
static JSValueRef __update_partition_fs__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 2) {
        js_fill_exception(context, exception,
            "the update_partition_fs except 2 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    gchar* p_1 = jsvalue_to_cstr(context, arguments[1]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gboolean c_return =  installer_update_partition_fs (p_0, p_1, data);
        r = JSValueMakeBoolean(context, c_return);

        g_free(data);

    }
    
    g_free(p_0);

    g_free(p_1);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern gboolean  installer_write_partition_mp(char * , char * , JSData*);
static JSValueRef __write_partition_mp__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 2) {
        js_fill_exception(context, exception,
            "the write_partition_mp except 2 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    gchar* p_1 = jsvalue_to_cstr(context, arguments[1]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gboolean c_return =  installer_write_partition_mp (p_0, p_1, data);
        r = JSValueMakeBoolean(context, c_return);

        g_free(data);

    }
    
    g_free(p_0);

    g_free(p_1);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern gboolean  installer_set_partition_flag(char * , char * , gboolean , JSData*);
static JSValueRef __set_partition_flag__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 3) {
        js_fill_exception(context, exception,
            "the set_partition_flag except 3 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    gchar* p_1 = jsvalue_to_cstr(context, arguments[1]);

    bool p_2 = JSValueToBoolean(context, arguments[2]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gboolean c_return =  installer_set_partition_flag (p_0, p_1, p_2, data);
        r = JSValueMakeBoolean(context, c_return);

        g_free(data);

    }
    
    g_free(p_0);

    g_free(p_1);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern gboolean  installer_write_disk(char * , JSData*);
static JSValueRef __write_disk__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 1) {
        js_fill_exception(context, exception,
            "the write_disk except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gboolean c_return =  installer_write_disk (p_0, data);
        r = JSValueMakeBoolean(context, c_return);

        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern gboolean  installer_mount_partition(char * , char * , JSData*);
static JSValueRef __mount_partition__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 2) {
        js_fill_exception(context, exception,
            "the mount_partition except 2 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    gchar* p_1 = jsvalue_to_cstr(context, arguments[1]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gboolean c_return =  installer_mount_partition (p_0, p_1, data);
        r = JSValueMakeBoolean(context, c_return);

        g_free(data);

    }
    
    g_free(p_0);

    g_free(p_1);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern void installer_update_bootloader(char * , gboolean , JSData*);
static JSValueRef __update_bootloader__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 2) {
        js_fill_exception(context, exception,
            "the update_bootloader except 2 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    bool p_1 = JSValueToBoolean(context, arguments[1]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

         installer_update_bootloader (p_0, p_1, data);
        r = JSValueMakeNull(context);

        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern void installer_start_part_operation(JSData*);
static JSValueRef __start_part_operation__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 0) {
        js_fill_exception(context, exception,
            "the start_part_operation except 0 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

         installer_start_part_operation (data);
        r = JSValueMakeNull(context);

        g_free(data);

    }
    
    if (_has_fatal_error)
        return NULL;
    else
        return r;
}


static const JSStaticFunction Installer_class_staticfuncs[] = {
    
{ "emit_webview_ok", __emit_webview_ok__, kJSPropertyAttributeReadOnly },

{ "restart_installer", __restart_installer__, kJSPropertyAttributeReadOnly },

{ "get_system_users", __get_system_users__, kJSPropertyAttributeReadOnly },

{ "create_user", __create_user__, kJSPropertyAttributeReadOnly },

{ "finish_install", __finish_install__, kJSPropertyAttributeReadOnly },

{ "finish_reboot", __finish_reboot__, kJSPropertyAttributeReadOnly },

{ "get_layout_description", __get_layout_description__, kJSPropertyAttributeReadOnly },

{ "get_keyboard_layouts", __get_keyboard_layouts__, kJSPropertyAttributeReadOnly },

{ "get_layout_variants", __get_layout_variants__, kJSPropertyAttributeReadOnly },

{ "get_current_layout_variant", __get_current_layout_variant__, kJSPropertyAttributeReadOnly },

{ "set_keyboard_layout_variant", __set_keyboard_layout_variant__, kJSPropertyAttributeReadOnly },

{ "keyboard_detect_read_step", __keyboard_detect_read_step__, kJSPropertyAttributeReadOnly },

{ "keyboard_detect_get_symbols", __keyboard_detect_get_symbols__, kJSPropertyAttributeReadOnly },

{ "keyboard_detect_get_present", __keyboard_detect_get_present__, kJSPropertyAttributeReadOnly },

{ "keyboard_detect_get_not_present", __keyboard_detect_get_not_present__, kJSPropertyAttributeReadOnly },

{ "keyboard_detect_get_keycodes", __keyboard_detect_get_keycodes__, kJSPropertyAttributeReadOnly },

{ "keyboard_detect_get_result", __keyboard_detect_get_result__, kJSPropertyAttributeReadOnly },

{ "get_timezone_list", __get_timezone_list__, kJSPropertyAttributeReadOnly },

{ "set_timezone", __set_timezone__, kJSPropertyAttributeReadOnly },

{ "extract_iso", __extract_iso__, kJSPropertyAttributeReadOnly },

{ "extract_squashfs", __extract_squashfs__, kJSPropertyAttributeReadOnly },

{ "extract_intelligent", __extract_intelligent__, kJSPropertyAttributeReadOnly },

{ "mount_procfs", __mount_procfs__, kJSPropertyAttributeReadOnly },

{ "chroot_target", __chroot_target__, kJSPropertyAttributeReadOnly },

{ "copy_whitelist", __copy_whitelist__, kJSPropertyAttributeReadOnly },

{ "draw_background", __draw_background__, kJSPropertyAttributeReadOnly },

{ "get_memory_size", __get_memory_size__, kJSPropertyAttributeReadOnly },

{ "get_keycode_from_keysym", __get_keycode_from_keysym__, kJSPropertyAttributeReadOnly },

{ "rand_uuid", __rand_uuid__, kJSPropertyAttributeReadOnly },

{ "list_disks", __list_disks__, kJSPropertyAttributeReadOnly },

{ "get_disk_path", __get_disk_path__, kJSPropertyAttributeReadOnly },

{ "get_disk_type", __get_disk_type__, kJSPropertyAttributeReadOnly },

{ "get_disk_model", __get_disk_model__, kJSPropertyAttributeReadOnly },

{ "get_disk_max_primary_count", __get_disk_max_primary_count__, kJSPropertyAttributeReadOnly },

{ "get_disk_length", __get_disk_length__, kJSPropertyAttributeReadOnly },

{ "get_disk_sector_size", __get_disk_sector_size__, kJSPropertyAttributeReadOnly },

{ "get_disk_partitions", __get_disk_partitions__, kJSPropertyAttributeReadOnly },

{ "is_device_slow", __is_device_slow__, kJSPropertyAttributeReadOnly },

{ "disk_support_efi", __disk_support_efi__, kJSPropertyAttributeReadOnly },

{ "get_partition_type", __get_partition_type__, kJSPropertyAttributeReadOnly },

{ "get_partition_name", __get_partition_name__, kJSPropertyAttributeReadOnly },

{ "get_partition_path", __get_partition_path__, kJSPropertyAttributeReadOnly },

{ "get_partition_mp", __get_partition_mp__, kJSPropertyAttributeReadOnly },

{ "unmount_partition", __unmount_partition__, kJSPropertyAttributeReadOnly },

{ "get_partition_start", __get_partition_start__, kJSPropertyAttributeReadOnly },

{ "get_partition_length", __get_partition_length__, kJSPropertyAttributeReadOnly },

{ "get_partition_end", __get_partition_end__, kJSPropertyAttributeReadOnly },

{ "get_partition_fs", __get_partition_fs__, kJSPropertyAttributeReadOnly },

{ "get_partition_label", __get_partition_label__, kJSPropertyAttributeReadOnly },

{ "get_partition_flag", __get_partition_flag__, kJSPropertyAttributeReadOnly },

{ "get_partition_busy", __get_partition_busy__, kJSPropertyAttributeReadOnly },

{ "get_partition_free", __get_partition_free__, kJSPropertyAttributeReadOnly },

{ "get_partition_os", __get_partition_os__, kJSPropertyAttributeReadOnly },

{ "new_disk_partition", __new_disk_partition__, kJSPropertyAttributeReadOnly },

{ "delete_disk_partition", __delete_disk_partition__, kJSPropertyAttributeReadOnly },

{ "update_partition_geometry", __update_partition_geometry__, kJSPropertyAttributeReadOnly },

{ "update_partition_fs", __update_partition_fs__, kJSPropertyAttributeReadOnly },

{ "write_partition_mp", __write_partition_mp__, kJSPropertyAttributeReadOnly },

{ "set_partition_flag", __set_partition_flag__, kJSPropertyAttributeReadOnly },

{ "write_disk", __write_disk__, kJSPropertyAttributeReadOnly },

{ "mount_partition", __mount_partition__, kJSPropertyAttributeReadOnly },

{ "update_bootloader", __update_bootloader__, kJSPropertyAttributeReadOnly },

{ "start_part_operation", __start_part_operation__, kJSPropertyAttributeReadOnly },

    { NULL, NULL, 0}
};
static const JSClassDefinition Installer_class_def = {
    0,
    kJSClassAttributeNone,
    "InstallerClass",
    NULL,
    NULL, //class_staticvalues,
    Installer_class_staticfuncs,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

JSClassRef get_Installer_class()
{
    static JSClassRef _class = NULL;
    if (_class == NULL) {
        _class = JSClassCreate(&Installer_class_def);
    }
    return _class;
}
