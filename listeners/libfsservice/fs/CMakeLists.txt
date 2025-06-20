project(libfsservice
	VERSION 1.0.0
	LANGUAGES C
)

add_compile_options(
	-Wstrict-prototypes -Wmissing-prototypes -Wbad-function-cast
	-Wswitch-default
)

# ''Source files''.

set(FS_SRC
	../cmn/helper.c
	fs_msg.h
	fs_main.c
	fs.h
	fs.c
)

add_library(fsservice SHARED ${FS_SRC})

# ''Headers and dependencies''.

target_include_directories(fsservice
	PRIVATE ../cmn
)

# ''Library version''.

set_target_properties(fsservice PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION ${PROJECT_VERSION_MAJOR}
)

# ''Install targets''.

install(TARGETS fsservice
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)
