bin_PROGRAMS = gspell-app@GSPELL_API_VERSION@

gspell_app@GSPELL_API_VERSION@_SOURCES =	\
	gspell-app.c

gspell_app@GSPELL_API_VERSION@_CPPFLAGS =	\
	-DG_LOG_DOMAIN=\"gspell-app\"		\
	-DGSPELL_DATADIR=\""$(datadir)"\"	\
	-I$(top_srcdir)				\
	-I$(top_builddir)

gspell_app@GSPELL_API_VERSION@_CFLAGS =	\
	$(WARN_CFLAGS) 	\
	$(DEP_CFLAGS)

gspell_app@GSPELL_API_VERSION@_LDFLAGS = \
	$(WARN_LDFLAGS)

gspell_app@GSPELL_API_VERSION@_LDADD =					\
	$(top_builddir)/gspell/libgspell-@GSPELL_API_VERSION@.la	\
	$(DEP_LIBS)

-include $(top_srcdir)/git.mk
