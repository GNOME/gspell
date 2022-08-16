cmake_minimum_required (VERSION 3.20)
project (gspell
  VERSION 1.11.1
  LANGUAGES C)

include (cmake/gsw.cmake)
GswInitBasic ()

pkg_check_modules (DEPS REQUIRED
  glib-2.0>=2.44
  gtk+-3.0>=3.20
  enchant-2>=2.1.3
  icu-uc)

set (GSPELL_API_VERSION 1)
set (GETTEXT_PACKAGE "gspell-${GSPELL_API_VERSION}")
configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h")
