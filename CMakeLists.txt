cmake_minimum_required(VERSION 3.0)
#set(CMAKE_VERBOSE_MAKEFILE ON)

####################
# TARGET PLATFORM
####################
option(PLATFORM_LINUX "Linux platform target" OFF)
option(PLATFORM_WINDOWS "Windows platform target (msys/mingw64)" OFF)
option(PLATFORM_RPI3 "RIP3 platform target" OFF)
option(PLATFORM_SWITCH "Nintendo Switch platform target" OFF)
option(PLATFORM_3DS "Nintendo 3DS platform target" OFF)
option(PLATFORM_VITA "Sony PS Vita platform target" OFF)
####################
# TARGET PLATFORM
####################

project(sscrap)

##############
# SCREENSCRAP
##############
option(BUILD_TEST "Build sscrap test binary" OFF)

set(INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/source/*.c*)
set(CFLAGS)
set(LDFLAGS curl tinyxml2 z)

#####################
# PLATORM SPECIFIC
#####################
if (PLATFORM_SWITCH)
    #####################
    # SWITCH PLATORM
    #####################
    list(APPEND CFLAGS -D__SWITCH__)
    list(APPEND LDFLAGS mbedtls mbedcrypto mbedx509 nx)
elseif (PLATFORM_LINUX)
    #####################
    # LINUX PLATORM
    #####################
    find_package(CURL REQUIRED)
    find_package(ZLIB REQUIRED)
    if (PLATFORM_WINDOWS)
        # TODO: try to fix nghttp2 static linking...
        list(APPEND CFLAGS -DCURL_STATICLIB -DNGHTTP2_STATICLIB)
        set(LDFLAGS curl.a tinyxml2.a nghttp2 idn2.a psl.a ssl.a
                crypto.a crypt32.a unistring brotlidec-static.a
                brotlicommon-static.a intl.a iconv.a wldap32.a ws2_32.a
                -static -static-libgcc -static-libstdc++)
    endif ()
endif ()

#####################
# SCREENSCRAP LIBRARY
#####################
add_library(${PROJECT_NAME} ${SOURCES} source/ss_game.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})
target_compile_options(${PROJECT_NAME} PUBLIC ${CFLAGS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${LDFLAGS})
# read tmdb api key
if (EXISTS "${CMAKE_SOURCE_DIR}/ss_dev_id.key")
    file(STRINGS "${CMAKE_SOURCE_DIR}/ss_dev_id.key" DEVID)
    file(STRINGS "${CMAKE_SOURCE_DIR}/ss_dev_pwd.key" DEVPWD)
    file(STRINGS "${CMAKE_SOURCE_DIR}/ss_user_id.key" SSID)
    file(STRINGS "${CMAKE_SOURCE_DIR}/ss_user_pwd.key" SSPWD)
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
if (BUILD_TEST)
    add_executable(${PROJECT_NAME}.elf test/main.cpp)
    target_link_libraries(${PROJECT_NAME}.elf ${PROJECT_NAME} pthread)
endif ()
