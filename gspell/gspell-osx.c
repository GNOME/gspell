/* SPDX-FileCopyrightText: 2011, 2014 - Jesse van den Kieboom
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "gspell-config.h"
#include "gspell-osx.h"
#import <Cocoa/Cocoa.h>

gchar *
_gspell_osx_get_preferred_spell_language ()
{
	gchar *ret = NULL;
	NSAutoreleasePool *pool;

	pool = [[NSAutoreleasePool alloc] init];

#if defined(MAC_OS_X_VERSION_10_5) && MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_5
	NSArray *langs;

	langs = [[NSSpellChecker sharedSpellChecker] userPreferredLanguages];

	if ([langs count] > 0)
	{
		ret = g_strdup ([[langs objectAtIndex:0] UTF8String]);
	}
#endif

	[pool release];
	return ret;
}
