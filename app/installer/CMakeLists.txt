SET(NAME "deepin-installer")

GEN_JSC_SOURCE2(JSC_GEN "jsc" "installer" "installer.cfg" "dbus.cfg")

pkg_check_modules(PARTED REQUIRED libparted)
pkg_check_modules(XKLAVIER REQUIRED libxklavier)

add_executable(${NAME} ${JSC_GEN} base.c fs_util.c part_util.c system_info.c keyboard.c installer.c info.c hooks.c scheduler.c esp.c ped_utils.c)

target_link_libraries(${NAME} dcom dbus ${DWEBVIEW_LIBRARIES} ${PARTED_LIBRARIES} ${XKLAVIER_LIBRARIES})

include_directories(${DWEBVIEW_INCLUDE_DIRS} ${PARTED_INCLUDE_DIRS} ${XKLAVIER_INCLUDE_DIRS})

execute_process(
    COMMAND make
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/resources/installer/js
    )

execute_process(
    COMMAND make
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/resources/installer/ppt/slideshow2014
    )

install(
    PROGRAMS ${PROJECT_BINARY_DIR}/${NAME}
    DESTINATION bin
    )

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/resources/installer
    DESTINATION ${RESOURCE_DIR}
    )

install(
    FILES ${PROJECT_SOURCE_DIR}/app/installer/deepin-installer.desktop
    DESTINATION /usr/share/applications/
    )

install(
    FILES ${PROJECT_SOURCE_DIR}/app/installer/com.deepin.pkexec.installer.policy
    DESTINATION /usr/share/polkit-1/actions/
    )
