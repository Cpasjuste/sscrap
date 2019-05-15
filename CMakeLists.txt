cmake_minimum_required(VERSION 3.0)
#set(CMAKE_VERBOSE_MAKEFILE ON)

# delete cmake cache folder before changing this options
option(BUILD_LINUX "Build with SDL2 support" OFF)
option(BUILD_WINDOWS "Build with Windows support (msys/mingw64)" OFF)
option(BUILD_VITA "Build with Ps Vita support (hardware rendering, fast)" OFF)
option(BUILD_PS3 "Build with PS3 support (hardware rendering, fast)" OFF)
option(BUILD_3DS "Build with 3DS support" OFF)
option(BUILD_SWITCH "Build with Nintendo Switch support" OFF)

include(Toolchain.cmake)

project(ssscrap)

##############
# SCREENSCRAP
##############
set(INCLUDES include)
file(GLOB SOURCES source/*.c*)
set(CFLAGS)
set(LDFLAGS curl json-c z)

#####################
# PLATORM SPECIFIC
#####################
if (BUILD_SWITCH)
    #####################
    # SWITCH PLATORM
    #####################
    list(APPEND CFLAGS -D__SWITCH__)
    list(APPEND INCLUDES)
    list(APPEND LDFLAGS mbedtls mbedcrypto mbedx509 nx)
elseif (BUILD_LINUX)
    #####################
    # LINUX PLATORM
    #####################
	find_package(CURL REQUIRED)
	find_package(ZLIB REQUIRED)
    list(APPEND CFLAGS)
    list(APPEND LDFLAGS)
endif ()

#####################
# SCREENSCRAP LIBRARY
#####################
add_library(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})
target_compile_options(${PROJECT_NAME} PRIVATE ${CFLAGS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${LDFLAGS})
# read tmdb api key
if(EXISTS "ss_dev_id.key")
	file(STRINGS "ss_dev_id.key" DEVID)
	file(STRINGS "ss_dev_pwd.key" DEVPWD)
	file(STRINGS "ss_user_id.key" SSID)
	file(STRINGS "ss_user_pwd.key" SSPWD)
else()
	message(WARNING "ss_dev_id.key, ss_dev_pwd.key, ss_user_id.key and ss_user_pwd.key files not found, using dummy ss keys")
	message(WARNING "Please fix by creating this files and add your keys...")
	set(DEVID "none")
	set(DEVPWD "none")
	set(SSID "none")
	set(SSPWD "none")
endif()
target_compile_options(
        ${PROJECT_NAME} PUBLIC -DDEVID=\"${DEVID}\"
        -DDEVPWD=\"${DEVPWD}\" -DSSID=\"${SSID}\" -DSSPWD=\"${SSPWD}\")

#####################
# SCREENSCRAP TEST
#####################
add_executable(${PROJECT_NAME}.elf test/main.cpp)
target_link_libraries(${PROJECT_NAME}.elf ${PROJECT_NAME})
