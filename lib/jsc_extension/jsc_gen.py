#!/usr/bin/python2

#Copyright (c) 2011 ~ 2012 Deepin, Inc.
#              2011 ~ 2012 snyh
#
#Author:      snyh <snyh@snyh.org>
#Maintainer:  snyh <snyh@snyh.org>
#
#This program is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 3 of the License, or
#(at your option) any later version.
#
#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with this program; if not, see <http://www.gnu.org/licenses/>.

import sys
import os
modules = []


def register(m):
    if m.up_class.name != "DCore" and modules.count(m.up_class) == 0:
        modules.append(m.up_class)
    modules.append(m);



class Params:
    def __init__(self, name=None, description=None):
        self.name = name
        self.description = description
    def set_position(self, pos):
        self.position = pos
    def in_after(self):
        return ""
    def doc(self):
        pass
    def is_array(self):
        return False
    def fetch_c_return(self):
        return ""

class Array(Params):
    temp_in = """
    ArrayContainer p_%(pos)d;
    if (jsvalue_instanceof(context, arguments[%(pos)d], "Array")) {

        JSPropertyNameArrayRef prop_names = JSObjectCopyPropertyNames(context, (JSObjectRef)arguments[%(pos)d]);
        p_%(pos)d.num = JSPropertyNameArrayGetCount(prop_names) - 1;
        JSPropertyNameArrayRelease(prop_names);

        %(element_type)s* _array = g_new0(%(element_type)s, p_%(pos)d.num);

        for (guint i=0; i<p_%(pos)d.num; i++) {
            JSValueRef value = JSObjectGetPropertyAtIndex(context, (JSObjectRef)arguments[%(pos)d], i, NULL);
            _array[i] = %(element_alloc)s;
        }
        p_%(pos)d.data = (void*)_array;
    }
"""
    temp_out = """
        JSValueRef *args = g_new(JSValueRef, ac.num);
        %(element_type)s* c_array = ac.data;
        for (size_t i=0; i<ac.num; i++) {
            %(element_type)s value = c_array[i];
            args[i] = %(fetch_js_value)s;
        }
        g_free(ac.data);
        r = JSObjectMakeArray(context, ac.num, args, NULL);
        g_free(args);
"""
    def is_array(self):
        return True
    def type(self):
        return "ArrayContainer"
    def fetch_c_return(self):
        return "ArrayContainer ac = "


class Property:
    def __init__(self, *args):
        self.properties = args
    def str(self):
        tmp = """
JSValueRef %(set_func)s (JSContextRef ctx, JSObjectRef obj,
                JSStringRef prop_name, JSValueRef* exception)
{
}
"""
        return tmp

class Object(Params):
    def type(self):
        return "void* "
    def __init__(self, name=None, desc=None, ref=None, unref=None):
        Params.__init__(self, name, desc)
        self.ref = ref or "g_object_ref"
        self.unref = unref or "g_object_unref"
    def fetch_c_return(self):
        return "void* c_return = "
    def convert_return_value(self):
        return "r = create_nobject_and_own(context, c_return, %s, %s);" % (self.ref, self.unref)

    def in_before(self):
        return """
    void* p_%(pos)d = jsvalue_to_nobject(context, arguments[%(pos)d]);
    if (p_%(pos)d == NULL) {
        _has_fatal_error =TRUE;
        js_fill_exception(context, exception, "the p_%(pos)d is not an DeepinNativeObject");
    }
""" % { "pos": self.position }

class Number(Params):
    def in_before(self):
        return """
    double p_%(pos)d = JSValueToNumber(context, arguments[%(pos)d], NULL);
""" % { "pos": self.position }

    def type(self):
        return "double "

    def fetch_c_return(self):
        return "double c_return = "
    def convert_return_value(self):
        return "r = JSValueMakeNumber(context, c_return);"

class Boolean(Params):
    def in_before(self):
        return """
    bool p_%(pos)d = JSValueToBoolean(context, arguments[%(pos)d]);
"""  % {"pos": self.position}

    def type(self):
        return "gboolean "
    def fetch_c_return(self):
        return "gboolean c_return = "
    def convert_return_value(self):
        return "r = JSValueMakeBoolean(context, c_return);";

class ABoolean(Array):
    def in_before(self):
        return Array.temp_in % {'element_type': 'gboolean', 'pos': self.position, 'element_alloc': "JSValueToBoolean(context, value)"}
    def in_after(self):
        return """
    g_free(p_%(pos)d.data);
""" % {'pos': self.position}

class ANumber(Array):
    def in_before(self):
        return Array.temp_in % {'element_type': 'double', 'pos': self.position, 'element_alloc': "JSValueToNumber(context, value, NULL)"}
    def in_after(self):
        return """
    g_free(p_%(pos)d.data);
""" % {'pos': self.position}


class AString(Array):
    def in_before(self):
        return Array.temp_in % {'element_type': 'char*', 'pos': self.position, 'element_alloc': "jsvalue_to_cstr(context, value)"}

    def in_after(self):
        temp_clear = """
    char** to_free_%(pos)d = p_%(pos)d.data;
    for (guint i=0; i<p_%(pos)d.num; i++) {
        g_free(to_free_%(pos)d[i]);
    }
    g_free(p_%(pos)d.data);
"""
        return temp_clear % {'pos': self.position}

    def convert_return_value(self):
        return Array.temp_out % {'fetch_js_value': "jsvalue_from_cstr(context, value)", 'element_type': "char*"}

class AObject(Array):
    def __init__(self, name=None, desc=None, ref=None, unref=None):
        Array.__init__(self, name, desc)
        self.ref = ref or "g_object_ref"
        self.unref = unref or "g_object_unref"
    def in_before(self):
        return Array.temp_in % {'element_type': 'void*', 'pos': self.position, 'element_alloc': "jsvalue_to_nobject(context, value)"}
    def convert_return_value(self):
        return Array.temp_out % {
                'fetch_js_value': "create_nobject_and_own(context, value, %s, %s)" % (self.ref, self.unref), "element_type": "JSObjectRef" }

class String(Params):
    def type(self):
        return "char * "

    def in_before(self):
        temp = """
    gchar* p_%(pos)d = jsvalue_to_cstr(context, arguments[%(pos)d]);
"""
        return temp % {'pos': self.position}

    def in_after(self):
        temp = """
    g_free(p_%(pos)d);
"""
        return temp % {'pos': self.position}

    def fetch_c_return(self):
        return "gchar* c_return = "

    def convert_return_value(self):
        return """
        if (c_return != NULL) {
            r = jsvalue_from_cstr(context, c_return);
            g_free(c_return);
        } else {
            r = JSValueMakeNull(context);
        }
"""

class CString(String):
    def fetch_c_return(self):
        return "const char* c_return = "
    def convert_return_value(self):
        return """
        if (c_return != NULL) {
            r = jsvalue_from_cstr(context, c_return);
        } else {
            _has_fatal_error = TRUE;
            js_fill_exception(context, exception, "the return string is NULL");
        }
"""

class Signal:
    def __init__(self, *params):
        pass

class CustomFunc:
    def __init__(self, name):
        self.name = name
    def set_module_name(self, name):
        self.module_name = name.lower()
    def str_def(self):
        return """
    { "%(name)s", %(name)s, kJSPropertyAttributeReadOnly },
""" % { "name" : self.name }
    def str(self):
        return """
extern JSValueRef %(name)s(JSContextRef context,
                        JSObjectRef function,
                        JSObjectRef thisObject,
                        size_t argumentCount,
                        const JSValueRef arguments[],
                        JSValueRef *exception);
""" % { "name" : self.name }


class Function:
    temp = """
extern %(raw_return)s %(module_name)s_%(name)s(%(raw_params)s);
static JSValueRef __%(name)s__ (JSContextRef noused_context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    (void)noused_context;
    (void)function;
    (void)thisObject;
    (void)argumentCount;
    (void)arguments;
    (void)exception;
    JSContextRef context = get_global_context();
    gboolean _has_fatal_error = FALSE;
    JSValueRef r = NULL;
    if (argumentCount != %(p_num)d) {
        js_fill_exception(context, exception,
            "the %(name)s except %(p_num)d paramters but passed in %%d", argumentCount);
        return NULL;
    }
    %(params_init)s
    if (!_has_fatal_error) {
        %(func_call)s
    }
    %(params_clear)s
    if (_has_fatal_error)
        return NULL;
    else
        return r;
}
"""
    test_function = """
#ifdef __DUI_DEBUG
extern void %(module_name)s_%(name)s();
gboolean %(module_name)s_test_wrap()
{
    %(module_name)s_%(name)s();
    return FALSE;
}
#endif
static JSValueRef __%(name)s__ (JSContextRef ctx, JSObjectRef f, JSObjectRef this, size_t c, const JSValueRef args[], JSValueRef* excp)
{
    (void)f;
    (void)this;
    (void)c;
    (void)args;
    (void)excp;
#ifdef __DUI_DEBUG
    g_timeout_add(3000, (GSourceFunc)%(module_name)s_test_wrap, NULL);
#endif
    return JSValueMakeNull(ctx);
}
"""

    def __init__(self, name, r_value, *params):
        self.params = params
        self.name = name
        self.r_value = r_value
    def set_module_name(self, name):
        self.module_name = name.lower()

    def str(self):
        if self.name == "test":
            return Function.test_function % {
                    "module_name": self.module_name,
                    "name": "test"
                    }
        i = 0
        params_init = ""
        params_clear = ""
        raw_params = []
        for p in self.params:
            p.set_position(i)
            i += 1
            params_init += p.in_before()
            params_clear += p.in_after()
            raw_params.append(p.type())

        raw_params.append("JSData*")
        return Function.temp % {
                "raw_return" : self.r_value.type(),
                "raw_params" : ', '.join(raw_params),
                "module_name": self.module_name,
                "name" : self.name,
                "p_num" : i,
                "params_init" : params_init,
                "func_call" : self.func_call(),
                "params_clear" : params_clear,
                }

    temp_return = """
        JSData* data = g_new0(JSData, 1);
        data->exception = exception;
        data->webview = get_global_webview();

        %(store_c_return)s %(module_name)s_%(name)s (%(params)s);
        %(convert_c_to_js)s

        g_free(data);
"""
    def func_call(self):
        params_str = []
        for p in self.params:
            params_str.append("p_%d" % p.position)
        params_str.append("data");
        return Function.temp_return % {
                "module_name": self.module_name,
                "store_c_return" : self.r_value.fetch_c_return(),
                "convert_c_to_js" : self.r_value.convert_return_value(),
                "name": self.name,
                "params" : ', '.join(params_str)
                }
    def str_def(self):
        return """
{ "%(name)s", __%(name)s__, kJSPropertyAttributeReadOnly },
""" % { "name": self.name}

class Class:
    temp_class_def = """
#include <JavaScriptCore/JSContextRef.h>
#include <JavaScriptCore/JSStringRef.h>
#include "jsextension.h"
#include <glib.h>
#include <glib-object.h>

extern void* get_global_webview();

%(funcs_def)s

static const JSStaticFunction %(name)s_class_staticfuncs[] = {
    %(funcs_state)s
    { NULL, NULL, 0}
};
static const JSClassDefinition %(name)s_class_def = {
    0,
    kJSClassAttributeNone,
    "%(name)sClass",
    NULL,
    NULL, //class_staticvalues,
    %(name)s_class_staticfuncs,
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

JSClassRef get_%(name)s_class()
{
    static JSClassRef _class = NULL;
    if (_class == NULL) {
        _class = JSClassCreate(&%(name)s_class_def);
    }
    return _class;
}
"""

    def __init__(self, name, desc=None, *args):
        self.name = name
        self.description = desc
        self.funcs = []
        self.values = []
        self.child_modules = []



        for arg in args:
            if isinstance(arg, Function) or isinstance(arg, CustomFunc):
                arg.set_module_name(self.name)
                self.funcs.append(arg)
            elif isinstance(arg, Value):
                self.values.append(arg)
            elif isinstance(arg, Class):
                arg.up_class = self
                arg.name = arg.up_class.name + "_" + arg.name
                self.child_modules.append(arg)

        class PseudoMoudle:
            name = "DCore"
        if not hasattr(self, "up_class"):
            self.up_class = PseudoMoudle

        register(self)


    def str(self):
        funcs_def = ""
        funcs_state = ""
        for f in self.funcs:
            funcs_def += f.str()
            funcs_state += f.str_def()
        contents = Class.temp_class_def % {
                "name" : self.name,
                "funcs_def" : funcs_def,
                "funcs_state" : funcs_state
                }

        for m in self.child_modules:
            contents += m.str()

        return contents;

    def str_install(self):
        temp = """
    JSObjectRef class_%(name)s = JSObjectMake(context, get_%(name)s_class(), NULL);
    JSStringRef str_%(name)s = JSStringCreateWithUTF8CString("%(name)s");
    JSObjectSetProperty(context, class_%(up_class)s, str_%(name)s, class_%(name)s,
            kJSClassAttributeNone, NULL);
    JSStringRelease(str_%(name)s);
"""
        return temp % {"name" : self.name, "up_class" : self.up_class.name}

class Null(Params):
    def __call__(self):
        return self
    def type(self):
        return "void"
    def convert_return_value(self):
        return "r = JSValueMakeNull(context);"

class JSCode(Params):
    def return_value(self):
        return """
    return r;
"""
    def type(self):
        return "char *";

    def fetch_c_return(self):
        return "gchar* c_return = "
    def convert_return_value(self):
        #TODO: should return JSValueNull or raise Exception?
        return """
        if (c_return == NULL) {
            r = JSValueMakeNull(context);
        } else {
            r = json_from_cstr(context, c_return);
            g_free(c_return);
        }
"""
class CJSCode(JSCode):
    def type(self):
        return "const char*";
    def fetch_c_return(self):
        return "const gchar* c_return = "
    def convert_return_value(self):
        #TODO: should return JSValueNull or raise Exception?
        return """
        if (c_return == NULL) {
            r = JSValueMakeNull(context);
        } else {
            r = json_from_cstr(context, c_return);
        }
"""

class JSValueRef(Params):
    def type(self):
        return "JSValueRef "
    def in_before(self):
        return "JSValueRef p_%(pos)d = arguments[%(pos)d];\n" % {"pos":self.position}

    def fetch_c_return(self):
        return "JSValueRef c_return = "
    def convert_return_value(self):
        return "r = c_return;"

class Data(Params):
    pass

class Description:
    def __init__(self, t):
        pass

class Value:
    def __init__(self, name):
        pass

def gen_init_c(output_path, init_name):
    temp = """
#include "jsextension.h"
#include <JavaScriptCore/JSStringRef.h>
extern JSClassRef get_DCore_class();
%(objs_state)s
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
    GRAB_CTX();
    JSGarbageCollect(global_ctx);
    UNGRAB_CTX();
    return TRUE;
}
void modules_reload()
{
    %(modules_reload)s;
}
void init_js_extension(JSGlobalContextRef context, void* webview)
{
    global_ctx = context;
    modules_reload();
    __webview = webview;
    JSObjectRef global_obj = JSContextGetGlobalObject(context);
    JSObjectRef class_DCore = JSObjectMake(context, get_DCore_class(), NULL);

    %(objs)s

    JSStringRef str = JSStringCreateWithUTF8CString("DCore");
    JSObjectSetProperty(context, global_obj, str, class_DCore,
            kJSClassAttributeNone, NULL);
    JSStringRelease(str);
}
"""
    objs = ""
    objs_state = ""
    modules_reload = ""
    modules.reverse()
    for m in modules:
        if m.name != "DCore":
            objs += m.str_install()
            objs_state += "extern JSClassRef get_%s_class();\n" % m.name
            modules_reload += "extern void %(name)s_reload() __attribute__((weak));\n if (& %(name)s_reload)%(name)s_reload();\n" % { "name" : m.name.lower() }

    f = open(output_path+ "/" + init_name, "w")
    f.write(temp % {"objs": objs, "objs_state": objs_state, "modules_reload": modules_reload })
    f.close()

def gen_module_c(output_path, cfg_dir, cfg_list=None):
    for root, dirs, files in os.walk(cfg_dir):
        for f in files:
            if f.endswith('.cfg') and f[0] != '.':
                if len(cfg_list) != 0 and not f in cfg_list:
                    continue
                path = os.path.join(root, f)
                path2 = os.path.join(output_path,  "_gen_" + f.rstrip(".cfg") + ".c")
                f = open(path)
                content = f.read()
                try :
                    m = eval(content)
                except:
                    print "Warnings: format error. (%s)" % path
                    f.close()
                    raise
                else:
                    f = open(path2, "w")
                    f.write(m.str())
                    f.close()

if __name__ == "__main__":
    from optparse import OptionParser
    parser = OptionParser()
    parser.add_option("-I", "--no-init", default=False, dest="NO_INIT", help="not generate the init.c file")
    parser.add_option("-i", "--init-name", dest="INIT_NAME", default="init.c", help="set the init module's name")
    parser.add_option("-s", "--source-dir", dest="CFG_DIR", help="the source directory of cfg files")
    parser.add_option("-d", "--dest-dir", dest="DEST", help="the directory to store the generate c source file")
    (options, cfg_list) = parser.parse_args()

    output_path = options.DEST or os.path.curdir
    try:
        os.mkdir(output_path)
    except:
        pass
    gen_module_c(output_path, options.CFG_DIR, cfg_list)
    if not options.NO_INIT:
        gen_init_c(output_path, options.INIT_NAME)
