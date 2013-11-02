#ifndef __WEBDESKIT_PLUGIN_H__
#define __WEBDESKIT_PLUGIN_H__

#include <stdlib.h>
#include <JavaScriptCore/JavaScript.h>

typedef JSValueRef(*webdeskit_js_callback_f)(JSContextRef context,
                                          JSObjectRef function,
                                          JSObjectRef self,
                                          size_t argc,
                                          const JSValueRef argv[],
                                          JSValueRef* exception);

typedef struct webdeskit_plugin_interface_s *webdeskit_plugin_interface_t;
typedef struct webdeskit_plugin_interface_s
{
    void(*exit)(webdeskit_plugin_interface_t);
} webdeskit_plugin_interface_s;

typedef struct webdeskit_host_interface_s *webdeskit_host_interface_t;
typedef struct webdeskit_host_interface_s
{
    int(*const register_native_method)(const char *, webdeskit_js_callback_f);
} webdeskit_host_interface_s;

typedef webdeskit_plugin_interface_t(*webdeskit_plugin_init_f)(webdeskit_host_interface_t);

#endif
