cmake_minimum_required(VERSION 3.20)
option(ENABLE_TESTS "Build tests" ON)

if(ENABLE_TESTS AND VCPKG_TOOLCHAIN)
  list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif()

project(iptview LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(DEFINED ENV{IPTVIEW_VERSION})
    SET(IPTVIEW_VERSION "$ENV{IPTVIEW_VERSION}")
endif()

if(NOT IPTVIEW_VERSION)
    SET(IPTVIEW_VERSION "0.0.1")
endif()

SET(IPTVIEW_COPYRIGHT "Copyright (c) 2022 Sergiu Giurgiu <sgiurgiu11@gmail.com>")
add_definitions(-DIPTVIEW_VERSION="${IPTVIEW_VERSION}")
add_definitions(-DIPTVIEW_STRING="M3U Edit v${IPTVIEW_VERSION}")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(EXECUTABLE_NAME iptview)
set(LIBRARY_NAME iptviewlib)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Threads REQUIRED)
find_package(Qt6 COMPONENTS Core Gui Test Widgets OpenGLWidgets Network Sql DBus REQUIRED)

set(IPTVIEW_APP_LIBARIES ${CMAKE_DL_LIBS} Threads::Threads Qt6::Core)
set(IPTVIEW_APP_INCLUDES)

set(IPTVIEW_LIB_LIBARIES Qt6::Core Qt6::Widgets Qt6::OpenGLWidgets Qt6::Network Qt6::Sql Qt6::DBus)
set(IPTVIEW_LIB_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(IPTVIEW_TEST_LIBARIES Qt6::Core Qt6::Test ${CMAKE_DL_LIBS} Threads::Threads)
set(IPTVIEW_TEST_INCLUDES)

if(UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(MPV REQUIRED IMPORTED_TARGET mpv)
    list(APPEND IPTVIEW_LIB_LIBARIES PkgConfig::MPV)
endif()
if(WIN32)
    find_library(LIBMPV mpv ${LIBMPV_DIR})
    list(APPEND IPTVIEW_LIB_LIBARIES ${LIBMPV} "crypt32.lib")
    list(APPEND IPTVIEW_LIB_INCLUDES ${LIBMPV_INCLUDE})
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_subdirectory(src)

if(ENABLE_TESTS)
   add_subdirectory(test)
endif()

SET(CPACK_PACKAGE_NAME "iptview")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "IPTV View Application")
SET(CPACK_RESOURCE_FILE_LICENSE_PROVIDED FALSE)
SET(CPACK_INTERACTIVE FALSE)
SET(CPACK_INCLUDE_SUBDIR TRUE)
SET(CPACK_PACKAGE_VERSION ${IPTVIEW_VERSION})
SET(CPACK_PACKAGE_CONTACT "Sergiu Giurgiu <sgiurgiu11@gmail.com>")
SET(CPACK_PACKAGE_VENDOR "Sergiu Giurgiu <sgiurgiu11@gmail.com>")
set(CPACK_RPM_PACKAGE_LICENSE "GPLv3")
set(CPACK_RPM_RELOCATION_PATHS "/${CMAKE_INSTALL_SYSCONFDIR}" "${CMAKE_INSTALL_BINDIR}")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0) # don't prepend package name
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libmpv1, libssl1.1, libfreetype6, libopengl0, ")
set(CPACK_FREEBSD_PACKAGE_MAINTAINER "Sergiu Giurgiu <sgiurgiu11@gmail.com>")
set(CPACK_FREEBSD_PACKAGE_DEPS "multimedia/mpv" "security/openssl" "graphics/mesa-dri")

#set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})

SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
if(CPACK_DISTRIBUTION)
    SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-${CPACK_DISTRIBUTION}")
endif()

install(FILES "${CMAKE_SOURCE_DIR}/icons/iptview-icon_24.png"  DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/24x24/apps" RENAME "iptview-icon.png")
install(FILES "${CMAKE_SOURCE_DIR}/icons/iptview-icon_32.png"  DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/32x32/apps" RENAME "iptview-icon.png")
install(FILES "${CMAKE_SOURCE_DIR}/icons/iptview-icon_48.png"  DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/48x48/apps" RENAME "iptview-icon.png")
install(FILES "${CMAKE_SOURCE_DIR}/icons/iptview-icon_64.png"  DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/64x64/apps" RENAME "iptview-icon.png")
install(FILES "${CMAKE_SOURCE_DIR}/icons/iptview-icon_128.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/128x128/apps" RENAME "iptview-icon.png")
install(FILES "${CMAKE_SOURCE_DIR}/icons/iptview-icon_256.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/256x256/apps" RENAME "iptview-icon.png")
install(FILES "${CMAKE_SOURCE_DIR}/iptview.desktop" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications" RENAME "iptview.desktop")

set(CPACK_PACKAGE_EXECUTABLES "${EXECUTABLE_NAME};IPTView")
if(CPACK_GENERATOR STREQUAL "WIX")
    set(CPACK_WIX_UPGRADE_GUID "2a9d812f-6f64-4c1b-b250-dbde0216e869")
    set(CPACK_WIX_PRODUCT_ICON "${CMAKE_SOURCE_DIR}/icons/iptview-icon_48.png")
    set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/icons/iptview-icon_48.png")
    set(CPACK_WIX_PROGRAM_MENU_FOLDER "IPTView")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/gpl-3.0.rtf")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "IPTView")
endif()


INCLUDE(CPack)
