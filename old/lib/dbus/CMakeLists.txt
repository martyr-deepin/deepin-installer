cmake_minimum_required(VERSION 2.8)

include_directories(${DWEBVIEW_INCLUDE_DIRS})

AUX_SOURCE_DIRECTORY(. DBUS)

set(SRC_LIST ${DBUS})

include(CheckCSourceRuns)
set(CMAKE_REQUIRED_INCLUDES  ${DBUS_INCLUDE_DIRS})
CHECK_C_SOURCE_RUNS(
    "#include <dbus/dbus.h>
    int main() { DBusBasicValue a;}
    "  HasDbusBasicValue)

if (HasDbusBasicValue) 
    add_definitions("-D__DBUSBASIC_VALUE__=1")
endif()

include_directories(${DWEBVIEW_INCLUDE_DIRS})
include_directories(${DBUS_INCLUDE_DIRS})

add_library(dbus ${SRC_LIST})
target_link_libraries(dcom ${DWEBVIEW_LIBRAIRES} ${DBUS_LIBRAIRES})

