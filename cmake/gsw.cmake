# "Gsw" as in "G swilmet" or "G software", as a namespace.


macro (GswInitBasic)
  find_package (PkgConfig)
  include (GNUInstallDirs)
  set (CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
endmacro ()

# Try to mimic the AX_COMPILER_FLAGS Autotools macro.
#
# For the rationale (having such a long list of flags instead of just relying on
# -Wall and -Wextra) see the GCC manpage for the -Wall option:
#
# """
# Note that some warning flags are not implied by -Wall. Some of them warn about
# constructions that users generally do not consider questionable, but which
# occasionally you might wish to check for; others warn about constructions that
# are necessary or hard to avoid in some cases, and there is no simple way to
# modify the code to suppress the warning. Some of them are enabled by -Wextra
# but many of them must be enabled individually.
# """
function (GswCompilerFlags)
  add_compile_options (
    "-Wall"
    "-Wextra"
    "-fno-strict-aliasing"
    "-Wundef"
    "-Wwrite-strings"
    "-Wpointer-arith"
    "-Wmissing-declarations"
    "-Wredundant-decls"
    "-Wno-unused-parameter"
    "-Wno-missing-field-initializers"
    "-Wformat=2"
    "-Wcast-align"
    "-Wformat-nonliteral"
    "-Wformat-security"
    "-Wsign-compare"
    "-Wstrict-aliasing"
    "-Wshadow"
    "-Winline"
    "-Wpacked"
    "-Wmissing-format-attribute"
    "-Wmissing-noreturn"
    "-Winit-self"
    "-Wredundant-decls"
    "-Wmissing-include-dirs"
    "-Wunused-but-set-variable"
    "-Warray-bounds"
    "-Wreturn-type"
    "-Wswitch-enum"
    "-Wswitch-default"
    "-Wduplicated-cond"
    "-Wduplicated-branches"
    "-Wlogical-op"
    "-Wrestrict"
    "-Wnull-dereference"
    "-Wdouble-promotion"
    "-Wnested-externs"
    "-Wmissing-prototypes"
    "-Wstrict-prototypes"
    "-Wdeclaration-after-statement"
    "-Wimplicit-function-declaration"
    "-Wold-style-definition"
    "-Wjump-misses-init"
  )
endfunction ()

# When pkg-config is used.
function (GswAddExecutable pkg_dep executable_name sources)
  include_directories (${${pkg_dep}_INCLUDE_DIRS})
  add_compile_options (${${pkg_dep}_CFLAGS_OTHER})
  add_executable ("${executable_name}" ${sources})
  target_link_libraries ("${executable_name}" ${${pkg_dep}_LDFLAGS})

  install (TARGETS "${executable_name}"
    DESTINATION "${CMAKE_INSTALL_BINDIR}")
endfunction ()

# When pkg-config is used.
function (GswAddLibrary pkg_dep library_name sources)
  include_directories (${${pkg_dep}_INCLUDE_DIRS})
  add_compile_options (${${pkg_dep}_CFLAGS_OTHER})
  add_library ("${library_name}" SHARED ${sources})
  target_link_libraries ("${library_name}" ${${pkg_dep}_LDFLAGS})

  install (TARGETS "${library_name}"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}")
endfunction ()

# Useful for printing a configuration summary.
function (GswYesOrNo condition result)
  if (${condition})
    set (${result} yes PARENT_SCOPE)
  else ()
    set (${result} no PARENT_SCOPE)
  endif ()
endfunction ()
