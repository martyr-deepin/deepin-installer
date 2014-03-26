
#include <JavaScriptCore/JSContextRef.h>
#include <JavaScriptCore/JSStringRef.h>
#include "jsextension.h"
#include <glib.h>
#include <glib-object.h>

extern void* get_global_webview();


extern char *  dcore_gen_id(char * , JSData*);
static JSValueRef __gen_id__ (JSContextRef noused_context,
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
            "the gen_id except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gchar* c_return =  dcore_gen_id (p_0, data);
        
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

extern void dcore_signal_connect(char * , JSValueRef , JSData*);
static JSValueRef __signal_connect__ (JSContextRef noused_context,
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
            "the signal_connect except 2 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);
JSValueRef p_1 = arguments[1];

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

         dcore_signal_connect (p_0, p_1, data);
        r = JSValueMakeNull(context);

        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern char *  dcore_gettext(char * , JSData*);
static JSValueRef __gettext__ (JSContextRef noused_context,
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
            "the gettext except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        const char* c_return =  dcore_gettext (p_0, data);
        
        if (c_return != NULL) {
            r = jsvalue_from_cstr(context, c_return);
        } else {
            _has_fatal_error = TRUE;
            js_fill_exception(context, exception, "the return string is NULL");
        }


        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern char *  dcore_dgettext(char * , char * , JSData*);
static JSValueRef __dgettext__ (JSContextRef noused_context,
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
            "the dgettext except 2 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    gchar* p_1 = jsvalue_to_cstr(context, arguments[1]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        const char* c_return =  dcore_dgettext (p_0, p_1, data);
        
        if (c_return != NULL) {
            r = jsvalue_from_cstr(context, c_return);
        } else {
            _has_fatal_error = TRUE;
            js_fill_exception(context, exception, "the return string is NULL");
        }


        g_free(data);

    }
    
    g_free(p_0);

    g_free(p_1);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern void dcore_bindtextdomain(char * , char * , JSData*);
static JSValueRef __bindtextdomain__ (JSContextRef noused_context,
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
            "the bindtextdomain except 2 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    gchar* p_1 = jsvalue_to_cstr(context, arguments[1]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

         dcore_bindtextdomain (p_0, p_1, data);
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

extern char *  dcore_get_theme_icon(char * , double , JSData*);
static JSValueRef __get_theme_icon__ (JSContextRef noused_context,
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
            "the get_theme_icon except 2 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    double p_1 = JSValueToNumber(context, arguments[1], NULL);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gchar* c_return =  dcore_get_theme_icon (p_0, p_1, data);
        
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

extern char *  dcore_get_name_by_appid(char * , JSData*);
static JSValueRef __get_name_by_appid__ (JSContextRef noused_context,
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
            "the get_name_by_appid except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gchar* c_return =  dcore_get_name_by_appid (p_0, data);
        
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

extern JSValueRef  dcore_get_plugins(char * , JSData*);
static JSValueRef __get_plugins__ (JSContextRef noused_context,
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
            "the get_plugins except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        JSValueRef c_return =  dcore_get_plugins (p_0, data);
        r = c_return;

        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern void dcore_enable_plugin(char * , gboolean , JSData*);
static JSValueRef __enable_plugin__ (JSContextRef noused_context,
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
            "the enable_plugin except 2 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    bool p_1 = JSValueToBoolean(context, arguments[1]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

         dcore_enable_plugin (p_0, p_1, data);
        r = JSValueMakeNull(context);

        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern void dcore_init_plugins(char * , JSData*);
static JSValueRef __init_plugins__ (JSContextRef noused_context,
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
            "the init_plugins except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

         dcore_init_plugins (p_0, data);
        r = JSValueMakeNull(context);

        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern JSValueRef  dcore_get_plugin_info(char * , JSData*);
static JSValueRef __get_plugin_info__ (JSContextRef noused_context,
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
            "the get_plugin_info except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        JSValueRef c_return =  dcore_get_plugin_info (p_0, data);
        r = c_return;

        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern void dcore_new_window(char * , char * , double , double , JSData*);
static JSValueRef __new_window__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != 4) {
        js_fill_exception(context, exception,
            "the new_window except 4 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    gchar* p_1 = jsvalue_to_cstr(context, arguments[1]);

    double p_2 = JSValueToNumber(context, arguments[2], NULL);

    double p_3 = JSValueToNumber(context, arguments[3], NULL);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

         dcore_new_window (p_0, p_1, p_2, p_3, data);
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

extern gboolean  dcore_open_browser(char * , JSData*);
static JSValueRef __open_browser__ (JSContextRef noused_context,
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
            "the open_browser except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        gboolean c_return =  dcore_open_browser (p_0, data);
        r = JSValueMakeBoolean(context, c_return);

        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}


static const JSStaticFunction DCore_class_staticfuncs[] = {
    
{ "gen_id", __gen_id__, kJSPropertyAttributeReadOnly },

{ "signal_connect", __signal_connect__, kJSPropertyAttributeReadOnly },

{ "gettext", __gettext__, kJSPropertyAttributeReadOnly },

{ "dgettext", __dgettext__, kJSPropertyAttributeReadOnly },

{ "bindtextdomain", __bindtextdomain__, kJSPropertyAttributeReadOnly },

{ "get_theme_icon", __get_theme_icon__, kJSPropertyAttributeReadOnly },

{ "get_name_by_appid", __get_name_by_appid__, kJSPropertyAttributeReadOnly },

{ "get_plugins", __get_plugins__, kJSPropertyAttributeReadOnly },

{ "enable_plugin", __enable_plugin__, kJSPropertyAttributeReadOnly },

{ "init_plugins", __init_plugins__, kJSPropertyAttributeReadOnly },

{ "get_plugin_info", __get_plugin_info__, kJSPropertyAttributeReadOnly },

{ "new_window", __new_window__, kJSPropertyAttributeReadOnly },

{ "open_browser", __open_browser__, kJSPropertyAttributeReadOnly },

    { NULL, NULL, 0}
};
static const JSClassDefinition DCore_class_def = {
    0,
    kJSClassAttributeNone,
    "DCoreClass",
    NULL,
    NULL, //class_staticvalues,
    DCore_class_staticfuncs,
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

JSClassRef get_DCore_class()
{
    static JSClassRef _class = NULL;
    if (_class == NULL) {
        _class = JSClassCreate(&DCore_class_def);
    }
    return _class;
}
