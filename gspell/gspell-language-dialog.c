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

#include "gspell-language-dialog.h"

/**
 * SECTION:spell-language-dialog
 * @Short_description: Dialog to choose a spell language.
 * @Title: GspellLanguageDialog
 * @See_also: #GspellCheckerLanguage.
 *
 * #GspellLanguageDialog is a #GtkDialog to choose an available
 * #GspellCheckerLanguage.
 */

enum
{
	COLUMN_LANGUAGE_NAME,
	COLUMN_LANGUAGE_POINTER,
	N_COLUMNS
};

struct _GspellLanguageDialog
{
	GtkDialog dialog;

	GtkTreeView *treeview;
};

G_DEFINE_TYPE (GspellLanguageDialog, gspell_language_dialog, GTK_TYPE_DIALOG)

static void
gspell_language_dialog_class_init (GspellLanguageDialogClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	/* Bind class to template */
	gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/gspell/language-dialog.ui");
	gtk_widget_class_bind_template_child (widget_class, GspellLanguageDialog, treeview);
}

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

static void
row_activated_cb (GtkTreeView              *tree_view,
		  GtkTreePath              *path,
		  GtkTreeViewColumn        *column,
		  GspellLanguageDialog *dialog)
{
	gtk_dialog_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
}

static void
gspell_language_dialog_init (GspellLanguageDialog *dialog)
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

	g_signal_connect (dialog->treeview,
			  "realize",
			  G_CALLBACK (scroll_to_selected),
			  dialog);

	g_signal_connect (dialog->treeview,
			  "row-activated",
			  G_CALLBACK (row_activated_cb),
			  dialog);
}

static void
populate_language_list (GspellLanguageDialog        *dialog,
			const GspellCheckerLanguage *cur_lang)
{
	GtkListStore *store;
	const GSList *available_langs;
	const GSList *l;

	store = GTK_LIST_STORE (gtk_tree_view_get_model (dialog->treeview));

	available_langs = gspell_checker_get_available_languages ();

	for (l = available_langs; l != NULL; l = l->next)
	{
		const GspellCheckerLanguage *lang = l->data;
		const gchar *name;
		GtkTreeIter iter;

		name = gspell_checker_language_to_string (lang);

		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter,
				    COLUMN_LANGUAGE_NAME, name,
				    COLUMN_LANGUAGE_POINTER, lang,
				    -1);

		if (lang == cur_lang)
		{
			GtkTreeSelection *selection;

			selection = gtk_tree_view_get_selection (dialog->treeview);

			gtk_tree_selection_select_iter (selection, &iter);
		}
	}
}

/**
 * gspell_language_dialog_new:
 * @parent: transient parent of the dialog.
 * @cur_lang: the #GspellCheckerLanguage to select initially.
 *
 * Returns: a new #GspellLanguageDialog widget.
 */
GtkWidget *
gspell_language_dialog_new (GtkWindow                       *parent,
			    const GspellCheckerLanguage *cur_lang)
{
	GspellLanguageDialog *dialog;

	g_return_val_if_fail (GTK_IS_WINDOW (parent), NULL);

	dialog = g_object_new (GSPELL_TYPE_LANGUAGE_DIALOG,
			       "transient-for", parent,
			       NULL);

	populate_language_list (dialog, cur_lang);

	return GTK_WIDGET (dialog);
}

/**
 * gspell_language_dialog_get_selected_language:
 * @dialog: a #GspellLanguageDialog.
 *
 * Returns: the currently selected language.
 */
const GspellCheckerLanguage *
gspell_language_dialog_get_selected_language (GspellLanguageDialog *dialog)
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;
	const GspellCheckerLanguage *lang;

	selection = gtk_tree_view_get_selection (dialog->treeview);

	if (!gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		return NULL;
	}

	gtk_tree_model_get (model, &iter,
			    COLUMN_LANGUAGE_POINTER, &lang,
			    -1);

	return lang;
}

/* ex:set ts=8 noet: */
