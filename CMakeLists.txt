cmake_minimum_required(VERSION 3.0)
#set(CMAKE_VERBOSE_MAKEFILE ON)

# delete cmake cache folder before changing this options
option(BUILD_LINUX "Build with SDL2 support" OFF)
option(BUILD_WINDOWS "Build with Windows support (msys/mingw64)" OFF)
option(BUILD_VITA "Build with Ps Vita support (hardware rendering, fast)" OFF)
option(BUILD_PS3 "Build with PS3 support (hardware rendering, fast)" OFF)
option(BUILD_3DS "Build with 3DS support" OFF)
option(BUILD_SWITCH "Build with Nintendo Switch support" OFF)

#include(Toolchain.cmake)

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
    if (BUILD_WINDOWS)
        # TODO: try to fix nghttp2 static linking...
        list(APPEND CFLAGS -DCURL_STATICLIB -DNGHTTP2_STATICLIB)
        set(LDFLAGS json-c curl.a nghttp2 idn2.a psl.a ssl.a
                crypto.a crypt32.a unistring brotlidec-static.a
                brotlicommon-static.a intl.a iconv.a wldap32.a ws2_32.a
                -static -static-libgcc -static-libstdc++)
    endif ()
endif ()

#####################
# SCREENSCRAP LIBRARY
#####################
add_library(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})
target_compile_options(${PROJECT_NAME} PRIVATE ${CFLAGS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${LDFLAGS})
# read tmdb api key
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ss_dev_id.key")
    file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/ss_dev_id.key" DEVID)
    file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/ss_dev_pwd.key" DEVPWD)
    file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/ss_user_id.key" SSID)
    file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/ss_user_pwd.key" SSPWD)
    message("-- Screensraper dev id: ${DEVID}")
else ()
    message(WARNING "ss_dev_id.key, ss_dev_pwd.key, ss_user_id.key and ss_user_pwd.key files not found, using dummy ss keys. Please fix by creating this files and add your keys...")
    set(DEVID "none")
    set(DEVPWD "none")
    set(SSID "none")
    set(SSPWD "none")
endif ()
target_compile_options(
        ${PROJECT_NAME} PUBLIC
        -DSS_DEV_ID=\"${DEVID}\" -DSS_DEV_PWD=\"${DEVPWD}\"
        -DSS_ID=\"${SSID}\" -DSS_PWD=\"${SSPWD}\")

#####################
# SCREENSCRAP TEST
#####################
add_executable(${PROJECT_NAME}.elf test/main.cpp)
target_link_libraries(${PROJECT_NAME}.elf ${PROJECT_NAME})
