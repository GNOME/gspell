dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.64)

# Package version of the form 'major.minor.micro'.
m4_define(gspell_package_version, 1.12.2)

AC_INIT([gspell],
	[gspell_package_version],
	[https://gitlab.gnome.org/GNOME/gspell/issues],
	[gspell],
	[https://gitlab.gnome.org/GNOME/gspell])

AX_REQUIRE_DEFINED([AX_IS_RELEASE])
AX_IS_RELEASE([git-directory])

# Libtool versioning
#
# For development releases (if the minor package version is odd), keep the same
# Libtool version.
#
# For a new minor stable release (when incrementing the minor package version
# to an even number), apply the following algorithm step by step:
# 1. If the library source code has changed at all since the last
#    update, then increment REVISION.
# 2. If any exported functions or data have been added, removed, or
#    changed since the last update, increment CURRENT and set REVISION
#    to 0.
# 3. If any exported functions or data have been added since the last
#    public release, increment AGE.
# 4. If any exported functions or data have been removed since the last
#    public release, set AGE to 0.
#
# When incrementing the API version (usually for a new major package version),
# set CURRENT, REVISION and AGE to 0 since it's like a new library.

GSPELL_LT_CURRENT=5
GSPELL_LT_REVISION=2
GSPELL_LT_AGE=3
GSPELL_LT_VERSION="$GSPELL_LT_CURRENT:$GSPELL_LT_REVISION:$GSPELL_LT_AGE"
AC_SUBST(GSPELL_LT_VERSION)

# API version, used for parallel installability.
# Not used in:
# - po/Makevars
# - the docs/reference/gspell-1.0-sections.txt filename
# Because as far as I've tested, it is not easily feasible to not hardcode the
# API version in those places (swilmet).
GSPELL_API_VERSION=1
AC_SUBST(GSPELL_API_VERSION)

# Dependencies
enchant_req=2.2
glib_req=2.44
gtk_req=3.20

AC_CONFIG_SRCDIR([gspell/gspell-checker.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# gnu strictness to generate the INSTALL file
AM_INIT_AUTOMAKE([1.11 tar-ustar dist-xz no-dist-gzip gnu -Wno-portability])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Check for programs
AC_PROG_CC
AC_PROG_INSTALL

AX_REQUIRE_DEFINED([PKG_PROG_PKG_CONFIG])
AX_REQUIRE_DEFINED([PKG_INSTALLDIR])
AX_REQUIRE_DEFINED([PKG_CHECK_VAR])
PKG_PROG_PKG_CONFIG
PKG_INSTALLDIR

PKG_CHECK_VAR([GLIB_MKENUMS], [glib-2.0], [glib_mkenums])
PKG_CHECK_VAR([GLIB_COMPILE_RESOURCES], [gio-2.0], [glib_compile_resources])

# Needed on Mac OS X
AC_PROG_OBJC

# Initialize libtool
LT_PREREQ([2.2.6])
LT_INIT([disable-static])

# Compile warnings. There can be deprecation warnings at any time, so disable
# -Werror by default with the [yes] argument.
AX_REQUIRE_DEFINED([AX_COMPILER_FLAGS])
AX_COMPILER_FLAGS([WARN_CFLAGS], [WARN_LDFLAGS], [yes])

# Dynamic linking dependencies
AX_REQUIRE_DEFINED([AX_PKG_CHECK_MODULES])
AX_PKG_CHECK_MODULES([DEP],
		     [glib-2.0 >= $glib_req  gtk+-3.0 >= $gtk_req  enchant-2 >= $enchant_req],
		     [icu-uc])

# i18n
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_REQUIRE_VERSION([0.19.6])
AC_SUBST([GETTEXT_PACKAGE], [gspell-$GSPELL_API_VERSION])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Define to the gettext package name.])

# Documentation
AX_REQUIRE_DEFINED([GTK_DOC_CHECK])
GTK_DOC_CHECK([1.25], [--flavour=no-tmpl])

# Installed Tests
#
# Initially done for Gnome Continuous, but also useful on Windows with MSYS2
# because with Autotools the executables need to be installed there in order to
# see the hunspell dictionaries.
AC_ARG_ENABLE(installed_tests,
	      AS_HELP_STRING([--enable-installed-tests],
			     [Install test programs (default: no)]),,
	      [enable_installed_tests=no])

AM_CONDITIONAL(INSTALLED_TESTS, test "x$enable_installed_tests" = "xyes")

# Introspection
AX_REQUIRE_DEFINED([GOBJECT_INTROSPECTION_CHECK])
GOBJECT_INTROSPECTION_CHECK([1.42.0])

# Vala
AX_REQUIRE_DEFINED([VAPIGEN_CHECK])
VAPIGEN_CHECK

# Code coverage for unit tests
AX_REQUIRE_DEFINED([AX_CODE_COVERAGE])
AX_CODE_COVERAGE

# Valgrind check
AX_REQUIRE_DEFINED([AX_VALGRIND_CHECK])
AX_VALGRIND_CHECK

# Check if the compiler supports -fvisibility=hidden
SAVED_CFLAGS="${CFLAGS}"
CFLAGS="-fvisibility=hidden"
AC_MSG_CHECKING([for -fvisibility=hidden compiler flag])
AC_TRY_COMPILE([], [return 0],
               AC_MSG_RESULT(yes)
               enable_fvisibility_hidden=yes,
               AC_MSG_RESULT(no)
               enable_fvisibility_hidden=no)
CFLAGS="${SAVED_CFLAGS}"

GSPELL_HIDDEN_VISIBILITY_CFLAGS=""
if test "x${enable_fvisibility_hidden}" = "xyes"
then
	GSPELL_HIDDEN_VISIBILITY_CFLAGS="-fvisibility=hidden"
fi
AC_SUBST([GSPELL_HIDDEN_VISIBILITY_CFLAGS])

# Check for Mac OS X platform
AC_MSG_CHECKING([for Mac OS X platform])
case "$host" in
  *-*-darwin*)
    platform_osx=yes
    ;;
  *)
    platform_osx=no
    ;;
esac
AC_MSG_RESULT([$platform_osx])
AM_CONDITIONAL(PLATFORM_OSX, test "$platform_osx" = "yes")

# Check for native Mac OS X
gdk_targets=`$PKG_CONFIG --variable=targets gdk-3.0`

gdk_windowing="x11"

for target in $gdk_targets;
do
	case "$target" in
		quartz)
			gdk_windowing=quartz
			;;
		win32)
			gdk_windowing=win32
			;;
	esac
done

AC_MSG_CHECKING([for native Mac OS X])
if test "$gdk_windowing" = "quartz"; then
	os_osx=yes
else
	os_osx=no
fi
AC_MSG_RESULT([$os_osx])
AM_CONDITIONAL(OS_OSX, test "$os_osx" = "yes")

if test "$os_osx" = "yes"; then
	AC_DEFINE([OS_OSX], [1], [Defined if OS is Mac OSX])
fi

# Output files
AC_CONFIG_FILES([
	gspell-${GSPELL_API_VERSION}.pc:gspell.pc.in
	Makefile
	docs/Makefile
	docs/reference/gspell-docs.xml
	docs/reference/intro.xml
	docs/reference/Makefile
	gspell/Makefile
	gspell/resources/Makefile
	gspell-app/Makefile
	po/Makefile.in
	tests/Makefile
	testsuite/Makefile
])

AC_OUTPUT

echo "
Configuration:

	${PACKAGE_NAME} version ${PACKAGE_VERSION}

	Source code location:	${srcdir}
	Prefix:			${prefix}
	Compiler:		${CC}
	Documentation:		${enable_gtk_doc}
	GObject introspection:	${found_introspection}
	Vala:			${enable_vala}
	Code coverage:		${enable_code_coverage}
	Valgrind check:		${enable_valgrind}
	Installed tests:	${enable_installed_tests}
"
