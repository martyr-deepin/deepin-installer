find_package(Gettext REQUIRED)

file(GLOB PO_FILES *.po)

#execute_process(
    #COMMAND python update_po
    #WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#)

foreach(f ${PO_FILES})
    GET_FILENAME_COMPONENT(f_we ${f} NAME_WE)
    set(out ${CMAKE_CURRENT_BINARY_DIR}/${f_we}.gmo)
    set(in "${f_we}.po")

    execute_process(
    #add_custom_command(
        #OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${out}
        COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -o ${out} ${in}
        #DEPENDS ${f}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        )
    #message("out ${out}  depends ${f} commd: ${GETTEXT_MSGFMT_EXECUTABLE} -o ${out} ${in}  workdir:${CMAKE_CURRENT_SOURCE_DIR}")
    install(
        FILES ${out}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/locale/${f_we}/LC_MESSAGES/
        RENAME INSTALLER.mo
        )

    set(_outputs ${_outputs} ${out})

endforeach(f)

