SET(NAME "language-selector")


add_subdirectory(language-selector-helper)

pkg_check_modules(LIGHTDM REQUIRED liblightdm-gobject-1)

GEN_JSC_SOURCE(JSC_GEN "jsc")

include_directories(${DWEBVIEW_INCLUDE_DIRS} ${LIGHTDM_INCLUDE_DIRS}
    ${GST_INCLUDE_DIRS}
)

add_executable(${NAME} greeter.c user.c session.c theme.c mutils.c greeter_util.c ${JSC_GEN} ${DBUS_GEN})

add_dependencies(${NAME} ${NAME}_js gogo)

target_link_libraries(${NAME} dcom dbus ${DWEBVIEW_LIBRARIES} ${LIGHTDM_LIBRARIES})

SET_TARGET_PROPERTIES(
    ${NAME}
    PROPERTIES OUTPUT_NAME deepin-lightdm-${NAME}
)

install(
    PROGRAMS ${PROJECT_BINARY_DIR}/deepin-lightdm-${NAME}
    DESTINATION bin
)

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/resources/${NAME}
    DESTINATION ${RESOURCE_DIR}
)

install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/deepin-lightdm-${NAME}.desktop
    DESTINATION /usr/share/xgreeters/
)
GEN_JS_FILE(${NAME})
