AM_CPPFLAGS = 			\
	-I$(top_srcdir) 	\
	$(WARN_CFLAGS) 		\
	$(DEP_CFLAGS)

AM_LDFLAGS = $(WARN_LDFLAGS)

LDADD = $(top_builddir)/gspell/libgspell-1.la	\
	$(DEP_LIBS)

noinst_PROGRAMS = $(UNIT_TEST_PROGS)
TESTS = $(UNIT_TEST_PROGS)
UNIT_TEST_PROGS =

UNIT_TEST_PROGS += test-checker
test_checker_SOURCES = test-checker.c

-include $(top_srcdir)/git.mk
