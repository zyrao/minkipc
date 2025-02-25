cmake_minimum_required(VERSION 3.4)
project(quic-teec-xts)

# Build Mink TEEC library
option(BUILD_MINKTEEC "Build MinkTEEC" TRUE)

option(BUILD_UNITTEST "Build unittest" FALSE)

include(GNUInstallDirs)

add_compile_options(
	-Wall -Wextra -Werror -Wshadow -Wcast-align
	-Wmissing-declarations -Wformat-security -Wmissing-noreturn
	-Wdeprecated -fPIC
)

add_subdirectory(libminkadaptor)

if (BUILD_MINKTEEC)
	add_subdirectory(libminkteec)
endif()

