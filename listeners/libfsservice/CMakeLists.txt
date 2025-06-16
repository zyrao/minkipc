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
	fs_msg.h
	fs_main.c
	fs.h
	fs.c
)

set(GPFS_SRC
	gpfs_msg.h
	gpfs_main.c
	gpfs.h
	gpfs.c
)

set(SRC
	cmn.h
	helper.h
	helper.c
)

if(BUILD_FS_LISTENER)
	list(APPEND SRC ${FS_SRC})
endif()

if(BUILD_GPFS_LISTENER)
	list(APPEND SRC ${GPFS_SRC})
endif()

add_library(fsservice SHARED ${SRC})

# ''Library version''.

set_target_properties(fsservice PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION ${PROJECT_VERSION_MAJOR}
)

# ''Install targets''.

install(TARGETS fsservice
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)
