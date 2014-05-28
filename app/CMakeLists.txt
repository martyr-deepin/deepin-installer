add_subdirectory(installer)

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/resources/common
    DESTINATION ${RESOURCE_DIR}
    )

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/resources/hooks
    DESTINATION ${RESOURCE_DIR}
    PATTERN "${PROJECT_SOURCE_DIR}/resources/hooks"
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                GROUP_EXECUTE GROUP_READ
    )
