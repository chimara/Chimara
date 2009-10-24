#include "style.h"
#include <stdio.h>
#include <fcntl.h>

extern GPrivate *glk_data_key;
static gboolean chimara_style_initialized = FALSE;

static gboolean style_accept(GScanner *scanner, GTokenType token);
static gboolean style_accept_style_selector(GScanner *scanner);
static gboolean style_accept_style_hint(GScanner *scanner, GtkTextTag *current_tag);
static void style_add_tag_to_textbuffer(gpointer key, gpointer tag, gpointer tag_table);
static void style_table_copy(gpointer key, gpointer tag, gpointer target_table);
static GtkTextTag* gtk_text_tag_copy(GtkTextTag *tag);

/**
 * glk_set_style:
 * @styl: The style to apply
 *
 * Changes the style of the current output stream. @styl should be one of the
 * <code>style_</code> constants listed above. However, any value is actually
 * legal; if the interpreter does not recognize the style value, it will treat
 * it as %style_Normal.
 * <note><para>
 *  This policy allows for the future definition of styles without breaking old
 *  Glk libraries.
 * </para></note>
 */
void
glk_set_style(glui32 styl)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	g_return_if_fail(glk_data->current_stream != NULL);
	glk_set_style_stream(glk_data->current_stream, styl);
}

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
	"user2"
};

/* Internal function: mapping from style enum to tag name */
static gchar*
get_tag_name(glui32 style)
{
	if(style >= style_NUMSTYLES) {
		WARNING("Unsupported style");
		return "normal";
	} else {
		return (gchar*) TAG_NAMES[style];
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
	str->style = get_tag_name(styl);
}

/* Internal function: call this to initialize the default styles to a textbuffer. */
void
style_init_textbuffer(GtkTextBuffer *buffer)
{
	g_return_if_fail(buffer != NULL);

	if( G_UNLIKELY(!chimara_style_initialized) ) {
		style_init();
	}

	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	g_hash_table_foreach(glk_data->current_styles->text_buffer, style_add_tag_to_textbuffer, gtk_text_buffer_get_tag_table(buffer));
}

/* Internal function: call this to initialize the default styles to a textgrid. */
void
style_init_textgrid(GtkTextBuffer *buffer)
{
	g_return_if_fail(buffer != NULL);

	if( G_UNLIKELY(!chimara_style_initialized) ) {
		style_init();
	}

	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	g_hash_table_foreach(glk_data->current_styles->text_grid, style_add_tag_to_textbuffer, gtk_text_buffer_get_tag_table(buffer));
}

/* Internal function used to iterate over the default text tag table, applying them to a textbuffer */
static void
style_add_tag_to_textbuffer(gpointer key, gpointer tag, gpointer tag_table)
{
	gtk_text_tag_table_add( tag_table, gtk_text_tag_copy(tag) );
}

/* Internal function used to iterate over a style table, copying it */
static void
style_table_copy(gpointer key, gpointer tag, gpointer target_table)
{
	g_return_if_fail(key != NULL);
	g_return_if_fail(tag != NULL);
	g_return_if_fail(target_table != NULL);

	g_hash_table_insert(target_table, key, gtk_text_tag_copy( GTK_TEXT_TAG(tag) ));
}

/* Internal function that copies a text tag */
static GtkTextTag*
gtk_text_tag_copy(GtkTextTag *tag)
{
	GtkTextTag *copy;

	g_return_val_if_fail(tag != NULL, NULL);

	copy = gtk_text_tag_new(tag->name);
	gtk_text_attributes_copy_values(tag->values, copy->values);

	#define _COPY_FLAG(flag) copy->flag = tag->flag
		_COPY_FLAG (bg_color_set);
		_COPY_FLAG (bg_color_set);
		_COPY_FLAG (bg_stipple_set);
		_COPY_FLAG (fg_color_set);
		_COPY_FLAG (fg_stipple_set);
		_COPY_FLAG (justification_set);
		_COPY_FLAG (left_margin_set);
		_COPY_FLAG (indent_set);
		_COPY_FLAG (rise_set);
		_COPY_FLAG (strikethrough_set);
		_COPY_FLAG (right_margin_set);
		_COPY_FLAG (pixels_above_lines_set);
		_COPY_FLAG (pixels_below_lines_set);
		_COPY_FLAG (pixels_inside_wrap_set);
		_COPY_FLAG (tabs_set);
		_COPY_FLAG (underline_set);
		_COPY_FLAG (wrap_mode_set);
		_COPY_FLAG (bg_full_height_set);
		_COPY_FLAG (invisible_set);
		_COPY_FLAG (editable_set);
		_COPY_FLAG (language_set);
	#undef _COPY_FLAG

	return copy;
}
    
/* Internal function that reads the default styles from a CSS file */
void
style_init()
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	GHashTable *default_text_grid_styles = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);
	GHashTable *default_text_buffer_styles = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);
	GHashTable *current_text_grid_styles = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);
	GHashTable *current_text_buffer_styles = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);
	GtkTextTag *tag;

	/* Create the CSS file scanner */
	GScanner *scanner = g_scanner_new(NULL);

	int f = open(glk_data->css_file, O_RDONLY);
	g_return_if_fail(f != -1);
	g_scanner_input_file(scanner, f);
	scanner->input_name = glk_data->css_file;
	scanner->config->cset_identifier_first = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ#";
	scanner->config->cset_identifier_nth = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_0123456789";
	scanner->config->symbol_2_token = TRUE;
	scanner->config->cpair_comment_single = NULL;
	scanner->config->scan_float = FALSE;

	/* Initialise the default styles */
	g_hash_table_insert(default_text_grid_styles, "normal", gtk_text_tag_new("normal"));

	tag = gtk_text_tag_new("emphasized");
	g_object_set(tag, "style", PANGO_STYLE_ITALIC, "style-set", TRUE, NULL);
	g_hash_table_insert(default_text_grid_styles, "emphasized", tag);

	tag = gtk_text_tag_new("preformatted");
	g_object_set(tag, "font-desc", glk_data->monospace_font_desc, NULL);
	g_hash_table_insert(default_text_grid_styles, "preformatted", tag);

	tag = gtk_text_tag_new("header");
	g_object_set(tag, "size-points", 18.0, "weight", PANGO_WEIGHT_BOLD, NULL);
	g_hash_table_insert(default_text_grid_styles, "header", tag);

	tag = gtk_text_tag_new("subheader");
	g_object_set(tag, "size-points", 14.0, "weight", PANGO_WEIGHT_BOLD, NULL);
	g_hash_table_insert(default_text_grid_styles, "subheader", tag);

	tag = gtk_text_tag_new("alert");
	g_object_set(tag, "foreground", "#aa0000", "weight", PANGO_WEIGHT_BOLD, NULL);
	g_hash_table_insert(default_text_grid_styles, "alert", tag);

	tag = gtk_text_tag_new("note");
	g_object_set(tag, "foreground", "#aaaa00", "weight", PANGO_WEIGHT_BOLD, NULL);
	g_hash_table_insert(default_text_grid_styles, "note", tag);

	tag = gtk_text_tag_new("block-quote");
	g_object_set(tag, "justification", GTK_JUSTIFY_CENTER, "style", PANGO_STYLE_ITALIC, NULL);
	g_hash_table_insert(default_text_grid_styles, "block-quote", tag);

	g_hash_table_insert(default_text_grid_styles, "input", gtk_text_tag_new("input"));
	g_hash_table_insert(default_text_grid_styles, "user1", gtk_text_tag_new("user1"));
	g_hash_table_insert(default_text_grid_styles, "user2", gtk_text_tag_new("user2"));

	g_hash_table_foreach(default_text_grid_styles, style_table_copy, default_text_buffer_styles);
	glk_data->default_styles->text_grid = default_text_grid_styles;
	glk_data->default_styles->text_buffer = default_text_buffer_styles;

	/* Run the scanner over the CSS file, overriding defaults */
	while( g_scanner_peek_next_token(scanner) != G_TOKEN_EOF) {
		if( !style_accept_style_selector(scanner) )
			break;
	}

	/* Set the current style to a copy of the default style */
	g_hash_table_foreach(default_text_grid_styles, style_table_copy, current_text_grid_styles);
	g_hash_table_foreach(default_text_buffer_styles, style_table_copy, current_text_buffer_styles);
	glk_data->current_styles->text_grid = current_text_grid_styles;
	glk_data->current_styles->text_buffer = current_text_buffer_styles;

	g_scanner_destroy(scanner);

	chimara_style_initialized = TRUE;
}

/* Internal function: parses a token */
static gboolean
style_accept(GScanner *scanner, GTokenType token)
{
	GTokenType next = g_scanner_get_next_token(scanner);
   	if(next	!= token ) {
		g_scanner_unexp_token(scanner, token, NULL, NULL, NULL, "CSS Error", 1);
		return FALSE;
	} else {
		return TRUE;
	}
}

/* Internal function: parses a style selector */
static gboolean
style_accept_style_selector(GScanner *scanner)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);

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

	if( !style_accept(scanner, '.') )
		return FALSE;

	token = g_scanner_get_next_token(scanner);
	value = g_scanner_cur_value(scanner);

	if(token != G_TOKEN_IDENTIFIER) {
		g_scanner_error(scanner, "CSS Error: style selector expected");
		return FALSE;
	}

	if( !strcmp(field, "buffer") )
		current_tag = g_hash_table_lookup(glk_data->default_styles->text_buffer, value.v_identifier);
	else
		current_tag = g_hash_table_lookup(glk_data->default_styles->text_grid, value.v_identifier);

	if(current_tag == NULL) {
		g_scanner_error(scanner, "CSS Error: invalid style identifier");
		return FALSE;
	}

	if( !style_accept(scanner, '{') )
		return FALSE;

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

/* Internal function: parses a glk color to a #hex-value */
static void
color_format(glui32 val, gchar *buffer)
{
	g_return_if_fail(buffer != NULL);

	sprintf(buffer, "#%02X%02X%02X",
		((val & 0xff0000) >> 16),
		((val & 0x00ff00) >> 8),
		(val & 0x0000ff)
	);
}

/* Internal function: parses a GdkColor to a glk color */
static glui32
color_parse_gdk(GdkColor *color)
{
	g_return_val_if_fail(color != NULL, 0);
	return (glui32) color->pixel;
}

/* Internal function: changes a GTK tag to correspond with the given style. */
static void
apply_stylehint_to_tag(GtkTextTag *tag, glui32 hint, glsi32 val)
{
	g_return_if_fail(tag != NULL);

	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	GObject *tag_object = G_OBJECT(tag);
	gint reverse_color = 0;

	/* FIXME where should we keep track of this?
	g_object_get(tag, "reverse_color", &reverse_color, NULL);
	*/

	int i = 0;
	gchar color[20];
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
		g_object_set(tag_object, "size", 14+(2*val), "size-set", TRUE, NULL);
		break;

	case stylehint_Oblique:
		g_object_set(tag_object, "style", val ? PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL, "style-set", TRUE, NULL);
		break;

	case stylehint_Proportional:
		g_object_set(tag_object, "font-desc", val ? glk_data->default_font_desc : glk_data->monospace_font_desc, NULL);
		break;

	case stylehint_TextColor:
		color_format(val, color);

		if(!reverse_color)
			g_object_set(tag_object, "foreground", color, "foreground-set", TRUE, NULL);
		else
			g_object_set(tag_object, "background", color, "background-set", TRUE, NULL);

		break;

	case stylehint_BackColor:
		color_format(val, color);

		if(!reverse_color)
			g_object_set(tag_object, "background", color, "background-set", TRUE, NULL);
		else
			g_object_set(tag_object, "foreground", color, "background-set", TRUE, NULL);

		break;

	case stylehint_ReverseColor:
		if(reverse_color != val) {
			/* Flip the fore- and background colors */
			GdkColor* foreground_color;
			GdkColor* background_color;
			gint f_set, b_set = 0;
			g_object_get(tag_object, "foreground-set", &f_set, "background-set", &b_set, NULL);

			if(f_set)
				g_object_get(tag_object, "foreground-gdk", &foreground_color, NULL);
			if(b_set)
				g_object_get(tag_object, "background-gdk", &background_color, NULL);

			if(b_set)
				g_object_set(tag_object, "foreground-gdk", background_color, NULL);
			else
				g_object_set(tag_object, "foreground", "#ffffff", NULL);

			if(f_set)
				g_object_set(tag_object, "background-gdk", foreground_color, NULL);
			else
				g_object_set(tag_object, "background", "#000000", NULL);
		}
		break;

	default:
		WARNING("Unknown style hint");
	}
}
/*Internal function: queries a text tag for the value of a given style hint */
static gint
query_tag(GtkTextTag *tag, glui32 hint)
{
	gint intval;
	GObject *objval;
	GdkColor *colval;

	g_return_val_if_fail(tag != NULL, 0);

	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);

	switch(hint) {
	case stylehint_Indentation:
		g_object_get(tag, "left_margin", &intval, NULL);
		return intval/5;
		break;
	
	case stylehint_ParaIndentation:
		g_object_get(tag, "indent", &intval, NULL);
		return intval/5;
		break;

	case stylehint_Justification:
		g_object_get(tag, "justification", &intval, NULL);
		switch(intval) {
			case GTK_JUSTIFY_LEFT: return stylehint_just_LeftFlush; break;
			case GTK_JUSTIFY_FILL: return stylehint_just_LeftRight; break;
			case GTK_JUSTIFY_CENTER: return stylehint_just_Centered; break;
			case GTK_JUSTIFY_RIGHT: return stylehint_just_RightFlush; break;
			default: 
				WARNING("Unknown justification");
				return stylehint_just_LeftFlush;
		}
		break;

	case stylehint_Weight:
		g_object_get(tag, "weight", &intval, NULL);
		switch(intval) {
			case PANGO_WEIGHT_LIGHT: return -1; break;
			case PANGO_WEIGHT_NORMAL: return 0; break;
			case PANGO_WEIGHT_BOLD: return 1; break;
			default: WARNING("Unknown font weight"); return 0;
		}
		break;

	case stylehint_Size:
		g_object_get(tag, "size", &intval, NULL);
		return (intval/2)-14;
		break;

	case stylehint_Oblique:
		g_object_get(tag, "style", &intval , NULL);
		return intval == PANGO_STYLE_ITALIC ? 1 : 0;
		break;

	case stylehint_Proportional:
		g_object_get(tag, "font-desc", &objval, NULL);
		return objval == (GObject *)glk_data->monospace_font_desc ? 0 : 1;
		break;

	case stylehint_TextColor:
		g_object_get(tag, "foreground-gdk", &colval, NULL);
		return color_parse_gdk(colval);
		break;

	case stylehint_BackColor:
		g_object_get(tag, "background-gdk", &colval, NULL);
		return color_parse_gdk(colval);
		break;

	case stylehint_ReverseColor:
		/* FIXME: implement this */
		return 0;
		break;

	default:
		WARNING("Unknown style hint");
	}
	
	return 0;
}

/**
 * glk_stylehint_set:
 * @wintype: The window type to set a style hint on, or %wintype_AllTypes.
 * @styl: The style to set a hint for.
 * @hint: The type of style hint, one of the <code>stylehint_</code> constants.
 * @val: The style hint. The meaning of this depends on @hint.
 *
 * Sets a hint about the appearance of one style for a particular type of 
 * window. You can also set wintype to %wintype_AllTypes, which sets a hint for 
 * all types of window.
 * <note><para>
 *  There is no equivalent constant to set a hint for all styles of a single 
 *  window type.
 * </para></note>
 */
void
glk_stylehint_set(glui32 wintype, glui32 styl, glui32 hint, glsi32 val)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);

	if( G_UNLIKELY(!chimara_style_initialized) ) {
		style_init();
	}

	GtkTextTag *to_change;
	if(wintype == wintype_TextBuffer || wintype == wintype_AllTypes) {
		to_change = g_hash_table_lookup( glk_data->current_styles->text_buffer, get_tag_name(styl) );
		apply_stylehint_to_tag(to_change, hint, val);
	}

	if(wintype == wintype_TextGrid || wintype == wintype_AllTypes) {
		to_change = g_hash_table_lookup( glk_data->current_styles->text_grid, get_tag_name(styl) );
		apply_stylehint_to_tag(to_change, hint, val);
	}
}

void
glk_stylehint_clear(glui32 wintype, glui32 styl, glui32 hint)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	GtkTextTag *tag;

	switch(wintype) {
	case wintype_TextBuffer:
		tag = g_hash_table_lookup( glk_data->default_styles->text_buffer, get_tag_name(styl) );
		glk_stylehint_set( wintype, styl, hint, query_tag(tag, hint) );
		break;
	case wintype_TextGrid:
		tag = g_hash_table_lookup( glk_data->default_styles->text_grid, get_tag_name(styl) );
		glk_stylehint_set( wintype, styl, hint, query_tag(tag, hint) );
	default:
		return;
	}
}

glui32
glk_style_distinguish(winid_t win, glui32 styl1, glui32 styl2)
{
	return styl1 != styl2;
}

glui32
glk_style_measure(winid_t win, glui32 styl, glui32 hint, glui32 *result)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	GtkTextTag *tag;

	switch(win->type) {
	case wintype_TextBuffer:
		tag = g_hash_table_lookup( glk_data->current_styles->text_buffer, get_tag_name(styl) );
		*result = query_tag(tag, hint);
		break;
	case wintype_TextGrid:
		tag = g_hash_table_lookup( glk_data->current_styles->text_grid, get_tag_name(styl) );
		*result = query_tag(tag, hint);
	default:
		return FALSE;
	}

	return TRUE;
}
