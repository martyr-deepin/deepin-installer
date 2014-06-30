SET(NAME "language-selector-helper")

add_custom_command(
    OUTPUT ${NAME}
    COMMAND go build 
    COMMAND mv ${NAME} ${CMAKE_BINARY_DIR}
    DEPENDS main.go
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(gogo DEPENDS ${NAME})

install(
    PROGRAMS ${CMAKE_BINARY_DIR}/language-selector-helper
    DESTINATION bin
)

install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/com.deepin.helper.LanguageSelector.service
    DESTINATION /usr/share/dbus-1/system-services/
)

install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/com.deepin.helper.LanguageSelector.conf
    DESTINATION /etc/dbus-1/system.d/
)

