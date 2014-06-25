SET(NAME "esp_detector")

pkg_check_modules(PARTED REQUIRED libparted)
pkg_check_modules(GLIB REQUIRED glib-2.0)

add_executable(${NAME} detect_esp.c)

target_link_libraries(${NAME} ${PARTED_LIBRARIES} ${GLIB_LIBRARIES})

include_directories(${PARTED_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS})

install(
    PROGRAMS ${PROJECT_BINARY_DIR}/${NAME}
    DESTINATION bin
)

install(
    PROGRAMS ${PROJECT_BINARY_DIR}/genfstab
    DESTINATION bin
)
