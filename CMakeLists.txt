cmake_minimum_required(VERSION 3.4)
project(quic-teec-xts)

include(GNUInstallDirs)

add_compile_options(
	-Wall -Wextra -Werror -Wshadow -Wcast-align
	-Wmissing-declarations -Wformat-security -Wmissing-noreturn
	-Wdeprecated -fPIC
)

add_subdirectory(libminkadaptor)

