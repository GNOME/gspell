cmake_minimum_required (VERSION 3.27)
project (gspell
	VERSION 1.12.2
	LANGUAGES C)

set (gsw_lib_api_version 1)

set (CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

include (cmake/gsw-c-flags.cmake)
GswCFlagsInit ()
GswCFlagsPrintMessage ()

include (cmake/gsw-pkg-config.cmake)
GswPkgConfigInit ()

include (cmake/gsw-mkenums.cmake)
GswMkenumsInit ()

include (cmake/gsw-resources.cmake)
GswResourcesInit ()

include (cmake/gsw-misc.cmake)

include (GNUInstallDirs)

pkg_check_modules (DEPS REQUIRED
	"glib-2.0>=2.44"
	"gtk+-3.0>=3.20"
	"enchant-2>=2.2"
	"icu-uc")

configure_file ("gspell-cmake.pc.in"
	"${PROJECT_BINARY_DIR}/${PROJECT_NAME}-${gsw_lib_api_version}.pc"
	@ONLY)
install (FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-${gsw_lib_api_version}.pc"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

set (GETTEXT_PACKAGE "${PROJECT_NAME}-${gsw_lib_api_version}")
configure_file ("config-cmake.h.in"
	"${PROJECT_BINARY_DIR}/config-h/config.h"
	@ONLY)

add_subdirectory (gspell)
