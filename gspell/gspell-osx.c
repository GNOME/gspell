/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2011, 2014 - Jesse van den Kieboom
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gspell-osx.h"
#include <gtkosxapplication.h>
#import <Cocoa/Cocoa.h>

gchar *
_gspell_osx_get_resource_path (void)
{
	gchar *id;
	gchar *ret = NULL;

	id = gtkosx_application_get_bundle_id ();

	if (id != NULL)
	{
		ret = gtkosx_application_get_resource_path ();
	}

	g_free (id);
	return ret;
}

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

/* ex:set ts=8 noet: */
