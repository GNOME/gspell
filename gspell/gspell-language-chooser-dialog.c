/* SPDX-FileCopyrightText: 2002 - Paolo Maggi
 * SPDX-FileCopyrightText: 2015, 2016 - Sébastien Wilmet
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "gspell-config.h"
#include "gspell-language-chooser-dialog.h"
#include "gspell-language-chooser.h"

/**
 * SECTION:language-chooser-dialog
 * @Short_description: Dialog to choose a GspellLanguage
 * @Title: GspellLanguageChooserDialog
 * @See_also: #GspellLanguage, #GspellLanguageChooser
 *
 * #GspellLanguageChooserDialog is a #GtkDialog to choose an available
 * #GspellLanguage. #GspellLanguageChooserDialog implements the
 * #GspellLanguageChooser interface.
 *
 * The #GspellLanguageChooser:language and #GspellLanguageChooser:language-code
 * properties are updated only when the Select button is pressed or when a row
 * is activated (e.g. with a double-click).
 *
 * The application is responsible to destroy the dialog, typically when the
 * #GtkDialog::response signal has been received or gtk_dialog_run() has
 * returned.
 */

typedef struct _GspellLanguageChooserDialogPrivate GspellLanguageChooserDialogPrivate;

struct _GspellLanguageChooserDialogPrivate
{
	GtkTreeView *treeview;
	const GspellLanguage *language;
	guint default_language : 1;
};

enum
{
	PROP_0,
	PROP_LANGUAGE,
	PROP_LANGUAGE_CODE,
};

enum
{
	COLUMN_LANGUAGE_NAME,
	COLUMN_LANGUAGE_POINTER,
	N_COLUMNS
};

static void gspell_language_chooser_dialog_iface_init (GspellLanguageChooserInterface *iface);

G_DEFINE_TYPE_WITH_CODE (GspellLanguageChooserDialog,
			 gspell_language_chooser_dialog,
			 GTK_TYPE_DIALOG,
			 G_ADD_PRIVATE (GspellLanguageChooserDialog)
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
gspell_language_chooser_dialog_get_language_full (GspellLanguageChooser *chooser,
						  gboolean              *default_language)
{
	GspellLanguageChooserDialog *dialog;
	GspellLanguageChooserDialogPrivate *priv;

	dialog = GSPELL_LANGUAGE_CHOOSER_DIALOG (chooser);
	priv = gspell_language_chooser_dialog_get_instance_private (dialog);

	if (default_language != NULL)
	{
		*default_language = priv->default_language;
	}

	return priv->language;
}

static void
gspell_language_chooser_dialog_set_language (GspellLanguageChooser *chooser,
					     const GspellLanguage  *language_param)
{
	GspellLanguageChooserDialog *dialog;
	GspellLanguageChooserDialogPrivate *priv;
	const GspellLanguage *language;
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;

	dialog = GSPELL_LANGUAGE_CHOOSER_DIALOG (chooser);
	priv = gspell_language_chooser_dialog_get_instance_private (dialog);

	language = language_param;

	if (language == NULL)
	{
		language = gspell_language_get_default ();
	}

	selection = gtk_tree_view_get_selection (priv->treeview);

	if (language == NULL)
	{
		gboolean notify_language_code = FALSE;

		gtk_tree_selection_unselect_all (selection);

		/* Update first the full state before notifying the properties. */
		if (!priv->default_language)
		{
			priv->default_language = TRUE;
			notify_language_code = TRUE;
		}

		if (priv->language != NULL)
		{
			priv->language = NULL;
			g_object_notify (G_OBJECT (dialog), "language");
		}

		if (notify_language_code)
		{
			g_object_notify (G_OBJECT (dialog), "language-code");
		}

		return;
	}

	model = gtk_tree_view_get_model (priv->treeview);

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
			gboolean default_language;
			gboolean notify_language_code = FALSE;

			gtk_tree_selection_select_iter (selection, &iter);
			scroll_to_selected (priv->treeview);

			/* Update first the full state before notifying the properties. */

			default_language = language_param == NULL;

			if (priv->default_language != default_language)
			{
				priv->default_language = default_language;
				notify_language_code = TRUE;
			}

			if (priv->language != language)
			{
				priv->language = language;
				g_object_notify (G_OBJECT (dialog), "language");
				notify_language_code = TRUE;
			}

			if (notify_language_code)
			{
				g_object_notify (G_OBJECT (dialog), "language-code");
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
	iface->get_language_full = gspell_language_chooser_dialog_get_language_full;
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
gspell_language_chooser_dialog_set_property (GObject      *object,
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
gspell_language_chooser_dialog_constructed (GObject *object)
{
	gint use_header_bar;

	if (G_OBJECT_CLASS (gspell_language_chooser_dialog_parent_class)->constructed != NULL)
	{
		G_OBJECT_CLASS (gspell_language_chooser_dialog_parent_class)->constructed (object);
	}

	g_object_get (object,
		      "use-header-bar", &use_header_bar,
		      NULL);

	if (use_header_bar)
	{
		/* Avoid the title being ellipsized, if possible (for translations too). */
		gtk_widget_set_size_request (GTK_WIDGET (object), 450, -1);
	}
}

static void
dialog_response_cb (GtkDialog *gtk_dialog,
		    gint       response)
{
	GspellLanguageChooserDialog *dialog;
	GspellLanguageChooserDialogPrivate *priv;
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;
	const GspellLanguage *lang;
	gboolean notify_language_code = FALSE;

	if (response != GTK_RESPONSE_OK)
	{
		return;
	}

	dialog = GSPELL_LANGUAGE_CHOOSER_DIALOG (gtk_dialog);
	priv = gspell_language_chooser_dialog_get_instance_private (dialog);

	selection = gtk_tree_view_get_selection (priv->treeview);

	if (!gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		return;
	}

	gtk_tree_model_get (model, &iter,
			    COLUMN_LANGUAGE_POINTER, &lang,
			    -1);

	/* Update first the full state before notifying the properties. */

	if (priv->default_language)
	{
		priv->default_language = FALSE;
		notify_language_code = TRUE;
	}

	if (priv->language != lang)
	{
		priv->language = lang;
		g_object_notify (G_OBJECT (dialog), "language");
		notify_language_code = TRUE;
	}

	if (notify_language_code)
	{
		g_object_notify (G_OBJECT (dialog), "language-code");
	}
}

static void
gspell_language_chooser_dialog_class_init (GspellLanguageChooserDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->get_property = gspell_language_chooser_dialog_get_property;
	object_class->set_property = gspell_language_chooser_dialog_set_property;
	object_class->constructed = gspell_language_chooser_dialog_constructed;

	g_object_class_override_property (object_class, PROP_LANGUAGE, "language");
	g_object_class_override_property (object_class, PROP_LANGUAGE_CODE, "language-code");

	/* Bind class to template */
	gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/gspell/language-dialog.ui");
	gtk_widget_class_bind_template_child_private (widget_class, GspellLanguageChooserDialog, treeview);
}

static void
row_activated_cb (GtkTreeView                 *tree_view,
		  GtkTreePath                 *path,
		  GtkTreeViewColumn           *column,
		  GspellLanguageChooserDialog *dialog)
{
	gtk_dialog_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
}

static void
populate_language_list (GspellLanguageChooserDialog *dialog)
{
	GspellLanguageChooserDialogPrivate *priv;
	GtkListStore *store;
	const GList *available_langs;
	const GList *l;

	priv = gspell_language_chooser_dialog_get_instance_private (dialog);

	store = GTK_LIST_STORE (gtk_tree_view_get_model (priv->treeview));

	available_langs = gspell_language_get_available ();

	for (l = available_langs; l != NULL; l = l->next)
	{
		const GspellLanguage *lang = l->data;
		const gchar *name;
		GtkTreeIter iter;

		name = gspell_language_get_name (lang);

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
	GspellLanguageChooserDialogPrivate *priv;
	GtkListStore *store;
	GtkTreeSelection *selection;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;

	priv = gspell_language_chooser_dialog_get_instance_private (dialog);

	priv->default_language = TRUE;

	gtk_widget_init_template (GTK_WIDGET (dialog));

	store = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING, GSPELL_TYPE_LANGUAGE);
	gtk_tree_view_set_model (priv->treeview, GTK_TREE_MODEL (store));
	g_object_unref (store);

	selection = gtk_tree_view_get_selection (priv->treeview);
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);

	/* Add the language column */
	column = gtk_tree_view_column_new ();
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_add_attribute (column, renderer,
					    "text", COLUMN_LANGUAGE_NAME);

	gtk_tree_view_append_column (priv->treeview, column);

	gtk_tree_view_set_search_column (priv->treeview, COLUMN_LANGUAGE_NAME);

	gtk_widget_grab_focus (GTK_WIDGET (priv->treeview));

	populate_language_list (dialog);

	g_signal_connect (priv->treeview,
			  "realize",
			  G_CALLBACK (scroll_to_selected),
			  dialog);

	g_signal_connect (priv->treeview,
			  "row-activated",
			  G_CALLBACK (row_activated_cb),
			  dialog);

	/* Be the first to receive the signal, to notify the property before the
	 * dialog gets destroyed by the app.
	 * It means that the behavior of the dialog is not fully overridable by
	 * an app, but I don't think it's really important and worst case a new
	 * GspellLanguageChooser implementation can be written. If our
	 * ::response handler was done in the object method handler, apps would
	 * need to connect to the ::response signal with the after flag to
	 * destroy the dialog, which is less convenient and needs more
	 * documentation.
	 */
	g_signal_connect (dialog,
			  "response",
			  G_CALLBACK (dialog_response_cb),
			  NULL);
}

/**
 * gspell_language_chooser_dialog_new:
 * @parent: transient parent of the dialog.
 * @current_language: (nullable): the #GspellLanguage to select initially, or
 *   %NULL to pick the default language.
 * @flags: #GtkDialogFlags
 *
 * Returns: a new #GspellLanguageChooserDialog widget.
 */
GtkWidget *
gspell_language_chooser_dialog_new (GtkWindow            *parent,
				    const GspellLanguage *current_language,
				    GtkDialogFlags        flags)
{
	g_return_val_if_fail (GTK_IS_WINDOW (parent), NULL);

	return g_object_new (GSPELL_TYPE_LANGUAGE_CHOOSER_DIALOG,
			     "transient-for", parent,
			     "language", current_language,
			     "modal", (flags & GTK_DIALOG_MODAL) != 0,
			     "destroy-with-parent", (flags & GTK_DIALOG_DESTROY_WITH_PARENT) != 0,
			     "use-header-bar", (flags & GTK_DIALOG_USE_HEADER_BAR) != 0,
			     NULL);
}
