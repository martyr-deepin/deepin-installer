
#include <JavaScriptCore/JSContextRef.h>
#include <JavaScriptCore/JSStringRef.h>
#include "jsextension.h"
#include <glib.h>
#include <glib-object.h>

extern void* get_global_webview();


extern JSValueRef  dbus_sys(char * , JSData*);
static JSValueRef __sys__ (JSContextRef noused_context,
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
            "the sys except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        JSValueRef c_return =  dbus_sys (p_0, data);
        r = c_return;

        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern JSValueRef  dbus_sys_object(char * , char * , char * , JSData*);
static JSValueRef __sys_object__ (JSContextRef noused_context,
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
            "the sys_object except 3 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    gchar* p_1 = jsvalue_to_cstr(context, arguments[1]);

    gchar* p_2 = jsvalue_to_cstr(context, arguments[2]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        JSValueRef c_return =  dbus_sys_object (p_0, p_1, p_2, data);
        r = c_return;

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

extern JSValueRef  dbus_session(char * , JSData*);
static JSValueRef __session__ (JSContextRef noused_context,
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
            "the session except 1 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        JSValueRef c_return =  dbus_session (p_0, data);
        r = c_return;

        g_free(data);

    }
    
    g_free(p_0);

    if (_has_fatal_error)
        return NULL;
    else
        return r;
}

extern JSValueRef  dbus_session_object(char * , char * , char * , JSData*);
static JSValueRef __session_object__ (JSContextRef noused_context,
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
            "the session_object except 3 paramters but passed in %d", argumentCount);
        return NULL;
    }
    
    gchar* p_0 = jsvalue_to_cstr(context, arguments[0]);

    gchar* p_1 = jsvalue_to_cstr(context, arguments[1]);

    gchar* p_2 = jsvalue_to_cstr(context, arguments[2]);

    if (!_has_fatal_error) {
        
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        JSValueRef c_return =  dbus_session_object (p_0, p_1, p_2, data);
        r = c_return;

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


static const JSStaticFunction DBus_class_staticfuncs[] = {
    
{ "sys", __sys__, kJSPropertyAttributeReadOnly },

{ "sys_object", __sys_object__, kJSPropertyAttributeReadOnly },

{ "session", __session__, kJSPropertyAttributeReadOnly },

{ "session_object", __session_object__, kJSPropertyAttributeReadOnly },

    { NULL, NULL, 0}
};
static const JSClassDefinition DBus_class_def = {
    0,
    kJSClassAttributeNone,
    "DBusClass",
    NULL,
    NULL, //class_staticvalues,
    DBus_class_staticfuncs,
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

JSClassRef get_DBus_class()
{
    static JSClassRef _class = NULL;
    if (_class == NULL) {
        _class = JSClassCreate(&DBus_class_def);
    }
    return _class;
}
