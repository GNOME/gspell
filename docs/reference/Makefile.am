## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS = 1.6

# The name of the module.
DOC_MODULE = gspell1

# The top-level XML file (SGML in the past).
DOC_MAIN_SGML_FILE = gspell-docs.xml

# Extra options to supply to gtkdoc-scan
SCAN_OPTIONS = --rebuild-types

# The directory containing the source code. Relative to $(srcdir)
DOC_SOURCE_DIR = $(top_srcdir)/gspell

# Used for dependencies
HFILE_GLOB = $(top_srcdir)/gspell/*.h
CFILE_GLOB = $(top_srcdir)/gspell/*.c

# Header files or dirs to ignore when scanning. Use base file/dir names
# e.g. IGNORE_HFILES=gtkdebug.h gtkintl.h private_code
IGNORE_HFILES =			\
	gspell-osx.h		\
	gspell-utils.h		\
	gtktextregion.h

# Extra options to supply to gtkdoc-mkdb
MKDB_OPTIONS = --xml-mode --output-format=xml

# Images to copy into HTML directory
HTML_IMAGES =

# Extra XML files that are included by $(DOC_MAIN_SGML_FILE)
content_files =

# Extra options to supply to gtkdoc-fixref
FIXXREF_OPTIONS =							\
	--extra-dir=$(GLIB_PREFIX)/share/gtk-doc/html/gobject		\
	--extra-dir=$(GLIB_PREFIX)/share/gtk-doc/html/glib		\
	--extra-dir=$(GTK_PREFIX)/share/gtk-doc/html/gtk3

# CFLAGS and LDFLAGS for compiling gtkdoc-scangobj with your library.
# Only needed if you are using gtkdoc-scangobj to dynamically query widget
# signals and properties.
GTKDOC_CFLAGS = -I$(top_srcdir) $(DEP_CFLAGS)
GTKDOC_LIBS = $(top_builddir)/gspell/libgspell-1.la $(DEP_LIBS)

MAINTAINERCLEANFILES =		\
	gspell1-overrides.txt	\
	gspell1.types

# include common portion ...
include $(top_srcdir)/gtk-doc.make

-include $(top_srcdir)/git.mk
