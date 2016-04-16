#include <stdio.h>
#include <string.h>
#include <math.h>
#include "chimara-glk-private.h"
#include "glk.h"
#include "style.h"
#include "magic.h"
#include "stream.h"
#include "strio.h"

extern GPrivate glk_data_key;

static gboolean style_accept(GScanner *scanner, GTokenType token);
static gboolean style_accept_style_selector(GScanner *scanner, ChimaraGlk *glk);
static gboolean style_accept_style_hint(GScanner *scanner, GtkTextTag *current_tag);
static void style_copy_tag_to_textbuffer(gpointer key, gpointer tag, gpointer target_table);
static void text_tag_to_attr_list(GtkTextTag *tag, PangoAttrList *list);
GtkTextTag* gtk_text_tag_copy(GtkTextTag *tag);
static void style_cascade_colors(GtkTextTag *tag, GtkTextTag *glk_tag, GtkTextTag *default_tag, GdkRGBA **foreground, GdkRGBA **background);

/**
 * glk_set_style:
 * @styl: The style to apply
 *
 * Changes the style of the current output stream. @styl should be one of the
 * `style_` constants.
 * However, any value is actually legal; if the interpreter does not recognize
 * the style value, it will treat it as %style_Normal.
 * <note><para>
 *  This policy allows for the future definition of styles without breaking old
 *  Glk libraries.
 * </para></note>
 */
void
glk_set_style(glui32 styl)
{
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
	g_return_if_fail(glk_data->current_stream != NULL);
	glk_set_style_stream(glk_data->current_stream, styl);
}

/* The first 11 tag names must correspond to the first 11 glk tag names as defined below */
static const gchar* TAG_NAMES[] = {
	"normal",
	"emphasized",
	"preformatted",
	"header",
	"subheader",
	"alert",
	"note",
	"block-quote",
	"input",
	"user1",
	"user2",
	"hyperlink",
	"default"
};

/* The first 11 glk tag names must correspond to the first 11 tag names as defined above */
static const gchar* GLK_TAG_NAMES[] = {
	"glk-normal",
	"glk-emphasized",
	"glk-preformatted",
	"glk-header",
	"glk-subheader",
	"glk-alert",
	"glk-note",
	"glk-block-quote",
	"glk-input",
	"glk-user1",
	"glk-user2"
};

const gchar**
style_get_tag_names()
{
	return TAG_NAMES;
}

/* Internal function: mapping from style enum to tag name */
static const gchar*
get_tag_name(glui32 style)
{
	if(style >= CHIMARA_NUM_STYLES) {
		WARNING("Unsupported style");
		return "normal";
	} else {
		return (gchar*) TAG_NAMES[style];
	}
}

/* Internal function: mapping from glk style enum to tag name */
static const gchar*
get_glk_tag_name(glui32 style)
{
	if(style >= style_NUMSTYLES) {
		WARNING("Unsupported style");
		return "normal";
	} else {
		return (gchar*) GLK_TAG_NAMES[style];
	}
}

/** 
 * glk_set_style_stream:
 * @str: Output stream to change the style of
 * @styl: The style to apply
 *
 * This changes the style of the stream @str. See glk_set_style().
 */
void
glk_set_style_stream(strid_t str, glui32 styl) {
#ifdef DEBUG_STYLES
	g_printf("glk_set_style(str->rock=%d, styl=%d)\n", str->rock, styl);
#endif

	if(str->window == NULL)
		return;

	flush_window_buffer(str->window);
	str->style = (gchar*) get_tag_name(styl);
	str->glk_style = (gchar*) get_glk_tag_name(styl);
}

/* Internal function: call this to initialize the default styles to a textbuffer. */
void
style_init_textbuffer(GtkTextBuffer *buffer)
{
	g_return_if_fail(buffer != NULL);

	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);

	/* Place the default text tags in the textbuffer's tag table */
	g_hash_table_foreach(glk_data->styles->text_buffer, style_copy_tag_to_textbuffer, gtk_text_buffer_get_tag_table(buffer));

	/* Copy the override text tags to the textbuffers's tag table */
	g_hash_table_foreach(glk_data->glk_styles->text_buffer, style_copy_tag_to_textbuffer, gtk_text_buffer_get_tag_table(buffer));

	/* Assign the 'default' tag the lowest priority */
	gtk_text_tag_set_priority( gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(buffer), "default"), 0 );
}


/* Internal function: call this to initialize the default styles to a textgrid. */
void
style_init_textgrid(GtkTextBuffer *buffer)
{
	g_return_if_fail(buffer != NULL);

	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);

	/* Place the default text tags in the textbuffer's tag table */
	g_hash_table_foreach(glk_data->styles->text_grid, style_copy_tag_to_textbuffer, gtk_text_buffer_get_tag_table(buffer));

	/* Copy the current text tags to the textbuffers's tag table */
	g_hash_table_foreach(glk_data->glk_styles->text_grid, style_copy_tag_to_textbuffer, gtk_text_buffer_get_tag_table(buffer));

	/* Assign the 'default' tag the lowest priority */
	gtk_text_tag_set_priority( gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(buffer), "default"), 0 );
}

/* Internal function used to iterate over a style table, copying it */
static void
style_copy_tag_to_textbuffer(gpointer key, gpointer tag, gpointer target_table)
{
	g_return_if_fail(key != NULL);
	g_return_if_fail(tag != NULL);
	g_return_if_fail(target_table != NULL);

	gtk_text_tag_table_add(target_table, gtk_text_tag_copy( GTK_TEXT_TAG(tag) ));
}

/* Internal function that copies a text tag */
GtkTextTag *
gtk_text_tag_copy(GtkTextTag *tag)
{
	GtkTextTag *copy;
	char *tag_name;
	GParamSpec **properties;
	unsigned nprops, count;

	g_return_val_if_fail(tag != NULL, NULL);

	g_object_get(tag, "name", &tag_name, NULL);
	copy = gtk_text_tag_new(tag_name);
	g_free(tag_name);

	/* Copy all the original tag's properties to the new tag */
	properties = g_object_class_list_properties( G_OBJECT_GET_CLASS(tag), &nprops );
	for(count = 0; count < nprops; count++) {

		/* Only copy properties that are readable, writable, not construct-only,
		and not deprecated */
		GParamFlags flags = properties[count]->flags;
		if(flags & G_PARAM_CONSTRUCT_ONLY
			|| flags & G_PARAM_DEPRECATED
			|| !(flags & G_PARAM_READABLE)
			|| !(flags & G_PARAM_WRITABLE))
			continue;

		const char *prop_name = g_param_spec_get_name(properties[count]);
		GValue prop_value = G_VALUE_INIT;

		g_value_init( &prop_value, G_PARAM_SPEC_VALUE_TYPE(properties[count]) );
		g_object_get_property( G_OBJECT(tag), prop_name, &prop_value );
		/* Don't copy the PangoTabArray if it is NULL, that prints a warning */
		if(strcmp(prop_name, "tabs") == 0 && g_value_get_boxed(&prop_value) == NULL) {
			g_value_unset(&prop_value);
			continue;
		}
		g_object_set_property( G_OBJECT(copy), prop_name, &prop_value );
		g_value_unset(&prop_value);
	}
	g_free(properties);

	/* Copy the data that was added manually */
	gpointer reverse_color = g_object_get_data( G_OBJECT(tag), "reverse-color" );

	if(reverse_color) {
		g_object_set_data( G_OBJECT(copy), "reverse-color", reverse_color );
	}

	return copy;
}

/* Internal function that constructs the default styles */
void
style_init(ChimaraGlk *glk)
{
	CHIMARA_GLK_USE_PRIVATE(glk, priv);
	
	GHashTable *default_text_grid_styles = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_object_unref);
	GHashTable *default_text_buffer_styles = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_object_unref);
	GtkTextTag *tag;

	/* Initialise the default styles for a text grid */
	tag = gtk_text_tag_new("default");
	g_object_set(tag, "family", "Monospace", "family-set", TRUE, NULL);
	g_hash_table_insert(default_text_grid_styles, "default", tag);

	tag = gtk_text_tag_new("normal");
	g_hash_table_insert(default_text_grid_styles, "normal", tag);

	tag = gtk_text_tag_new("emphasized");
	g_object_set(tag, "style", PANGO_STYLE_ITALIC, "style-set", TRUE, NULL);
	g_hash_table_insert(default_text_grid_styles, "emphasized", tag);

	tag = gtk_text_tag_new("preformatted");
	g_hash_table_insert(default_text_grid_styles, "preformatted", tag);

	tag = gtk_text_tag_new("header");
	g_object_set(tag, "weight", PANGO_WEIGHT_BOLD, "weight-set", TRUE, NULL);
	g_hash_table_insert(default_text_grid_styles, "header", tag);

	tag = gtk_text_tag_new("subheader");
	g_object_set(tag, "weight", PANGO_WEIGHT_BOLD, "weight-set", TRUE, NULL);
	g_hash_table_insert(default_text_grid_styles, "subheader", tag);

	tag = gtk_text_tag_new("alert");
	g_object_set(tag, "foreground", "#aa0000", "foreground-set", TRUE, "weight", PANGO_WEIGHT_BOLD, "weight-set", TRUE, NULL);
	g_hash_table_insert(default_text_grid_styles, "alert", tag);

	tag = gtk_text_tag_new("note");
	g_object_set(tag, "foreground", "#aaaa00", "foreground-set", TRUE, "weight", PANGO_WEIGHT_BOLD, "weight-set", TRUE, NULL);
	g_hash_table_insert(default_text_grid_styles, "note", tag);

	tag = gtk_text_tag_new("block-quote");
	g_object_set(tag, "style", PANGO_STYLE_ITALIC, "style-set", TRUE, NULL);
	g_hash_table_insert(default_text_grid_styles, "block-quote", tag);

	tag = gtk_text_tag_new("input");
	g_hash_table_insert(default_text_grid_styles, "input", tag);

	tag = gtk_text_tag_new("user1");
	g_hash_table_insert(default_text_grid_styles, "user1", tag);

	tag = gtk_text_tag_new("user2");
	g_hash_table_insert(default_text_grid_styles, "user2", tag);

	tag = gtk_text_tag_new("hyperlink");
	g_object_set(tag, "foreground", "#0000ff", "foreground-set", TRUE, "underline", PANGO_UNDERLINE_SINGLE, "underline-set", TRUE, NULL);
	g_hash_table_insert(default_text_grid_styles, "hyperlink", tag);

	/* Initialise the default styles for a text buffer */
	tag = gtk_text_tag_new("default");
	g_object_set(tag, "family", "Serif", "family-set", TRUE, NULL);
	g_hash_table_insert(default_text_buffer_styles, "default", tag);

	tag = gtk_text_tag_new("normal");
	g_hash_table_insert(default_text_buffer_styles, "normal", tag);

	tag = gtk_text_tag_new("emphasized");
	g_object_set(tag, "style", PANGO_STYLE_ITALIC, "style-set", TRUE, NULL);
	g_hash_table_insert(default_text_buffer_styles, "emphasized", tag);

	tag = gtk_text_tag_new("preformatted");
	g_object_set(tag, "family", "Monospace", "family-set", TRUE, NULL);
	g_hash_table_insert(default_text_buffer_styles, "preformatted", tag);

	tag = gtk_text_tag_new("header");
	g_object_set(tag, "weight", PANGO_WEIGHT_BOLD, "weight-set", TRUE, NULL);
	g_hash_table_insert(default_text_buffer_styles, "header", tag);

	tag = gtk_text_tag_new("subheader");
	g_object_set(tag, "weight", PANGO_WEIGHT_BOLD, "weight-set", TRUE, NULL);
	g_hash_table_insert(default_text_buffer_styles, "subheader", tag);

	tag = gtk_text_tag_new("alert");
	g_object_set(tag, "foreground", "#aa0000", "foreground-set", TRUE, "weight", PANGO_WEIGHT_BOLD, "weight-set", TRUE, NULL);
	g_hash_table_insert(default_text_buffer_styles, "alert", tag);

	tag = gtk_text_tag_new("note");
	g_object_set(tag, "foreground", "#aaaa00", "foreground-set", TRUE, "weight", PANGO_WEIGHT_BOLD, "weight-set", TRUE, NULL);
	g_hash_table_insert(default_text_buffer_styles, "note", tag);

	tag = gtk_text_tag_new("block-quote");
	g_object_set(tag, "justification", GTK_JUSTIFY_CENTER, "justification-set", TRUE, "style", PANGO_STYLE_ITALIC, "style-set", TRUE, NULL);
	g_hash_table_insert(default_text_buffer_styles, "block-quote", tag);

	tag = gtk_text_tag_new("input");
	g_hash_table_insert(default_text_buffer_styles, "input", tag);

	tag = gtk_text_tag_new("user1");
	g_hash_table_insert(default_text_buffer_styles, "user1", tag);

	tag = gtk_text_tag_new("user2");
	g_hash_table_insert(default_text_buffer_styles, "user2", tag);

	tag = gtk_text_tag_new("hyperlink");
	g_object_set(tag, "foreground", "#0000ff", "foreground-set", TRUE, "underline", PANGO_UNDERLINE_SINGLE, "underline-set", TRUE, NULL);
	g_hash_table_insert(default_text_buffer_styles, "hyperlink", tag);

	priv->styles->text_grid = default_text_grid_styles;
	priv->styles->text_buffer = default_text_buffer_styles;

	style_reset_glk(glk);
}

/* Reset the style hints set from the Glk program to be blank. Call this when
starting a new game so that style hints from the previous game don't carry
over. */
void
style_reset_glk(ChimaraGlk *glk)
{
	CHIMARA_GLK_USE_PRIVATE(glk, priv);

	GHashTable *glk_text_grid_styles = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_object_unref);
	GHashTable *glk_text_buffer_styles = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_object_unref);
	GtkTextTag *tag;

	/* Initialize the GLK styles to empty tags */
	int i;
	for(i=0; i<style_NUMSTYLES; i++) {
		tag = gtk_text_tag_new(GLK_TAG_NAMES[i]);
		g_hash_table_insert(glk_text_grid_styles, (gchar*) GLK_TAG_NAMES[i], tag);
		tag = gtk_text_tag_new(GLK_TAG_NAMES[i]);
		g_hash_table_insert(glk_text_buffer_styles, (gchar*) GLK_TAG_NAMES[i], tag);
	}

	priv->glk_styles->text_grid = glk_text_grid_styles;
	priv->glk_styles->text_buffer = glk_text_buffer_styles;

}

/* Reset style tables to the library's internal defaults */
void
reset_default_styles(ChimaraGlk *glk)
{
	/* TODO: write this function */
}

/* Create the CSS file scanner */
GScanner *
create_css_file_scanner(void)
{
	GScanner *scanner = g_scanner_new(NULL);
	scanner->config->cset_identifier_first = G_CSET_a_2_z G_CSET_A_2_Z "#";
	scanner->config->cset_identifier_nth = G_CSET_a_2_z G_CSET_A_2_Z "-_" G_CSET_DIGITS;
	scanner->config->symbol_2_token = TRUE;
	scanner->config->cpair_comment_single = NULL;
	scanner->config->scan_float = FALSE;
	return scanner;
}

/* Run the scanner over the CSS file, overriding the default styles */
void
scan_css_file(GScanner *scanner, ChimaraGlk *glk)
{
	while( g_scanner_peek_next_token(scanner) != G_TOKEN_EOF) {
		if( !style_accept_style_selector(scanner, glk) )
			break;
	}

	g_scanner_destroy(scanner);
}

/* Internal function: parses a token */
static gboolean
style_accept(GScanner *scanner, GTokenType token)
{
	GTokenType next = g_scanner_get_next_token(scanner);
   	if(next	!= token) {
		g_scanner_unexp_token(scanner, token, NULL, NULL, NULL, "CSS Error", 1);
		return FALSE;
	}
	return TRUE;
}

/* Internal function: parses a style selector */
static gboolean
style_accept_style_selector(GScanner *scanner, ChimaraGlk *glk)
{
	CHIMARA_GLK_USE_PRIVATE(glk, priv);

	GtkTextTag *current_tag;
	gchar *field;
	GTokenType token = g_scanner_get_next_token(scanner);
	GTokenValue value = g_scanner_cur_value(scanner);

	if(
		token != G_TOKEN_IDENTIFIER ||
		( strcmp(value.v_identifier, "buffer") && strcmp(value.v_identifier, "grid") )
	) {
		g_scanner_error(scanner, "CSS Error: buffer/grid expected");
		return FALSE;
	}

	field = g_strdup(value.v_identifier);

	/* Parse the tag name to change */
	if( g_scanner_peek_next_token(scanner) == '{') {
		style_accept(scanner, '{');
		if( !strcmp(field, "buffer") )
			current_tag = g_hash_table_lookup(priv->styles->text_buffer, "default");
		else
			current_tag = g_hash_table_lookup(priv->styles->text_grid, "default");
	} else {
		if( !style_accept(scanner, '.') )
			return FALSE;

		token = g_scanner_get_next_token(scanner);
		value = g_scanner_cur_value(scanner);

		if(token != G_TOKEN_IDENTIFIER) {
			g_scanner_error(scanner, "CSS Error: style selector expected");
			return FALSE;
		}

		if( !strcmp(field, "buffer") )
			current_tag = g_hash_table_lookup(priv->styles->text_buffer, value.v_identifier);
		else
			current_tag = g_hash_table_lookup(priv->styles->text_grid, value.v_identifier);

		if(current_tag == NULL) {
			g_scanner_error(scanner, "CSS Error: invalid style identifier");
			return FALSE;
		}

		if( !style_accept(scanner, '{') )
			return FALSE;
	}

	while( g_scanner_peek_next_token(scanner) != '}') {
		if( !style_accept_style_hint(scanner, current_tag) )
			return FALSE;
	}
		
	if( !style_accept(scanner, '}') )
		return FALSE;

	return TRUE;
}

/* Internal function: parses a style hint */
static gboolean
style_accept_style_hint(GScanner *scanner, GtkTextTag *current_tag)
{
	GTokenType token = g_scanner_get_next_token(scanner);
	GTokenValue value = g_scanner_cur_value(scanner);
	gchar *hint;

	if(token != G_TOKEN_IDENTIFIER) {
		g_scanner_error(scanner, "CSS Error: style hint expected");
		return FALSE;
	}

	hint = g_strdup(value.v_identifier);

	if( !style_accept(scanner, ':') )
		return FALSE;

	token = g_scanner_get_next_token(scanner);
	value = g_scanner_cur_value(scanner);

	if( !strcmp(hint, "font-family") ) {
		if(token != G_TOKEN_STRING) {
			g_scanner_error(scanner, "CSS Error: string expected");
			return FALSE;
		}
		g_object_set(current_tag, "family", value.v_string, "family-set", TRUE, NULL);
	}
	else if( !strcmp(hint, "font-weight") ) {
		if(token != G_TOKEN_IDENTIFIER) {
			g_scanner_error(scanner, "CSS Error: bold/normal expected");
			return FALSE;
		}

		if( !strcmp(value.v_identifier, "bold") )
			g_object_set(current_tag, "weight", PANGO_WEIGHT_BOLD, "weight-set", TRUE, NULL);
		else if( !strcmp(value.v_identifier, "normal") )
			g_object_set(current_tag, "weight", PANGO_WEIGHT_NORMAL, "weight-set", TRUE, NULL);
		else {
			g_scanner_error(scanner, "CSS Error: bold/normal expected");
			return FALSE;
		}
	}
	else if( !strcmp(hint, "font-style") ) {
		if(token != G_TOKEN_IDENTIFIER) {
			g_scanner_error(scanner, "CSS Error: italic/normal expected");
			return FALSE;
		}

		if( !strcmp(value.v_identifier, "italic") )
			g_object_set(current_tag, "style", PANGO_STYLE_ITALIC, "style-set", TRUE, NULL);
		else if( !strcmp(value.v_identifier, "normal") )
			g_object_set(current_tag, "style", PANGO_STYLE_NORMAL, "style-set", TRUE, NULL);
		else {
			g_scanner_error(scanner, "CSS Error: italic/normal expected");
			return FALSE;
		}
	}
	else if( !strcmp(hint, "font-size") ) {
		if(token == G_TOKEN_INT) 
			g_object_set(current_tag, "size-points", (float)value.v_int, "size-set", TRUE, NULL);
		else if(token == G_TOKEN_FLOAT)
			g_object_set(current_tag, "size-points", value.v_float, "size-set", TRUE, NULL);
		else {
			g_scanner_error(scanner, "CSS Error: integer or float expected");
			return FALSE;
		}
	}
	else if( !strcmp(hint, "color") ) {
		if(token != G_TOKEN_IDENTIFIER) {
			g_scanner_error(scanner, "CSS Error: hex color expected");
			return FALSE;
		}

		g_object_set(current_tag, "foreground", value.v_identifier, "foreground-set", TRUE, NULL);
	}
	else if( !strcmp(hint, "background-color") ) {
		if(token != G_TOKEN_IDENTIFIER) {
			g_scanner_error(scanner, "CSS Error: hex color expected");
			return FALSE;
		}
		g_object_set(current_tag, "background", value.v_identifier, "background-set", TRUE, NULL);
	}
	else if( !strcmp(hint, "text-align") ) {
		if(token != G_TOKEN_IDENTIFIER) {
			g_scanner_error(scanner, "CSS Error: left/right/center expected");
			return FALSE;
		}
		
		if( !strcmp(value.v_identifier, "left") )
			g_object_set(current_tag, "justification", GTK_JUSTIFY_LEFT, "justification-set", TRUE, NULL);
		else if( !strcmp(value.v_identifier, "right") )
			g_object_set(current_tag, "justification", GTK_JUSTIFY_RIGHT, "justification-set", TRUE, NULL);
		else if( !strcmp(value.v_identifier, "center") )
			g_object_set(current_tag, "justification", GTK_JUSTIFY_CENTER, "justification-set", TRUE, NULL);
		else {
			g_scanner_error(scanner, "CSS Error: left/right/center expected");
			return FALSE;
		}
	}
	else if( !strcmp(hint, "margin-left") ) {
		if(token != G_TOKEN_INT) {
			g_scanner_error(scanner, "CSS Error: integer expected");
			return FALSE;
		}
		g_object_set(current_tag, "left-margin", value.v_int, "left-margin-set", TRUE, NULL);
	}
	else if( !strcmp(hint, "margin-right") ) {
		if(token != G_TOKEN_INT) {
			g_scanner_error(scanner, "CSS Error: integer expected");
			return FALSE;
		}
		g_object_set(current_tag, "right-margin", value.v_int, "right-margin-set", TRUE, NULL);
	}
	else if( !strcmp(hint, "margin-top") ) {
		if(token != G_TOKEN_INT) {
			g_scanner_error(scanner, "CSS Error: integer expected");
			return FALSE;
		}
		g_object_set(current_tag, "pixels-above-lines", value.v_int, "pixels-above-lines-set", TRUE, NULL);
	}
	else if( !strcmp(hint, "margin-bottom") ) {
		if(token != G_TOKEN_INT) {
			g_scanner_error(scanner, "CSS Error: integer expected");
			return FALSE;
		}
		g_object_set(current_tag, "pixels-below-lines", value.v_int, "pixels-below-lines-set", TRUE, NULL);
	}
		
	else {
		g_scanner_error(scanner, "CSS Error: invalid style hint %s", hint);
		return FALSE;
	}

	if( !style_accept(scanner, ';') )
		return FALSE;

	return TRUE;
}

/* Internal function: parses a glk color to a GdkRGBA */
void
glkcolor_to_gdkrgba(glui32 val, GdkRGBA *color)
{
	color->red = ((val & 0xff0000) >> 16) / 255.0;
	color->green = ((val & 0x00ff00) >> 8) / 255.0;
	color->blue = (val & 0x0000ff) / 255.0;
	color->alpha = 1.0;
}

/* Internal function: parses a GdkRGBA to a glk color */
static glui32
gdkrgba_to_glkcolor(GdkRGBA *color)
{
	g_return_val_if_fail(color != NULL, 0);
	return (glui32) ((int)(color->red * 255) << 16
	               | (int)(color->green * 255) << 8
	               | (int)(color->blue * 255));
}

/* Internal function: changes a GTK tag to correspond with the given style. */
static void
apply_stylehint_to_tag(GtkTextTag *tag, glui32 wintype, glui32 styl, glui32 hint, glsi32 val)
{
	g_return_if_fail(tag != NULL);

	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
	GObject *tag_object = G_OBJECT(tag);

	gint reverse_color = GPOINTER_TO_INT( g_object_get_data(tag_object, "reverse-color") );

	int i = 0;
	GdkRGBA color;
	switch(hint) {
	case stylehint_Indentation:
		g_object_set(tag_object, "left-margin", 5*val, "left-margin-set", TRUE, NULL);
		g_object_set(tag_object, "right-margin", 5*val, "right-margin-set", TRUE, NULL);
		break;
	
	case stylehint_ParaIndentation:
		g_object_set(tag_object, "indent", 5*val, "indent-set", TRUE, NULL);
		break;

	case stylehint_Justification:
		switch(val) {
			case stylehint_just_LeftFlush:  i = GTK_JUSTIFY_LEFT; break;
			case stylehint_just_LeftRight:  i = GTK_JUSTIFY_FILL; break;
			case stylehint_just_Centered:   i = GTK_JUSTIFY_CENTER; break;
			case stylehint_just_RightFlush: i = GTK_JUSTIFY_RIGHT; break;
			default: 
				WARNING("Unknown justification");
				i = GTK_JUSTIFY_LEFT;
		}
		g_object_set(tag_object, "justification", i, "justification-set", TRUE, NULL);
		break;

	case stylehint_Weight:
		switch(val) {
			case -1: i = PANGO_WEIGHT_LIGHT; break;
			case  0: i = PANGO_WEIGHT_NORMAL; break;
			case  1: i = PANGO_WEIGHT_BOLD; break;
			default: WARNING("Unknown font weight");
		}
		g_object_set(tag_object, "weight", i, "weight-set", TRUE, NULL);
		break;

	case stylehint_Size:
		{
			gdouble scale = PANGO_SCALE_MEDIUM;
			switch(val) {
				case -3: scale = PANGO_SCALE_XX_SMALL; break;
				case -2: scale = PANGO_SCALE_X_SMALL; break;
				case -1: scale = PANGO_SCALE_SMALL; break;
				case  0: scale = PANGO_SCALE_MEDIUM; break;
				case  1: scale = PANGO_SCALE_LARGE; break;
				case  2: scale = PANGO_SCALE_X_LARGE; break;
				case  3: scale = PANGO_SCALE_XX_LARGE; break;
				default:
					/* We follow Pango's convention of having each magnification
					step be a scaling of 1.2 */
					scale = pow(1.2, (double)val);
			}
			g_object_set(tag_object, "scale", scale, "scale-set", TRUE, NULL);
		}
		break;

	case stylehint_Oblique:
		g_object_set(tag_object, "style", val ? PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL, "style-set", TRUE, NULL);
		break;

	case stylehint_Proportional:
	{
		gchar *font_family;
		gboolean family_set;

		if(wintype != wintype_TextBuffer) {
		   	if(val)
				WARNING("Style hint 'propotional' only supported on text buffers.");

			break;
		}

		GtkTextTag *font_tag = g_hash_table_lookup(glk_data->styles->text_buffer, val? "default" : "preformatted");
		g_object_get(font_tag, "family", &font_family, "family-set", &family_set, NULL);
		g_object_set(tag_object, "family", font_family, "family-set", family_set, NULL);
		g_free(font_family);
	}
		break;

	case stylehint_TextColor:
		glkcolor_to_gdkrgba(val, &color);

		if(!reverse_color)
			g_object_set(tag_object, "foreground-rgba", &color, "foreground-set", TRUE, NULL);
		else
			g_object_set(tag_object, "background-rgba", &color, "background-set", TRUE, NULL);

		break;

	case stylehint_BackColor:
		glkcolor_to_gdkrgba(val, &color);

		if(!reverse_color)
			g_object_set(tag_object, "background-rgba", &color, "background-set", TRUE, NULL);
		else
			g_object_set(tag_object, "foreground-rgba", &color, "background-set", TRUE, NULL);

		break;

	case stylehint_ReverseColor:
	{
		// Determine the current colors
		
		// If all fails, use black/white
		// FIXME: Use system theme here
		GdkRGBA foreground, background;
		gdk_rgba_parse(&foreground, "black");
		gdk_rgba_parse(&background, "white");
		GdkRGBA *current_foreground = &foreground;
		GdkRGBA *current_background = &background;

		if(wintype == wintype_TextBuffer) {
			GtkTextTag* default_tag = g_hash_table_lookup(glk_data->styles->text_buffer, "default");
			GtkTextTag* base_tag = g_hash_table_lookup(glk_data->styles->text_buffer, get_tag_name(styl));
			style_cascade_colors(base_tag, tag, default_tag, &current_foreground, &current_background);
		}
		else if(wintype == wintype_TextGrid) {
			GtkTextTag* default_tag = g_hash_table_lookup(glk_data->styles->text_grid, "default");
			GtkTextTag* base_tag = g_hash_table_lookup(glk_data->styles->text_grid, get_tag_name(styl));
			style_cascade_colors(base_tag, tag, default_tag, &current_foreground, &current_background);
		}

		if(val) {
			/* Flip the fore- and background colors */
			GdkRGBA *temp = current_foreground;
			current_foreground = current_background;
			current_background = temp;
		}

		g_object_set(tag,
			"foreground-rgba", current_foreground,
			"foreground-set", TRUE,
			"background-rgba", current_background,
			"background-set", TRUE,
			NULL
		);

		g_object_set_data( tag_object, "reverse-color", GINT_TO_POINTER(val != 0) );
		break;
	}

	default:
		WARNING("Unknown style hint");
	}
}

/* Internal function: queries a text tag for the value of a given style hint */
static gint
query_tag(GtkTextTag *tag, glui32 wintype, glui32 hint)
{
	gint intval;
	gdouble doubleval;
	GdkRGBA *colval;

	g_return_val_if_fail(tag != NULL, 0);

	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);

	switch(hint) {
	case stylehint_Indentation:
		g_object_get(tag, "left_margin", &intval, NULL);
		return intval/5;
	
	case stylehint_ParaIndentation:
		g_object_get(tag, "indent", &intval, NULL);
		return intval/5;

	case stylehint_Justification:
		g_object_get(tag, "justification", &intval, NULL);
		switch(intval) {
			case GTK_JUSTIFY_LEFT: return stylehint_just_LeftFlush;
			case GTK_JUSTIFY_FILL: return stylehint_just_LeftRight;
			case GTK_JUSTIFY_CENTER: return stylehint_just_Centered;
			case GTK_JUSTIFY_RIGHT: return stylehint_just_RightFlush;
			default: 
				WARNING("Unknown justification");
				return stylehint_just_LeftFlush;
		}

	case stylehint_Weight:
		g_object_get(tag, "weight", &intval, NULL);
		switch(intval) {
			case PANGO_WEIGHT_LIGHT: return -1;
			case PANGO_WEIGHT_NORMAL: return 0;
			case PANGO_WEIGHT_BOLD: return 1;
			default: WARNING("Unknown font weight"); return 0;
		}

	case stylehint_Size:
		g_object_get(tag, "scale", &doubleval, NULL);
		return (gint)round(log(doubleval) / log(1.2));

	case stylehint_Oblique:
		g_object_get(tag, "style", &intval , NULL);
		return intval == PANGO_STYLE_ITALIC ? 1 : 0;

	case stylehint_Proportional:
		/* Use pango_font_family_is_monospace()? */
	{
		gchar *font_family, *query_font_family;
		GtkTextTag *font_tag = g_hash_table_lookup(
		    wintype == wintype_TextBuffer? glk_data->styles->text_buffer : glk_data->styles->text_grid,
		    "preformatted");
		g_object_get(font_tag, "family", &font_family, NULL);
		g_object_get(tag, "family", &query_font_family, NULL);
		gint retval = strcmp(font_family, query_font_family)? 0 : 1;
		g_free(font_family);
		g_free(query_font_family);
		return retval;
	}

	case stylehint_TextColor:
		g_object_get(tag, "foreground-rgba", &colval, NULL);
		return gdkrgba_to_glkcolor(colval);

	case stylehint_BackColor:
		g_object_get(tag, "background-rgba", &colval, NULL);
		return gdkrgba_to_glkcolor(colval);

	case stylehint_ReverseColor:
		return GPOINTER_TO_INT( g_object_get_data(G_OBJECT(tag), "reverse_color") );

	default:
		WARNING("Unknown style hint");
	}
	
	return 0;
}

/**
 * glk_stylehint_set:
 * @wintype: The window type to set a style hint on, or %wintype_AllTypes.
 * @styl: The style to set a hint for.
 * @hint: The type of style hint, one of the `stylehint_` constants.
 * @val: The style hint. The meaning of this depends on @hint.
 *
 * Sets a hint about the appearance of one style for a particular type of 
 * window. You can also set @wintype to %wintype_AllTypes, which sets a hint for 
 * all types of window.
 * <note><para>
 *  There is no equivalent constant to set a hint for all styles of a single 
 *  window type.
 * </para></note>
 */
void
glk_stylehint_set(glui32 wintype, glui32 styl, glui32 hint, glsi32 val)
{
#ifdef DEBUG_STYLES
	g_printf("glk_stylehint_set(wintype=%d, styl=%d, hint=%d, val=%d)\n", wintype, styl, hint, val);
#endif

	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);

	GtkTextTag *to_change;
	if(wintype == wintype_TextBuffer || wintype == wintype_AllTypes) {
		to_change = g_hash_table_lookup( glk_data->glk_styles->text_buffer, get_glk_tag_name(styl) );
		apply_stylehint_to_tag(to_change, wintype_TextBuffer, styl, hint, val);
	}

	if(wintype == wintype_TextGrid || wintype == wintype_AllTypes) {
		to_change = g_hash_table_lookup( glk_data->glk_styles->text_grid, get_glk_tag_name(styl) );
		apply_stylehint_to_tag(to_change, wintype_TextGrid, styl, hint, val);
	}
}

/**
 * glk_stylehint_clear:
 * @wintype: The window type to set a style hint on, or %wintype_AllTypes.
 * @styl: The style to set a hint for.
 * @hint: The type of style hint, one of the `stylehint_` constants.
 *
 * Clears a hint about the appearance of one style for a particular type of 
 * window to its default value. You can also set @wintype to %wintype_AllTypes, 
 * which clears a hint for all types of window.
 * <note><para>
 *  There is no equivalent constant to reset a hint for all styles of a single 
 *  window type.
 * </para></note>
 */
void
glk_stylehint_clear(glui32 wintype, glui32 styl, glui32 hint)
{
#ifdef DEBUG_STYLES
	g_printf("glk_stylehint_clear(wintype=%d, styl=%d, hint=%d)\n", wintype, styl, hint);
#endif

	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
	GtkTextTag *tag;

	switch(wintype) {
	case wintype_TextBuffer:
		tag = g_hash_table_lookup( glk_data->glk_styles->text_buffer, get_glk_tag_name(styl) );
		if(tag) {
			glk_stylehint_set( wintype, styl, hint, query_tag(tag, wintype, hint) );
		}
		break;
	case wintype_TextGrid:
		tag = g_hash_table_lookup( glk_data->glk_styles->text_grid, get_glk_tag_name(styl) );
		if(tag) {
			glk_stylehint_set( wintype, styl, hint, query_tag(tag, wintype, hint) );
		}
	default:
		return;
	}
}

/**
 * glk_style_distinguish:
 * @win: The window in which the styles are to be distinguished.
 * @styl1: The first style to be distinguished from the second style.
 * @styl2: The second style to be distinguished from the first style.
 * 
 * Decides whether two styles are visually distinguishable in the given window.
 * The exact meaning of this is left for the library to determine.
 * <note><title>Chimara</title><para>
 *   Currently, all styles of one window are assumed to be mutually
 *   distinguishable.
 * </para></note>
 * 
 * Returns: %TRUE (1) if the two styles are visually distinguishable. If they 
 * are not, it returns %FALSE (0).
 */
glui32
glk_style_distinguish(winid_t win, glui32 styl1, glui32 styl2)
{
#ifdef DEBUG_STYLES
	g_printf("glk_style_distinguish(win->rock=%d, styl1=%d, styl2=%d)\n", win->rock, styl1, styl2);
#endif

	/* FIXME */
	return styl1 != styl2;
}

/**
 * glk_style_measure:
 * @win: The window from which to take the style.
 * @styl: The style to perform the measurement on.
 * @hint: The stylehint to measure.
 * @result: Address to write the result to.
 * 
 * Tries to test an attribute of one style in the given window @win. The library
 * may not be able to determine the attribute; if not, this returns %FALSE (0).
 * If it can, it returns %TRUE (1) and stores the value in the location pointed
 * at by @result. 
 * <note><para>
 *   As usual, it is legal for @result to be %NULL, although fairly pointless.
 * </para></note>
 *
 * The meaning of the value depends on the hint which was tested:
 * <variablelist>
 * <varlistentry>
 *   <term>%stylehint_Indentation, %stylehint_ParaIndentation</term>
 *   <listitem><para>The indentation and paragraph indentation. These are in a
 *   metric which is platform-dependent.</para>
 *   <note><para>Most likely either characters or pixels.</para></note>
 *   </listitem>
 * </varlistentry>
 * <varlistentry>
 *   <term>%stylehint_Justification</term>
 *   <listitem><para>One of the constants %stylehint_just_LeftFlush,
 *   %stylehint_just_LeftRight, %stylehint_just_Centered, or
 *   %stylehint_just_RightFlush.</para></listitem>
 * </varlistentry>
 * <varlistentry>
 *   <term>%stylehint_Size</term>
 *   <listitem><para>The font size. Again, this is in a platform-dependent
 *   metric.</para>
 *   <note><para>Pixels, points, or simply 1 if the library does not support
 *   varying font sizes.</para></note>
 *   </listitem>
 * </varlistentry>
 * <varlistentry>
 *   <term>%stylehint_Weight</term>
 *   <listitem><para>1 for heavy-weight fonts (boldface), 0 for normal weight,
 *   and -1 for light-weight fonts.</para></listitem>
 * </varlistentry>
 * <varlistentry>
 *   <term>%stylehint_Oblique</term>
 *   <listitem><para>1 for oblique fonts (italic), or 0 for normal angle.</para>
 *   </listitem>
 * </varlistentry>
 * <varlistentry>
 *   <term>%stylehint_Proportional</term>
 *   <listitem><para>1 for proportional-width fonts, or 0 for fixed-width.
 *   </para></listitem>
 * </varlistentry>
 * <varlistentry>
 *   <term>%stylehint_TextColor, %stylehint_BackColor</term>
 *   <listitem><para>These are values from 0x00000000 to 0x00FFFFFF, encoded as
 *   described in <link 
 *   linkend="chimara-Suggesting-the-Appearance-of-Styles">Suggesting the
 *   Appearance of Styles</link>.</para></listitem>
 * </varlistentry>
 * <varlistentry>
 *   <term>%stylehint_ReverseColor</term>
 *   <listitem><para>0 for normal printing, 1 if the foreground and background
 *   colors are reversed.</para></listitem>
 * </varlistentry>
 * </variablelist>
 * Signed values, such as the %stylehint_Weight value, are cast to #glui32.
 * They may be cast to #glsi32 to be dealt with in a more natural context.
 *
 * Returns: TRUE upon successul retrieval, otherwise FALSE.
 */
glui32
glk_style_measure(winid_t win, glui32 styl, glui32 hint, glui32 *result)
{
#ifdef DEBUG_STYLES
	g_printf("glk_style_measure(win->rock=%d, styl=%d, hint=%d, result=...)\n", win->rock, styl, hint);
#endif

	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
	GtkTextTag *tag;

	switch(win->type) {
	case wintype_TextBuffer:
		tag = g_hash_table_lookup( glk_data->glk_styles->text_buffer, get_glk_tag_name(styl) );
		if(result)
			*result = query_tag(tag, win->type, hint);
		break;
	case wintype_TextGrid:
		tag = g_hash_table_lookup( glk_data->glk_styles->text_grid, get_glk_tag_name(styl) );
		if(result)
			*result = query_tag(tag, win->type, hint);
	default:
		return FALSE;
	}

	return TRUE;
}

/* Internal function returning the current default font for a window type
 * This can be used later for size calculations. Only wintype_TextGrid and wintype_TextBuffer are
 * supported for now */
PangoFontDescription *
get_current_font(guint32 wintype)
{
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
	GHashTable *styles, *glk_styles;
	PangoFontDescription *font;

	switch(wintype) {
	case wintype_TextGrid:
		styles = glk_data->styles->text_grid;
		glk_styles = glk_data->glk_styles->text_grid;
		font = pango_font_description_from_string("Monospace");
		break;
	case wintype_TextBuffer:
		styles = glk_data->styles->text_buffer;
		glk_styles = glk_data->glk_styles->text_buffer;
		font = pango_font_description_from_string("Serif");
		break;
	default:
		return NULL;
	}

	PangoAttrList *list = pango_attr_list_new();

	text_tag_to_attr_list( g_hash_table_lookup(styles, "default"), list );
	PangoAttrIterator *it = pango_attr_list_get_iterator(list);
	pango_attr_iterator_get_font(it, font, NULL, NULL);
	pango_attr_iterator_destroy(it);

	text_tag_to_attr_list( g_hash_table_lookup(styles, "normal"), list );
	it = pango_attr_list_get_iterator(list);
	pango_attr_iterator_get_font(it, font, NULL, NULL);
	pango_attr_iterator_destroy(it);

	text_tag_to_attr_list( g_hash_table_lookup(glk_styles, "glk-normal"), list );
	it = pango_attr_list_get_iterator(list);
	pango_attr_iterator_get_font(it, font, NULL, NULL);
	pango_attr_iterator_destroy(it);

	/* Make a copy of the family, preventing it's destruction at the end of this function. */
	pango_font_description_set_family( font, pango_font_description_get_family(font) );

	pango_attr_list_unref(list);

	return font;
}

/* Internal function copying the attributes of a text tag to a pango attribute list */
static void
text_tag_to_attr_list(GtkTextTag *tag, PangoAttrList *list)
{
	gboolean set;
	GdkRGBA *foreground, *background;
	gchar *string;
	PangoFontDescription *font_desc;
	gboolean strikethrough;
	PangoUnderline underline;

	g_object_get(tag, "foreground-set", &set, "foreground-rgba", &foreground, NULL);
	if(set) {
		pango_attr_list_insert(
			list,
			pango_attr_foreground_new(foreground->red, foreground->green, foreground->blue)
		);
	}
	g_object_get(tag, "background-set", &set, "background-rgba", &background, NULL);
	if(set) {
		pango_attr_list_insert(
			list,
			pango_attr_background_new(background->red, background->green, background->blue)
		);
	}
	g_object_get(tag, "language-set", &set, "language", &string, NULL);
	if(set) {
		pango_attr_list_insert(
			list,
			pango_attr_language_new( pango_language_from_string(string) )
		);
	}

	/* Font description updates the following properties simultaniously:
	 * family, style, weight, variant, stretch, size.
	 * FIXME: Except it doesn't really.
	 */
	g_object_get(tag, "font-desc", &font_desc, NULL);
	pango_attr_list_insert(
		list,
		pango_attr_font_desc_new(font_desc)
	);

	g_object_get(tag, "strikethrough-set", &set, "strikethrough", &strikethrough, NULL);
	if(set) {
		pango_attr_list_insert(
			list,
			pango_attr_strikethrough_new(strikethrough)
		);
	}
	g_object_get(tag, "underline-set", &set, "underline", &underline, NULL);
	if(set) {
		pango_attr_list_insert(
			list,
			pango_attr_underline_new(underline)
		);
	}
}

/* Determine the current colors used to render the text for a given stream. 
 * This can be set in a number of places */
static void
style_cascade_colors(GtkTextTag *tag, GtkTextTag *glk_tag, GtkTextTag *default_tag, GdkRGBA **foreground, GdkRGBA **background)
{
	gboolean foreground_set = FALSE;
	gboolean background_set = FALSE;
	gint reverse_color;

	// Default color
	reverse_color = GPOINTER_TO_INT( g_object_get_data(G_OBJECT(default_tag), "reverse-color") );
	g_object_get(default_tag, "foreground-set", &foreground_set, "background-set", &background_set, NULL);
	if(foreground_set)
		g_object_get(default_tag, "foreground-rgba", reverse_color ? background : foreground, NULL);
	if(background_set)
		g_object_get(default_tag, "background-rgba", reverse_color ? foreground : background, NULL);

	// Player defined color
	reverse_color = GPOINTER_TO_INT( g_object_get_data(G_OBJECT(tag), "reverse-color") );
	g_object_get(tag, "foreground-set", &foreground_set, "background-set", &background_set, NULL);
	if(foreground_set)
		g_object_get(tag, "foreground-rgba", reverse_color ? background : foreground, NULL);
	if(background_set)
		g_object_get(tag, "background-rgba", reverse_color ? foreground : background, NULL);

	// GLK defined color
	reverse_color = GPOINTER_TO_INT( g_object_get_data(G_OBJECT(glk_tag), "reverse-color") );
	g_object_get(glk_tag, "foreground-set", &foreground_set, "background-set", &background_set, NULL);
	if(foreground_set)
		g_object_get(glk_tag, "foreground-rgba", reverse_color ? background : foreground, NULL);
	if(background_set)
		g_object_get(glk_tag, "background-rgba", reverse_color ? foreground : background, NULL);

}

/* Determine the current colors used to render the text for a given stream. 
 * This can be set in a number of places */
void
style_stream_colors(strid_t str, GdkRGBA **foreground, GdkRGBA **background)
{
	VALID_STREAM(str, return);
	g_return_if_fail(str->window != NULL);
	g_return_if_fail(str->window->type != wintype_TextBuffer || str->window->type != wintype_TextGrid);
	
	GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(str->window->widget) );	
	GtkTextTagTable *tags = gtk_text_buffer_get_tag_table(buffer);
	GtkTextTag* default_tag = gtk_text_tag_table_lookup(tags, "default");
	GtkTextTag* tag = gtk_text_tag_table_lookup(tags, str->style);
	GtkTextTag* glk_tag = gtk_text_tag_table_lookup(tags, str->glk_style);

	style_cascade_colors(tag, glk_tag, default_tag, foreground, background);

	gboolean foreground_set, background_set;

	// Windows can have zcolors defined
	if(str->window->zcolor) {
		g_object_get(str->window->zcolor, "foreground-set", &foreground_set, "background-set", &background_set, NULL);
		if(foreground_set)
			g_object_get(str->window->zcolor, "foreground-rgba", foreground, NULL);
		if(background_set)
			g_object_get(str->window->zcolor, "background-rgba", background, NULL);
	}
}

/* Apply styles to a segment of text in a GtkTextBuffer, combining multiple
 * GtkTextTags.
 */
void
style_apply(winid_t win, GtkTextIter *start, GtkTextIter *end)
{
	GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
	GtkTextTagTable *tags = gtk_text_buffer_get_tag_table(buffer);

	GtkTextTag *default_tag = gtk_text_tag_table_lookup(tags, "default");
	GtkTextTag *style_tag = gtk_text_tag_table_lookup(tags, win->window_stream->style);
	GtkTextTag *glk_style_tag = gtk_text_tag_table_lookup(tags, win->window_stream->glk_style);

	// Player's style overrides
	gtk_text_buffer_apply_tag(buffer, style_tag, start, end);

	// GLK Program's style overrides
	gtk_text_buffer_apply_tag(buffer, glk_style_tag, start, end);

	// Default style
	gtk_text_buffer_apply_tag(buffer, default_tag, start, end);

	// Link style overrides
	if(win->window_stream->hyperlink_mode) {
		GtkTextTag *link_style_tag = gtk_text_tag_table_lookup(tags, "hyperlink");
		GtkTextTag *link_tag = win->current_hyperlink->tag;
		gtk_text_buffer_apply_tag(buffer, link_style_tag, start, end);
		gtk_text_buffer_apply_tag(buffer, link_tag, start, end);
	}

	// GLK Program's style overrides using garglk_set_zcolors()
	if(win->zcolor != NULL) {
		gtk_text_buffer_apply_tag(buffer, win->zcolor, start, end);
	}

	// GLK Program's style overrides using garglk_set_reversevideo()
	if(win->zcolor_reversed != NULL) {
		gtk_text_buffer_apply_tag(buffer, win->zcolor_reversed, start, end);
	}
}
	
