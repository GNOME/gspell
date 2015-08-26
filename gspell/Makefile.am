SUBDIRS = resources

AM_CPPFLAGS =			\
	-I$(top_srcdir)		\
	$(WARN_CFLAGS)		\
	$(DEP_CFLAGS)

AM_LDFLAGS = $(WARN_LDFLAGS)

BUILT_SOURCES =			\
	gspell-resources.c

LTLIBRARIES = libgspell.la

libgspell_la_SOURCES =			\
	gtktextregion.c			\
	gtktextregion.h

# do not distribute generated files
nodist_libgspell_la_SOURCES = $(BUILT_SOURCES)

libgspell_la_LIBADD = $(DEP_LIBS)

xml_res = $(srcdir)/resources/gspell.gresource.xml
gspell-resources.c: $(xml_res) $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir)/resources --generate-dependencies $(xml_res))
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(srcdir)/resources --generate-source $(xml_res)

CLEANFILES = $(BUILT_SOURCES)

-include $(top_srcdir)/git.mk
