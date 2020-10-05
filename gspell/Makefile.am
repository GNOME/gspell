SUBDIRS = resources

include $(top_srcdir)/aminclude_static.am
clean-local: code-coverage-clean
distclean-local: code-coverage-dist-clean

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

gspell_public_headers =				\
	gspell.h				\
	gspell-checker.h			\
	gspell-checker-dialog.h			\
	gspell-entry.h				\
	gspell-entry-buffer.h			\
	gspell-language.h			\
	gspell-language-chooser.h		\
	gspell-language-chooser-button.h	\
	gspell-language-chooser-dialog.h	\
	gspell-navigator.h			\
	gspell-navigator-text-view.h		\
	gspell-text-buffer.h			\
	gspell-text-view.h			\
	gspell-version.h

gspell_public_c_files =				\
	gspell-checker.c			\
	gspell-checker-dialog.c			\
	gspell-entry.c				\
	gspell-entry-buffer.c			\
	gspell-language.c			\
	gspell-language-chooser.c		\
	gspell-language-chooser-button.c	\
	gspell-language-chooser-dialog.c	\
	gspell-navigator.c			\
	gspell-navigator-text-view.c		\
	gspell-text-buffer.c			\
	gspell-text-view.c

gspell_private_headers =			\
	gconstructor.h				\
	gspellregion.h				\
	gspell-checker-private.h		\
	gspell-context-menu.h			\
	gspell-current-word-policy.h		\
	gspell-entry-private.h			\
	gspell-entry-utils.h			\
	gspell-icu.h				\
	gspell-inline-checker-text-buffer.h	\
	gspell-text-iter.h			\
	gspell-utils.h

gspell_private_c_files =			\
	gspellregion.c				\
	gspell-context-menu.c			\
	gspell-current-word-policy.c		\
	gspell-entry-utils.c			\
	gspell-icu.c				\
	gspell-init.c				\
	gspell-inline-checker-text-buffer.c	\
	gspell-text-iter.c			\
	gspell-utils.c

gspell_built_public_headers =		\
	gspell-enum-types.h

gspell_built_public_c_files =		\
	gspell-enum-types.c

BUILT_SOURCES =				\
	gspell-resources.c		\
	$(gspell_built_public_c_files)	\
	$(gspell_built_public_headers)

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

libgspell_core_la_LIBADD =	\
	$(CODE_COVERAGE_LIBS)

libgspell_core_la_CFLAGS =			\
	$(GSPELL_HIDDEN_VISIBILITY_CFLAGS)	\
	$(CODE_COVERAGE_CFLAGS)

libgspell_core_la_LDFLAGS =	\
	-no-undefined		\
	$(WARN_LDFLAGS)

# The real library.
lib_LTLIBRARIES = libgspell-@GSPELL_API_VERSION@.la

libgspell_@GSPELL_API_VERSION@_la_SOURCES =

libgspell_@GSPELL_API_VERSION@_la_LIBADD =	\
	libgspell-core.la			\
	$(DEP_LIBS)				\
	$(GTK_MAC_LIBS)				\
	$(CODE_COVERAGE_LIBS)

libgspell_@GSPELL_API_VERSION@_la_CFLAGS =	\
	$(GSPELL_HIDDEN_VISIBILITY_CFLAGS)	\
	$(CODE_COVERAGE_CFLAGS)

libgspell_@GSPELL_API_VERSION@_la_LDFLAGS =	\
	-version-info $(GSPELL_LT_VERSION)	\
	-no-undefined				\
	-export-dynamic				\
	$(WARN_LDFLAGS)

libgspell_includedir = $(includedir)/gspell-@GSPELL_API_VERSION@/gspell
libgspell_include_HEADERS = $(gspell_public_headers)
nodist_libgspell_include_HEADERS = $(gspell_built_public_headers)

xml_res = $(srcdir)/resources/gspell.gresource.xml
gspell-resources.c: $(xml_res) $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir)/resources --generate-dependencies $(xml_res))
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(srcdir)/resources --generate-source $(xml_res)

ENUM_TYPES = $(gspell_public_headers)

gspell-enum-types.h: gspell-enum-types.h.template $(ENUM_TYPES) $(GLIB_MKENUMS)
	$(AM_V_GEN) (cd $(srcdir) && $(GLIB_MKENUMS) --template gspell-enum-types.h.template $(ENUM_TYPES)) > $@

gspell-enum-types.c: gspell-enum-types.c.template $(ENUM_TYPES) $(GLIB_MKENUMS)
	$(AM_V_GEN) (cd $(srcdir) && $(GLIB_MKENUMS) --template gspell-enum-types.c.template $(ENUM_TYPES)) > $@

EXTRA_DIST =				\
	gspell-enum-types.c.template	\
	gspell-enum-types.h.template

CLEANFILES = $(BUILT_SOURCES)

if OS_OSX
libgspell_@GSPELL_API_VERSION@_la_LDFLAGS += \
	-framework Cocoa

libgspell_@GSPELL_API_VERSION@_la_CFLAGS += \
	-xobjective-c

gspell_private_headers += \
	gspell-osx.h

gspell_private_c_files += \
	gspell-osx.c

endif # OS_OSX

if HAVE_INTROSPECTION
-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS = Gspell-@GSPELL_API_VERSION@.gir

INTROSPECTION_SCANNER_ENV = CC="$(CC)"

Gspell-@GSPELL_API_VERSION@.gir: libgspell-@GSPELL_API_VERSION@.la $(BUILT_SOURCES)
INTROSPECTION_SCANNER_ARGS =		\
	-I$(top_srcdir)			\
	-I$(top_builddir)		\
	--c-include=gspell/gspell.h	\
	--warn-all
Gspell_@GSPELL_API_VERSION@_gir_NAMESPACE = Gspell
Gspell_@GSPELL_API_VERSION@_gir_VERSION = @GSPELL_API_VERSION@
Gspell_@GSPELL_API_VERSION@_gir_CFLAGS = $(DEP_CFLAGS)
Gspell_@GSPELL_API_VERSION@_gir_SCANNERFLAGS = $(WARN_SCANNERFLAGS)
Gspell_@GSPELL_API_VERSION@_gir_LIBS = libgspell-@GSPELL_API_VERSION@.la
Gspell_@GSPELL_API_VERSION@_gir_FILES =	\
	$(gspell_public_headers)	\
	$(gspell_public_c_files)	\
	$(gspell_built_public_headers)	\
	$(gspell_built_public_c_files)

Gspell_@GSPELL_API_VERSION@_gir_INCLUDES = Gtk-3.0
Gspell_@GSPELL_API_VERSION@_gir_PACKAGES = gtk+-3.0
Gspell_@GSPELL_API_VERSION@_gir_EXPORT_PACKAGES = gspell-@GSPELL_API_VERSION@

girdir = $(datadir)/gir-1.0
gir_DATA = Gspell-@GSPELL_API_VERSION@.gir

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = Gspell-@GSPELL_API_VERSION@.typelib

CLEANFILES +=		\
	$(gir_DATA)	\
	$(typelib_DATA)

endif # HAVE_INTROSPECTION

if ENABLE_VAPIGEN
-include $(VAPIGEN_MAKEFILE)

gspell-@GSPELL_API_VERSION@.vapi: Gspell-@GSPELL_API_VERSION@.gir

VAPIGEN_VAPIS = gspell-@GSPELL_API_VERSION@.vapi

gspell_@GSPELL_API_VERSION@_vapi_DEPS = gtk+-3.0

gspell-@GSPELL_API_VERSION@.deps:
	$(AM_V_GEN) for pkg in $(gspell_@GSPELL_API_VERSION@_vapi_DEPS); do \
		echo $$pkg >> $@; \
	done

gspell_@GSPELL_API_VERSION@_vapi_METADATADIRS = $(srcdir)
gspell_@GSPELL_API_VERSION@_vapi_FILES = Gspell-@GSPELL_API_VERSION@.gir

vapidir = $(datadir)/vala/vapi
vapi_DATA =					\
	gspell-@GSPELL_API_VERSION@.vapi	\
	gspell-@GSPELL_API_VERSION@.deps

CLEANFILES += $(vapi_DATA)

endif # ENABLE_VAPIGEN

-include $(top_srcdir)/git.mk
