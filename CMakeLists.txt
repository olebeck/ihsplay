cmake_minimum_required(VERSION 3.16)

project(ihsplay VERSION 0.2.2 LANGUAGES C)

enable_testing()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_SOURCE_DIR}/cmake/sanitizers/cmake)

# Use `pkg-config` to link needed libraries.
find_package(PkgConfig REQUIRED)
find_package(Freetype REQUIRED)
find_package(Fontconfig REQUIRED)

# Use SDL2 for window creation and event handling.
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(PROTOBUF_C libprotobuf-c)
pkg_check_modules(OPUS opus)

find_library(MBEDCRYPTO_LIBRARY mbedcrypto)

option(IHSPLAY_WIP_FEATURES "Enable Work-in-Progress Features" OFF)
option(IHSPLAY_FEATURE_FORCE_FULLSCREEN "Force full screen mode" OFF)

set(IHSPLAY_FEATURE_RELMOUSE ON)
set(IHSPLAY_FEATURE_LIBCEC ON)

get_filename_component(CMAKE_C_COMPILER_NAME "${CMAKE_C_COMPILER}" NAME)

if (CMAKE_C_COMPILER_NAME MATCHES "^arm-webos-linux-gnueabi-")
    set(TARGET_WEBOS TRUE)
endif ()

if (CMAKE_C_COMPILER_NAME MATCHES "^armv7-w64-mingw32-")
    set(TARGET_WINRT TRUE)
endif ()

if (NOT PROTOBUF_C_LIBRARIES OR NOT PROTOBUF_C_INCLUDEDIR)
    message("Using protobuf-c submodule")
    option(BUILD_PROTOC "Disable protoc-c" OFF)
    option(BUILD_TESTS "Disable protoc-c tests" OFF)
    add_subdirectory(third_party/protobuf-c/build-cmake EXCLUDE_FROM_ALL)
    set(PROTOBUF_C_FOUND 1)
    set(PROTOBUF_C_LIBRARIES protobuf-c)
    set(PROTOBUF_C_INCLUDEDIR ${CMAKE_SOURCE_DIR}/third_party/protobuf-c)
endif ()

if (NOT OPUS_FOUND)
    message("Using opus submodule")
    set(BUILD_TESTING OFF)
    add_subdirectory(third_party/opus EXCLUDE_FROM_ALL)
    unset(BUILD_TESTING)
    set(OPUS_FOUND 1)
    set(OPUS_LIBRARIES opus)
    set(OPUS_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/third_party/opus/include)
endif ()

if (NOT MBEDCRYPTO_LIBRARY)
    option(ENABLE_TESTING "Disable MbedTLS testing" OFF)
    option(ENABLE_PROGRAMS "Disable MbedTLS programs" OFF)
    add_subdirectory(third_party/mbedtls EXCLUDE_FROM_ALL)
    set(MBEDTLS_FOUND 1)
    set(MBEDCRYPTO_LIBRARY mbedcrypto)
endif ()

if (TARGET_WEBOS)
    set(IHSLIB_SDL_TARGETVERSION "2.0.0")
    set(SS4S_MODULE_DISABLE_MMAL ON)
endif ()

if (TARGET_WINRT)
    add_compile_definitions(_POSIX)
endif ()

add_subdirectory(core)

add_subdirectory(third_party/lvgl EXCLUDE_FROM_ALL)

include(BuildINIH)

target_include_directories(lvgl SYSTEM PRIVATE ${SDL2_INCLUDE_DIRS})
target_include_directories(lvgl SYSTEM PRIVATE ${FREETYPE_INCLUDE_DIRS})

target_link_libraries(lvgl PRIVATE ${SDL2_LIBRARIES} ${FREETYPE_LIBRARIES})
target_compile_definitions(lvgl PUBLIC LV_CONF_PATH=../../../app/lvgl/lv_conf.h)


if (TARGET_WEBOS)
    add_subdirectory(system/webos/tv-native-apis EXCLUDE_FROM_ALL)
endif ()

add_subdirectory(third_party/ss4s)

set(CMAKE_C_STANDARD 11)

add_subdirectory(app)
target_include_directories(ihsplay PRIVATE app)

if(TARGET_WINRT)
    target_link_libraries(ihslib PRIVATE ssp)
endif()

target_link_libraries(ihsplay PRIVATE lvgl ihslib ihslib-hid-sdl ss4s inih)
target_link_libraries(ihsplay PRIVATE lv_gridview)

get_target_property(SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY ss4s SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY)

set_target_properties(ihsplay PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH_USE_LINK_PATH TRUE
        INSTALL_RPATH "${SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY}")

target_include_directories(ihsplay SYSTEM PRIVATE ${SDL2_INCLUDE_DIRS} ${OPUS_INCLUDE_DIRS} ${MBEDCRYPTO_INCLUDE_DIRS})
target_link_libraries(ihsplay PRIVATE ${SDL2_LIBRARIES} ${OPUS_LIBRARIES} Fontconfig::Fontconfig Freetype::Freetype
        ${MBEDCRYPTO_LIBRARY})

if (TARGET_WEBOS)
    set(SS4S_MODULES_INI_PATH_RUNTIME "${CMAKE_INSTALL_LIBDIR}/ss4s_modules.ini")
    set(IHSPLAY_FEATURE_RELMOUSE OFF)
    set(IHSPLAY_FEATURE_LIBCEC OFF)
    set(IHSPLAY_FEATURE_FORCE_FULLSCREEN ON)
    target_link_libraries(ihsplay PRIVATE rt)
    include(PackageWebOS)
else ()
    # TODO: remove for packaged builds
    set(SS4S_MODULES_INI_PATH_RUNTIME ${SS4S_MODULES_INI_OUTPUT_FILE})
    set(CMAKE_INSTALL_PREFIX /usr CACHE PATH "Install Prefix")
    include(GNUInstallDirs)
endif ()

if (IHSPLAY_SANITIZE_ADDRESS)
    target_compile_options(ihsplay PRIVATE -fsanitize=address)
    target_link_options(ihsplay PRIVATE -fsanitize=address -static-libasan)
    target_link_libraries(ihsplay PRIVATE asan)
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(IHSPLAY_IS_DEBUG ON)
endif ()

configure_file(app/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)
target_include_directories(ihsplay PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(tests)


install(TARGETS ihsplay RUNTIME)
if(WINRT)
    install(FILES ${MXE}/usr/${TRIPLET}/bin/SDL2.dll DESTINATION bin)
    install(FILES ${MXE}/usr/${TRIPLET}/bin/SDL2_net.dll DESTINATION bin)
    install(FILES ${MXE}/usr/${TRIPLET}/bin/libfontconfig-1.dll DESTINATION bin)
    install(FILES ${MXE}/usr/${TRIPLET}/bin/libfreetype-6.dll DESTINATION bin)
    install(FILES ${MXE}/usr/${TRIPLET}/bin/libssp-0.dll DESTINATION bin)
    install(FILES ${MXE}/usr/${TRIPLET}/bin/libwinpthread-1.dll DESTINATION bin)
    install(FILES ${MXE}/usr/${TRIPLET}/bin/libsamplerate-0.dll DESTINATION bin)
    install(FILES ${MXE}/usr/${TRIPLET}/bin/libexpat-1.dll DESTINATION bin)
    install(FILES ${MXE}/usr/${TRIPLET}/bin/libintl-8.dll DESTINATION bin)
    install(FILES ${MXE}/usr/${TRIPLET}/bin/libbz2.dll DESTINATION bin)
    install(FILES ${MXE}/usr/${TRIPLET}/bin/libpng16-16.dll DESTINATION bin)
    install(FILES ${MXE}/usr/${TRIPLET}/bin/libz.dll DESTINATION bin)
    install(FILES ${MXE}/usr/${TRIPLET}/bin/libharfbuzz-0.dll DESTINATION bin)
    install(FILES ${MXE}/usr/${TRIPLET}/bin/libiconv-2.dll DESTINATION bin)
    install(FILES ${MXE}/usr/${TRIPLET}/bin/libglib-2.0-0.dll DESTINATION bin)
    install(FILES ${MXE}/usr/${TRIPLET}/bin/libpcre-1.dll DESTINATION bin)
endif()
