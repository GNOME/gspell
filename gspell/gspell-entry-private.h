/* SPDX-FileCopyrightText: 2016 - Sébastien Wilmet
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include "gspell/gspell-entry.h"

G_BEGIN_DECLS

G_GNUC_INTERNAL
const GSList *	_gspell_entry_get_misspelled_words	(GspellEntry *gspell_entry);

G_END_DECLS
