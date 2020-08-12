option( PLUGIN_IOMyDescriptor_IO "Install example I/O plugin" OFF )

if( PLUGIN_IOMyDescriptor_IO )
	project( IOMyDescriptorPlugin )

    AddPlugin( NAME ${PROJECT_NAME} TYPE io )
    
    add_subdirectory( include )
    add_subdirectory( src )
endif()
