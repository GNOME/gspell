/* SPDX-FileCopyrightText: 2015, 2016, 2017 - Sébastien Wilmet
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "gspell-config.h"
#include "gspell-text-view.h"
#include <glib/gi18n-lib.h>
#include "gspell-inline-checker-text-buffer.h"
#include "gspell-checker.h"
#include "gspell-language.h"
#include "gspell-text-buffer.h"
#include "gspell-context-menu.h"

/**
 * SECTION:text-view
 * @Title: GspellTextView
 * @Short_description: Spell checking support for GtkTextView
 *
 * #GspellTextView extends the #GtkTextView class with inline spell checking.
 * Misspelled words are highlighted with a red %PANGO_UNDERLINE_SINGLE.
 * Right-clicking a misspelled word pops up a context menu of suggested
 * replacements. The context menu also contains an “Ignore All” item to add the
 * misspelled word to the session dictionary. And an “Add” item to add the word
 * to the personal dictionary.
 *
 * For a basic use-case, there is the gspell_text_view_basic_setup() convenience
 * function.
 *
 * The spell is checked only on the visible region of the #GtkTextView. Note
 * that if a same #GtkTextBuffer is used for several views, the misspelled words
 * are visible in all views, because the highlighting is achieved with a
 * #GtkTextTag added to the buffer.
 *
 * If you don't use the gspell_text_view_basic_setup() function, you need to
 * call gspell_text_buffer_set_spell_checker() to associate a #GspellChecker to
 * the #GtkTextBuffer.
 *
 * Note that #GspellTextView extends the #GtkTextView class but without
 * subclassing it, because the GtkSourceView library has already a #GtkTextView
 * subclass.
 *
 * If you want a %PANGO_UNDERLINE_ERROR instead (a wavy underline), please fix
 * [this bug](https://bugzilla.gnome.org/show_bug.cgi?id=763741) first.
 */

typedef struct _GspellTextViewPrivate GspellTextViewPrivate;

struct _GspellTextViewPrivate
{
	GtkTextView *view;
	GspellInlineCheckerTextBuffer *inline_checker;
	guint enable_language_menu : 1;
};

enum
{
	PROP_0,
	PROP_VIEW,
	PROP_INLINE_SPELL_CHECKING,
	PROP_ENABLE_LANGUAGE_MENU,
};

#define GSPELL_TEXT_VIEW_KEY "gspell-text-view-key"

G_DEFINE_TYPE_WITH_PRIVATE (GspellTextView, gspell_text_view, G_TYPE_OBJECT)

static void
create_inline_checker (GspellTextView *gspell_view)
{
	GspellTextViewPrivate *priv;
	GtkTextBuffer *buffer;

	priv = gspell_text_view_get_instance_private (gspell_view);

	if (priv->inline_checker != NULL)
	{
		return;
	}

	buffer = gtk_text_view_get_buffer (priv->view);
	priv->inline_checker = _gspell_inline_checker_text_buffer_new (buffer);
	_gspell_inline_checker_text_buffer_attach_view (priv->inline_checker,
							priv->view);
}

static void
destroy_inline_checker (GspellTextView *gspell_view)
{
	GspellTextViewPrivate *priv;

	priv = gspell_text_view_get_instance_private (gspell_view);

	if (priv->view == NULL || priv->inline_checker == NULL)
	{
		return;
	}

	_gspell_inline_checker_text_buffer_detach_view (priv->inline_checker,
							priv->view);
	g_clear_object (&priv->inline_checker);
}

static void
notify_buffer_cb (GtkTextView    *gtk_view,
		  GParamSpec     *pspec,
		  GspellTextView *gspell_view)
{
	GspellTextViewPrivate *priv;

	priv = gspell_text_view_get_instance_private (gspell_view);

	if (priv->inline_checker == NULL)
	{
		return;
	}

	destroy_inline_checker (gspell_view);
	create_inline_checker (gspell_view);
}

static void
language_activated_cb (const GspellLanguage *lang,
		       gpointer              user_data)
{
	GspellTextView *gspell_view;
	GspellTextViewPrivate *priv;
	GtkTextBuffer *gtk_buffer;
	GspellTextBuffer *gspell_buffer;
	GspellChecker *checker;

	g_return_if_fail (GSPELL_IS_TEXT_VIEW (user_data));

	gspell_view = GSPELL_TEXT_VIEW (user_data);
	priv = gspell_text_view_get_instance_private (gspell_view);

	gtk_buffer = gtk_text_view_get_buffer (priv->view);
	gspell_buffer = gspell_text_buffer_get_from_gtk_text_buffer (gtk_buffer);
	checker = gspell_text_buffer_get_spell_checker (gspell_buffer);

	gspell_checker_set_language (checker, lang);
}

static const GspellLanguage *
get_current_language (GspellTextView *gspell_view)
{
	GspellTextViewPrivate *priv;
	GtkTextBuffer *gtk_buffer;
	GspellTextBuffer *gspell_buffer;
	GspellChecker *checker;

	priv = gspell_text_view_get_instance_private (gspell_view);

	if (priv->view == NULL)
	{
		return NULL;
	}

	gtk_buffer = gtk_text_view_get_buffer (priv->view);
	gspell_buffer = gspell_text_buffer_get_from_gtk_text_buffer (gtk_buffer);
	checker = gspell_text_buffer_get_spell_checker (gspell_buffer);

	return gspell_checker_get_language (checker);
}

static void
populate_popup_cb (GtkTextView    *gtk_view,
		   GtkWidget      *popup,
		   GspellTextView *gspell_view)
{
	GspellTextViewPrivate *priv;
	GtkMenu *menu;
	GtkWidget *menu_item;

	priv = gspell_text_view_get_instance_private (gspell_view);

	if (!GTK_IS_MENU (popup))
	{
		return;
	}

	menu = GTK_MENU (popup);

	if (!priv->enable_language_menu &&
	    priv->inline_checker == NULL)
	{
		return;
	}

	/* Prepend separator */
	menu_item = gtk_separator_menu_item_new ();
	gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), menu_item);
	gtk_widget_show (menu_item);

	if (priv->enable_language_menu)
	{
		const GspellLanguage *current_language;
		GtkMenuItem *lang_menu_item;

		current_language = get_current_language (gspell_view);
		lang_menu_item = _gspell_context_menu_get_language_menu_item (current_language,
									      language_activated_cb,
									      gspell_view);

		/* Prepend language sub-menu */
		gtk_menu_shell_prepend (GTK_MENU_SHELL (menu),
					GTK_WIDGET (lang_menu_item));
	}

	if (priv->inline_checker != NULL)
	{
		/* Prepend suggestions */
		_gspell_inline_checker_text_buffer_populate_popup (priv->inline_checker, menu);
	}
}

static void
set_view (GspellTextView *gspell_view,
	  GtkTextView    *gtk_view)
{
	GspellTextViewPrivate *priv;

	g_return_if_fail (GTK_IS_TEXT_VIEW (gtk_view));

	priv = gspell_text_view_get_instance_private (gspell_view);

	g_assert (priv->view == NULL);
	g_assert (priv->inline_checker == NULL);

	priv->view = gtk_view;

	g_signal_connect_object (priv->view,
				 "notify::buffer",
				 G_CALLBACK (notify_buffer_cb),
				 gspell_view,
				 0);

	/* G_CONNECT_AFTER, so when menu items are prepended, they have more
	 * chances to be the first in the menu.
	 */
	g_signal_connect_object (priv->view,
				 "populate-popup",
				 G_CALLBACK (populate_popup_cb),
				 gspell_view,
				 G_CONNECT_AFTER);

	g_object_notify (G_OBJECT (gspell_view), "view");
}

static void
gspell_text_view_get_property (GObject    *object,
			       guint       prop_id,
			       GValue     *value,
			       GParamSpec *pspec)
{
	GspellTextView *gspell_view = GSPELL_TEXT_VIEW (object);

	switch (prop_id)
	{
		case PROP_VIEW:
			g_value_set_object (value, gspell_text_view_get_view (gspell_view));
			break;

		case PROP_INLINE_SPELL_CHECKING:
			g_value_set_boolean (value, gspell_text_view_get_inline_spell_checking (gspell_view));
			break;

		case PROP_ENABLE_LANGUAGE_MENU:
			g_value_set_boolean (value, gspell_text_view_get_enable_language_menu (gspell_view));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_text_view_set_property (GObject      *object,
			       guint         prop_id,
			       const GValue *value,
			       GParamSpec   *pspec)
{
	GspellTextView *gspell_view = GSPELL_TEXT_VIEW (object);

	switch (prop_id)
	{
		case PROP_VIEW:
			set_view (gspell_view, g_value_get_object (value));
			break;

		case PROP_INLINE_SPELL_CHECKING:
			gspell_text_view_set_inline_spell_checking (gspell_view, g_value_get_boolean (value));
			break;

		case PROP_ENABLE_LANGUAGE_MENU:
			gspell_text_view_set_enable_language_menu (gspell_view, g_value_get_boolean (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_text_view_dispose (GObject *object)
{
	GspellTextViewPrivate *priv;

	priv = gspell_text_view_get_instance_private (GSPELL_TEXT_VIEW (object));

	if (priv->view != NULL && priv->inline_checker != NULL)
	{
		_gspell_inline_checker_text_buffer_detach_view (priv->inline_checker,
								priv->view);
	}

	priv->view = NULL;
	g_clear_object (&priv->inline_checker);

	G_OBJECT_CLASS (gspell_text_view_parent_class)->dispose (object);
}

static void
gspell_text_view_class_init (GspellTextViewClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gspell_text_view_get_property;
	object_class->set_property = gspell_text_view_set_property;
	object_class->dispose = gspell_text_view_dispose;

	/**
	 * GspellTextView:view:
	 *
	 * The #GtkTextView.
	 */
	g_object_class_install_property (object_class,
					 PROP_VIEW,
					 g_param_spec_object ("view",
							      "View",
							      "",
							      GTK_TYPE_TEXT_VIEW,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY |
							      G_PARAM_STATIC_STRINGS));

	/**
	 * GspellTextView:inline-spell-checking:
	 *
	 * Whether the inline spell checking is enabled.
	 */
	g_object_class_install_property (object_class,
					 PROP_INLINE_SPELL_CHECKING,
					 g_param_spec_boolean ("inline-spell-checking",
							       "Inline Spell Checking",
							       "",
							       FALSE,
							       G_PARAM_READWRITE |
							       G_PARAM_STATIC_STRINGS));

	/**
	 * GspellTextView:enable-language-menu:
	 *
	 * When the context menu is shown, whether to add a sub-menu to select
	 * the language for the spell checking.
	 *
	 * Since: 1.2
	 */
	g_object_class_install_property (object_class,
					 PROP_ENABLE_LANGUAGE_MENU,
					 g_param_spec_boolean ("enable-language-menu",
							       "Enable Language Menu",
							       "",
							       FALSE,
							       G_PARAM_READWRITE |
							       G_PARAM_STATIC_STRINGS));
}

static void
gspell_text_view_init (GspellTextView *gspell_view)
{
}

/**
 * gspell_text_view_get_from_gtk_text_view:
 * @gtk_view: a #GtkTextView.
 *
 * Returns the #GspellTextView of @gtk_view. The returned object is guaranteed
 * to be the same for the lifetime of @gtk_view.
 *
 * Returns: (transfer none): the #GspellTextView of @gtk_view.
 */
GspellTextView *
gspell_text_view_get_from_gtk_text_view (GtkTextView *gtk_view)
{
	GspellTextView *gspell_view;

	g_return_val_if_fail (GTK_IS_TEXT_VIEW (gtk_view), NULL);

	gspell_view = g_object_get_data (G_OBJECT (gtk_view), GSPELL_TEXT_VIEW_KEY);

	if (gspell_view == NULL)
	{
		gspell_view = g_object_new (GSPELL_TYPE_TEXT_VIEW,
					    "view", gtk_view,
					    NULL);

		g_object_set_data_full (G_OBJECT (gtk_view),
					GSPELL_TEXT_VIEW_KEY,
					gspell_view,
					g_object_unref);
	}

	g_return_val_if_fail (GSPELL_IS_TEXT_VIEW (gspell_view), NULL);
	return gspell_view;
}

/**
 * gspell_text_view_basic_setup:
 * @gspell_view: a #GspellTextView.
 *
 * This function is a convenience function that does the following:
 * - Set a spell checker. The language chosen is the one returned by
 *   gspell_language_get_default().
 * - Set the #GspellTextView:inline-spell-checking property to %TRUE.
 * - Set the #GspellTextView:enable-language-menu property to %TRUE.
 *
 * Example:
 * |[
 * GtkTextView *gtk_view;
 * GspellTextView *gspell_view;
 *
 * gspell_view = gspell_text_view_get_from_gtk_text_view (gtk_view);
 * gspell_text_view_basic_setup (gspell_view);
 * ]|
 *
 * This is equivalent to:
 * |[
 * GtkTextView *gtk_view;
 * GspellTextView *gspell_view;
 * GspellChecker *checker;
 * GtkTextBuffer *gtk_buffer;
 * GspellTextBuffer *gspell_buffer;
 *
 * checker = gspell_checker_new (NULL);
 * gtk_buffer = gtk_text_view_get_buffer (gtk_view);
 * gspell_buffer = gspell_text_buffer_get_from_gtk_text_buffer (gtk_buffer);
 * gspell_text_buffer_set_spell_checker (gspell_buffer, checker);
 * g_object_unref (checker);
 *
 * gspell_view = gspell_text_view_get_from_gtk_text_view (gtk_view);
 * gspell_text_view_set_inline_spell_checking (gspell_view, TRUE);
 * gspell_text_view_set_enable_language_menu (gspell_view, TRUE);
 * ]|
 *
 * Since: 1.2
 */
void
gspell_text_view_basic_setup (GspellTextView *gspell_view)
{
	GspellTextViewPrivate *priv;
	GspellChecker *checker;
	GtkTextBuffer *gtk_buffer;
	GspellTextBuffer *gspell_buffer;

	g_return_if_fail (GSPELL_IS_TEXT_VIEW (gspell_view));

	priv = gspell_text_view_get_instance_private (gspell_view);

	checker = gspell_checker_new (NULL);
	gtk_buffer = gtk_text_view_get_buffer (priv->view);
	gspell_buffer = gspell_text_buffer_get_from_gtk_text_buffer (gtk_buffer);
	gspell_text_buffer_set_spell_checker (gspell_buffer, checker);
	g_object_unref (checker);

	gspell_text_view_set_inline_spell_checking (gspell_view, TRUE);
	gspell_text_view_set_enable_language_menu (gspell_view, TRUE);
}

/**
 * gspell_text_view_get_view:
 * @gspell_view: a #GspellTextView.
 *
 * Returns: (transfer none): the #GtkTextView of @gspell_view.
 */
GtkTextView *
gspell_text_view_get_view (GspellTextView *gspell_view)
{
	GspellTextViewPrivate *priv;

	g_return_val_if_fail (GSPELL_IS_TEXT_VIEW (gspell_view), NULL);

	priv = gspell_text_view_get_instance_private (gspell_view);
	return priv->view;
}

/**
 * gspell_text_view_get_inline_spell_checking:
 * @gspell_view: a #GspellTextView.
 *
 * Returns: whether the inline spell checking is enabled.
 */
gboolean
gspell_text_view_get_inline_spell_checking (GspellTextView *gspell_view)
{
	GspellTextViewPrivate *priv;

	g_return_val_if_fail (GSPELL_IS_TEXT_VIEW (gspell_view), FALSE);

	priv = gspell_text_view_get_instance_private (gspell_view);
	return priv->inline_checker != NULL;
}

/**
 * gspell_text_view_set_inline_spell_checking:
 * @gspell_view: a #GspellTextView.
 * @enable: the new state.
 *
 * Enables or disables the inline spell checking.
 */
void
gspell_text_view_set_inline_spell_checking (GspellTextView *gspell_view,
					    gboolean        enable)
{
	g_return_if_fail (GSPELL_IS_TEXT_VIEW (gspell_view));

	enable = enable != FALSE;

	if (enable == gspell_text_view_get_inline_spell_checking (gspell_view))
	{
		return;
	}

	if (enable)
	{
		create_inline_checker (gspell_view);
	}
	else
	{
		destroy_inline_checker (gspell_view);
	}

	g_object_notify (G_OBJECT (gspell_view), "inline-spell-checking");
}

/**
 * gspell_text_view_get_enable_language_menu:
 * @gspell_view: a #GspellTextView.
 *
 * Returns: whether the language context menu is enabled.
 * Since: 1.2
 */
gboolean
gspell_text_view_get_enable_language_menu (GspellTextView *gspell_view)
{
	GspellTextViewPrivate *priv;

	g_return_val_if_fail (GSPELL_IS_TEXT_VIEW (gspell_view), FALSE);

	priv = gspell_text_view_get_instance_private (gspell_view);
	return priv->enable_language_menu;
}

/**
 * gspell_text_view_set_enable_language_menu:
 * @gspell_view: a #GspellTextView.
 * @enable_language_menu: whether to enable the language context menu.
 *
 * Sets whether to enable the language context menu. If enabled, doing a right
 * click on the #GtkTextView will show a sub-menu to choose the language for the
 * spell checking. If another language is chosen, it changes the
 * #GspellChecker:language property of the #GspellTextBuffer:spell-checker of
 * the #GtkTextView:buffer of the #GspellTextView:view.
 *
 * Since: 1.2
 */
void
gspell_text_view_set_enable_language_menu (GspellTextView *gspell_view,
					   gboolean        enable_language_menu)
{
	GspellTextViewPrivate *priv;

	g_return_if_fail (GSPELL_IS_TEXT_VIEW (gspell_view));

	priv = gspell_text_view_get_instance_private (gspell_view);

	enable_language_menu = enable_language_menu != FALSE;

	if (priv->enable_language_menu != enable_language_menu)
	{
		priv->enable_language_menu = enable_language_menu;
		g_object_notify (G_OBJECT (gspell_view), "enable-language-menu");
	}
}
