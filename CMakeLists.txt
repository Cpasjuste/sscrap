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
include(Toolchain.cmake)
####################
# TARGET PLATFORM
####################

project(sscrap)

##############
# SCREENSCRAP
##############
option(BUILD_SSCRAP "Build sscrap binary" OFF)

# handle deps
list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(ZLIB REQUIRED)
find_package(CURL REQUIRED)
find_package(TinyXML2 REQUIRED)

# sources
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/source/*.c*)

# includes
set(INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CURL_INCLUDE_DIRS}
        ${TinyXML2_INCLUDE_DIRS}
        ${ZLIB_INCLUDE_DIRS})

# ldflags
set(LDFLAGS
        ${CURL_LIBRARIES}
        ${TinyXML2_LIBRARIES}
        ${ZLIB_LIBRARIES})

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
add_library(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})
target_compile_options(${PROJECT_NAME} PUBLIC ${CFLAGS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${LDFLAGS})
# read tmdb api key
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ss_dev_id.key")
    file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/ss_dev_id.key" DEVID)
    file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/ss_dev_pwd.key" DEVPWD)
    message("-- Screensraper dev id: ${DEVID}")
else ()
    message(WARNING "ss_dev_id.key and ss_dev_pwd.key files not found,
        using dummy ss keys. Please fix if you want to use live scrapping...")
    set(DEVID "none")
    set(DEVPWD "none")
endif ()
target_compile_options(
        ${PROJECT_NAME} PUBLIC
        -DSS_DEV_ID=\"${DEVID}\" -DSS_DEV_PWD=\"${DEVPWD}\")

#####################
# SCREENSCRAP TEST
#####################
if (BUILD_SSCRAP)
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    find_package(Threads REQUIRED)
    find_package(MiniZip REQUIRED)
    add_executable(${PROJECT_NAME}.elf test/main.cpp test/utility.cpp)
    target_link_libraries(${PROJECT_NAME}.elf ${PROJECT_NAME}
            ${CMAKE_THREAD_LIBS_INIT}
            ${MINIZIP_LIBRARIES}
            ${ZLIB_LIBRARIES})
    add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
            COMMAND cp -rf ${CMAKE_SOURCE_DIR}/databases ${CMAKE_BINARY_DIR}/
            COMMENT "Copying databases to binary directory..."
            )
endif ()
