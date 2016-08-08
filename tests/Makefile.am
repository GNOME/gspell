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

TEST_PROGS += test-spell
test_spell_SOURCES = test-spell.c

TEST_PROGS += test-spell-basic
test_spell_basic_SOURCES = test-spell-basic.c

noinst_PROGRAMS = $(TEST_PROGS)

-include $(top_srcdir)/git.mk
