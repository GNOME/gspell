#!/bin/sh

# Fetch the gtksourceregion utility files from gtksourceview
# but rename the symbols to avoid possible symbol clashes
# if both gtksourceview and gspell are used in the same application
# with different versons of textregion.
# G_GNUC_INTERNAL should protect us, but it could be a no-op with
# some compilers. Besides in theory gtktextregion could become
# public API one day.

GSVURL=https://git.gnome.org/browse/gtksourceview/plain/gtksourceview

update_file () {
    _source="${GSVURL}/$1"
    _dest="$2"

    echo "/* Do not edit: this file is generated from ${_source} */" > "${_dest}"
    echo >> "${_dest}"

    # gtksourcetypes-private.h is only needed for INTERNAL that we are removing.
    # We may need to define our own INTERNAL macro for MSVC support.
    curl "${_source}" | sed \
        -e '/gtksourcetypes-private.h/d' \
        -e 's/GTK_SOURCE_INTERNAL/G_GNUC_INTERNAL/g' \
        -e 's/gtktextregion/gspell-text-region/g' \
        -e 's/gtk_text_region/_gspell_text_region/g' \
        -e 's/GtkTextRegion/GspellTextRegion/g' \
        -e 's/GTK_TEXT_REGION/GSPELL_TEXT_REGION/g' >> "${_dest}"
}

update_file "gtktextregion.c" "gspell-text-region.c"
update_file "gtktextregion.h" "gspell-text-region.h"
