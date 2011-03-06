/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * callbacks.c
 * Copyright (C) Philip en Marijn 2008 <>
 * 
 * preferences.c is free software copyrighted by Philip en Marijn.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name ``Philip en Marijn'' nor the name of any other
 *    contributor may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 
 * preferences.c IS PROVIDED BY Philip en Marijn ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Philip en Marijn OR ANY OTHER CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <libchimara/chimara-glk.h>
#include <libchimara/chimara-if.h>
#include <config.h>
#include "error.h"

GObject *load_object(const gchar *name);
static GtkTextTag *current_tag;

static void style_tree_select_callback(GtkTreeSelection *selection, ChimaraGlk *glk);

/* Create the preferences dialog. */
void
preferences_create(ChimaraGlk *glk)
{
	/* Initialize the tree of style names */
	GtkTreeStore *style_list = gtk_tree_store_new(1, G_TYPE_STRING);
	GtkTreeIter buffer, grid, buffer_child, grid_child;

	gtk_tree_store_append(style_list, &buffer, NULL);
	gtk_tree_store_append(style_list, &grid, NULL);
	gtk_tree_store_set(style_list, &buffer, 0, "Text buffer", -1);
	gtk_tree_store_set(style_list, &grid, 0, "Text grid", -1);

	int i;
    gint num_tags = chimara_glk_get_num_tag_names(glk);
	const gchar **tag_names = chimara_glk_get_tag_names(glk);
	for(i=0; i<num_tags; i++) {
		gtk_tree_store_append(style_list, &buffer_child, &buffer);
		gtk_tree_store_append(style_list, &grid_child, &grid);
		gtk_tree_store_set(style_list, &buffer_child, 0, tag_names[i], -1);
		gtk_tree_store_set(style_list, &grid_child, 0, tag_names[i], -1);
	}

	/* Attach the model to the treeview */
	GtkTreeView *view = GTK_TREE_VIEW( load_object("style-treeview") );
	gtk_tree_view_set_model(view, GTK_TREE_MODEL(style_list));
	g_object_unref(style_list);

	/* Set the columns */
	GtkTreeViewColumn *column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, "Style Name");
	gtk_tree_view_append_column(view, column);

	/* Set the renderers */
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_add_attribute(column, renderer, "text", 0);

	/* Set selection mode to single select */
	GtkTreeSelection *selection = gtk_tree_view_get_selection(view);
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);

	g_signal_connect(selection, "changed", G_CALLBACK(style_tree_select_callback), glk);

	/* Bind the preferences to the entries in the preferences file */
	extern GSettings *prefs_settings;
	GObject *flep = G_OBJECT( load_object("flep") );
	g_settings_bind(prefs_settings, "flep", flep, "active", G_SETTINGS_BIND_DEFAULT);
	GtkFileChooser *blorb_chooser = GTK_FILE_CHOOSER( load_object("blorb_file_chooser") );
	char *filename;
	g_settings_get(prefs_settings, "resource-path", "ms", &filename);
	if(filename) {
		gtk_file_chooser_set_filename(blorb_chooser, filename);
		g_free(filename);
	}
}

static void
style_tree_select_callback(GtkTreeSelection *selection, ChimaraGlk *glk)
{
	GtkTreeIter child, parent;
	gchar *child_name, *parent_name;
	GtkTreeModel *model;

	if( !gtk_tree_selection_get_selected(selection, &model, &child) )
		return;

	gtk_tree_model_get(model, &child, 0, &child_name, -1);
		
	if( !gtk_tree_model_iter_parent(model, &parent, &child) )
		return;

	gtk_tree_model_get(model, &parent, 0, &parent_name, -1);
	if( !strcmp(parent_name, "Text buffer") ) 
		current_tag = chimara_glk_get_tag(glk, CHIMARA_GLK_TEXT_BUFFER, child_name);
	else
		current_tag = chimara_glk_get_tag(glk, CHIMARA_GLK_TEXT_GRID, child_name);
}

void
on_toggle_left(GtkToggleToolButton *button, ChimaraGlk *glk) {
	/* No nothing if the button is deactivated */
	if( !gtk_toggle_tool_button_get_active(button) )
		return;

	/* Untoggle other alignment options */
	GtkToggleToolButton *center = GTK_TOGGLE_TOOL_BUTTON(load_object("toolbutton-center"));
	GtkToggleToolButton *right = GTK_TOGGLE_TOOL_BUTTON(load_object("toolbutton-right"));
	GtkToggleToolButton *justify = GTK_TOGGLE_TOOL_BUTTON(load_object("toolbutton-justify"));
	gtk_toggle_tool_button_set_active(center, FALSE);
	gtk_toggle_tool_button_set_active(right, FALSE);
	gtk_toggle_tool_button_set_active(justify, FALSE);

	g_object_set(current_tag, "justification", GTK_JUSTIFY_LEFT, "justification-set", TRUE, NULL);
	chimara_glk_update_style(glk);
}

void
on_toggle_center(GtkToggleToolButton *button, ChimaraGlk *glk) {
	if( !gtk_toggle_tool_button_get_active(button) )
		return;

	/* Untoggle other alignment options */
	GtkToggleToolButton *left = GTK_TOGGLE_TOOL_BUTTON(load_object("toolbutton-left"));
	GtkToggleToolButton *right = GTK_TOGGLE_TOOL_BUTTON(load_object("toolbutton-right"));
	GtkToggleToolButton *justify = GTK_TOGGLE_TOOL_BUTTON(load_object("toolbutton-justify"));
	gtk_toggle_tool_button_set_active(left, FALSE);
	gtk_toggle_tool_button_set_active(right, FALSE);
	gtk_toggle_tool_button_set_active(justify, FALSE);

	g_object_set(current_tag, "justification", GTK_JUSTIFY_CENTER, "justification-set", TRUE, NULL);
	chimara_glk_update_style(glk);
}

void
on_toggle_right(GtkToggleToolButton *button, ChimaraGlk *glk) {
	if( !gtk_toggle_tool_button_get_active(button) )
		return;

	/* Untoggle other alignment options */
	GtkToggleToolButton *left = GTK_TOGGLE_TOOL_BUTTON(load_object("toolbutton-left"));
	GtkToggleToolButton *center = GTK_TOGGLE_TOOL_BUTTON(load_object("toolbutton-center"));
	GtkToggleToolButton *justify = GTK_TOGGLE_TOOL_BUTTON(load_object("toolbutton-justify"));
	gtk_toggle_tool_button_set_active(left, FALSE);
	gtk_toggle_tool_button_set_active(center, FALSE);
	gtk_toggle_tool_button_set_active(justify, FALSE);

	g_object_set(current_tag, "justification", GTK_JUSTIFY_RIGHT, "justification-set", TRUE, NULL);
	chimara_glk_update_style(glk);
}

void
on_toggle_justify(GtkToggleToolButton *button, ChimaraGlk *glk) {
	if( !gtk_toggle_tool_button_get_active(button) )
		return;

	/* Untoggle other alignment options */
	GtkToggleToolButton *left = GTK_TOGGLE_TOOL_BUTTON(load_object("toolbutton-left"));
	GtkToggleToolButton *center = GTK_TOGGLE_TOOL_BUTTON(load_object("toolbutton-center"));
	GtkToggleToolButton *right = GTK_TOGGLE_TOOL_BUTTON(load_object("toolbutton-right"));
	gtk_toggle_tool_button_set_active(left, FALSE);
	gtk_toggle_tool_button_set_active(center, FALSE);
	gtk_toggle_tool_button_set_active(right, FALSE);

	g_object_set(current_tag, "justification", GTK_JUSTIFY_FILL, "justification-set", TRUE, NULL);
	chimara_glk_update_style(glk);
}

void
on_toggle_bold(GtkToggleToolButton *button, ChimaraGlk *glk) {
	if( gtk_toggle_tool_button_get_active(button) )
		g_object_set(current_tag, "weight", PANGO_WEIGHT_BOLD, "weight-set", TRUE, NULL);
	else
		g_object_set(current_tag, "weight", PANGO_WEIGHT_NORMAL, "weight-set", TRUE, NULL);

	chimara_glk_update_style(glk);
}

void
on_toggle_italic(GtkToggleToolButton *button, ChimaraGlk *glk) {
	if( gtk_toggle_tool_button_get_active(button) )
		g_object_set(current_tag, "style", PANGO_STYLE_ITALIC, "style-set", TRUE, NULL);
	else
		g_object_set(current_tag, "style", PANGO_STYLE_NORMAL, "style-set", TRUE, NULL);

	chimara_glk_update_style(glk);
}

void
on_toggle_underline(GtkToggleToolButton *button, ChimaraGlk *glk) {
	if( gtk_toggle_tool_button_get_active(button) )
		g_object_set(current_tag, "underline", PANGO_UNDERLINE_SINGLE, "underline-set", TRUE, NULL);
	else
		g_object_set(current_tag, "underline", PANGO_UNDERLINE_NONE, "underline-set", TRUE, NULL);

	chimara_glk_update_style(glk);
}

void
on_foreground_color_set(GtkColorButton *button, ChimaraGlk *glk)
{
	GdkColor color;
    gtk_color_button_get_color(button, &color);
	g_object_set(current_tag, "foreground-gdk", &color, "foreground-set", TRUE, NULL);
	chimara_glk_update_style(glk);
}

void
on_background_color_set(GtkColorButton *button, ChimaraGlk *glk)
{
	GdkColor color;
    gtk_color_button_get_color(button, &color);
	g_object_set(current_tag, "background-gdk", &color, "background-set", TRUE, NULL);
	chimara_glk_update_style(glk);
}

void
on_font_set(GtkFontButton *button, ChimaraGlk *glk)
{
	const gchar *font_name = gtk_font_button_get_font_name(button);
	PangoFontDescription *font_description = pango_font_description_from_string(font_name);
	g_object_set(current_tag, "font-desc", font_description, NULL);
	chimara_glk_update_style(glk);
}

void
on_resource_file_set(GtkFileChooserButton *button, ChimaraGlk *glk)
{
	extern GSettings *prefs_settings;
	char *filename = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER(button) );
	g_settings_set(prefs_settings, "resource-path", "ms", filename);
	g_free(filename);
}
