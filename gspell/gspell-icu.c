/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "gspell-icu.h"
#include <unicode/ustring.h>
#include <unicode/uloc.h>

/* Implementation notes:
 *
 * Before using the ICU library, the iso-codes package was used instead. It was
 * fine on Linux since the iso-codes package is usually already installed. On
 * MS Windows, to package an application the iso-codes package needed to be
 * bundled with the app, which is not great because it's a quite sizeable dep.
 *
 * The ICU library is usually already installed too on Linux, and for Windows
 * it's already an (indirect) dep of gedit (the Tepl library uses the ICU).
 *
 * MS Windows also provides a native API for something similar to
 * _gspell_icu_get_language_name_from_code(). But it's easier to manage only one
 * implementation (namely, based on the ICU library).
 */

/* Some of these functions are copied from the Tepl library. */

/* Wrapper around u_strToUTF8() that handles the pre-flighting.
 *
 * Returns: (transfer full) (nullable): the newly-allocated string with the
 * right size. Free with g_free() when no longer needed.
 */
static char *
_gspell_icu_strToUTF8 (int32_t     *pDestLength,
		       const UChar *src,
		       int32_t      srcLength,
		       UErrorCode  *pErrorCode)
{
	int32_t my_DestLength = 0;
	UErrorCode my_ErrorCode = U_ZERO_ERROR;
	char *dest = NULL;

	u_strToUTF8 (NULL, 0, &my_DestLength,
		     src, srcLength,
		     &my_ErrorCode);

	if (my_ErrorCode != U_BUFFER_OVERFLOW_ERROR &&
	    my_ErrorCode != U_STRING_NOT_TERMINATED_WARNING)
	{
		if (pDestLength != NULL)
		{
			*pDestLength = my_DestLength;
		}
		if (pErrorCode != NULL)
		{
			*pErrorCode = my_ErrorCode;
		}

		return NULL;
	}

	dest = g_malloc0 (my_DestLength + 1);

	u_strToUTF8 (dest, my_DestLength + 1, pDestLength,
		     src, srcLength,
		     pErrorCode);

	return dest;
}

/* Returns: (transfer full) (nullable): a nul-terminated UTF-8 string. Free with
 * g_free() when no longer needed.
 */
static char *
_gspell_icu_strToUTF8Simple (const UChar *uchars)
{
	char *utf8_str;
	UErrorCode error_code = U_ZERO_ERROR;

	utf8_str = _gspell_icu_strToUTF8 (NULL, uchars, -1, &error_code);

	if (U_FAILURE (error_code))
	{
		g_free (utf8_str);
		return NULL;
	}

	return utf8_str;
}

/* Wrapper around uloc_getDisplayName() that handles the pre-flighting.
 *
 * Returns: (transfer full) (nullable): the result as a newly-allocated buffer
 * with the right size. Free with g_free() when no longer needed.
 */
static UChar *
_gspell_icu_loc_getDisplayName (const char *localeID,
				const char *inLocaleID,
				UErrorCode *err)
{
	UChar *result = NULL;
	int32_t result_size;
	UErrorCode my_err = U_ZERO_ERROR;

	result_size = uloc_getDisplayName (localeID,
					   inLocaleID,
					   NULL, 0,
					   &my_err);

	if (my_err != U_BUFFER_OVERFLOW_ERROR &&
	    my_err != U_STRING_NOT_TERMINATED_WARNING)
	{
		if (err != NULL)
		{
			*err = my_err;
		}

		return NULL;
	}

	result = g_new0 (UChar, result_size + 1);

	uloc_getDisplayName (localeID,
			     inLocaleID,
			     result, result_size + 1,
			     err);

	return result;
}

/* Returns: (transfer full) (nullable): the result as a UTF-8 string. Free with
 * g_free() when no longer needed.
 */
char *
_gspell_icu_loc_getDisplayNameSimple (const char *localeID,
				      const char *inLocaleID)
{
	UChar *result;
	char *utf8_result;
	UErrorCode err = U_ZERO_ERROR;

	result = _gspell_icu_loc_getDisplayName (localeID, inLocaleID, &err);

	if (U_FAILURE (err))
	{
		g_free (result);
		return NULL;
	}

	utf8_result = _gspell_icu_strToUTF8Simple (result);
	g_free (result);
	return utf8_result;
}

/* Wrapper around uloc_canonicalize() that handles the pre-flighting.
 *
 * Returns: (transfer full) (nullable): the result as a newly-allocated buffer
 * with the right size. Free with g_free() when no longer needed.
 */
static char *
_gspell_icu_loc_canonicalize (const char *localeID,
			      UErrorCode *err)
{
	char *result = NULL;
	int32_t result_size;
	UErrorCode my_err = U_ZERO_ERROR;

	result_size = uloc_canonicalize (localeID, NULL, 0, &my_err);

	if (my_err != U_BUFFER_OVERFLOW_ERROR &&
	    my_err != U_STRING_NOT_TERMINATED_WARNING)
	{
		if (err != NULL)
		{
			*err = my_err;
		}

		return NULL;
	}

	result = g_new0 (char, result_size + 1);
	uloc_canonicalize (localeID, result, result_size + 1, err);
	return result;
}

/* Returns: (transfer full) (nullable): the result, or %NULL in case of error.
 * Free with g_free() when no longer needed.
 */
static char *
_gspell_icu_loc_canonicalizeSimple (const char *localeID)
{
	char *result;
	UErrorCode err = U_ZERO_ERROR;

	result = _gspell_icu_loc_canonicalize (localeID, &err);

	if (U_FAILURE (err))
	{
		g_free (result);
		return NULL;
	}

	return result;
}

/* The name of this function uses gspell's terminology:
 * "Language code": as in gspell_language_get_code().
 * "Language name": as in gspell_language_get_name(), except that the
 * translation is controlled by the @inLocaleID parameter which has the same
 * meaning as for _gspell_icu_loc_getDisplayNameSimple(). If @inLocaleID is
 * %NULL then the default locale is used to translate the name. The @inLocaleID
 * parameter is kept for this function for unit tests.
 *
 * Returns: (transfer full) (nullable): the language name, or %NULL in case of
 * error. Free with g_free() when no longer needed.
 */
char *
_gspell_icu_get_language_name_from_code (const char *language_code,
					 const char *inLocaleID)
{
	char *canonicalized_language_code;
	char *language_name;

	/* language_code can come from an outside/foreign source, so it's better
	 * to pass it through level 2 canonicalization.
	 */
	canonicalized_language_code = _gspell_icu_loc_canonicalizeSimple (language_code);
	if (canonicalized_language_code == NULL)
	{
		return NULL;
	}

	language_name = _gspell_icu_loc_getDisplayNameSimple (canonicalized_language_code, inLocaleID);
	g_free (canonicalized_language_code);
	return language_name;
}
