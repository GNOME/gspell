/*
 * This file is part of gspell.
 *
 * Copyright 2015 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "gspell-language-chooser-button.h"
#include <glib/gi18n-lib.h>
#include "gspell-language-chooser.h"
#include "gspell-language-chooser-dialog.h"

/**
 * SECTION:language-chooser-button
 * @Short_description: Button to choose a spell language
 * @Title: GspellLanguageChooserButton
 * @See_also: #GspellLanguage, #GspellLanguageChooser
 *
 * #GspellLanguageChooserButton is a #GtkButton to choose an available
 * #GspellLanguage. #GspellLanguageChooserButton implements the
 * #GspellLanguageChooser interface.
 *
 * When the button is clicked, a #GspellLanguageChooserDialog is launched to
 * choose the language.
 */

typedef struct _GspellLanguageChooserButtonPrivate GspellLanguageChooserButtonPrivate;

struct _GspellLanguageChooserButtonPrivate
{
	const GspellLanguage *language;
	GspellLanguageChooserDialog *dialog;
};

enum
{
	PROP_0,
	PROP_LANGUAGE,
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
				      gspell_language_to_string (priv->language));
	}
	else
	{
		gtk_button_set_label (GTK_BUTTON (button),
				      _("No language selected"));
	}
}

static const GspellLanguage *
gspell_language_chooser_button_get_language (GspellLanguageChooser *chooser)
{
	GspellLanguageChooserButtonPrivate *priv;

	priv = gspell_language_chooser_button_get_instance_private (GSPELL_LANGUAGE_CHOOSER_BUTTON (chooser));

	return priv->language;
}

static void
gspell_language_chooser_button_set_language (GspellLanguageChooser *chooser,
					     const GspellLanguage  *language)
{
	GspellLanguageChooserButton *button;
	GspellLanguageChooserButtonPrivate *priv;

	button = GSPELL_LANGUAGE_CHOOSER_BUTTON (chooser);
	priv = gspell_language_chooser_button_get_instance_private (button);

	if (priv->language != language)
	{
		priv->language = language;

		update_button_label (button);

		g_object_notify (G_OBJECT (chooser), "language");
	}
}

static void
gspell_language_chooser_button_iface_init (GspellLanguageChooserInterface *iface)
{
	iface->get_language = gspell_language_chooser_button_get_language;
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
			g_value_set_pointer (value, (gpointer) gspell_language_chooser_button_get_language (chooser));
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
			gspell_language_chooser_button_set_language (chooser, g_value_get_pointer (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
dialog_response_after_cb (GtkDialog *dialog,
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

	priv->dialog = GSPELL_LANGUAGE_CHOOSER_DIALOG (gspell_language_chooser_dialog_new (parent,
											   priv->language));

	if (parent != NULL)
	{
		gtk_window_set_modal (GTK_WINDOW (priv->dialog),
				      gtk_window_get_modal (parent));
	}

	g_object_bind_property (priv->dialog, "language",
				button, "language",
				G_BINDING_DEFAULT);

	g_signal_connect_after (priv->dialog,
				"response",
				G_CALLBACK (dialog_response_after_cb),
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
					      priv->language);

	gtk_window_present (GTK_WINDOW (priv->dialog));
}

static void
gspell_language_chooser_button_class_init (GspellLanguageChooserButtonClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkButtonClass *button_class = GTK_BUTTON_CLASS (klass);

	object_class->get_property = gspell_language_chooser_button_get_property;
	object_class->set_property = gspell_language_chooser_button_set_property;

	button_class->clicked = gspell_language_chooser_button_clicked;

	g_object_class_override_property (object_class, PROP_LANGUAGE, "language");
}

static void
gspell_language_chooser_button_init (GspellLanguageChooserButton *button)
{
	update_button_label (button);
}

/**
 * gspell_language_chooser_button_new:
 * @current_language: a #GspellLanguage.
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

/* ex:set ts=8 noet: */
