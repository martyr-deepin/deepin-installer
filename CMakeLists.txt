cmake_minimum_required(VERSION 2.8)

option(DEVELOP "enable develop mode (use src's resource)" false)
option(TEST "enable test mode (run project's test code)" false)
option(USE_GCCGO "use gccgo or cgo" false)

set(LIVE_METHOD "live-boot" CACHE STRING "live-method support (live-boot or casper)")

macro(GEN_JSC_SOURCE2 JSC_GEN_SRC CFG_DIR OUTPUT)
    set(OUT ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT})
    execute_process(
        COMMAND ${BIN_JSC_GEN} -d "${OUT}" -s "${CMAKE_CURRENT_SOURCE_DIR}/${CFG_DIR}" ${ARGN}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
    AUX_SOURCE_DIRECTORY(${OUT} ${JSC_GEN_SRC})
endmacro()


macro(GEN_JSC_SOURCE JSC_GEN CFG_DIR)
    execute_process(
        COMMAND ${BIN_JSC_GEN} -s "${CMAKE_CURRENT_SOURCE_DIR}/${CFG_DIR}" ${ARGN}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
    AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_BINARY_DIR} ${JSC_GEN})
endmacro()

macro(GEN_JSC_SOURCE_WITH_NO_INIT JSC_GEN CFG_DIR)
    execute_process(
        COMMAND ${BIN_JSC_GEN} -s "${CMAKE_CURRENT_SOURCE_DIR}/${CFG_DIR}" "-I=True"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        )
    AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_BINARY_DIR} ${JSC_GEN})
endmacro()

macro(GEN_JS_FILE NAME)
    set(_DIR ${PROJECT_SOURCE_DIR}/resources/${NAME}/js)
    add_custom_target(${NAME}_js
        COMMAND make -C ${_DIR}
        DEPENDS ${_DIR}/makefile
    )
    execute_process(
        COMMAND make -C ${_DIR}
    )
endmacro()

if (DEVELOP)
    SET(RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")
    add_definitions("-DHOOKS_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/hooks\"")
else()
    SET(RESOURCE_DIR "${CMAKE_INSTALL_PREFIX}/share/deepin-installer/resources")
    add_definitions("-DHOOKS_DIR=\"${CMAKE_INSTALL_PREFIX}/share/deepin-installer/hooks\"")
endif()

if (TEST)
    add_definitions("-D__DUI_DEBUG=1")
endif()

add_definitions("-DRESOURCE_DIR=\"${RESOURCE_DIR}\"")

find_package(PkgConfig)
pkg_check_modules(WEBKIT REQUIRED deepin-webkit-3.0)

set(CMAKE_C_FLAGS "-std=c99")


set(JSC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/lib/jsc_extension/)
include_directories(${JSC_HEADER})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)
include_directories(${PROJECT_SOURCE_DIR}/test/)

set(BIN_JSC_GEN ${CMAKE_CURRENT_SOURCE_DIR}/lib/jsc_extension/jsc_gen.py)

set(DWEBVIEW_INCLUDE_DIRS ${WEBKIT_INCLUDE_DIRS})
set(DTEST_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/test)
set(DWEBVIEW_LIBRARIES ${WEBKIT_LIBRARIES} ${DBUS_LIBRARIES} m X11 Xext sqlite3)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})


add_subdirectory(lib)
add_subdirectory(app)
add_subdirectory(po)
add_subdirectory(test)
add_subdirectory(tools)

#install(
#    DIRECTORY ${PROJECT_SOURCE_DIR}/schemas
#    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/glib-2.0
#)
install(DIRECTORY DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin-installer)

install(
   DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/hooks
   DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin-installer
   FILES_MATCHING PATTERN "*"
   PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

if (LIVE_METHOD STREQUAL "live-boot")
    install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/live-config/
        DESTINATION /lib/live/config
        USE_SOURCE_PERMISSIONS
    )
    install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/live-boot/
        DESTINATION /lib/live/boot
        USE_SOURCE_PERMISSIONS
    )
    install (
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/deepin-installer.conf
        DESTINATION /etc/live/config.conf.d/
    )
elseif (LIVE_METHOD STREQUAL "casper")
    install(
       DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/casper-bottom
       DESTINATION ${CMAKE_INSTALL_PREFIX}/share/initramfs-tools/scripts
       USE_SOURCE_PERMISSIONS
    )
else()
    message(FATAL_ERROR "no live method parameter")    
endif()

install(
   DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources
   DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin-installer
)
install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/deepin-installer.svg
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps/
)

install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/deepin-installer.png
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/48x48/apps/
)
install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/com.deepin.deepin-installer.appearance.gschema.xml
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas/
)
