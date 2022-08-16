cmake_minimum_required (VERSION 3.20)
project (gspell
  VERSION 1.11.1
  LANGUAGES C)

include (cmake/gsw.cmake)
GswInitBasic ()

set (dep_glib "glib-2.0 >= 2.44")
set (dep_gtk "gtk+-3.0 >= 3.20")
set (dep_enchant "enchant-2 >= 2.1.3")
set (dep_icu "icu-uc")

set (PKG_REQUIRES "${dep_glib} ${dep_gtk} ${dep_enchant}")
set (PKG_REQUIRES_PRIVATE "${dep_icu}")

pkg_check_modules (DEPS REQUIRED
  ${dep_glib}
  ${dep_gtk}
  ${dep_enchant}
  ${dep_icu})

set (GSPELL_API_VERSION 1)
set (GETTEXT_PACKAGE "gspell-${GSPELL_API_VERSION}")
configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h")

configure_file (
  "${PROJECT_SOURCE_DIR}/gspell.pc.in"
  "${PROJECT_BINARY_DIR}/gspell-${GSPELL_API_VERSION}.pc"
  @ONLY)
install (FILES "${PROJECT_BINARY_DIR}/gspell-${GSPELL_API_VERSION}.pc"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
