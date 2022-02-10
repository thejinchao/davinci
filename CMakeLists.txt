#
#Davinci (a software render)
#

cmake_minimum_required (VERSION 3.0)

project(davinci)

########
#system
########
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(DV_SYS_LINUX TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(DV_SYS_WINDOWS TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	set(DV_SYS_MACOS TRUE)
else()
    message(FATAL_ERROR "Unknown target system \"${CMAKE_SYSTEM_NAME}\".")
endif()

########
#set davinci files path
########
set(DV_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/source)

########
#check include file
########
include(CheckIncludeFile)
check_include_file(unistd.h         DV_HAVE_UNISTD_H)
check_include_file(inttypes.h       DV_HAVE_INTTYPES_H)
check_include_file(limits.h         DV_HAVE_LIMITS_H)

########
#compiler flag
########
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
else()
    set(CMAKE_CXX_FLAGS	"${CMAKE_CXX_FLAGS} -g -O2 -std=c++11 -Wall -Wextra -Werror -Wconversion -Wno-unused-parameter	-Woverloaded-virtual -Wpointer-arith -Wshadow -Wwrite-strings -Wno-deprecated")
endif()

########
#make configure files
########
set(DV_AUTO_INCLUDE_PATH ${CMAKE_CURRENT_BINARY_DIR}/source)
set(DV_AUTO_CONFIG_FILE ${DV_AUTO_INCLUDE_PATH}/dv_config.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/source/dv_config.h.in ${DV_AUTO_CONFIG_FILE})

########
#Thirdparty libraries
########

# rapidjson
find_package(RapidJSON CONFIG REQUIRED
    PATHS $ENV{LIBRARIES_SDK_ROOT}/rapidjson/cmake)

#zlib
set(ZLIB_ROOT $ENV{LIBRARIES_SDK_ROOT}/zlib)
find_package(ZLIB REQUIRED)

#libpng
set(PNG_PNG_INCLUDE_DIR $ENV{LIBRARIES_SDK_ROOT}/libpng/include)
list(APPEND CMAKE_LIBRARY_PATH $ENV{LIBRARIES_SDK_ROOT}/libpng/lib)
find_package(PNG REQUIRED)

#GLFW
find_package(glfw3 CONFIG REQUIRED
	PATHS $ENV{LIBRARIES_SDK_ROOT}/glfw/lib/cmake/glfw3)
find_package(OpenGL REQUIRED)

#glm
find_package(glm CONFIG REQUIRED
	PATHS $ENV{LIBRARIES_SDK_ROOT}/glm/include/cmake/glm)

########
#sub dictionary
########
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_subdirectory(source)
add_subdirectory(console)

########
#test dictionary
########
add_subdirectory(test)
