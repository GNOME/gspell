/*
 * This file is part of gspell.
 *
 * Copyright 2002 - Paolo Maggi
 * Copyright 2015 - Sébastien Wilmet
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

#include "gspell-language-chooser-dialog.h"
#include "gspell-language-chooser.h"

/**
 * SECTION:language-chooser-dialog
 * @Short_description: Dialog to choose a spell language.
 * @Title: GspellLanguageChooserDialog
 * @See_also: #GspellLanguage.
 *
 * #GspellLanguageChooserDialog is a #GtkDialog to choose an available
 * #GspellLanguage. #GspellLanguageChooserDialog implements the
 * #GspellLanguageChooser interface.
 *
 * The #GspellLanguageChooser:language property is updated only when the Select
 * button is pressed or when a row is activated (e.g. with a double-click).
 */

enum
{
	COLUMN_LANGUAGE_NAME,
	COLUMN_LANGUAGE_POINTER,
	N_COLUMNS
};

enum
{
	PROP_0,
	PROP_LANGUAGE,
};

struct _GspellLanguageChooserDialog
{
	GtkDialog dialog;

	GtkTreeView *treeview;
	const GspellLanguage *language;
};

static void gspell_language_chooser_dialog_iface_init (GspellLanguageChooserInterface *iface);

G_DEFINE_TYPE_WITH_CODE (GspellLanguageChooserDialog,
			 gspell_language_chooser_dialog,
			 GTK_TYPE_DIALOG,
			 G_IMPLEMENT_INTERFACE (GSPELL_TYPE_LANGUAGE_CHOOSER,
						gspell_language_chooser_dialog_iface_init))

static void
scroll_to_selected (GtkTreeView *tree_view)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeIter iter;

	model = gtk_tree_view_get_model (tree_view);
	g_return_if_fail (model != NULL);

	selection = gtk_tree_view_get_selection (tree_view);

	if (gtk_tree_selection_get_selected (selection, NULL, &iter))
	{
		GtkTreePath *path;

		path = gtk_tree_model_get_path (model, &iter);
		g_return_if_fail (path != NULL);

		gtk_tree_view_scroll_to_cell (tree_view, path, NULL, TRUE, 1.0, 0.0);
		gtk_tree_path_free (path);
	}
}

static const GspellLanguage *
gspell_language_chooser_dialog_get_language (GspellLanguageChooser *chooser)
{
	GspellLanguageChooserDialog *dialog = GSPELL_LANGUAGE_CHOOSER_DIALOG (chooser);

	return dialog->language;
}

static void
gspell_language_chooser_dialog_set_language (GspellLanguageChooser *chooser,
					     const GspellLanguage  *language)
{
	GspellLanguageChooserDialog *dialog;
	GtkTreeModel *model;
	GtkTreeIter iter;

	dialog = GSPELL_LANGUAGE_CHOOSER_DIALOG (chooser);

	model = gtk_tree_view_get_model (dialog->treeview);

	if (!gtk_tree_model_get_iter_first (model, &iter))
	{
		goto warning;
	}

	do
	{
		const GspellLanguage *cur_lang;

		gtk_tree_model_get (model, &iter,
				    COLUMN_LANGUAGE_POINTER, &cur_lang,
				    -1);

		if (language == cur_lang)
		{
			GtkTreeSelection *selection;

			selection = gtk_tree_view_get_selection (dialog->treeview);
			gtk_tree_selection_select_iter (selection, &iter);
			scroll_to_selected (dialog->treeview);

			if (dialog->language != language)
			{
				dialog->language = language;
				g_object_notify (G_OBJECT (dialog), "language");
			}

			return;
		}
	}
	while (gtk_tree_model_iter_next (model, &iter));

warning:
	g_warning ("GspellLanguageChooserDialog: setting language failed, language not found.");
}

static void
gspell_language_chooser_dialog_iface_init (GspellLanguageChooserInterface *iface)
{
	iface->get_language = gspell_language_chooser_dialog_get_language;
	iface->set_language = gspell_language_chooser_dialog_set_language;
}

static void
gspell_language_chooser_dialog_get_property (GObject    *object,
					     guint       prop_id,
					     GValue     *value,
					     GParamSpec *pspec)
{
	GspellLanguageChooser *chooser = GSPELL_LANGUAGE_CHOOSER (object);

	switch (prop_id)
	{
		case PROP_LANGUAGE:
			g_value_set_pointer (value, (gpointer) gspell_language_chooser_dialog_get_language (chooser));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_language_chooser_dialog_set_property (GObject      *object,
					     guint         prop_id,
					     const GValue *value,
					     GParamSpec   *pspec)
{
	GspellLanguageChooser *chooser = GSPELL_LANGUAGE_CHOOSER (object);

	switch (prop_id)
	{
		case PROP_LANGUAGE:
			gspell_language_chooser_dialog_set_language (chooser, g_value_get_pointer (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_language_chooser_dialog_response (GtkDialog *gtk_dialog,
					 gint       response)
{
	GspellLanguageChooserDialog *dialog;
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;
	const GspellLanguage *lang;

	if (response != GTK_RESPONSE_OK)
	{
		return;
	}

	dialog = GSPELL_LANGUAGE_CHOOSER_DIALOG (gtk_dialog);

	selection = gtk_tree_view_get_selection (dialog->treeview);

	if (!gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		return;
	}

	gtk_tree_model_get (model, &iter,
			    COLUMN_LANGUAGE_POINTER, &lang,
			    -1);

	if (dialog->language != lang)
	{
		dialog->language = lang;
		g_object_notify (G_OBJECT (dialog), "language");
	}
}

static void
gspell_language_chooser_dialog_class_init (GspellLanguageChooserDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
	GtkDialogClass *dialog_class = GTK_DIALOG_CLASS (klass);

	object_class->get_property = gspell_language_chooser_dialog_get_property;
	object_class->set_property = gspell_language_chooser_dialog_set_property;

	dialog_class->response = gspell_language_chooser_dialog_response;

	g_object_class_override_property (object_class, PROP_LANGUAGE, "language");

	/* Bind class to template */
	gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/gspell/language-dialog.ui");
	gtk_widget_class_bind_template_child (widget_class, GspellLanguageChooserDialog, treeview);
}

static void
row_activated_cb (GtkTreeView          *tree_view,
		  GtkTreePath          *path,
		  GtkTreeViewColumn    *column,
		  GspellLanguageChooserDialog *dialog)
{
	gtk_dialog_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
}

static void
populate_language_list (GspellLanguageChooserDialog *dialog)
{
	GtkListStore *store;
	const GSList *available_langs;
	const GSList *l;

	store = GTK_LIST_STORE (gtk_tree_view_get_model (dialog->treeview));

	available_langs = gspell_checker_get_available_languages ();

	for (l = available_langs; l != NULL; l = l->next)
	{
		const GspellLanguage *lang = l->data;
		const gchar *name;
		GtkTreeIter iter;

		name = gspell_language_to_string (lang);

		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter,
				    COLUMN_LANGUAGE_NAME, name,
				    COLUMN_LANGUAGE_POINTER, lang,
				    -1);
	}
}

static void
gspell_language_chooser_dialog_init (GspellLanguageChooserDialog *dialog)
{
	GtkListStore *store;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;

	gtk_widget_init_template (GTK_WIDGET (dialog));

	store = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER);
	gtk_tree_view_set_model (dialog->treeview, GTK_TREE_MODEL (store));
	g_object_unref (store);

	/* Add the language column */
	column = gtk_tree_view_column_new ();
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_add_attribute (column, renderer,
					    "text", COLUMN_LANGUAGE_NAME);

	gtk_tree_view_append_column (dialog->treeview, column);

	gtk_tree_view_set_search_column (dialog->treeview, COLUMN_LANGUAGE_NAME);

	gtk_widget_grab_focus (GTK_WIDGET (dialog->treeview));

	populate_language_list (dialog);

	g_signal_connect (dialog->treeview,
			  "realize",
			  G_CALLBACK (scroll_to_selected),
			  dialog);

	g_signal_connect (dialog->treeview,
			  "row-activated",
			  G_CALLBACK (row_activated_cb),
			  dialog);
}

/**
 * gspell_language_chooser_dialog_new:
 * @parent: transient parent of the dialog.
 * @current_language: the #GspellLanguage to select initially.
 *
 * Returns: a new #GspellLanguageChooserDialog widget.
 */
GtkWidget *
gspell_language_chooser_dialog_new (GtkWindow            *parent,
				    const GspellLanguage *current_language)
{
	g_return_val_if_fail (GTK_IS_WINDOW (parent), NULL);

	return g_object_new (GSPELL_TYPE_LANGUAGE_CHOOSER_DIALOG,
			     "transient-for", parent,
			     "language", current_language,
			     NULL);
}

/* ex:set ts=8 noet: */
