AM_CPPFLAGS = 			\
	-I$(top_srcdir) 	\
	$(WARN_CFLAGS) 		\
	$(DEP_CFLAGS)		\
	$(GTK_MAC_CFLAGS)

AM_LDFLAGS = $(WARN_LDFLAGS)

LDADD =	$(top_builddir)/gspell/libgspell-@GSPELL_API_VERSION@.la	\
	$(DEP_LIBS)							\
	$(GTK_MAC_LIBS)

TEST_PROGS =

TEST_PROGS += test-entry
test_entry_SOURCES = test-entry.c

TEST_PROGS += test-text-view
test_text_view_SOURCES = test-text-view.c

TEST_PROGS += test-text-view-basic
test_text_view_basic_SOURCES = test-text-view-basic.c

noinst_PROGRAMS = $(TEST_PROGS)

-include $(top_srcdir)/git.mk
