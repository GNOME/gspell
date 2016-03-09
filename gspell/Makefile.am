SUBDIRS = resources

@CODE_COVERAGE_RULES@

AM_CPPFLAGS =				\
	-DDATADIR=\""$(datadir)"\"	\
	-DG_LOG_DOMAIN=\"gspell\"	\
	-DGSPELL_COMPILATION		\
	-I$(top_builddir)		\
	-I$(top_srcdir)			\
	$(WARN_CFLAGS)			\
	$(CODE_COVERAGE_CPPFLAGS)	\
	$(DEP_CFLAGS)			\
	$(GTK_MAC_CFLAGS)

BUILT_SOURCES =			\
	gspell-resources.c

gspell_public_headers =				\
	gspell.h				\
	gspell-checker.h			\
	gspell-checker-dialog.h			\
	gspell-inline-checker-text-view.h	\
	gspell-language.h			\
	gspell-language-chooser.h		\
	gspell-language-chooser-button.h	\
	gspell-language-chooser-dialog.h	\
	gspell-navigator.h			\
	gspell-navigator-text-view.h		\
	gspell-text-buffer.h

gspell_public_c_files =				\
	gspell-checker.c			\
	gspell-checker-dialog.c			\
	gspell-inline-checker-text-view.c	\
	gspell-language.c			\
	gspell-language-chooser.c		\
	gspell-language-chooser-button.c	\
	gspell-language-chooser-dialog.c	\
	gspell-navigator.c			\
	gspell-navigator-text-view.c		\
	gspell-text-buffer.c

gspell_private_headers =			\
	gconstructor.h				\
	gspell-buffer-notifier.h		\
	gspell-inline-checker-text-buffer.h	\
	gspell-text-iter.h			\
	gspell-text-region.h			\
	gspell-utils.h

gspell_private_c_files =			\
	gspell-buffer-notifier.c		\
	gspell-init.c				\
	gspell-inline-checker-text-buffer.c	\
	gspell-text-iter.c			\
	gspell-text-region.c			\
	gspell-utils.c

# Helper Libtool library, so that the private functions can be used in unit
# tests.
noinst_LTLIBRARIES = libgspell-core.la

libgspell_core_la_SOURCES =		\
	$(gspell_private_c_files)	\
	$(gspell_private_headers)	\
	$(gspell_public_c_files)	\
	$(gspell_public_headers)

# Do not distribute generated files.
nodist_libgspell_core_la_SOURCES = \
	$(BUILT_SOURCES)

libgspell_core_la_CFLAGS = \
	$(CODE_COVERAGE_CFLAGS)

libgspell_core_la_LDFLAGS =		\
	-no-undefined			\
	$(WARN_LDFLAGS)			\
	$(CODE_COVERAGE_LDFLAGS)

# The real library.
lib_LTLIBRARIES = libgspell-1.la

libgspell_1_la_SOURCES =

libgspell_1_la_LIBADD =		\
	libgspell-core.la	\
	$(DEP_LIBS)		\
	$(GTK_MAC_LIBS)

libgspell_1_la_CFLAGS = \
	$(CODE_COVERAGE_CFLAGS)

libgspell_1_la_LDFLAGS =			\
	-version-info $(GSPELL_LT_VERSION)	\
	-no-undefined				\
	-export-symbols-regex "^gspell_.*"	\
	$(WARN_LDFLAGS)				\
	$(CODE_COVERAGE_LDFLAGS)

libgspell_1_includedir = $(includedir)/gspell-1/gspell
libgspell_1_include_HEADERS = $(gspell_public_headers)

xml_res = $(srcdir)/resources/gspell.gresource.xml
gspell-resources.c: $(xml_res) $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir)/resources --generate-dependencies $(xml_res))
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(srcdir)/resources --generate-source $(xml_res)

CLEANFILES = $(BUILT_SOURCES)

if OS_OSX
libgspell_1_la_LDFLAGS += \
	-framework Cocoa

libgspell_1_la_CFLAGS += \
	-xobjective-c

gspell_private_headers += \
	gspell-osx.h

gspell_private_c_files += \
	gspell-osx.c

endif # OS_OSX

if HAVE_INTROSPECTION
-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS = Gspell-1.gir

INTROSPECTION_SCANNER_ENV = CC="$(CC)"

Gspell-1.gir: libgspell-1.la $(BUILT_SOURCES)
INTROSPECTION_SCANNER_ARGS =		\
	-I$(top_srcdir)			\
	-I$(top_builddir)		\
	--c-include=gspell/gspell.h	\
	--warn-all
Gspell_1_gir_NAMESPACE = Gspell
Gspell_1_gir_VERSION = 1
Gspell_1_gir_CFLAGS = $(DEP_CFLAGS)
Gspell_1_gir_SCANNERFLAGS = $(WARN_SCANNERFLAGS)
Gspell_1_gir_LIBS = libgspell-1.la
Gspell_1_gir_FILES =			\
	$(gspell_public_headers)	\
	$(gspell_public_c_files)	\
	$(BUILT_SOURCES)

Gspell_1_gir_INCLUDES = Gtk-3.0
Gspell_1_gir_PACKAGES = gtk+-3.0
Gspell_1_gir_EXPORT_PACKAGES = gspell-1

girdir = $(datadir)/gir-1.0
gir_DATA = Gspell-1.gir

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = Gspell-1.typelib

CLEANFILES +=		\
	$(gir_DATA)	\
	$(typelib_DATA)

endif # HAVE_INTROSPECTION

if ENABLE_VAPIGEN
-include $(VAPIGEN_MAKEFILE)

gspell-1.vapi: Gspell-1.gir

VAPIGEN_VAPIS = gspell-1.vapi

gspell_1_vapi_DEPS = gtk+-3.0

gspell-1.deps:
	$(AM_V_GEN) for pkg in $(gspell_1_vapi_DEPS); do \
		echo $$pkg >> $@; \
	done

gspell_1_vapi_METADATADIRS = $(srcdir)
gspell_1_vapi_FILES = Gspell-1.gir

vapidir = $(datadir)/vala/vapi
vapi_DATA =		\
	gspell-1.vapi	\
	gspell-1.deps

CLEANFILES += $(vapi_DATA)

endif # ENABLE_VAPIGEN

-include $(top_srcdir)/git.mk
