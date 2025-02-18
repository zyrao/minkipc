project(smcinvoke_client CXX)

# ''Packages''.

# Append modules directory.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(QCBOR_DIR_HINT "" CACHE PATH "Hint path for QCBOR directory")

find_package(QCBOR REQUIRED)
if(NOT QCBOR_FOUND)
	message(FATAL_ERROR "QCBOR not found")
endif()

include_directories(${QCBOR_INCLUDE_DIRS})

# ''Source files''.

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SRC
	src/smcinvoke_client.cpp
	src/CTestCallable.cpp
	src/CIO.c
)

# ''Built binary''.

add_executable(${PROJECT_NAME} ${SRC})

# ''Headers and dependencies''.

target_include_directories(${PROJECT_NAME}
	PRIVATE src
	PRIVATE include
	PRIVATE idl
)

target_link_libraries(${PROJECT_NAME}
	PRIVATE minkadaptor
	PRIVATE ${QCBOR_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} DESTINATION "${CMAKE_INSTALL_BINDIR}")
