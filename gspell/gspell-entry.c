/* SPDX-FileCopyrightText: 2016, 2017 - Sébastien Wilmet
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "gspell-config.h"
#include "gspell-entry.h"
#include "gspell-entry-private.h"
#include "gspell-entry-buffer.h"
#include "gspell-entry-utils.h"
#include "gspell-context-menu.h"
#include "gspell-current-word-policy.h"
#include "gspell-utils.h"

/**
 * SECTION:entry
 * @Title: GspellEntry
 * @Short_description: Spell checking support for GtkEntry
 *
 * #GspellEntry extends the #GtkEntry class with inline spell checking.
 * Misspelled words are highlighted with a red %PANGO_UNDERLINE_SINGLE.
 * Right-clicking a misspelled word pops up a context menu of suggested
 * replacements. The context menu also contains an “Ignore All” item to add the
 * misspelled word to the session dictionary. And an “Add” item to add the word
 * to the personal dictionary.
 *
 * For a basic use-case, there is the gspell_entry_basic_setup() convenience
 * function.
 *
 * If you don't use the gspell_entry_basic_setup() function, you need to call
 * gspell_entry_buffer_set_spell_checker() to associate a #GspellChecker to the
 * #GtkEntryBuffer.
 *
 * Note that #GspellEntry extends the #GtkEntry class but without subclassing
 * it, because #GtkEntry is already subclassed by #GtkSearchEntry for example.
 *
 * %PANGO_UNDERLINE_SINGLE is used for consistency with #GspellTextView.
 * If you want a %PANGO_UNDERLINE_ERROR instead (a wavy underline), please fix
 * [this bug](https://bugzilla.gnome.org/show_bug.cgi?id=763741) first.
 */

struct _GspellEntry
{
	GObject parent;

	GtkEntry *entry;
	GtkEntryBuffer *buffer;
	GspellChecker *checker;

	GspellCurrentWordPolicy *current_word_policy;

	/* List elements: GspellEntryWord*.
	 * Used for unit tests.
	 */
	GSList *misspelled_words;

	/* The position is in characters, not in bytes. */
	gint popup_char_position;

	gulong notify_attributes_handler_id;
	guint notify_attributes_idle_id;

	guint inline_spell_checking : 1;
};

enum
{
	PROP_0,
	PROP_ENTRY,
	PROP_INLINE_SPELL_CHECKING,
};

#define GSPELL_ENTRY_KEY "gspell-entry-key"

G_DEFINE_TYPE (GspellEntry, gspell_entry, G_TYPE_OBJECT)

/* This function should be called instead of accessing the inline_spell_checking
 * attribute.
 */
static gboolean
inline_spell_checking_is_enabled (GspellEntry *gspell_entry)
{
	/* The GtkEntry:input-purpose and/or GtkEntry:input-hints could be taken
	 * into account here, but it is not the case. There is already the
	 * GspellEntry:inline-spell-checking property, which needs to be FALSE
	 * by default. If it was TRUE by default, an application would just need
	 * to call gspell_entry_get_from_gtk_entry(), but it would be strange to
	 * do nothing with the returned GspellEntry. So inline-spell-checking is
	 * FALSE by default and the application anyway needs to set it to TRUE
	 * manually to enable the *inline* spell checking (a GtkEntry could have
	 * other types of spell checking, for example based on GspellNavigator
	 * to check an entire form or check a list of forms, even though such
	 * feature is probably rare).
	 *
	 * In other words, it might be desirable to set
	 * GTK_INPUT_HINT_SPELLCHECK but keeping the inline spell checking of
	 * GspellEntry disabled. But when the inline spell checker of
	 * GspellEntry is enabled, it is normally always desirable to set
	 * GTK_INPUT_HINT_SPELLCHECK, which can be seen as duplicated state, but
	 * it is not, because if the GspellEntry:inline-spell-checking property
	 * is removed, another boolean property would be needed to tell
	 * GspellEntry whether it needs to bind the input-hints settings to its
	 * inline spell checker.
	 *
	 * Anyway, the mere fact of calling gspell_entry_get_from_gtk_entry()
	 * should not have unexpected side effects.
	 */

	return (gspell_entry->inline_spell_checking &&
		gtk_entry_get_visibility (gspell_entry->entry));
}

static void
set_attributes (GspellEntry   *gspell_entry,
		PangoAttrList *attributes)
{
	g_signal_handler_block (gspell_entry->entry,
				gspell_entry->notify_attributes_handler_id);

	gtk_entry_set_attributes (gspell_entry->entry, attributes);

	g_signal_handler_unblock (gspell_entry->entry,
				  gspell_entry->notify_attributes_handler_id);
}

static void
update_attributes (GspellEntry *gspell_entry)
{
	PangoAttrList *attr_list;

	/* If attributes have been added or removed from an existing
	 * PangoAttrList, GtkEntry doesn't know that the :attributes property
	 * has been modified. Without this code, GtkEntry can become buggy,
	 * especially with multi-byte characters (displaying them as unknown
	 * char boxes).
	 */
	attr_list = gtk_entry_get_attributes (gspell_entry->entry);
	set_attributes (gspell_entry, attr_list);
}

static gboolean
remove_underlines_filter (PangoAttribute *attr,
			  gpointer        user_data)
{
	return (attr->klass->type == PANGO_ATTR_UNDERLINE ||
		attr->klass->type == PANGO_ATTR_UNDERLINE_COLOR);
}

static void
remove_all_underlines (GspellEntry *gspell_entry)
{
	PangoAttrList *attr_list;

	attr_list = gtk_entry_get_attributes (gspell_entry->entry);

	if (attr_list == NULL)
	{
		return;
	}

	pango_attr_list_filter (attr_list,
				remove_underlines_filter,
				NULL);

	update_attributes (gspell_entry);
}

static void
insert_underline (GspellEntry *gspell_entry,
		  guint        byte_start,
		  guint        byte_end)
{
	PangoAttribute *attr_underline;
	PangoAttribute *attr_underline_color;
	PangoAttrList *attr_list;

	attr_underline = pango_attr_underline_new (PANGO_UNDERLINE_SINGLE);
	attr_underline->start_index = byte_start;
	attr_underline->end_index = byte_end;

	attr_underline_color = _gspell_utils_create_pango_attr_underline_color ();
	attr_underline_color->start_index = byte_start;
	attr_underline_color->end_index = byte_end;

	attr_list = gtk_entry_get_attributes (gspell_entry->entry);

	if (attr_list == NULL)
	{
		attr_list = pango_attr_list_new ();
		set_attributes (gspell_entry, attr_list);
		pango_attr_list_unref (attr_list);
	}

	/* Do not use pango_attr_list_change(), because all previous underlines
	 * are anyway removed by remove_all_underlines().
	 */
	pango_attr_list_insert (attr_list, attr_underline);
	pango_attr_list_insert (attr_list, attr_underline_color);
}

static void
update_misspelled_words_list (GspellEntry *gspell_entry)
{
	GSList *all_words;

	g_slist_free_full (gspell_entry->misspelled_words, _gspell_entry_word_free);
	gspell_entry->misspelled_words = NULL;

	if (!inline_spell_checking_is_enabled (gspell_entry))
	{
		return;
	}

	if (gspell_entry->checker == NULL ||
	    gspell_checker_get_language (gspell_entry->checker) == NULL)
	{
		return;
	}

	all_words = _gspell_entry_utils_get_words (gspell_entry->entry);

	while (all_words != NULL)
	{
		GspellEntryWord *cur_word = all_words->data;
		gboolean correctly_spelled;
		GError *error = NULL;

		correctly_spelled = gspell_checker_check_word (gspell_entry->checker,
							       cur_word->word_str, -1,
							       &error);

		if (error != NULL)
		{
			g_warning ("Inline spell checker: %s", error->message);
			g_clear_error (&error);
			g_slist_free_full (all_words, _gspell_entry_word_free);
			all_words = NULL;
			break;
		}

		if (correctly_spelled)
		{
			_gspell_entry_word_free (cur_word);
		}
		else
		{
			gspell_entry->misspelled_words = g_slist_prepend (gspell_entry->misspelled_words,
									  cur_word);
		}

		all_words = g_slist_delete_link (all_words, all_words);
	}

	g_assert (all_words == NULL);

	gspell_entry->misspelled_words = g_slist_reverse (gspell_entry->misspelled_words);
}

static gboolean
is_current_word (GspellEntry     *gspell_entry,
		 GspellEntryWord *word)
{
	gint cursor_pos;

	cursor_pos = gtk_editable_get_position (GTK_EDITABLE (gspell_entry->entry));

	return (word->char_start <= cursor_pos && cursor_pos <= word->char_end);
}

/* If another feature wants to insert underlines in another color (e.g. for
 * grammar checking), this won't work well. A previous implementation used the
 * GtkEditable::changed signal: removing all underlines in the second emission
 * stage, and inserting new underlines in the fourth emission stage. That way
 * another feature could connect to the ::changed signal and insert other
 * underlines. But it broke the semantics of the ::changed signal, since it was
 * emitted a lot of times without changes in the content.
 *
 * So, if one day someone wants to implement another feature that inserts other
 * underlines, a new GtkEntry API would be needed to have a clean solution,
 * instead of stepping on other's feet. For example GtkTextView has a
 * higher-level API to insert tags, set priorities on them, etc.
 */
static void
recheck_all (GspellEntry *gspell_entry)
{
	GSList *l;

	remove_all_underlines (gspell_entry);

	update_misspelled_words_list (gspell_entry);

	for (l = gspell_entry->misspelled_words; l != NULL; l = l->next)
	{
		GspellEntryWord *cur_word = l->data;

		if (!_gspell_current_word_policy_get_check_current_word (gspell_entry->current_word_policy) &&
		    is_current_word (gspell_entry, cur_word))
		{
			continue;
		}

		insert_underline (gspell_entry,
				  cur_word->byte_start,
				  cur_word->byte_end);
	}

	update_attributes (gspell_entry);
}

static void
changed_after_cb (GtkEditable *editable,
		  GspellEntry *gspell_entry)
{
	recheck_all (gspell_entry);
}

static gboolean
notify_attributes_idle_cb (gpointer user_data)
{
	GspellEntry *gspell_entry = GSPELL_ENTRY (user_data);

	/* Re-apply our attributes. Do it in an idle function, to not be inside
	 * a notify::attributes signal emission. If we call recheck_all() during
	 * the signal emission, there is an infinite loop.
	 */
	recheck_all (gspell_entry);

	gspell_entry->notify_attributes_idle_id = 0;
	return G_SOURCE_REMOVE;
}

static void
notify_attributes_cb (GtkEntry    *gtk_entry,
		      GParamSpec  *pspec,
		      GspellEntry *gspell_entry)
{
	if (gspell_entry->notify_attributes_idle_id == 0)
	{
		gspell_entry->notify_attributes_idle_id =
			g_idle_add_full (G_PRIORITY_HIGH_IDLE,
					 notify_attributes_idle_cb,
					 gspell_entry,
					 NULL);
	}
}

static void
language_notify_cb (GspellChecker *checker,
		    GParamSpec    *pspec,
		    GspellEntry   *gspell_entry)
{
	_gspell_current_word_policy_language_changed (gspell_entry->current_word_policy);
	recheck_all (gspell_entry);
}

static void
session_cleared_cb (GspellChecker *checker,
		    GspellEntry   *gspell_entry)
{
	_gspell_current_word_policy_session_cleared (gspell_entry->current_word_policy);
	recheck_all (gspell_entry);
}

static void
set_checker (GspellEntry   *gspell_entry,
	     GspellChecker *checker)
{
	if (gspell_entry->checker == checker)
	{
		return;
	}

	if (gspell_entry->checker != NULL)
	{
		g_signal_handlers_disconnect_by_func (gspell_entry->checker,
						      language_notify_cb,
						      gspell_entry);

		g_signal_handlers_disconnect_by_func (gspell_entry->checker,
						      session_cleared_cb,
						      gspell_entry);

		g_signal_handlers_disconnect_by_func (gspell_entry->checker,
						      recheck_all,
						      gspell_entry);

		g_object_unref (gspell_entry->checker);
	}

	gspell_entry->checker = checker;

	if (gspell_entry->checker != NULL)
	{
		g_signal_connect (gspell_entry->checker,
				  "notify::language",
				  G_CALLBACK (language_notify_cb),
				  gspell_entry);

		g_signal_connect (gspell_entry->checker,
				  "session-cleared",
				  G_CALLBACK (session_cleared_cb),
				  gspell_entry);

		g_signal_connect_swapped (gspell_entry->checker,
					  "word-added-to-personal",
					  G_CALLBACK (recheck_all),
					  gspell_entry);

		g_signal_connect_swapped (gspell_entry->checker,
					  "word-added-to-session",
					  G_CALLBACK (recheck_all),
					  gspell_entry);

		g_object_ref (gspell_entry->checker);
	}
}

static void
update_checker (GspellEntry *gspell_entry)
{
	GspellChecker *checker = NULL;

	if (gspell_entry->buffer != NULL)
	{
		GspellEntryBuffer *gspell_buffer;

		gspell_buffer = gspell_entry_buffer_get_from_gtk_entry_buffer (gspell_entry->buffer);
		checker = gspell_entry_buffer_get_spell_checker (gspell_buffer);
	}

	set_checker (gspell_entry, checker);
}

static void
notify_spell_checker_cb (GspellEntryBuffer *gspell_buffer,
			 GParamSpec        *pspec,
			 GspellEntry       *gspell_entry)
{
	update_checker (gspell_entry);

	_gspell_current_word_policy_checker_changed (gspell_entry->current_word_policy);
	recheck_all (gspell_entry);
}

static void
inserted_text_cb (GtkEntryBuffer *buffer,
		  guint           position,
		  gchar          *chars,
		  guint           n_chars,
		  GspellEntry    *gspell_entry)
{
	if (n_chars > 1)
	{
		_gspell_current_word_policy_several_chars_inserted (gspell_entry->current_word_policy);
	}
	else
	{
		gunichar ch;
		gboolean empty_selection;
		gint cursor_pos;
		gboolean at_cursor_pos;

		ch = g_utf8_get_char (chars);

		empty_selection = !gtk_editable_get_selection_bounds (GTK_EDITABLE (gspell_entry->entry),
								      NULL,
								      NULL);

		cursor_pos = gtk_editable_get_position (GTK_EDITABLE (gspell_entry->entry));
		at_cursor_pos = cursor_pos == (gint)position;

		_gspell_current_word_policy_single_char_inserted (gspell_entry->current_word_policy,
								  ch,
								  empty_selection,
								  at_cursor_pos);
	}
}

static void
set_buffer (GspellEntry    *gspell_entry,
	    GtkEntryBuffer *gtk_buffer)
{
	GspellEntryBuffer *gspell_buffer;

	if (gspell_entry->buffer == gtk_buffer)
	{
		return;
	}

	if (gspell_entry->buffer != NULL)
	{
		gspell_buffer = gspell_entry_buffer_get_from_gtk_entry_buffer (gspell_entry->buffer);

		g_signal_handlers_disconnect_by_func (gspell_buffer,
						      notify_spell_checker_cb,
						      gspell_entry);

		g_signal_handlers_disconnect_by_func (gspell_entry->buffer,
						      inserted_text_cb,
						      gspell_entry);

		g_object_unref (gspell_entry->buffer);
	}

	gspell_entry->buffer = gtk_buffer;

	if (gspell_entry->buffer != NULL)
	{
		gspell_buffer = gspell_entry_buffer_get_from_gtk_entry_buffer (gspell_entry->buffer);

		g_signal_connect (gspell_buffer,
				  "notify::spell-checker",
				  G_CALLBACK (notify_spell_checker_cb),
				  gspell_entry);

		g_signal_connect (gspell_entry->buffer,
				  "inserted-text",
				  G_CALLBACK (inserted_text_cb),
				  gspell_entry);

		g_object_ref (gspell_entry->buffer);
	}

	update_checker (gspell_entry);
}

static void
update_buffer (GspellEntry *gspell_entry)
{
	set_buffer (gspell_entry, gtk_entry_get_buffer (gspell_entry->entry));
}

static void
notify_buffer_cb (GtkEntry    *gtk_entry,
		  GParamSpec  *pspec,
		  GspellEntry *gspell_entry)
{
	update_buffer (gspell_entry);
	recheck_all (gspell_entry);
}

/* Free the return value with _gspell_entry_word_free(). */
static GspellEntryWord *
get_entry_word_at_popup_position (GspellEntry *gspell_entry)
{
	gint pos;
	GSList *words;
	GSList *l;
	GspellEntryWord *entry_word = NULL;

	pos = gspell_entry->popup_char_position;

	words = _gspell_entry_utils_get_words (gspell_entry->entry);

	for (l = words; l != NULL; l = l->next)
	{
		GspellEntryWord *cur_word = l->data;

		if (cur_word->char_start <= pos && pos <= cur_word->char_end)
		{
			entry_word = cur_word;
			l->data = NULL;
			break;
		}
	}

	g_slist_free_full (words, _gspell_entry_word_free);
	return entry_word;
}

static gboolean
popup_menu_cb (GtkEntry    *gtk_entry,
	       GspellEntry *gspell_entry)
{
	/* Save the position before popping up the menu, otherwise it will
	 * contain the wrong set of suggestions.
	 */
	gspell_entry->popup_char_position = gtk_editable_get_position (GTK_EDITABLE (gtk_entry));

	return FALSE;
}

static gboolean
button_press_event_cb (GtkEntry       *gtk_entry,
		       GdkEventButton *event,
		       GspellEntry    *gspell_entry)
{
	if (event->button == GDK_BUTTON_SECONDARY)
	{
		gspell_entry->popup_char_position =
			_gspell_entry_utils_get_char_position_at_event (gtk_entry, event);
	}

	_gspell_current_word_policy_cursor_moved (gspell_entry->current_word_policy);
	recheck_all (gspell_entry);

	return GDK_EVENT_PROPAGATE;
}

static void
language_activated_cb (const GspellLanguage *lang,
		       gpointer              user_data)
{
	GspellEntry *gspell_entry;

	g_return_if_fail (GSPELL_IS_ENTRY (user_data));

	gspell_entry = GSPELL_ENTRY (user_data);

	if (gspell_entry->checker != NULL)
	{
		gspell_checker_set_language (gspell_entry->checker, lang);
	}
}

static void
suggestion_activated_cb (const gchar *suggested_word,
			 gpointer     user_data)
{
	GspellEntry *gspell_entry;
	GspellEntryWord *word;
	gint pos;

	g_return_if_fail (GSPELL_IS_ENTRY (user_data));

	gspell_entry = GSPELL_ENTRY (user_data);

	word = get_entry_word_at_popup_position (gspell_entry);
	if (word == NULL)
	{
		return;
	}

	gtk_editable_delete_text (GTK_EDITABLE (gspell_entry->entry),
				  word->char_start,
				  word->char_end);

	pos = word->char_start;
	gtk_editable_insert_text (GTK_EDITABLE (gspell_entry->entry),
				  suggested_word, -1,
				  &pos);

	_gspell_entry_word_free (word);
}

static void
populate_popup_cb (GtkEntry    *gtk_entry,
		   GtkWidget   *popup,
		   GspellEntry *gspell_entry)
{
	GtkMenu *menu;
	GtkWidget *menu_item;
	GtkMenuItem *lang_menu_item;
	GtkMenuItem *suggestions_menu_item;
	const GspellLanguage *current_language;
	GspellEntryWord *word;
	gboolean correctly_spelled;
	GError *error = NULL;

	if (!GTK_IS_MENU (popup))
	{
		return;
	}

	menu = GTK_MENU (popup);

	if (!inline_spell_checking_is_enabled (gspell_entry))
	{
		return;
	}

	if (gspell_entry->checker == NULL)
	{
		return;
	}

	/* Prepend separator */
	menu_item = gtk_separator_menu_item_new ();
	gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), menu_item);
	gtk_widget_show (menu_item);

	/* Prepend language sub-menu */
	current_language = gspell_checker_get_language (gspell_entry->checker);
	lang_menu_item = _gspell_context_menu_get_language_menu_item (current_language,
								      language_activated_cb,
								      gspell_entry);

	gtk_menu_shell_prepend (GTK_MENU_SHELL (menu),
				GTK_WIDGET (lang_menu_item));

	/* Prepend suggestions sub-menu */
	word = get_entry_word_at_popup_position (gspell_entry);

	if (word == NULL)
	{
		return;
	}

	correctly_spelled = gspell_checker_check_word (gspell_entry->checker,
						       word->word_str, -1,
						       &error);

	if (error != NULL)
	{
		g_warning ("Inline spell checker: %s", error->message);
		g_clear_error (&error);
		_gspell_entry_word_free (word);
		return;
	}

	if (!correctly_spelled)
	{
		suggestions_menu_item = _gspell_context_menu_get_suggestions_menu_item (gspell_entry->checker,
											word->word_str,
											suggestion_activated_cb,
											gspell_entry);

		gtk_menu_shell_prepend (GTK_MENU_SHELL (menu),
					GTK_WIDGET (suggestions_menu_item));
	}

	_gspell_entry_word_free (word);
}

static void
move_cursor_cb (GspellEntry *gspell_entry)
{
	_gspell_current_word_policy_cursor_moved (gspell_entry->current_word_policy);
	recheck_all (gspell_entry);
}

static gboolean
is_inside_word (const GSList *words,
		gint          char_pos)
{
	const GSList *l;

	for (l = words; l != NULL; l = l->next)
	{
		const GspellEntryWord *cur_word = l->data;

		if (cur_word->char_start <= char_pos && char_pos < cur_word->char_end)
		{
			return TRUE;
		}
	}

	return FALSE;
}

static gboolean
ends_word (const GSList *words,
	   gint          char_pos)
{
	const GSList *l;

	for (l = words; l != NULL; l = l->next)
	{
		const GspellEntryWord *cur_word = l->data;

		if (cur_word->char_end == char_pos)
		{
			return TRUE;
		}
	}

	return FALSE;
}

static void
delete_text_before_cb (GtkEditable *editable,
		       gint         start_pos,
		       gint         end_pos,
		       GspellEntry *gspell_entry)
{
	gint real_start_pos;
	gint real_end_pos;
	gint cursor_pos;
	GSList *words;
	gboolean empty_selection;
	gboolean spans_several_lines;
	gboolean several_chars;
	gboolean cursor_pos_at_start;
	gboolean cursor_pos_at_end;
	gboolean start_is_inside_word;
	gboolean start_ends_word;
	gboolean end_is_inside_word;
	gboolean end_ends_word;

	real_start_pos = start_pos;

	if (end_pos < 0)
	{
		real_end_pos = gtk_entry_get_text_length (gspell_entry->entry);
	}
	else
	{
		real_end_pos = end_pos;
	}

	if (real_start_pos == real_end_pos)
	{
		return;
	}

	if (real_start_pos > real_end_pos)
	{
		gint real_start_pos_copy;

		/* swap */
		real_start_pos_copy = real_start_pos;
		real_start_pos = real_end_pos;
		real_end_pos = real_start_pos_copy;
	}

	g_assert_cmpint (real_start_pos, <, real_end_pos);

	empty_selection = !gtk_editable_get_selection_bounds (editable, NULL, NULL);
	spans_several_lines = FALSE;
	several_chars = (real_end_pos - real_start_pos) > 1;

	cursor_pos = gtk_editable_get_position (editable);
	cursor_pos_at_start = cursor_pos == real_start_pos;
	cursor_pos_at_end = cursor_pos == real_end_pos;

	words = _gspell_entry_utils_get_words (gspell_entry->entry);

	start_is_inside_word = is_inside_word (words, real_start_pos);
	start_ends_word = ends_word (words, real_start_pos);

	end_is_inside_word = is_inside_word (words, real_end_pos);
	end_ends_word = ends_word (words, real_end_pos);

	g_slist_free_full (words, _gspell_entry_word_free);

	_gspell_current_word_policy_text_deleted (gspell_entry->current_word_policy,
						  empty_selection,
						  spans_several_lines,
						  several_chars,
						  cursor_pos_at_start,
						  cursor_pos_at_end,
						  start_is_inside_word,
						  start_ends_word,
						  end_is_inside_word,
						  end_ends_word);
}

static void
set_entry (GspellEntry *gspell_entry,
	   GtkEntry    *gtk_entry)
{
	g_return_if_fail (GTK_IS_ENTRY (gtk_entry));

	g_assert (gspell_entry->entry == NULL);
	gspell_entry->entry = gtk_entry;

	g_signal_connect_after (gtk_entry,
				"changed",
				G_CALLBACK (changed_after_cb),
				gspell_entry);

	g_signal_connect (gtk_entry,
			  "notify::buffer",
			  G_CALLBACK (notify_buffer_cb),
			  gspell_entry);

	g_assert (gspell_entry->notify_attributes_handler_id == 0);
	gspell_entry->notify_attributes_handler_id =
		g_signal_connect (gtk_entry,
				  "notify::attributes",
				  G_CALLBACK (notify_attributes_cb),
				  gspell_entry);

	g_signal_connect (gtk_entry,
			  "popup-menu",
			  G_CALLBACK (popup_menu_cb),
			  gspell_entry);

	g_signal_connect (gtk_entry,
			  "button-press-event",
			  G_CALLBACK (button_press_event_cb),
			  gspell_entry);

	/* connect_after, so when menu items are prepended, they have more
	 * chances to be the first in the menu.
	 */
	g_signal_connect_after (gtk_entry,
				"populate-popup",
				G_CALLBACK (populate_popup_cb),
				gspell_entry);

	/* What we want here is to be notified when the cursor moved, but _not_
	 * when the cursor moved because of a text insertion/deletion. To call
	 * _gspell_current_word_policy_cursor_moved().
	 *
	 * Connecting to notify::cursor-position is not suitable because we have
	 * notifications also when text is inserted/deleted. And we get the
	 * notification *after* the GtkEditable::insert-text signal (not
	 * *during* its emission). So it seems that the only simple solution is
	 * to connect to ::move-cursor, even if its documentation doesn't
	 * recommend that (normally, it should be used only to emit the signal).
	 *
	 * Note that _gspell_current_word_policy_cursor_moved() is also called
	 * in button_press_event_cb().
	 *
	 * The GtkEntry API is not really convenient, if you find a better
	 * solution, or if you improve the GtkEntry API...
	 */
	g_signal_connect_swapped (gtk_entry,
				  "move-cursor",
				  G_CALLBACK (move_cursor_cb),
				  gspell_entry);

	g_signal_connect (GTK_EDITABLE (gtk_entry),
			  "delete-text",
			  G_CALLBACK (delete_text_before_cb),
			  gspell_entry);

	g_signal_connect_swapped (gtk_entry,
				  "notify::visibility",
				  G_CALLBACK (recheck_all),
				  gspell_entry);

	update_buffer (gspell_entry);

	g_object_notify (G_OBJECT (gspell_entry), "entry");
}

static void
gspell_entry_get_property (GObject    *object,
			   guint       prop_id,
			   GValue     *value,
			   GParamSpec *pspec)
{
	GspellEntry *gspell_entry = GSPELL_ENTRY (object);

	switch (prop_id)
	{
		case PROP_ENTRY:
			g_value_set_object (value, gspell_entry_get_entry (gspell_entry));
			break;

		case PROP_INLINE_SPELL_CHECKING:
			g_value_set_boolean (value, gspell_entry_get_inline_spell_checking (gspell_entry));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_entry_set_property (GObject      *object,
			   guint         prop_id,
			   const GValue *value,
			   GParamSpec   *pspec)
{
	GspellEntry *gspell_entry = GSPELL_ENTRY (object);

	switch (prop_id)
	{
		case PROP_ENTRY:
			set_entry (gspell_entry, g_value_get_object (value));
			break;

		case PROP_INLINE_SPELL_CHECKING:
			gspell_entry_set_inline_spell_checking (gspell_entry, g_value_get_boolean (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_entry_dispose (GObject *object)
{
	GspellEntry *gspell_entry = GSPELL_ENTRY (object);

	gspell_entry->entry = NULL;
	set_buffer (gspell_entry, NULL);
	set_checker (gspell_entry, NULL);

	if (gspell_entry->notify_attributes_idle_id != 0)
	{
		g_source_remove (gspell_entry->notify_attributes_idle_id);
		gspell_entry->notify_attributes_idle_id = 0;
	}

	G_OBJECT_CLASS (gspell_entry_parent_class)->dispose (object);
}

static void
gspell_entry_finalize (GObject *object)
{
	GspellEntry *gspell_entry = GSPELL_ENTRY (object);

	/* Internal GObject, we can release it in finalize. */
	g_clear_object (&gspell_entry->current_word_policy);

	G_OBJECT_CLASS (gspell_entry_parent_class)->finalize (object);
}

static void
gspell_entry_class_init (GspellEntryClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gspell_entry_get_property;
	object_class->set_property = gspell_entry_set_property;
	object_class->dispose = gspell_entry_dispose;
	object_class->finalize = gspell_entry_finalize;

	/**
	 * GspellEntry:entry:
	 *
	 * The #GtkEntry.
	 *
	 * Since: 1.4
	 */
	g_object_class_install_property (object_class,
					 PROP_ENTRY,
					 g_param_spec_object ("entry",
							      "Entry",
							      "",
							      GTK_TYPE_ENTRY,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY |
							      G_PARAM_STATIC_STRINGS));

	/**
	 * GspellEntry:inline-spell-checking:
	 *
	 * Whether the inline spell checking is enabled.
	 *
	 * Even if this property is %TRUE, #GspellEntry disables internally the
	 * inline spell checking in case the #GtkEntry:visibility property is
	 * %FALSE.
	 *
	 * Since: 1.4
	 */
	g_object_class_install_property (object_class,
					 PROP_INLINE_SPELL_CHECKING,
					 g_param_spec_boolean ("inline-spell-checking",
							       "Inline Spell Checking",
							       "",
							       FALSE,
							       G_PARAM_READWRITE |
							       G_PARAM_STATIC_STRINGS));
}

static void
gspell_entry_init (GspellEntry *gspell_entry)
{
	gspell_entry->current_word_policy = _gspell_current_word_policy_new ();
}

/**
 * gspell_entry_get_from_gtk_entry:
 * @gtk_entry: a #GtkEntry.
 *
 * Returns the #GspellEntry of @gtk_entry. The returned object is guaranteed
 * to be the same for the lifetime of @gtk_entry.
 *
 * Returns: (transfer none): the #GspellEntry of @gtk_entry.
 * Since: 1.4
 */
GspellEntry *
gspell_entry_get_from_gtk_entry (GtkEntry *gtk_entry)
{
	GspellEntry *gspell_entry;

	g_return_val_if_fail (GTK_IS_ENTRY (gtk_entry), NULL);

	gspell_entry = g_object_get_data (G_OBJECT (gtk_entry), GSPELL_ENTRY_KEY);

	if (gspell_entry == NULL)
	{
		gspell_entry = g_object_new (GSPELL_TYPE_ENTRY,
					     "entry", gtk_entry,
					     NULL);

		g_object_set_data_full (G_OBJECT (gtk_entry),
					GSPELL_ENTRY_KEY,
					gspell_entry,
					g_object_unref);
	}

	g_return_val_if_fail (GSPELL_IS_ENTRY (gspell_entry), NULL);
	return gspell_entry;
}

/**
 * gspell_entry_basic_setup:
 * @gspell_entry: a #GspellEntry.
 *
 * This function is a convenience function that does the following:
 * - Set a spell checker. The language chosen is the one returned by
 *   gspell_language_get_default().
 * - Set the #GspellEntry:inline-spell-checking property to %TRUE.
 *
 * Example:
 * |[
 * GtkEntry *gtk_entry;
 * GspellEntry *gspell_entry;
 *
 * gspell_entry = gspell_entry_get_from_gtk_entry (gtk_entry);
 * gspell_entry_basic_setup (gspell_entry);
 * ]|
 *
 * This is equivalent to:
 * |[
 * GtkEntry *gtk_entry;
 * GspellEntry *gspell_entry;
 * GspellChecker *checker;
 * GtkEntryBuffer *gtk_buffer;
 * GspellEntryBuffer *gspell_buffer;
 *
 * checker = gspell_checker_new (NULL);
 * gtk_buffer = gtk_entry_get_buffer (gtk_entry);
 * gspell_buffer = gspell_entry_buffer_get_from_gtk_entry_buffer (gtk_buffer);
 * gspell_entry_buffer_set_spell_checker (gspell_buffer, checker);
 * g_object_unref (checker);
 *
 * gspell_entry = gspell_entry_get_from_gtk_entry (gtk_entry);
 * gspell_entry_set_inline_spell_checking (gspell_entry, TRUE);
 * ]|
 *
 * Since: 1.4
 */
void
gspell_entry_basic_setup (GspellEntry *gspell_entry)
{
	GspellChecker *checker;
	GtkEntryBuffer *gtk_buffer;
	GspellEntryBuffer *gspell_buffer;

	g_return_if_fail (GSPELL_IS_ENTRY (gspell_entry));

	checker = gspell_checker_new (NULL);
	gtk_buffer = gtk_entry_get_buffer (gspell_entry->entry);
	gspell_buffer = gspell_entry_buffer_get_from_gtk_entry_buffer (gtk_buffer);
	gspell_entry_buffer_set_spell_checker (gspell_buffer, checker);
	g_object_unref (checker);

	gspell_entry_set_inline_spell_checking (gspell_entry, TRUE);
}

/**
 * gspell_entry_get_entry:
 * @gspell_entry: a #GspellEntry.
 *
 * Returns: (transfer none): the #GtkEntry of @gspell_entry.
 * Since: 1.4
 */
GtkEntry *
gspell_entry_get_entry (GspellEntry *gspell_entry)
{
	g_return_val_if_fail (GSPELL_IS_ENTRY (gspell_entry), NULL);

	return gspell_entry->entry;
}

/**
 * gspell_entry_get_inline_spell_checking:
 * @gspell_entry: a #GspellEntry.
 *
 * Returns: the value of the #GspellEntry:inline-spell-checking property.
 * Since: 1.4
 */
gboolean
gspell_entry_get_inline_spell_checking (GspellEntry *gspell_entry)
{
	g_return_val_if_fail (GSPELL_IS_ENTRY (gspell_entry), FALSE);

	return gspell_entry->inline_spell_checking;
}

/**
 * gspell_entry_set_inline_spell_checking:
 * @gspell_entry: a #GspellEntry.
 * @enable: the new state.
 *
 * Sets the #GspellEntry:inline-spell-checking property.
 *
 * Since: 1.4
 */
void
gspell_entry_set_inline_spell_checking (GspellEntry *gspell_entry,
					gboolean     enable)
{
	g_return_if_fail (GSPELL_IS_ENTRY (gspell_entry));

	enable = enable != FALSE;

	if (gspell_entry->inline_spell_checking != enable)
	{
		gspell_entry->inline_spell_checking = enable;
		recheck_all (gspell_entry);
		g_object_notify (G_OBJECT (gspell_entry), "inline-spell-checking");
	}
}

/* For unit tests.
 * Returns: (transfer none) (element-type GspellEntryWord).
 */
const GSList *
_gspell_entry_get_misspelled_words (GspellEntry *gspell_entry)
{
	g_return_val_if_fail (GSPELL_IS_ENTRY (gspell_entry), NULL);

	return gspell_entry->misspelled_words;
}
