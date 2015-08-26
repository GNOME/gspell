SUBDIRS = resources

AM_CPPFLAGS =				\
	-DG_LOG_DOMAIN=\"gspell\"	\
	-I$(top_builddir)		\
	-I$(top_srcdir)			\
	$(WARN_CFLAGS)			\
	$(DEP_CFLAGS)			\
	$(GTK_MAC_CFLAGS)

AM_LDFLAGS = $(WARN_LDFLAGS)

BUILT_SOURCES =			\
	gspell-resources.c

gspell_public_headers =				\
	gedit-automatic-spell-checker.h		\
	gedit-spell-checker.h			\
	gedit-spell-checker-dialog.h		\
	gedit-spell-checker-language.h		\
	gedit-spell-language-dialog.h		\
	gedit-spell-navigator.h			\
	gedit-spell-navigator-gtv.h

gspell_public_c_files =				\
	gedit-automatic-spell-checker.c		\
	gedit-spell-checker.c			\
	gedit-spell-checker-dialog.c		\
	gedit-spell-checker-language.c		\
	gedit-spell-language-dialog.c		\
	gedit-spell-navigator.c			\
	gedit-spell-navigator-gtv.c

gspell_private_headers =	\
	gedit-spell-utils.h	\
	gtktextregion.h

gspell_private_c_files =	\
	gedit-spell-utils.c	\
	gtktextregion.c

lib_LTLIBRARIES = libgspell-3.0.la

libgspell_3_0_la_SOURCES =		\
	$(gspell_public_headers)	\
	$(gspell_public_c_files)	\
	$(gspell_private_headers)	\
	$(gspell_private_c_files)

# do not distribute generated files
nodist_libgspell_3_0_la_SOURCES = \
	$(BUILT_SOURCES)

libgspell_3_0_la_LIBADD =	\
	$(DEP_LIBS)		\
	$(GTK_MAC_LIBS)

libgspell_3_0_la_LDFLAGS =			\
	-no-undefined				\
	-export-symbols-regex "^gedit_.*"

libgspell_3_0_includedir = $(includedir)/gspell-3.0/gspell
libgspell_3_0_include_HEADERS = $(gspell_public_headers)

xml_res = $(srcdir)/resources/gspell.gresource.xml
gspell-resources.c: $(xml_res) $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir)/resources --generate-dependencies $(xml_res))
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(srcdir)/resources --generate-source $(xml_res)

CLEANFILES = $(BUILT_SOURCES)

# Mac OSX convenience library and ldflags
if OS_OSX
noinst_LTLIBRARIES = libosx.la

libosx_la_LDFLAGS =		\
	-framework Cocoa	\
	$(GTK_MAC_LIBS)

libosx_la_CFLAGS =		\
	-xobjective-c		\
	$(GTK_MAC_CFLAGS)

libosx_la_SOURCES =		\
	gedit-spell-osx.c	\
	gedit-spell-osx.h

libgspell_3_0_la_LIBADD += libosx.la
endif # OS_OSX

-include $(top_srcdir)/git.mk
