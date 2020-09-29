include $(top_srcdir)/aminclude_static.am
clean-local: code-coverage-clean
distclean-local: code-coverage-dist-clean

@VALGRIND_CHECK_RULES@

# https://github.com/dtrebbien/GNOME.supp
# https://github.com/pwithnall/walbottle
VALGRIND_SUPPRESSIONS_FILES =						\
	$(top_srcdir)/../walbottle/libwalbottle/tests/walbottle.supp	\
	$(top_srcdir)/../GNOME.supp/build/base.supp			\
	$(top_srcdir)/../GNOME.supp/build/pango.supp			\
	$(top_srcdir)/../GNOME.supp/build/gdk.supp			\
	$(top_srcdir)/../GNOME.supp/build/gtk3.supp

AM_CPPFLAGS = 			\
	-I$(top_srcdir) 	\
	$(WARN_CFLAGS) 		\
	$(DEP_CFLAGS)

AM_LDFLAGS = $(WARN_LDFLAGS)

LDADD = $(top_builddir)/gspell/libgspell-core.la	\
	$(DEP_LIBS)

UNIT_TEST_PROGS =

UNIT_TEST_PROGS += test-checker
test_checker_SOURCES = test-checker.c

UNIT_TEST_PROGS += test-entry
test_entry_SOURCES = test-entry.c

UNIT_TEST_PROGS += test-icu
test_icu_SOURCES = test-icu.c

UNIT_TEST_PROGS += test-inline-checker-text-buffer
test_inline_checker_text_buffer_SOURCES = test-inline-checker-text-buffer.c

UNIT_TEST_PROGS += test-text-iter
test_text_iter_SOURCES = test-text-iter.c

UNIT_TEST_PROGS += test-utils
test_utils_SOURCES = test-utils.c

noinst_PROGRAMS = $(UNIT_TEST_PROGS)
TESTS = $(UNIT_TEST_PROGS)

if INSTALLED_TESTS
insttestdir = $(libexecdir)/installed-tests/$(PACKAGE)-@GSPELL_API_VERSION@
insttest_PROGRAMS = $(UNIT_TEST_PROGS)

%.test: %$(EXEEXT) Makefile
	$(AM_V_GEN) (echo '[Test]' > $@.tmp; \
	echo 'Description=gspell test' >> $@.tmp; \
	echo 'Exec=$(insttestdir)/$<' >> $@.tmp; \
	echo 'Type=session' >> $@.tmp; \
	echo 'Output=TAP' >> $@.tmp; \
	mv $@.tmp $@)

test_files = $(UNIT_TEST_PROGS:=.test)

CLEANFILES = $(test_files)

testmetadir = $(datadir)/installed-tests/$(PACKAGE)-@GSPELL_API_VERSION@
testmeta_DATA = $(test_files)

endif # INSTALLED_TESTS

-include $(top_srcdir)/git.mk
