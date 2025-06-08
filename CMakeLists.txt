cmake_minimum_required(VERSION 3.4)
project(minkipc)

# Build Mink TEEC library
option(BUILD_MINKTEEC "Build MinkTEEC" TRUE)

option(BUILD_UNITTEST "Build unittest" FALSE)

# Build Time listener
option(BUILD_TIME_LISTENER "Build Time Listener" TRUE)
# Build TA Autoload listener
option(BUILD_TA_AUTOLOAD_LISTENER "Build TA autoload Listener" TRUE)

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

add_subdirectory(qtee_supplicant)
if(BUILD_TIME_LISTENER)
	add_subdirectory(listeners/libdrmtime)
endif()

if(BUILD_TA_AUTOLOAD_LISTENER)
	add_subdirectory(listeners/libtaautoload)
endif()
