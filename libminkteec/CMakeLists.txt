project(libminkteec
	VERSION 1.0.0
	LANGUAGES C
)

if (BUILD_UNITTEST)
	add_subdirectory(tests/gp_test_client)
endif()

add_compile_options(
	-Wstrict-prototypes -Wmissing-prototypes -Wbad-function-cast
)

# ''Packages''.

find_package(Threads REQUIRED)
if(NOT THREADS_FOUND)
	message(FATAL_ERROR "Threads not found")
endif()

# ''Source files''.

set(SRC
	src/tee_client_api.c
	src/mink_teec.c
	src/CWait.c
)

add_library(minkteec SHARED ${SRC})

# ''Library pkg-config file and version''.

set(libminkteectgt minkteec)

configure_file(minkteec.pc.in minkteec.pc @ONLY)

set_target_properties(minkteec PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION ${PROJECT_VERSION_MAJOR}
)

# ''Headers and dependencies''.

target_include_directories(minkteec
	PUBLIC include
	PRIVATE idl
	PRIVATE src
)

target_link_libraries(minkteec
	PRIVATE minkadaptor
)

# ''Install targets''.

install(TARGETS minkteec
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)

install(DIRECTORY include
	DESTINATION ${CMAKE_INSTALL_PREFIX}
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/minkteec.pc"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)
