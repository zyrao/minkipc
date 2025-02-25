project(gp_test_client C)

# ''Packages''.

# Append modules directory.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(QCBOR_DIR_HINT "" CACHE PATH "Hint path for QCBOR directory")

find_package(QCBOR REQUIRED)
if(NOT QCBOR_FOUND)
	message(FATAL_ERROR "QCBOR not found")
endif()

find_package(Threads REQUIRED)
if(NOT THREADS_FOUND)
	message(FATAL_ERROR "Threads not found")
endif()

# ''Source files''.

set(SRC
	src/gp_test_client.c
	src/gp_load_ta.c
)

# ''Built binary''.

add_executable(${PROJECT_NAME} ${SRC})

# ''Headers and dependencies''.

target_include_directories(${PROJECT_NAME}
	PRIVATE src
	PRIVATE idl
)

target_link_libraries(${PROJECT_NAME}
	PRIVATE minkteec
	PRIVATE minkadaptor
	PRIVATE ${QCBOR_LIBRARIES}
	PRIVATE ${CMAKE_THREAD_LIBS_INIT}
)

install(TARGETS ${PROJECT_NAME} DESTINATION "${CMAKE_INSTALL_BINDIR}")
