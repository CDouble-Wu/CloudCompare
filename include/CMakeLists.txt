
target_sources( ${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/IOMyDescriptorPlugin.h
        ${CMAKE_CURRENT_LIST_DIR}/FooFilter.h
)

target_include_directories( ${PROJECT_NAME}
	PRIVATE
	    ${CMAKE_CURRENT_SOURCE_DIR}
)
