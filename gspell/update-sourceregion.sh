#!/bin/sh

# Copy the GtkSourceRegion utility files from GtkSourceView but rename the
# symbols to avoid possible symbol clashes if both GtkSourceView and gspell are
# used in the same application with different versons of GtkSourceRegion.
# G_GNUC_INTERNAL should protect us, but it could be a no-op with some
# compilers.

update_file () {
	_source="$1"
	_dest="$2"

	echo "/* Do not edit: this file is generated from ${_source} */" > "${_dest}"
	echo >> "${_dest}"

	# The "Only <gtksourceview/gtksource.h> can be included directly" error
	# is removed manually.

	cat "${_source}" | sed \
		-e 's/G_MODULE_EXPORT/G_GNUC_INTERNAL/g' \
		-e 's/gtksourceregion/gspellregion/g' \
		-e 's#/\*\*#/*#g' \
		-e 's/GTK_SOURCE/GSPELL/g' \
		-e 's/GtkSourceRegion/GspellRegion/g' \
		-e 's/gtk_source_region/_gspell_region/g' >> "${_dest}"
}

update_file 'gtksourceregion.c' 'gspellregion.c'
update_file 'gtksourceregion.h' 'gspellregion.h'
