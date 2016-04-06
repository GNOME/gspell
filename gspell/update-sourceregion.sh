#!/bin/sh

# Fetch the GtkSourceRegion utility files from GtkSourceView
# but rename the symbols to avoid possible symbol clashes
# if both GtkSourceView and gspell are used in the same application
# with different versons of GtkSourceRegion.
# G_GNUC_INTERNAL should protect us, but it could be a no-op with
# some compilers.

GSVURL=https://git.gnome.org/browse/gtksourceview/plain/gtksourceview

update_file () {
    _source="${GSVURL}/$1"
    _dest="$2"

    echo "/* Do not edit: this file is generated from ${_source} */" > "${_dest}"
    echo >> "${_dest}"

    # gtksourceversion.h is only needed for GTK_SOURCE_AVAILABLE* that we are
    # removing. We may need to define our own INTERNAL macro for MSVC support.
    curl "${_source}" | sed \
        -e '/gtksourceversion.h/d' \
        -e 's/GTK_SOURCE_AVAILABLE_IN_3_22/G_GNUC_INTERNAL/g' \
        -e 's/gtksourceregion/gspellregion/g' \
	-e 's#/\*\*#/*#g' \
        -e 's/GTK_SOURCE/GSPELL/g' \
        -e 's/GtkSourceRegion/GspellRegion/g' \
        -e 's/gtk_source_region/_gspell_region/g' >> "${_dest}"
}

update_file "gtksourceregion.c" "gspellregion.c"
update_file "gtksourceregion.h" "gspellregion.h"
