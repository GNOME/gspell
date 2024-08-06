/* SPDX-FileCopyrightText: 2015, 2016 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "gspell-config.h"
#include "gspell-language-chooser-button.h"
#include <glib/gi18n-lib.h>
#include "gspell-language-chooser.h"
#include "gspell-language-chooser-dialog.h"

/**
 * SECTION:language-chooser-button
 * @Short_description: Button to choose a GspellLanguage
 * @Title: GspellLanguageChooserButton
 * @See_also: #GspellLanguage, #GspellLanguageChooser
 *
 * #GspellLanguageChooserButton is a #GtkButton to choose an available
 * #GspellLanguage. #GspellLanguageChooserButton implements the
 * #GspellLanguageChooser interface.
 *
 * The button contains a label with the #GspellLanguageChooser:language name, as
 * returned by gspell_language_get_name(). When the button is clicked, a
 * #GspellLanguageChooserDialog is launched to choose the language.
 */

typedef struct _GspellLanguageChooserButtonPrivate GspellLanguageChooserButtonPrivate;

struct _GspellLanguageChooserButtonPrivate
{
	GspellLanguageChooserDialog *dialog;
	const GspellLanguage *language;
	guint default_language : 1;
};

enum
{
	PROP_0,
	PROP_LANGUAGE,
	PROP_LANGUAGE_CODE,
};

static void gspell_language_chooser_button_iface_init (GspellLanguageChooserInterface *iface);

G_DEFINE_TYPE_WITH_CODE (GspellLanguageChooserButton,
			 gspell_language_chooser_button,
			 GTK_TYPE_BUTTON,
			 G_ADD_PRIVATE (GspellLanguageChooserButton)
			 G_IMPLEMENT_INTERFACE (GSPELL_TYPE_LANGUAGE_CHOOSER,
						gspell_language_chooser_button_iface_init))

static void
update_button_label (GspellLanguageChooserButton *button)
{
	GspellLanguageChooserButtonPrivate *priv;

	priv = gspell_language_chooser_button_get_instance_private (button);

	if (priv->language != NULL)
	{
		gtk_button_set_label (GTK_BUTTON (button),
				      gspell_language_get_name (priv->language));
	}
	else
	{
		gtk_button_set_label (GTK_BUTTON (button),
				      _("No language selected"));
	}
}

static const GspellLanguage *
gspell_language_chooser_button_get_language_full (GspellLanguageChooser *chooser,
						  gboolean              *default_language)
{
	GspellLanguageChooserButtonPrivate *priv;

	priv = gspell_language_chooser_button_get_instance_private (GSPELL_LANGUAGE_CHOOSER_BUTTON (chooser));

	if (default_language != NULL)
	{
		*default_language = priv->default_language;
	}

	return priv->language;
}

static void
gspell_language_chooser_button_set_language (GspellLanguageChooser *chooser,
					     const GspellLanguage  *language)
{
	GspellLanguageChooserButton *button;
	GspellLanguageChooserButtonPrivate *priv;
	gboolean default_language;
	gboolean notify_language_code = FALSE;

	button = GSPELL_LANGUAGE_CHOOSER_BUTTON (chooser);
	priv = gspell_language_chooser_button_get_instance_private (button);

	default_language = (language == NULL);

	if (priv->default_language != default_language)
	{
		priv->default_language = default_language;
		notify_language_code = TRUE;
	}

	if (language == NULL)
	{
		language = gspell_language_get_default ();
	}

	if (priv->language != language)
	{
		priv->language = language;

		update_button_label (button);

		g_object_notify (G_OBJECT (chooser), "language");
		notify_language_code = TRUE;
	}

	if (notify_language_code)
	{
		g_object_notify (G_OBJECT (chooser), "language-code");
	}
}

static void
gspell_language_chooser_button_iface_init (GspellLanguageChooserInterface *iface)
{
	iface->get_language_full = gspell_language_chooser_button_get_language_full;
	iface->set_language = gspell_language_chooser_button_set_language;
}

static void
gspell_language_chooser_button_get_property (GObject    *object,
					     guint       prop_id,
					     GValue     *value,
					     GParamSpec *pspec)
{
	GspellLanguageChooser *chooser = GSPELL_LANGUAGE_CHOOSER (object);

	switch (prop_id)
	{
		case PROP_LANGUAGE:
			g_value_set_boxed (value, gspell_language_chooser_get_language (chooser));
			break;

		case PROP_LANGUAGE_CODE:
			g_value_set_string (value, gspell_language_chooser_get_language_code (chooser));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_language_chooser_button_set_property (GObject      *object,
					     guint         prop_id,
					     const GValue *value,
					     GParamSpec   *pspec)
{
	GspellLanguageChooser *chooser = GSPELL_LANGUAGE_CHOOSER (object);

	switch (prop_id)
	{
		case PROP_LANGUAGE:
			gspell_language_chooser_set_language (chooser, g_value_get_boxed (value));
			break;

		case PROP_LANGUAGE_CODE:
			gspell_language_chooser_set_language_code (chooser, g_value_get_string (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_language_chooser_button_constructed (GObject *object)
{
	G_OBJECT_CLASS (gspell_language_chooser_button_parent_class)->constructed (object);

	update_button_label (GSPELL_LANGUAGE_CHOOSER_BUTTON (object));
}

static void
dialog_response_cb (GtkDialog *dialog,
		    gint       response)
{
	gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
dialog_destroy_cb (GtkWidget                   *dialog,
		   GspellLanguageChooserButton *button)
{
	GspellLanguageChooserButtonPrivate *priv;

	priv = gspell_language_chooser_button_get_instance_private (button);

	priv->dialog = NULL;
}

static void
ensure_dialog (GspellLanguageChooserButton *button)
{
	GspellLanguageChooserButtonPrivate *priv;
	GtkWidget *toplevel;
	GtkWindow *parent = NULL;

	priv = gspell_language_chooser_button_get_instance_private (button);

	if (priv->dialog != NULL)
	{
		return;
	}

	toplevel = gtk_widget_get_toplevel (GTK_WIDGET (button));
	if (gtk_widget_is_toplevel (toplevel) && GTK_IS_WINDOW (toplevel))
	{
		parent = GTK_WINDOW (toplevel);
	}

	priv->dialog = GSPELL_LANGUAGE_CHOOSER_DIALOG (
		gspell_language_chooser_dialog_new (parent,
						    priv->default_language ? NULL : priv->language,
						    GTK_DIALOG_DESTROY_WITH_PARENT |
						    GTK_DIALOG_USE_HEADER_BAR));

	if (parent != NULL)
	{
		gtk_window_set_modal (GTK_WINDOW (priv->dialog),
				      gtk_window_get_modal (parent));
	}

	g_object_bind_property (priv->dialog, "language-code",
				button, "language-code",
				G_BINDING_DEFAULT);

	g_signal_connect (priv->dialog,
			  "response",
			  G_CALLBACK (dialog_response_cb),
			  NULL);

	g_signal_connect_object (priv->dialog,
				 "destroy",
				 G_CALLBACK (dialog_destroy_cb),
				 button,
				 0);
}

static void
gspell_language_chooser_button_clicked (GtkButton *gtk_button)
{
	GspellLanguageChooserButton *button;
	GspellLanguageChooserButtonPrivate *priv;

	button = GSPELL_LANGUAGE_CHOOSER_BUTTON (gtk_button);
	priv = gspell_language_chooser_button_get_instance_private (button);

	/* If the dialog isn't modal, the button can be clicked several times. */
	ensure_dialog (button);

	gspell_language_chooser_set_language (GSPELL_LANGUAGE_CHOOSER (priv->dialog),
					      priv->default_language ? NULL : priv->language);

	gtk_window_present (GTK_WINDOW (priv->dialog));
}

static void
gspell_language_chooser_button_class_init (GspellLanguageChooserButtonClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkButtonClass *button_class = GTK_BUTTON_CLASS (klass);

	object_class->get_property = gspell_language_chooser_button_get_property;
	object_class->set_property = gspell_language_chooser_button_set_property;
	object_class->constructed = gspell_language_chooser_button_constructed;

	button_class->clicked = gspell_language_chooser_button_clicked;

	g_object_class_override_property (object_class, PROP_LANGUAGE, "language");
	g_object_class_override_property (object_class, PROP_LANGUAGE_CODE, "language-code");
}

static void
gspell_language_chooser_button_init (GspellLanguageChooserButton *button)
{
}

/**
 * gspell_language_chooser_button_new:
 * @current_language: (nullable): a #GspellLanguage, or %NULL to pick the
 *   default language.
 *
 * Returns: a new #GspellLanguageChooserButton widget.
 */
GtkWidget *
gspell_language_chooser_button_new (const GspellLanguage *current_language)
{
	return g_object_new (GSPELL_TYPE_LANGUAGE_CHOOSER_BUTTON,
			     "language", current_language,
			     NULL);
}
