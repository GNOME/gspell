/* SPDX-FileCopyrightText: 2015, 2016, 2017 - Sébastien Wilmet
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "gspell-config.h"
#include "gspell-text-buffer.h"

/**
 * SECTION:text-buffer
 * @Title: GspellTextBuffer
 * @Short_description: Spell checking support for GtkTextBuffer
 *
 * #GspellTextBuffer extends the #GtkTextBuffer class but without subclassing
 * it, because the GtkSourceView library has already a #GtkTextBuffer subclass.
 *
 * # Support of the no-spell-check tag defined by GtkSourceView
 *
 * The syntax highlighting engines of
 * [GtkSourceView](https://gitlab.gnome.org/GNOME/gtksourceview/) and
 * [libgedit-gtksourceview](https://gedit-text-editor.org/technology.html)
 * have a feature called “context classes”. One of the standard context classes
 * is “<emphasis>no-spell-check</emphasis>”: it defines the regions in the
 * #GtkTextBuffer that should not be spell-checked.
 *
 * GtkSourceView creates a #GtkTextTag named
 * `"gtksourceview:context-classes:no-spell-check"`. gspell reads this tag, to
 * skip the text contained within the tag.
 *
 * If you use the GtkSourceView library in your application, keep in mind that
 * the #GtkTextTag created by GtkSourceView is for read-only purposes; you
 * cannot apply it yourself to other regions.
 *
 * On the other hand if the GtkSourceView library is not used, you can create a
 * #GtkTextTag with the same name to mark certain regions in the text that
 * gspell should skip. As it is not a great API, it is
 * [planned](https://gitlab.gnome.org/GNOME/gspell/-/issues/25) to add an
 * explicit API in #GspellTextBuffer to set a #GtkTextTag that gspell should
 * skip.
 *
 * See the class description of #GtkSourceBuffer for more information about
 * context classes.
 */

struct _GspellTextBuffer
{
	GObject parent;

	GtkTextBuffer *buffer;
	GspellChecker *spell_checker;
};

enum
{
	PROP_0,
	PROP_BUFFER,
	PROP_SPELL_CHECKER,
};

#define GSPELL_TEXT_BUFFER_KEY "gspell-text-buffer-key"

G_DEFINE_TYPE (GspellTextBuffer, gspell_text_buffer, G_TYPE_OBJECT)

static void
gspell_text_buffer_get_property (GObject    *object,
				 guint       prop_id,
				 GValue     *value,
				 GParamSpec *pspec)
{
	GspellTextBuffer *gspell_buffer = GSPELL_TEXT_BUFFER (object);

	switch (prop_id)
	{
		case PROP_BUFFER:
			g_value_set_object (value, gspell_text_buffer_get_buffer (gspell_buffer));
			break;

		case PROP_SPELL_CHECKER:
			g_value_set_object (value, gspell_text_buffer_get_spell_checker (gspell_buffer));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_text_buffer_set_property (GObject      *object,
				 guint         prop_id,
				 const GValue *value,
				 GParamSpec   *pspec)
{
	GspellTextBuffer *gspell_buffer = GSPELL_TEXT_BUFFER (object);

	switch (prop_id)
	{
		case PROP_BUFFER:
			g_assert (gspell_buffer->buffer == NULL);
			gspell_buffer->buffer = g_value_get_object (value);
			break;

		case PROP_SPELL_CHECKER:
			gspell_text_buffer_set_spell_checker (gspell_buffer, g_value_get_object (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_text_buffer_dispose (GObject *object)
{
	GspellTextBuffer *gspell_buffer = GSPELL_TEXT_BUFFER (object);

	gspell_buffer->buffer = NULL;
	g_clear_object (&gspell_buffer->spell_checker);

	G_OBJECT_CLASS (gspell_text_buffer_parent_class)->dispose (object);
}

static void
gspell_text_buffer_class_init (GspellTextBufferClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gspell_text_buffer_get_property;
	object_class->set_property = gspell_text_buffer_set_property;
	object_class->dispose = gspell_text_buffer_dispose;

	/**
	 * GspellTextBuffer:buffer:
	 *
	 * The #GtkTextBuffer.
	 */
	g_object_class_install_property (object_class,
					 PROP_BUFFER,
					 g_param_spec_object ("buffer",
							      "Buffer",
							      "",
							      GTK_TYPE_TEXT_BUFFER,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY |
							      G_PARAM_STATIC_STRINGS));

	/**
	 * GspellTextBuffer:spell-checker:
	 *
	 * The #GspellChecker.
	 */
	g_object_class_install_property (object_class,
					 PROP_SPELL_CHECKER,
					 g_param_spec_object ("spell-checker",
							      "Spell Checker",
							      "",
							      GSPELL_TYPE_CHECKER,
							      G_PARAM_READWRITE |
							      G_PARAM_STATIC_STRINGS));
}

static void
gspell_text_buffer_init (GspellTextBuffer *gspell_buffer)
{
}

/**
 * gspell_text_buffer_get_from_gtk_text_buffer:
 * @gtk_buffer: a #GtkTextBuffer.
 *
 * Returns the #GspellTextBuffer of @gtk_buffer. The returned object is
 * guaranteed to be the same for the lifetime of @gtk_buffer.
 *
 * Returns: (transfer none): the #GspellTextBuffer of @gtk_buffer.
 */
/* Yes I know, the function name is a bit long. But at least there is no
 * possible confusions. Other names that came to my mind:
 * - get_from_buffer(), but it's confusing: which buffer is it?
 * - get_from_sibling(): less clear.
 */
GspellTextBuffer *
gspell_text_buffer_get_from_gtk_text_buffer (GtkTextBuffer *gtk_buffer)
{
	GspellTextBuffer *gspell_buffer;

	g_return_val_if_fail (GTK_IS_TEXT_BUFFER (gtk_buffer), NULL);

	gspell_buffer = g_object_get_data (G_OBJECT (gtk_buffer), GSPELL_TEXT_BUFFER_KEY);

	if (gspell_buffer == NULL)
	{
		gspell_buffer = g_object_new (GSPELL_TYPE_TEXT_BUFFER,
					      "buffer", gtk_buffer,
					      NULL);

		g_object_set_data_full (G_OBJECT (gtk_buffer),
					GSPELL_TEXT_BUFFER_KEY,
					gspell_buffer,
					g_object_unref);
	}

	g_return_val_if_fail (GSPELL_IS_TEXT_BUFFER (gspell_buffer), NULL);
	return gspell_buffer;
}

/**
 * gspell_text_buffer_get_buffer:
 * @gspell_buffer: a #GspellTextBuffer.
 *
 * Returns: (transfer none): the #GtkTextBuffer of @gspell_buffer.
 */
GtkTextBuffer *
gspell_text_buffer_get_buffer (GspellTextBuffer *gspell_buffer)
{
	g_return_val_if_fail (GSPELL_IS_TEXT_BUFFER (gspell_buffer), NULL);

	return gspell_buffer->buffer;
}

/**
 * gspell_text_buffer_get_spell_checker:
 * @gspell_buffer: a #GspellTextBuffer.
 *
 * Returns: (nullable) (transfer none): the #GspellChecker if one has been set,
 *   or %NULL.
 */
GspellChecker *
gspell_text_buffer_get_spell_checker (GspellTextBuffer *gspell_buffer)
{
	g_return_val_if_fail (GSPELL_IS_TEXT_BUFFER (gspell_buffer), NULL);

	return gspell_buffer->spell_checker;
}

/**
 * gspell_text_buffer_set_spell_checker:
 * @gspell_buffer: a #GspellTextBuffer.
 * @spell_checker: (nullable): a #GspellChecker, or %NULL to unset the spell
 *   checker.
 *
 * Sets a #GspellChecker to a #GspellTextBuffer. The @gspell_buffer will own a
 * reference to @spell_checker, so you can release your reference to
 * @spell_checker if you no longer need it.
 */
void
gspell_text_buffer_set_spell_checker (GspellTextBuffer *gspell_buffer,
				      GspellChecker    *spell_checker)
{
	g_return_if_fail (GSPELL_IS_TEXT_BUFFER (gspell_buffer));
	g_return_if_fail (spell_checker == NULL || GSPELL_IS_CHECKER (spell_checker));

	if (g_set_object (&gspell_buffer->spell_checker, spell_checker))
	{
		g_object_notify (G_OBJECT (gspell_buffer), "spell-checker");
	}
}
