
#include "jsextension.h"
#include <JavaScriptCore/JSStringRef.h>
extern JSClassRef get_DCore_class();
extern JSClassRef get_DBus_class();
extern JSClassRef get_Installer_class();

JSGlobalContextRef global_ctx = NULL;
void* __webview = NULL;
void* get_global_webview()
{
    return __webview;
}

JSGlobalContextRef get_global_context()
{
    return global_ctx;
}
gboolean invoke_js_garbage()
{
    JSGarbageCollect(global_ctx);
    return TRUE;
}
void modules_reload()
{
    extern void dbus_reload() __attribute__((weak));
 if (& dbus_reload)dbus_reload();
extern void installer_reload() __attribute__((weak));
 if (& installer_reload)installer_reload();
;
}
void init_js_extension(JSGlobalContextRef context, void* webview)
{
    if (global_ctx == NULL)
        g_timeout_add_seconds(5, (GSourceFunc)invoke_js_garbage, NULL);
    global_ctx = context;
    modules_reload();
    __webview = webview;
    JSObjectRef global_obj = JSContextGetGlobalObject(context);
    JSObjectRef class_DCore = JSObjectMake(context, get_DCore_class(), NULL);

    
    JSObjectRef class_DBus = JSObjectMake(context, get_DBus_class(), NULL);
    JSStringRef str_DBus = JSStringCreateWithUTF8CString("DBus");
    JSObjectSetProperty(context, class_DCore, str_DBus, class_DBus,
            kJSClassAttributeNone, NULL);
    JSStringRelease(str_DBus);

    JSObjectRef class_Installer = JSObjectMake(context, get_Installer_class(), NULL);
    JSStringRef str_Installer = JSStringCreateWithUTF8CString("Installer");
    JSObjectSetProperty(context, class_DCore, str_Installer, class_Installer,
            kJSClassAttributeNone, NULL);
    JSStringRelease(str_Installer);


    JSStringRef str = JSStringCreateWithUTF8CString("DCore");
    JSObjectSetProperty(context, global_obj, str, class_DCore,
            kJSClassAttributeNone, NULL);
    JSStringRelease(str);
}
