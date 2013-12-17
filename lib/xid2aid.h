#ifndef _XID2AID_H__
#define _XID2AID_H__
#include <glib.h>

enum APPID_FINDER_FILTER {
    APPID_FILTER_ARGS=1,
    APPID_FILTER_WMCLASS,
    APPID_FILTER_WMINSTANCE,
    APPID_FILTER_WMNAME,
    APPID_FILTER_ICON_NAME,
    APPID_FILTER_EXEC_NAME,
};

enum APPID_ICON_OPERATOR {
    ICON_OPERATOR_USE_ICONNAME=0,
    ICON_OPERATOR_USE_RUNTIME_WITH_BOARD,
    ICON_OPERATOR_USE_RUNTIME_WITHOUT_BOARD,
    ICON_OPERATOR_USE_PATH,
    ICON_OPERATOR_SET_DOMINANTCOLOR
};

char* find_app_id(const char* exec_name, const char* key, int filter);
void get_pid_info(int pid, char** exec_name, char** exec_args);
char* get_exe(const char* app_id, int pid);
gboolean is_app_in_white_list(const char* name);

gboolean is_deepin_app_id(const char* app_id);
int get_deepin_app_id_operator(const char* app_id);
char* get_deepin_app_id_value(const char* app_id);

#endif
