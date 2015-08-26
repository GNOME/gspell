#include "gedit-spell-osx.h"
#include <gtkosxapplication.h>
#import <Cocoa/Cocoa.h>

gchar *
gedit_spell_osx_get_resource_path (void)
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
gedit_spell_osx_get_preferred_spell_language ()
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
