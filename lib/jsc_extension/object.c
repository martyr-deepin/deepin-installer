#include "jsextension.h"
#include <glib-object.h>

struct _ObjectData {
    long id;
    void* core;
    NObjectRef ref;
    NObjectUnref unref;
};

static
void object_init(JSContextRef ctx, JSObjectRef object)
{
    (void)ctx;
    struct _ObjectData* data = JSObjectGetPrivate(object);
    g_assert(data != NULL);
    if (data->ref) {
        data->ref(data->core);
    }
}

static
void object_finlize(JSObjectRef object)
{
    struct _ObjectData* data = JSObjectGetPrivate(object);
    g_assert(data != NULL);
    if (data->unref) {
        data->unref(data->core);
    }
    data->core = NULL;
    g_free(data);
}

static
JSClassRef obj_class()
{
    static JSClassRef objclass = NULL;
    if (objclass == NULL) {
        JSClassDefinition class_def = {
            0,
            kJSClassAttributeNone,
            "DeepinObject",
            NULL,

            NULL, //static value
            NULL, //static function

            object_init,
            object_finlize,
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
        objclass = JSClassCreate(&class_def);
    }
    return objclass;
}

JSObjectRef create_nobject(JSContextRef ctx,
                           void* obj,
                           NObjectRef ref,
                           NObjectUnref unref)
{
    g_assert(obj != NULL);
    struct _ObjectData* data = g_new(struct _ObjectData, 1);
    data->id = (long)obj;
    data->core = obj;
    data->ref = ref;
    data->unref = unref;
    GRAB_CTX();
    JSObjectRef r = JSObjectMake(ctx, obj_class(), data);
    UNGRAB_CTX();
    return r;
}

JSObjectRef create_nobject_and_own(JSContextRef ctx,
                                   void* obj,
                                   NObjectRef ref,
                                   NObjectUnref unref)
{
    JSObjectRef r = create_nobject(ctx, obj, ref, unref);
    if (unref) {
        unref(obj);
    }
    return r;
}

static
void* object_to_core(JSObjectRef object)
{
    struct _ObjectData* data = JSObjectGetPrivate(object);
    g_assert(data != NULL);
    return data->core;
}

void* jsvalue_to_nobject(JSContextRef ctx, JSValueRef value)
{
    GRAB_CTX();
    if (JSValueIsObjectOfClass(ctx, value, obj_class())) {
        JSObjectRef obj = JSValueToObject(ctx, value, NULL);
        void* core = object_to_core(obj);
        UNGRAB_CTX();
        return core;
    } else {
        UNGRAB_CTX();
        g_warning("[%s] This JSValueRef is not an DeepinObject!\n", __func__);
        return NULL;
    }
}
