SET(NAME "language-selector-helper")
set(GOPATH "/usr/share/gocode")

if (USE_GCCGO)
	add_custom_command(
		OUTPUT ${NAME}
		#find_package(PkgConfig REQUIRED)

		#pkg_search_module(Gdk3 REQUIRED gdk-3.0)
		#pkg_search_module(Gio2 REQUIRED gio-2.0)

		#set(LDFLAGS ${Gio2_LIBRARIES} ${Gdk3_LIBRARIES})
		COMMAND GOPATH=${GOPATH} go build -compiler gccgo -gccgoflags "-lgio-2.0 -lgdk-3 -lpangocairo-1.0 -lpango-1.0 -lgdk_pixbuf-2.0 -lcairo-gobject -lcairo -lgobject-2.0 -lglib-2.0"
		COMMAND mv ${NAME} ${CMAKE_BINARY_DIR}
		DEPENDS main.go
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
else()
	add_custom_command(
		OUTPUT ${NAME}
		COMMAND GOPATH=${GOPATH} go build
		COMMAND mv ${NAME} ${CMAKE_BINARY_DIR}
		DEPENDS main.go
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
endif()

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

