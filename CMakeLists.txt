cmake_minimum_required(VERSION 3.4)
project(quic-teec-xts)

option(BUILD_UNITTEST "Build unittest" FALSE)

include(GNUInstallDirs)

add_compile_options(
	-Wall -Wextra -Werror -Wshadow -Wcast-align
	-Wmissing-declarations -Wformat-security -Wmissing-noreturn
	-Wdeprecated -fPIC
)

add_subdirectory(libminkadaptor)

if(BUILD_UNITTEST)
	add_subdirectory(tests)
endif()

