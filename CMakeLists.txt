cmake_minimum_required(VERSION 3.10)
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
option(PLATFORM_PS4 "Sony PS4 platform target" OFF)
option(PLATFORM_ANDROID "Android platform target (aarch64)" OFF)
#include(Toolchain.cmake)
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

cmake_host_system_information(RESULT HOST_OS_NAME QUERY OS_NAME)
if(PLATFORM_WINDOWS)
  find_package(TinyXML2 REQUIRED)
elseif(HOST_OS_NAME MATCHES "MSYS")
  find_package(TinyXML2 CONFIG QUIET)
  if(TinyXML2_FOUND)
    find_path(TINYXML2_INCLUDE_DIR NAMES tinyxml2.h)
    find_library(TINYXML2_LIBRARY tinyxml2)
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(TinyXML2 DEFAULT_MSG TINYXML2_LIBRARY TINYXML2_INCLUDE_DIR)
    mark_as_advanced(TINYXML2_INCLUDE_DIR TINYXML2_LIBRARY)
  endif()
  # Set mixed case INCLUDE_DIRS and LIBRARY variables from upper case ones.
  if(NOT TinyXML2_INCLUDE_DIRS)
    set(TinyXML2_INCLUDE_DIRS ${TINYXML2_INCLUDE_DIR})
  endif()
  if(NOT TinyXML2_LIBRARIES)
    set(TinyXML2_LIBRARIES ${TINYXML2_LIBRARY})
  endif()
else()
  find_package(TinyXML2 REQUIRED)
endif()

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

set(CMAKE_CXX_STANDARD 14)

#####################
# PLATORM SPECIFIC
#####################
if (PLATFORM_SWITCH)
    #####################
    # SWITCH PLATFORM
    #####################
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    list(APPEND CFLAGS -D__SWITCH__)
    list(APPEND LDFLAGS mbedtls mbedcrypto mbedx509 nx)
elseif (PLATFORM_VITA)
    #####################
    # VITA PLATFORM
    #####################
    list(APPEND CFLAGS -D__VITA__)
    list(APPEND LDFLAGS ssl crypto)
elseif (PLATFORM_PS4)
    #####################
    # PS4 PLATFORM
    #####################
    list(APPEND CFLAGS)
    list(APPEND LDFLAGS tinyxml2 mbedtls mbedcrypto mbedx509 SceNet)
elseif (PLATFORM_3DS)
    #####################
    # 3DS PLATFORM
    #####################
    list(APPEND CFLAGS -D__3DS__)
    list(APPEND LDFLAGS mbedtls mbedcrypto mbedx509 ctru)
elseif (PLATFORM_LINUX)
    #####################
    # LINIX PLATFORM
    #####################
    # ubuntu does not come with a static tinyxml2 build...
    list(REMOVE_ITEM LDFLAGS ${TinyXML2_LIBRARIES})
    list(APPEND SOURCES external/tinyxml2/tinyxml2.cpp)
elseif (PLATFORM_WINDOWS)
    #####################
    # WINDOWS PLATFORM
    #####################
    # TODO: try to fix nghttp2 static linking...
    # https://github.com/google/brotli/issues/795
    list(APPEND CFLAGS -D__WINDOWS__ -DCURL_STATICLIB -DNGHTTP2_STATICLIB)
    set(LDFLAGS
            curl.a ssh2.a zstd.a
            tinyxml2.a nghttp2 idn2.a psl.a ssl.a
            crypto.a crypt32.a bcrypt.a unistring
            brotlidec.a brotlicommon.a
            intl.a iconv.a wldap32.a ws2_32.a z.a
            -static -static-libgcc -static-libstdc++)
elseif (PLATFORM_ANDROID)
    list(APPEND LDFLAGS -ltinyxml2 -lssl -lcrypto)
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
    file(GLOB HASH_SRC sscrap-utility/hashlibpp/*.cpp)
    add_executable(${PROJECT_NAME}-utility sscrap-utility/main.cpp sscrap-utility/utility.cpp ${HASH_SRC})
    target_link_libraries(${PROJECT_NAME}-utility ${PROJECT_NAME}
            ${CMAKE_THREAD_LIBS_INIT}
            ${MINIZIP_LIBRARIES}
            ${ZLIB_LIBRARIES})
    add_custom_command(TARGET ${PROJECT_NAME}-utility POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/databases ${CMAKE_BINARY_DIR}/databases
            COMMENT "Copying databases to binary directory..."
            )
    if (PLATFORM_WINDOWS)
        add_custom_command(TARGET ${PROJECT_NAME}-utility POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/release
                COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/release/sscrap
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-utility.exe ${CMAKE_CURRENT_BINARY_DIR}/release/sscrap/
                COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/databases ${CMAKE_CURRENT_BINARY_DIR}/release/sscrap/databases
                # ouch...
                COMMAND ${CMAKE_COMMAND} -E copy C:/msys64/mingw64/bin/libtinyxml2.dll ${CMAKE_CURRENT_BINARY_DIR}/release/sscrap/
                COMMAND ${CMAKE_COMMAND} -E copy C:/msys64/mingw64/bin/libgcc_s_seh-1.dll ${CMAKE_CURRENT_BINARY_DIR}/release/sscrap/
                COMMAND ${CMAKE_COMMAND} -E copy C:/msys64/mingw64/bin/libwinpthread-1.dll ${CMAKE_CURRENT_BINARY_DIR}/release/sscrap/
                COMMAND ${CMAKE_COMMAND} -E copy C:/msys64/mingw64/bin/libminizip-1.dll ${CMAKE_CURRENT_BINARY_DIR}/release/sscrap/
                COMMAND ${CMAKE_COMMAND} -E copy C:/msys64/mingw64/bin/libcurl-4.dll ${CMAKE_CURRENT_BINARY_DIR}/release/sscrap/
                COMMAND ${CMAKE_COMMAND} -E copy C:/msys64/mingw64/bin/libstdc++-6.dll ${CMAKE_CURRENT_BINARY_DIR}/release/sscrap/
                COMMAND ${CMAKE_COMMAND} -E copy C:/msys64/mingw64/bin/zlib1.dll ${CMAKE_CURRENT_BINARY_DIR}/release/sscrap/
                COMMAND ${CMAKE_COMMAND} -E copy C:/msys64/mingw64/bin/libcrypto-1_1-x64.dll ${CMAKE_CURRENT_BINARY_DIR}/release/sscrap/
                COMMAND ${CMAKE_COMMAND} -E copy C:/msys64/mingw64/bin/libbrotlidec.dll ${CMAKE_CURRENT_BINARY_DIR}/release/sscrap/
                COMMAND ${CMAKE_COMMAND} -E copy C:/msys64/mingw64/bin/libbrotlicommon.dll ${CMAKE_CURRENT_BINARY_DIR}/release/sscrap/
                COMMAND ${CMAKE_COMMAND} -E copy C:/msys64/mingw64/bin/libidn2-0.dll ${CMAKE_CURRENT_BINARY_DIR}/release/sscrap/
                COMMAND ${CMAKE_COMMAND} -E copy C:/msys64/mingw64/bin/libnghttp2-14.dll ${CMAKE_CURRENT_BINARY_DIR}/release/sscrap/
                COMMAND ${CMAKE_COMMAND} -E copy C:/msys64/mingw64/bin/libpsl-5.dll ${CMAKE_CURRENT_BINARY_DIR}/release/sscrap/
                COMMAND ${CMAKE_COMMAND} -E copy C:/msys64/mingw64/bin/libssh2-1.dll ${CMAKE_CURRENT_BINARY_DIR}/release/sscrap/
                COMMAND ${CMAKE_COMMAND} -E copy C:/msys64/mingw64/bin/libssl-1_1-x64.dll ${CMAKE_CURRENT_BINARY_DIR}/release/sscrap/
                COMMAND ${CMAKE_COMMAND} -E copy C:/msys64/mingw64/bin/libzstd.dll ${CMAKE_CURRENT_BINARY_DIR}/release/sscrap/
                COMMAND ${CMAKE_COMMAND} -E copy C:/msys64/mingw64/bin/libiconv-2.dll ${CMAKE_CURRENT_BINARY_DIR}/release/sscrap/
                COMMAND ${CMAKE_COMMAND} -E copy C:/msys64/mingw64/bin/libunistring-2.dll ${CMAKE_CURRENT_BINARY_DIR}/release/sscrap/
                COMMAND ${CMAKE_COMMAND} -E copy C:/msys64/mingw64/bin/libintl-8.dll ${CMAKE_CURRENT_BINARY_DIR}/release/sscrap/
                )
        add_custom_target(${PROJECT_NAME}-retrodream
                DEPENDS ${PROJECT_NAME}-utility
                # utils (dc)
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/sscrap-utility/dreamcast/ffmpeg.exe ${CMAKE_CURRENT_BINARY_DIR}/release/sscrap/
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/sscrap-utility/dreamcast/magick.exe ${CMAKE_CURRENT_BINARY_DIR}/release/sscrap/
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/sscrap-utility/dreamcast/pngquant.exe ${CMAKE_CURRENT_BINARY_DIR}/release/sscrap/
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/sscrap-utility/dreamcast/sscrap-retrodream.ps1 ${CMAKE_CURRENT_BINARY_DIR}/release/
                )
    endif ()
endif ()
