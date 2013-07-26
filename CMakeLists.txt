cmake_minimum_required(VERSION 2.8)

option(DEVELOP "enable develop mode (use src's resource)" false)
option(TEST "enable test mode (run project's test code)" false)


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
    SET(RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources/")
    add_definitions("-DDATA_DIR=\"../data\"")
else()
    SET(RESOURCE_DIR "${CMAKE_INSTALL_PREFIX}/share/dde/resources/")
    add_definitions("-DDATA_DIR=\"${CMAKE_INSTALL_PREFIX}/share/dde/data\"")
endif()

if (TEST)
    add_definitions("-D__DUI_DEBUG=1")
endif()

add_definitions("-DRESOURCE_DIR=\"${RESOURCE_DIR}\"")

find_package(PkgConfig)
pkg_check_modules(WEBKIT REQUIRED deepin-webkit-3.0)
pkg_check_modules(DBUS REQUIRED dbus-glib-1)

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

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/schemas
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/glib-2.0
)
install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/dde
    )
