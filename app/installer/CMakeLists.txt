SET(NAME "installer")

GEN_JSC_SOURCE2(JSC_GEN "jsc" "installer" "installer.cfg" "dbus.cfg")

pkg_check_modules(PARTED REQUIRED libparted)
pkg_check_modules(XKLAVIER REQUIRED libxklavier)

add_executable(${NAME} ${JSC_GEN} base.c fs_util.c part_util.c system_info.c timezone.c keyboard.c installer.c info.c hooks.c scheduler.c esp.c ped_utils.c)
add_dependencies(${NAME} ${NAME}_js)

target_link_libraries(${NAME} dcom dbus ${DWEBVIEW_LIBRARIES} ${PARTED_LIBRARIES} ${XKLAVIER_LIBRARIES})

include_directories(${DWEBVIEW_INCLUDE_DIRS} ${PARTED_INCLUDE_DIRS} ${XKLAVIER_INCLUDE_DIRS})

SET_TARGET_PROPERTIES(${NAME}
    PROPERTIES OUTPUT_NAME deepin-${NAME}
    )

execute_process(
    COMMAND make
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/resources/${NAME}/js
    )

execute_process(
    COMMAND make
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/resources/${NAME}/ppt/slideshow2014
    )

install(
    PROGRAMS ${PROJECT_BINARY_DIR}/deepin-${NAME}
    DESTINATION bin
    )

install(
    PROGRAMS ${PROJECT_SOURCE_DIR}/app/installer/deepin-installer-launcher
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

GEN_JS_FILE(${NAME})
