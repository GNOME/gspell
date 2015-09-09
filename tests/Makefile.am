AM_CPPFLAGS = 			\
	-I$(top_srcdir) 	\
	$(WARN_CFLAGS) 		\
	$(DEP_CFLAGS)		\
	$(GTK_MAC_CFLAGS)

AM_LDFLAGS = $(WARN_LDFLAGS)

noinst_PROGRAMS = $(TEST_PROGS)

TEST_PROGS = test-spell
test_spell_SOURCES = test-spell.c
test_spell_LDADD =				\
	$(top_builddir)/gspell/libgspell-3.0.la	\
	$(DEP_LIBS)				\
	$(GTK_MAC_LIBS)

-include $(top_srcdir)/git.mk
