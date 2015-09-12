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
	gspell.h				\
	gspell-checker.h			\
	gspell-checker-dialog.h			\
	gspell-checker-language.h		\
	gspell-inline-checker-gtv.h		\
	gspell-language-dialog.h		\
	gspell-navigator.h			\
	gspell-navigator-gtv.h

gspell_public_c_files =				\
	gspell-checker.c			\
	gspell-checker-dialog.c			\
	gspell-checker-language.c		\
	gspell-inline-checker-gtv.c		\
	gspell-language-dialog.c		\
	gspell-navigator.c			\
	gspell-navigator-gtv.c

gspell_private_headers =	\
	gspell-utils.h		\
	gtktextregion.h

gspell_private_c_files =	\
	gspell-utils.c		\
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
	-export-symbols-regex "^gspell_.*"

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

libosx_la_SOURCES =	\
	gspell-osx.c	\
	gspell-osx.h

libgspell_3_0_la_LIBADD += libosx.la
endif # OS_OSX

-include $(top_srcdir)/git.mk
