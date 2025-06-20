project(libgpfsservice
	VERSION 1.0.0
	LANGUAGES C
)

add_compile_options(
	-Wstrict-prototypes -Wmissing-prototypes -Wbad-function-cast
	-Wswitch-default
)

# ''Source files''.

set(GPFS_SRC
	../cmn/helper.c
	gpfs_msg.h
	gpfs_main.c
	gpfs.h
	gpfs.c
)

add_library(gpfsservice SHARED ${GPFS_SRC})

# ''Headers and dependencies''.

target_include_directories(gpfsservice
	PRIVATE ../cmn
)

# ''Library version''.

set_target_properties(gpfsservice PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION ${PROJECT_VERSION_MAJOR}
)

# ''Install targets''.

install(TARGETS gpfsservice
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)
