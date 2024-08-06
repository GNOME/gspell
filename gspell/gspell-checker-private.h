/* SPDX-FileCopyrightText: 2016 - Sébastien Wilmet
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include "gspell-checker.h"

G_BEGIN_DECLS

G_GNUC_INTERNAL
void		_gspell_checker_force_set_language	(GspellChecker        *checker,
							 const GspellLanguage *language);

G_END_DECLS
