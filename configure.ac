dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.64)

AC_INIT([gspell],
        [0.0.1],
	[],
	[gspell],
	[])

AX_REQUIRE_DEFINED([AX_IS_RELEASE])
AX_IS_RELEASE([git-directory])

# Dependencies
enchant_req=1.6.0
iso_codes_req=0.35
glib_req=2.44
gtk_req=3.16
gtksourceview_req=3.16
libxml_req=2.5.0

AC_CONFIG_SRCDIR([gspell/gedit-spell-checker.c])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.11 tar-ustar dist-xz no-dist-gzip foreign])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Check for programs
AC_PROG_CC
AC_PROG_INSTALL
AX_REQUIRE_DEFINED([PKG_PROG_PKG_CONFIG])
PKG_PROG_PKG_CONFIG
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
AC_SUBST([GETTEXT_PACKAGE], [$PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Define to the gettext package name.])

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
	Makefile
	po/Makefile.in
	gspell/Makefile
	gspell/resources/Makefile
])

AC_OUTPUT
