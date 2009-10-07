#include "style.h"
#include <stdio.h>
#include <fcntl.h>

extern GPrivate *glk_data_key;
static gboolean chimara_style_initialized = FALSE;
static gboolean style_accept(GScanner *scanner, GTokenType token);
static gboolean style_accept_style_identifier(GScanner *scanner);
static gboolean style_accept_style_hint(GScanner *scanner, GtkTextTag *current_tag);
static void style_add_tag_to_textbuffer(gpointer key, gpointer tag, gpointer tag_table);

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

/* Internal function: mapping from style enum to tag name */
static gchar *
get_tag_name(glui32 style)
{
	switch(style) {
		case style_Normal: return "normal";
		case style_Emphasized: return "emphasized";
		case style_Preformatted: return "preformatted";
		case style_Header: return "header";
		case style_Subheader: return "subheader";
		case style_Alert: return "alert";
		case style_Note: return "note";
		case style_BlockQuote: return "block-quote";
		case style_Input: return "input";
		case style_User1: return "user1";
		case style_User2: return "user2";
	}

	WARNING("Unsupported style");
	return "normal";
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
	g_hash_table_foreach(glk_data->default_styles, style_add_tag_to_textbuffer, gtk_text_buffer_get_tag_table(buffer));
}

static void
style_add_tag_to_textbuffer(gpointer key, gpointer tag, gpointer tag_table)
{
	gtk_text_tag_table_add(tag_table, tag);
}

void
style_init()
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
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

	/* Initialise the default styles */
	g_hash_table_insert(glk_data->default_styles, "normal", gtk_text_tag_new("normal"));

	tag = gtk_text_tag_new("emphasized");
	g_object_set(tag, "style", PANGO_STYLE_ITALIC, "style-set", TRUE, NULL);
	g_hash_table_insert(glk_data->default_styles, "emphasized", tag);

	tag = gtk_text_tag_new("preformatted");
	g_object_set(tag, "font-desc", glk_data->monospace_font_desc, NULL);
	g_hash_table_insert(glk_data->default_styles, "preformatted", tag);

	tag = gtk_text_tag_new("header");
	g_object_set(tag, "size-points", 18.0, "weight", PANGO_WEIGHT_BOLD, NULL);
	g_hash_table_insert(glk_data->default_styles, "header", tag);

	tag = gtk_text_tag_new("subheader");
	g_object_set(tag, "size-points", 14.0, "weight", PANGO_WEIGHT_BOLD, NULL);
	g_hash_table_insert(glk_data->default_styles, "subheader", tag);

	tag = gtk_text_tag_new("alert");
	g_object_set(tag, "foreground", "#aa0000", "weight", PANGO_WEIGHT_BOLD, NULL);
	g_hash_table_insert(glk_data->default_styles, "alert", tag);

	tag = gtk_text_tag_new("note");
	g_object_set(tag, "foreground", "#aaaa00", "weight", PANGO_WEIGHT_BOLD, NULL);
	g_hash_table_insert(glk_data->default_styles, "note", tag);

	tag = gtk_text_tag_new("block-quote");
	g_object_set(tag, "justification", GTK_JUSTIFY_CENTER, "style", PANGO_STYLE_ITALIC, NULL);
	g_hash_table_insert(glk_data->default_styles, "block-quote", tag);

	g_hash_table_insert(glk_data->default_styles, "input", gtk_text_tag_new("input"));
	g_hash_table_insert(glk_data->default_styles, "user1", gtk_text_tag_new("user1"));
	g_hash_table_insert(glk_data->default_styles, "user2", gtk_text_tag_new("user2"));

	/* Run the scanner over the CSS file */
	while( g_scanner_peek_next_token(scanner) != G_TOKEN_EOF) {
		if( !style_accept_style_identifier(scanner) )
			break;
	}

	g_scanner_destroy(scanner);
}

static gboolean
style_accept(GScanner *scanner, GTokenType token)
{
	if( g_scanner_get_next_token(scanner) != token ) {
		g_scanner_unexp_token(scanner, token, NULL, NULL, NULL, "CSS Error", 1);
		return FALSE;
	} else {
		return TRUE;
	}
}

static gboolean
style_accept_style_identifier(GScanner *scanner)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);

	GtkTextTag *current_tag;
	GTokenType token = g_scanner_get_next_token(scanner);
	GTokenValue value = g_scanner_cur_value(scanner);

	if(token != G_TOKEN_IDENTIFIER) {
		g_scanner_error(scanner, "CSS Error: style identifier expected");
		return FALSE;
	}

	printf("Identifier: %s\n", value.v_identifier);
	current_tag = g_hash_table_lookup(glk_data->default_styles, value.v_identifier);

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
	printf("Hint: %s\n", hint);

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

static void
color_format(glui32 val, gchar *buffer)
{
	sprintf(buffer, "#%02X%02X%02X",
		((val & 0xff0000) >> 16),
		((val & 0x00ff00) >> 8),
		(val & 0x0000ff)
	);
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
		g_object_set(tag_object, "left_margin", 5*val, NULL);
		g_object_set(tag_object, "right_margin", 5*val, NULL);
		break;
	
	case stylehint_ParaIndentation:
		g_object_set(tag_object, "indent", 5*val, NULL);
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
		g_object_set(tag_object, "justification", i, NULL);
		break;

	case stylehint_Weight:
		switch(val) {
			case -1: i = PANGO_WEIGHT_LIGHT; break;
			case  0: i = PANGO_WEIGHT_NORMAL; break;
			case  1: i = PANGO_WEIGHT_BOLD; break;
			default: WARNING("Unknown font weight");
		}
		g_object_set(tag_object, "weight", i, NULL);
		break;

	case stylehint_Size:
		g_object_set(tag_object, "size", 14+(2*val), NULL);
		break;

	case stylehint_Oblique:
		g_object_set(tag_object, "style", val ? PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL, NULL);
		break;

	case stylehint_Proportional:
		g_object_set(tag_object, "font-desc", val ? glk_data->default_font_desc : glk_data->monospace_font_desc, NULL);
		break;

	case stylehint_TextColor:
		color_format(val, color);

		if(!reverse_color)
			g_object_set(tag_object, "foreground", color, NULL);
		else
			g_object_set(tag_object, "background", color, NULL);

		break;

	case stylehint_BackColor:
		color_format(val, color);

		if(!reverse_color)
			g_object_set(tag_object, "background", color, NULL);
		else
			g_object_set(tag_object, "foreground", color, NULL);

		break;

	case stylehint_ReverseColor:
		if(reverse_color != val) {
			/* Flip the fore- and background colors */
			gchar* foreground_color;
			gchar* background_color;
			g_object_get(tag_object, "foreground", &foreground_color, NULL);
			g_object_get(tag_object, "background", &background_color, NULL);
			g_object_set(tag_object, "foreground", background_color, NULL);
			g_object_set(tag_object, "background", foreground_color, NULL);
			g_free(foreground_color);
			g_free(background_color);
		}
		break;

	default:
		WARNING("Unknown style hint");
	}
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
	gchar *tag_name = get_tag_name(styl);

	/* Iterate over all the window and update their styles if nessecary */
	winid_t win;
	for(win = glk_window_iterate(NULL, NULL); win; win = glk_window_iterate(win, NULL)) {
		if(wintype != wintype_TextBuffer)
			continue; /* FIXME: add support for text grid windows */

		if(wintype == wintype_AllTypes || glk_window_get_type(win) == wintype) {
			GtkWidget *textview = win->widget;
			GtkTextBuffer *textbuffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(textview) );
			GtkTextTagTable *table = gtk_text_buffer_get_tag_table(textbuffer);
			GtkTextTag *to_change = gtk_text_tag_table_lookup(table, tag_name);

			apply_stylehint_to_tag(to_change, hint, val);
		}
	}
}

void
glk_stylehint_clear(glui32 wintype, glui32 styl, glui32 hint)
{
}

glui32
glk_style_distinguish(winid_t win, glui32 styl1, glui32 styl2)
{
	return styl1 != styl2;
}

glui32
glk_style_measure(winid_t win, glui32 styl, glui32 hint, glui32 *result)
{
	return FALSE;
}
