project(libtimeservice
	VERSION 1.0.0
	LANGUAGES C
)

add_compile_options(
	-Wstrict-prototypes -Wmissing-prototypes -Wbad-function-cast
	-Wswitch-default
)

# ''Source files''.

set(SRC
	atime.c
	time_msg.h
)

add_library(timeservice SHARED ${SRC})

# ''Library version''.

set_target_properties(timeservice PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION ${PROJECT_VERSION_MAJOR}
)

# ''Install targets''.

install(TARGETS timeservice
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)
