cmake_minimum_required (VERSION 3.20)
project (gspell
  LANGUAGES C)

include (cmake/gsw.cmake)
GswInitBasic ()

pkg_check_modules (DEPS REQUIRED
  glib-2.0>=2.44
  gtk+-3.0>=3.20
  enchant-2>=2.1.3
  icu-uc)
