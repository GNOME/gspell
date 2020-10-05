## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS = 1.6

# The name of the module.
# For library-web, it's simpler if the DOC_MODULE follows the scheme:
# $name-$major.$minor
# (to show the API online at developer.gnome.org)
# So, add ".0".
DOC_MODULE = gspell-@GSPELL_API_VERSION@.0

# The top-level XML file (SGML in the past).
DOC_MAIN_SGML_FILE = gspell-docs.xml

# Extra options to supply to gtkdoc-scan
SCAN_OPTIONS = --rebuild-types

# The directory containing the source code. Relative to $(srcdir)
DOC_SOURCE_DIR = $(top_srcdir)/gspell

# Used for dependencies
HFILE_GLOB = $(top_srcdir)/gspell/*.h
CFILE_GLOB = $(top_srcdir)/gspell/*.c

# Extra header to include when scanning, which are not under DOC_SOURCE_DIR
# e.g. EXTRA_HFILES = $(top_srcdir)/contrib/extra.h
EXTRA_HFILES =						\
	$(top_builddir)/gspell/gspell-enum-types.h

# Header files or dirs to ignore when scanning. Use base file/dir names
# e.g. IGNORE_HFILES=gtkdebug.h gtkintl.h private_code
IGNORE_HFILES =					\
	gspell.h				\
	gspellregion.h				\
	gspell-checker-private.h		\
	gspell-context-menu.h			\
	gspell-current-word-policy.h		\
	gspell-entry-private.h			\
	gspell-entry-utils.h			\
	gspell-icu.h				\
	gspell-inline-checker-text-buffer.h	\
	gspell-osx.h				\
	gspell-text-iter.h			\
	gspell-utils.h				\
	gspell-version.h

# Extra options to supply to gtkdoc-mkdb
MKDB_OPTIONS = --xml-mode --output-format=xml

# Images to copy into HTML directory
HTML_IMAGES =

# Extra XML files that are included by $(DOC_MAIN_SGML_FILE)
content_files = intro.xml

# Extra options to supply to gtkdoc-fixref
FIXXREF_OPTIONS =

# CFLAGS and LDFLAGS for compiling gtkdoc-scangobj with your library.
# Only needed if you are using gtkdoc-scangobj to dynamically query widget
# signals and properties.
GTKDOC_CFLAGS = -I$(top_srcdir) $(DEP_CFLAGS)
GTKDOC_LIBS =								\
	$(top_builddir)/gspell/libgspell-@GSPELL_API_VERSION@.la	\
	$(DEP_LIBS)

MAINTAINERCLEANFILES =			\
	$(DOC_MODULE)-overrides.txt	\
	$(DOC_MODULE).types

# include common portion ...
include $(top_srcdir)/gtk-doc.make

EXTRA_DIST +=			\
	gspell-docs.xml.in	\
	intro.xml.in

DISTCLEANFILES =		\
	gspell-docs.xml		\
	intro.xml

-include $(top_srcdir)/git.mk
