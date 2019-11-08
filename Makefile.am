SUBDIRS = po gspell gspell-app tests testsuite docs

include $(top_srcdir)/aminclude_static.am
clean-local: code-coverage-clean
distclean-local: code-coverage-dist-clean

AM_DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc --enable-introspection --enable-vala

pkgconfig_DATA = gspell-$(GSPELL_API_VERSION).pc
DISTCLEANFILES = $(pkgconfig_DATA)

EXTRA_DIST =		\
	gspell.doap	\
	gspell.pc.in	\
	HACKING

MAINTAINERCLEANFILES = \
	$(GITIGNORE_MAINTAINERCLEANFILES_TOPLEVEL) \
	$(GITIGNORE_MAINTAINERCLEANFILES_MAKEFILE_IN) \
	$(GITIGNORE_MAINTAINERCLEANFILES_M4_LIBTOOL) \
	aminclude_static.am \
	gtk-doc.make \
	INSTALL

-include $(top_srcdir)/git.mk
