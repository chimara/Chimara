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

#include <stdlib.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <libchimara/chimara-glk.h>
#include <libchimara/chimara-if.h>
#include <config.h>
#include "error.h"

GObject *load_object(const gchar *name);
static GtkTextTag *current_tag;
static GtkListStore *preferred_list;

static void style_tree_select_callback(GtkTreeSelection *selection, ChimaraGlk *glk);

/* Internal functions to convert from human-readable names in the config file
to enums and back. Later: replace with plugin functions. */
static ChimaraIFFormat
parse_format(const char *format)
{
	if(strcmp(format, "z5") == 0)
		return CHIMARA_IF_FORMAT_Z5;
	if(strcmp(format, "z6") == 0)
		return CHIMARA_IF_FORMAT_Z6;
	if(strcmp(format, "z8") == 0)
		return CHIMARA_IF_FORMAT_Z8;
	if(strcmp(format, "zblorb") == 0)
		return CHIMARA_IF_FORMAT_Z_BLORB;
	if(strcmp(format, "glulx") == 0)
		return CHIMARA_IF_FORMAT_GLULX;
	if(strcmp(format, "gblorb") == 0)
		return CHIMARA_IF_FORMAT_GLULX_BLORB;
	return CHIMARA_IF_FORMAT_NONE;
}

static const char *format_strings[CHIMARA_IF_NUM_FORMATS] = {
	"z5", "z6", "z8", "zblorb", "glulx", "gblorb"
};

static const char *format_to_string(ChimaraIFFormat format)
{
	if(format >= 0 && format < CHIMARA_IF_NUM_FORMATS)
		return format_strings[format];
	return "unknown";
}

static const char *format_display_strings[CHIMARA_IF_NUM_FORMATS] = {
	N_("Z-machine version 5"),
	N_("Z-machine version 6"),
	N_("Z-machine version 8"),
	N_("Z-machine Blorb file"),
	N_("Glulx"),
	N_("Glulx Blorb file")
};

static const char *
format_to_display_string(ChimaraIFFormat format)
{
	if(format >= 0 && format < CHIMARA_IF_NUM_FORMATS)
		return gettext(format_display_strings[format]);
	return _("Unknown");
}

static ChimaraIFInterpreter
parse_interpreter(const char *interp)
{
	if(strcmp(interp, "frotz") == 0)
		return CHIMARA_IF_INTERPRETER_FROTZ;
	if(strcmp(interp, "nitfol") == 0)
		return CHIMARA_IF_INTERPRETER_NITFOL;
	if(strcmp(interp, "glulxe") == 0)
		return CHIMARA_IF_INTERPRETER_GLULXE;
	if(strcmp(interp, "git") == 0)
		return CHIMARA_IF_INTERPRETER_GIT;
	return CHIMARA_IF_INTERPRETER_NONE;
}

static const char *interpreter_strings[CHIMARA_IF_NUM_INTERPRETERS] = {
	"frotz", "nitfol", "glulxe", "git"
};

static const char *
interpreter_to_string(ChimaraIFInterpreter interp)
{
	if(interp >= 0 && interp < CHIMARA_IF_NUM_INTERPRETERS)
		return interpreter_strings[interp];
	return "unknown";
}

static const char *interpreter_display_strings[CHIMARA_IF_NUM_INTERPRETERS] = {
	N_("Frotz"),
	N_("Nitfol"),
	N_("Glulxe"),
	N_("Git")
};

static const char *
interpreter_to_display_string(ChimaraIFInterpreter interp)
{
	if(interp >= 0 && interp < CHIMARA_IF_NUM_INTERPRETERS)
		return gettext(interpreter_display_strings[interp]);
	return _("Unknown");
}

/* Create the preferences dialog. */
void
preferences_create(ChimaraGlk *glk)
{
	/* Initialize the tree of style names */
	GtkTreeStore *style_list = GTK_TREE_STORE( load_object("style-list") );
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

	/* Set selection mode to single select */
	GtkTreeView *view = GTK_TREE_VIEW( load_object("style-treeview") );
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

	/* Populate the list of available interpreters */
	GtkListStore *interp_list = GTK_LIST_STORE( load_object("available_interpreters") );
	unsigned int count;
	GtkTreeIter tree_iter;
	for(count = 0; count < CHIMARA_IF_NUM_INTERPRETERS; count++) {
		gtk_list_store_append(interp_list, &tree_iter);
		gtk_list_store_set(interp_list, &tree_iter,
			0, interpreter_to_display_string(count),
			-1);
	}

	/* Get the list of preferred interpreters from the preferences */
	GVariantIter *iter;
	char *format, *plugin;
	g_settings_get(prefs_settings, "preferred-interpreters", "a{ss}", &iter);
	while(g_variant_iter_loop(iter, "{ss}", &format, &plugin)) {
		ChimaraIFFormat format_num = parse_format(format);
		if(format_num == CHIMARA_IF_FORMAT_NONE)
			continue;
		ChimaraIFInterpreter interp_num = parse_interpreter(plugin);
		if(interp_num == CHIMARA_IF_INTERPRETER_NONE)
			continue;
		chimara_if_set_preferred_interpreter(CHIMARA_IF(glk), format_num, interp_num);
	}
	g_variant_iter_free(iter);

	/* Display it all in the list */
	preferred_list = GTK_LIST_STORE( load_object("interpreters") );
	for(count = 0; count < CHIMARA_IF_NUM_FORMATS; count++) {
		gtk_list_store_append(preferred_list, &tree_iter);
		gtk_list_store_set(preferred_list, &tree_iter,
			0, format_to_display_string(count),
			1, interpreter_to_display_string(chimara_if_get_preferred_interpreter(CHIMARA_IF(glk), count)),
			-1);
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
on_toggle_left(GtkToggleButton *button, ChimaraGlk *glk) {
	/* No nothing if the button is deactivated */
	if( !gtk_toggle_button_get_active(button) )
		return;
	g_object_set(current_tag, "justification", GTK_JUSTIFY_LEFT, "justification-set", TRUE, NULL);
	chimara_glk_update_style(glk);
}

void
on_toggle_center(GtkToggleButton *button, ChimaraGlk *glk) {
	if( !gtk_toggle_button_get_active(button) )
		return;
	g_object_set(current_tag, "justification", GTK_JUSTIFY_CENTER, "justification-set", TRUE, NULL);
	chimara_glk_update_style(glk);
}

void
on_toggle_right(GtkToggleButton *button, ChimaraGlk *glk) {
	if( !gtk_toggle_button_get_active(button) )
		return;
	g_object_set(current_tag, "justification", GTK_JUSTIFY_RIGHT, "justification-set", TRUE, NULL);
	chimara_glk_update_style(glk);
}

void
on_toggle_justify(GtkToggleButton *button, ChimaraGlk *glk) {
	if( !gtk_toggle_button_get_active(button) )
		return;
	g_object_set(current_tag, "justification", GTK_JUSTIFY_FILL, "justification-set", TRUE, NULL);
	chimara_glk_update_style(glk);
}

void
on_toggle_bold(GtkToggleButton *button, ChimaraGlk *glk) {
	if( gtk_toggle_button_get_active(button) )
		g_object_set(current_tag, "weight", PANGO_WEIGHT_BOLD, "weight-set", TRUE, NULL);
	else
		g_object_set(current_tag, "weight", PANGO_WEIGHT_NORMAL, "weight-set", TRUE, NULL);

	chimara_glk_update_style(glk);
}

void
on_toggle_italic(GtkToggleButton *button, ChimaraGlk *glk) {
	if( gtk_toggle_button_get_active(button) )
		g_object_set(current_tag, "style", PANGO_STYLE_ITALIC, "style-set", TRUE, NULL);
	else
		g_object_set(current_tag, "style", PANGO_STYLE_NORMAL, "style-set", TRUE, NULL);

	chimara_glk_update_style(glk);
}

void
on_toggle_underline(GtkToggleButton *button, ChimaraGlk *glk) {
	if( gtk_toggle_button_get_active(button) )
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

void
on_interpreter_cell_changed(GtkCellRendererCombo *combo, char *path_string, GtkTreeIter *new_iter, ChimaraGlk *glk)
{
	unsigned int format, interpreter;
	format = (unsigned int)strtol(path_string, NULL, 10);
	GtkTreeModel *combo_model;
	g_object_get(combo, "model", &combo_model, NULL);
	char *combo_string = gtk_tree_model_get_string_from_iter(combo_model, new_iter);
	interpreter = (unsigned int)strtol(combo_string, NULL, 10);
	g_free(combo_string);

	chimara_if_set_preferred_interpreter(CHIMARA_IF(glk), format, interpreter);

	/* Display the new setting in the list */
	GtkTreeIter iter;
	GtkTreePath *path = gtk_tree_path_new_from_string(path_string);
	gtk_tree_model_get_iter(GTK_TREE_MODEL(preferred_list), &iter, path);
	gtk_tree_path_free(path);
	gtk_list_store_set(preferred_list, &iter,
		1, interpreter_to_display_string(interpreter),
		-1);

	/* Save the new settings in the preferences file */
	extern GSettings *prefs_settings;
	GVariantBuilder *builder = g_variant_builder_new( G_VARIANT_TYPE("a{ss}") );
	unsigned int count;
	for(count = 0; count < CHIMARA_IF_NUM_FORMATS; count++) {
		g_variant_builder_add(builder, "{ss}",
			format_to_string(count),
			interpreter_to_string(chimara_if_get_preferred_interpreter(CHIMARA_IF(glk), count)));
	}
	g_settings_set(prefs_settings, "preferred-interpreters", "a{ss}", builder);
	g_variant_builder_unref(builder);
}