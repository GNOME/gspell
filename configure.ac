dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.64)

AC_INIT([gspell],
	[0.1.2],
	[https://bugzilla.gnome.org/enter_bug.cgi?product=gspell],
	[gspell],
	[https://wiki.gnome.org/Projects/gspell])

AX_REQUIRE_DEFINED([AX_IS_RELEASE])
AX_IS_RELEASE([git-directory])

# Libtool versioning
#
# For development releases, keep the same numbers.
#
# For stable releases, apply the following algorithm step by step:
# 1. If the library source code has changed at all since the last
#    update, then increment REVISION.
# 2. If any exported functions or data have been added, removed, or
#    changed since the last update, increment CURRENT and set REVISION
#    to 0.
# 3. If any exported functions or data have been added since the last
#    public release, increment AGE.
# 4. If any exported functions or data have been removed since the last
#    public release, set AGE to 0.

GSPELL_LT_CURRENT=0
GSPELL_LT_REVISION=0
GSPELL_LT_AGE=0
GSPELL_LT_VERSION="$GSPELL_LT_CURRENT:$GSPELL_LT_REVISION:$GSPELL_LT_AGE"
AC_SUBST(GSPELL_LT_VERSION)

# Dependencies
enchant_req=1.6.0
iso_codes_req=0.35
glib_req=2.44
gtk_req=3.16
gtksourceview_req=3.16
libxml_req=2.5.0

AC_CONFIG_SRCDIR([gspell/gspell-checker.c])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

# gnu strictness to generate the INSTALL file
AM_INIT_AUTOMAKE([1.11 tar-ustar dist-xz no-dist-gzip gnu])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Check for programs
AC_PROG_CC
AC_PROG_INSTALL

AX_REQUIRE_DEFINED([PKG_PROG_PKG_CONFIG])
AX_REQUIRE_DEFINED([PKG_INSTALLDIR])
PKG_PROG_PKG_CONFIG
PKG_INSTALLDIR

# needed on osx
AC_PROG_OBJC

# Initialize libtool
LT_PREREQ([2.2.6])
LT_INIT([disable-static])

# Pull glib-compile-resources & co.
AC_PATH_PROG(GLIB_COMPILE_RESOURCES, glib-compile-resources)

# Compile warnings
AX_REQUIRE_DEFINED([AX_COMPILER_FLAGS])
AX_COMPILER_FLAGS([WARN_CFLAGS], [WARN_LDFLAGS])

# pkg-config
AX_REQUIRE_DEFINED([AX_PKG_CHECK_MODULES])
AX_PKG_CHECK_MODULES([DEP],
		     [glib-2.0 >= $glib_req  gtk+-3.0 >= $gtk_req  gtksourceview-3.0 >= $gtksourceview_req],
		     [enchant >= $enchant_req  libxml-2.0 >= $libxml_req])

# iso-codes
AX_REQUIRE_DEFINED([PKG_CHECK_EXISTS])
PKG_CHECK_EXISTS([iso-codes >= $iso_codes_req],
		 [have_iso_codes=yes],
		 [have_iso_codes=no])

if test "x$have_iso_codes" = "xyes"; then
	AC_MSG_CHECKING([whether iso-codes has iso-639 and iso-3166 domains])
	if $PKG_CONFIG --variable=domains iso-codes | grep 639 > /dev/null && \
	   $PKG_CONFIG --variable=domains iso-codes | grep 3166 > /dev/null ; then
		result=yes
	else
		result=no
		have_iso_codes=no
	fi
	AC_MSG_RESULT([$result])
fi

if test "x$have_iso_codes" = "xyes"; then
	AC_DEFINE_UNQUOTED([ISO_CODES_PREFIX],
			   ["`$PKG_CONFIG --variable=prefix iso-codes`"],
			   [ISO codes prefix])
else
	AC_MSG_ERROR([iso-codes is required.])
fi

# i18n
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.19.4])
AC_SUBST([GETTEXT_PACKAGE], [gspell-1])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Define to the gettext package name.])

# Documentation
AX_REQUIRE_DEFINED([GTK_DOC_CHECK])
# TODO bump to 1.25 once released.
GTK_DOC_CHECK([1.24], [--flavour=no-tmpl])

GLIB_PREFIX="`$PKG_CONFIG --variable=prefix glib-2.0`"
GTK_PREFIX="`$PKG_CONFIG --variable=prefix gtk+-3.0`"

AC_SUBST(GLIB_PREFIX)
AC_SUBST(GTK_PREFIX)

# Introspection
AX_REQUIRE_DEFINED([GOBJECT_INTROSPECTION_CHECK])
GOBJECT_INTROSPECTION_CHECK([1.42.0])

# Vala
AX_REQUIRE_DEFINED([VAPIGEN_CHECK])
VAPIGEN_CHECK

# check for osx platform
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

# check for native osx
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

	AX_PKG_CHECK_MODULES([GTK_MAC],
			     [],
			     [gtk-mac-integration-gtk3 >= 2.0.8])

	AC_SUBST(GTK_MAC_CFLAGS)
	AC_SUBST(GTK_MAC_LIBS)
fi

# Output files
AC_CONFIG_FILES([
	gspell-1.pc
	Makefile
	docs/Makefile
	docs/reference/Makefile
	gspell/Makefile
	gspell/resources/Makefile
	po/Makefile.in
	tests/Makefile
])

AC_OUTPUT

echo "
Configuration:

	Source code location:	${srcdir}
	Compiler:		${CC}
	GObject introspection:	${found_introspection}
	Vala:			${enable_vala}
"
