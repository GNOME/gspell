/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2016 - Sébastien Wilmet
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef GSPELL_ENTRY_PRIVATE_H
#define GSPELL_ENTRY_PRIVATE_H

#include "gspell/gspell-entry.h"

G_BEGIN_DECLS

G_GNUC_INTERNAL
const GSList *	_gspell_entry_get_misspelled_words	(GspellEntry *gspell_entry);

G_END_DECLS

#endif /* GSPELL_ENTRY_PRIVATE_H */
