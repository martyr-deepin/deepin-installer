cmake_minimum_required(VERSION 2.8)
include_directories(${DWEBVIEW_INCLUDE_DIRS})


GEN_JSC_SOURCE_WITH_NO_INIT(JSC_GEN "jsc_extension")

AUX_SOURCE_DIRECTORY(jsc_extension JSC_LIB)

set(SRC_LIST background.c utils.c webview.c xdg_misc.c pixbuf.c X_misc.c
    ${JSC_LIB} ${JSC_GEN} singleton.c i18n.c display_info.c)

add_library(dcom ${SRC_LIST})
target_link_libraries(dcom ${DWEBVIEW_LIBRAIRES})

add_subdirectory(dbus)
