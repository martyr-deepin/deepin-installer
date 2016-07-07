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


add_executable(unsquashfs_progress unsquashfs_progress.c)
target_link_libraries(unsquashfs_progress ${GLIB_LIBRARIES})
install(
    PROGRAMS ${PROJECT_BINARY_DIR}/unsquashfs_progress
    DESTINATION bin
)

install(
	PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/00reuse-machine-id
	DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/os-probes/mounted/
)

