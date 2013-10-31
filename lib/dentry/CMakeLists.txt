cmake_minimum_required(VERSION 2.8)

find_package(PkgConfig REQUIRED)

include_directories(${DWEBVIEW_INCLUDE_DIRS})

AUX_SOURCE_DIRECTORY(. DEntry)

set(SRC_LIST ${DEntry})

include_directories(${DWEBVIEW_INCLUDE_DIRS})

add_library(dentry ${SRC_LIST} ../category.c)
target_link_libraries(dentry ${DWEBVIEW_LIBRAIRES})

