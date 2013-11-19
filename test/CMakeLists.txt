add_library(dtest test.c)
include_directories(${DWEBVIEW_INCLUDE_DIRS})
target_link_libraries(dcom ${DWEBVIEW_LIBRAIRES})
