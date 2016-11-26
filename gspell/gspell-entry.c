/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2016 - Sébastien Wilmet
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

#include "gspell-entry.h"
#include "gspell-entry-private.h"
#include "gspell-entry-buffer.h"
#include "gspell-entry-utils.h"
#include "gspell-context-menu.h"

/**
 * SECTION:entry
 * @Title: GspellEntry
 * @Short_description: Spell checking support for GtkEntry
 *
 * #GspellEntry extends the #GtkEntry class with inline spell checking.
 * Misspelled words are highlighted with a %PANGO_UNDERLINE_ERROR, usually a red
 * wavy underline. Right-clicking a misspelled word pops up a context menu of
 * suggested replacements. The context menu also contains an “Ignore All” item
 * to add the misspelled word to the session dictionary. And an “Add” item to
 * add the word to the personal dictionary.
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
 */

struct _GspellEntry
{
	GObject parent;

	GtkEntry *entry;
	GtkEntryBuffer *buffer;
	GspellChecker *checker;

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

static GspellChecker *
get_checker (GspellEntry *gspell_entry)
{
	GspellEntryBuffer *gspell_buffer;

	if (gspell_entry->buffer == NULL)
	{
		return NULL;
	}

	gspell_buffer = gspell_entry_buffer_get_from_gtk_entry_buffer (gspell_entry->buffer);

	return gspell_entry_buffer_get_spell_checker (gspell_buffer);
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

	attr_underline = pango_attr_underline_new (PANGO_UNDERLINE_ERROR);
	attr_underline->start_index = byte_start;
	attr_underline->end_index = byte_end;

	attr_underline_color = pango_attr_underline_color_new (65535, 0, 0);
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
	GspellChecker *checker;
	GSList *all_words;

	g_slist_free_full (gspell_entry->misspelled_words, _gspell_entry_word_free);
	gspell_entry->misspelled_words = NULL;

	if (!gspell_entry->inline_spell_checking)
	{
		return;
	}

	checker = get_checker (gspell_entry);
	if (checker == NULL ||
	    gspell_checker_get_language (checker) == NULL)
	{
		return;
	}

	all_words = _gspell_entry_utils_get_words (gspell_entry->entry);

	while (all_words != NULL)
	{
		GspellEntryWord *cur_word = all_words->data;
		gboolean correctly_spelled;
		GError *error = NULL;

		correctly_spelled = gspell_checker_check_word (checker,
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

static void
recheck_all (GspellEntry *gspell_entry)
{
	GSList *l;

	update_misspelled_words_list (gspell_entry);

	for (l = gspell_entry->misspelled_words; l != NULL; l = l->next)
	{
		GspellEntryWord *cur_word = l->data;

		insert_underline (gspell_entry,
				  cur_word->byte_start,
				  cur_word->byte_end);
	}

	update_attributes (gspell_entry);
}

/* Connect to the ::changed signal before/after, so that other features (in
 * other libraries or apps) can insert other underline attributes (e.g. for
 * grammar checking) in another phase of the ::changed signal emission.
 *
 * But the GtkEntry API is not as nice as the GtkTextView API for inserting
 * tags, setting priorities on them, etc. So, do the best that we can with the
 * current GtkEntry API. If you find a better solution...
 */
static void
changed_before_cb (GtkEditable *editable,
		   GspellEntry *gspell_entry)
{
	remove_all_underlines (gspell_entry);
}

static void
changed_after_cb (GtkEditable *editable,
		  GspellEntry *gspell_entry)
{
	recheck_all (gspell_entry);
}

/* When the underlines need to be updated, call this function, so that all the
 * underlines attributes are always removed in changed_before_cb().
 */
static void
emit_changed_signal (GspellEntry *gspell_entry)
{
	g_signal_emit_by_name (gspell_entry->entry, "changed");
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
						      emit_changed_signal,
						      gspell_entry);

		g_object_unref (gspell_entry->checker);
	}

	gspell_entry->checker = checker;

	if (gspell_entry->checker != NULL)
	{
		g_signal_connect_swapped (gspell_entry->checker,
					  "notify::language",
					  G_CALLBACK (emit_changed_signal),
					  gspell_entry);

		g_signal_connect_swapped (gspell_entry->checker,
					  "session-cleared",
					  G_CALLBACK (emit_changed_signal),
					  gspell_entry);

		g_signal_connect_swapped (gspell_entry->checker,
					  "word-added-to-personal",
					  G_CALLBACK (emit_changed_signal),
					  gspell_entry);

		g_signal_connect_swapped (gspell_entry->checker,
					  "word-added-to-session",
					  G_CALLBACK (emit_changed_signal),
					  gspell_entry);

		g_object_ref (gspell_entry->checker);
	}
}

static void
update_checker (GspellEntry *gspell_entry)
{
	set_checker (gspell_entry, get_checker (gspell_entry));
}

static void
notify_spell_checker_cb (GspellEntryBuffer *gspell_buffer,
			 GParamSpec        *pspec,
			 GspellEntry       *gspell_entry)
{
	update_checker (gspell_entry);
	emit_changed_signal (gspell_entry);
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
	emit_changed_signal (gspell_entry);
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

	return GDK_EVENT_PROPAGATE;
}

static void
language_activated_cb (const GspellLanguage *lang,
		       gpointer              user_data)
{
	GspellEntry *gspell_entry;
	GspellChecker *checker;

	g_return_if_fail (GSPELL_IS_ENTRY (user_data));

	gspell_entry = GSPELL_ENTRY (user_data);

	checker = get_checker (gspell_entry);
	if (checker != NULL)
	{
		gspell_checker_set_language (checker, lang);
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
	GspellChecker *checker;
	const GspellLanguage *current_language;
	GspellEntryWord *word;
	gboolean correctly_spelled;
	GError *error = NULL;

	if (!GTK_IS_MENU (popup))
	{
		return;
	}

	menu = GTK_MENU (popup);

	if (!gspell_entry->inline_spell_checking)
	{
		return;
	}

	checker = get_checker (gspell_entry);
	if (checker == NULL)
	{
		return;
	}

	/* Prepend separator */
	menu_item = gtk_separator_menu_item_new ();
	gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), menu_item);
	gtk_widget_show (menu_item);

	/* Prepend language sub-menu */
	current_language = gspell_checker_get_language (checker);
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

	correctly_spelled = gspell_checker_check_word (checker,
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
		suggestions_menu_item = _gspell_context_menu_get_suggestions_menu_item (checker,
											word->word_str,
											suggestion_activated_cb,
											gspell_entry);

		gtk_menu_shell_prepend (GTK_MENU_SHELL (menu),
					GTK_WIDGET (suggestions_menu_item));
	}

	_gspell_entry_word_free (word);
}

static void
set_entry (GspellEntry *gspell_entry,
	   GtkEntry    *gtk_entry)
{
	g_return_if_fail (GTK_IS_ENTRY (gtk_entry));

	g_assert (gspell_entry->entry == NULL);
	gspell_entry->entry = gtk_entry;

	g_signal_connect (gtk_entry,
			  "changed",
			  G_CALLBACK (changed_before_cb),
			  gspell_entry);

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
gspell_entry_class_init (GspellEntryClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gspell_entry_get_property;
	object_class->set_property = gspell_entry_set_property;
	object_class->dispose = gspell_entry_dispose;

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
 * Returns: whether the inline spell checking is enabled.
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
 * Enables or disables the inline spell checking.
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
		emit_changed_signal (gspell_entry);
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

/* ex:set ts=8 noet: */
