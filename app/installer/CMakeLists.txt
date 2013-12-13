SET(NAME "installer")

GEN_JSC_SOURCE2(JSC_GEN "jsc" "installer" "installer.cfg")
GEN_JSC_SOURCE2(JSC_HELP_GEN "jsc" "installerhelp" "help.cfg")

pkg_check_modules(PARTED REQUIRED libparted)
pkg_check_modules(XKLAVIER REQUIRED libxklavier)

add_executable(${NAME} ${JSC_GEN} base.c fs_util.c part_util.c misc.c extract.c account.c keyboard.c installer.c)

add_executable(installerhelp ${JSC_HELP_GEN} help.c)

target_link_libraries(${NAME} dcom dbus ${DWEBVIEW_LIBRARIES} ${PARTED_LIBRARIES} ${XKLAVIER_LIBRARIES})

target_link_libraries(installerhelp dcom ${DWEBVIEW_LIBRARIES})

include_directories(${DWEBVIEW_INCLUDE_DIRS} ${PARTED_INCLUDE_DIRS} ${XKLAVIER_INCLUDE_DIRS})

execute_process(
    COMMAND make
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/resources/${NAME}/js
    )

install(
    PROGRAMS ${PROJECT_BINARY_DIR}/${NAME}
    DESTINATION bin
    )

install(
    PROGRAMS ${PROJECT_BINARY_DIR}/installerhelp
    DESTINATION bin
    )

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/resources/${NAME}
    DESTINATION ${RESOURCE_DIR}
    )

install(
    FILES ${PROJECT_SOURCE_DIR}/app/installer/deepin-installer.desktop
    DESTINATION /usr/share/applications/
    )
