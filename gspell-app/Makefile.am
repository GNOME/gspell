bin_PROGRAMS = gspell-app

gspell_app_SOURCES =	\
	gspell-app.c

gspell_app_CPPFLAGS =				\
	-DG_LOG_DOMAIN=\"gspell-app\"		\
	-DGSPELL_DATADIR=\""$(datadir)"\"	\
	-I$(top_srcdir)				\
	-I$(top_builddir)

gspell_app_CFLAGS =	\
	$(WARN_CFLAGS) 	\
	$(DEP_CFLAGS)

gspell_app_LDFLAGS =	\
	$(WARN_LDFLAGS)

gspell_app_LDADD =							\
	$(top_builddir)/gspell/libgspell-@GSPELL_API_VERSION@.la	\
	$(DEP_LIBS)

-include $(top_srcdir)/git.mk
