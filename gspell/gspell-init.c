/* SPDX-FileCopyrightText: 2015, 2016, 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* Init i18n */

/* Part of the code taken from the GtkSourceView library (gtksourceview-i18n.c).
 * SPDX-FileCopyrightText: (C) 1997, 1998, 1999, 2000 Free Software Foundation
 */

#include "gspell-config.h"
#include <glib/gi18n-lib.h>
#include "gconstructor.h"

#ifdef G_OS_WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
static HMODULE gspell_dll;
#endif

#if OS_MACOS
#include <Cocoa/Cocoa.h>

static gchar *
dirs_os_x_get_bundle_resource_dir (void)
{
	NSAutoreleasePool *pool;
	gchar *str = NULL;
	NSString *path;

	pool = [[NSAutoreleasePool alloc] init];

	if ([[NSBundle mainBundle] bundleIdentifier] == nil)
	{
		[pool release];
		return NULL;
	}

	path = [[NSBundle mainBundle] resourcePath];

	if (!path)
	{
		[pool release];
		return NULL;
	}

	str = g_strdup ([path UTF8String]);
	[pool release];
	return str;
}

static gchar *
dirs_os_x_get_locale_dir (void)
{
	gchar *res_dir;
	gchar *ret;

	res_dir = dirs_os_x_get_bundle_resource_dir ();

	if (res_dir == NULL)
	{
		ret = g_strdup (GSPELL_LOCALEDIR);
	}
	else
	{
		ret = g_build_filename (res_dir, "share", "locale", NULL);
		g_free (res_dir);
	}

	return ret;
}
#endif /* OS_MACOS */

static gchar *
get_locale_dir (void)
{
	gchar *locale_dir;

#ifdef G_OS_WIN32
	gchar *win32_dir;

	win32_dir = g_win32_get_package_installation_directory_of_module (gspell_dll);

	locale_dir = g_build_filename (win32_dir, "share", "locale", NULL);

	g_free (win32_dir);
#elif OS_MACOS
	locale_dir = dirs_os_x_get_locale_dir ();
#else
	locale_dir = g_strdup (GSPELL_LOCALEDIR);
#endif

	return locale_dir;
}

static void
gspell_init (void)
{
	gchar *locale_dir;

	locale_dir = get_locale_dir ();
	bindtextdomain (GETTEXT_PACKAGE, locale_dir);
	g_free (locale_dir);

	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
}

#if defined (G_OS_WIN32)

BOOL WINAPI DllMain (HINSTANCE hinstDLL,
		     DWORD     fdwReason,
		     LPVOID    lpvReserved);

BOOL WINAPI
DllMain (HINSTANCE hinstDLL,
	 DWORD     fdwReason,
	 LPVOID    lpvReserved)
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			gspell_dll = hinstDLL;
			gspell_init ();
			break;

		case DLL_THREAD_DETACH:
		default:
			/* do nothing */
			break;
	}

	return TRUE;
}

#elif defined (G_HAS_CONSTRUCTORS)

#  ifdef G_DEFINE_CONSTRUCTOR_NEEDS_PRAGMA
#    pragma G_DEFINE_CONSTRUCTOR_PRAGMA_ARGS(gspell_constructor)
#  endif
G_DEFINE_CONSTRUCTOR (gspell_constructor)

static void
gspell_constructor (void)
{
	gspell_init ();
}

#else
#  error Your platform/compiler is missing constructor support
#endif
