AM_CPPFLAGS = 			\
	-I$(top_srcdir) 	\
	$(WARN_CFLAGS) 		\
	$(DEP_CFLAGS)

AM_LDFLAGS = $(WARN_LDFLAGS)

LDADD = $(top_builddir)/gspell/libgspell-core.la	\
	$(DEP_LIBS)

noinst_PROGRAMS = $(UNIT_TEST_PROGS)
TESTS = $(UNIT_TEST_PROGS)
UNIT_TEST_PROGS =

UNIT_TEST_PROGS += test-checker
test_checker_SOURCES = test-checker.c

UNIT_TEST_PROGS += test-inline-checker-text-buffer
test_inline_checker_text_buffer_SOURCES = test-inline-checker-text-buffer.c

-include $(top_srcdir)/git.mk
