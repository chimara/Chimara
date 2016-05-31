#include <string.h>

#include <glib.h>

#include "chimara-glk-private.h"
#include "stream.h"
#include "ui-message.h"
#include "window.h"

extern GPrivate glk_data_key;

static gboolean style_accept(GScanner *scanner, GTokenType token);
static gboolean style_accept_style_selector(GScanner *scanner, ChimaraGlk *glk);
static gboolean style_accept_style_hint(GScanner *scanner, GtkTextTag *current_tag);

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

/**
 * glk_set_style_stream:
 * @str: Output stream to change the style of
 * @styl: The style to apply
 *
 * This changes the style of the stream @str. See glk_set_style().
 */
void
glk_set_style_stream(strid_t str, glui32 styl) {
	g_debug("glk_set_style(str->rock=%d, styl=%d)", str->rock, styl);

	if(str->window == NULL)
		return;

	UiMessage *msg = ui_message_new(UI_MESSAGE_SET_STYLE, str->window);
	msg->uintval1 = styl;
	ui_message_queue(msg);
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
	g_debug("glk_stylehint_set(wintype=%d, styl=%d, hint=%d, val=%d)", wintype, styl, hint, val);

	if(wintype != wintype_TextGrid && wintype != wintype_TextBuffer && wintype != wintype_AllTypes)
		return;

	UiMessage *msg = ui_message_new(UI_MESSAGE_SET_STYLEHINT, NULL);
	msg->uintval1 = wintype;
	msg->uintval2 = styl;
	msg->uintval3 = hint;
	msg->intval = val;
	ui_message_queue(msg);
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
	g_debug("glk_stylehint_clear(wintype=%d, styl=%d, hint=%d)", wintype, styl, hint);

	if(wintype != wintype_TextGrid && wintype != wintype_TextBuffer && wintype != wintype_AllTypes)
		return;

	UiMessage *msg = ui_message_new(UI_MESSAGE_CLEAR_STYLEHINT, NULL);
	msg->uintval1 = wintype;
	msg->uintval2 = styl;
	msg->uintval3 = hint;
	ui_message_queue(msg);
}

/**
 * glk_style_distinguish:
 * @win: The window in which the styles are to be distinguished.
 * @styl1: The first style to be distinguished from the second style.
 * @styl2: The second style to be distinguished from the first style.
 * 
 * Decides whether two styles are visually distinguishable in the given window.
 * The exact meaning of this is left for the library to determine.
 *
 * > # Chimara #
 * > Currently, all styles of one window are assumed to be mutually
 * > distinguishable.
 *
 * Returns: %TRUE (1) if the two styles are visually distinguishable. If they 
 * are not, it returns %FALSE (0).
 */
glui32
glk_style_distinguish(winid_t win, glui32 styl1, glui32 styl2)
{
	g_debug("glk_style_distinguish(win->rock=%d, styl1=%d, styl2=%d)", win->rock, styl1, styl2);

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
	g_debug("glk_style_measure(win->rock=%d, styl=%d, hint=%d, result=...)", win->rock, styl, hint);

	/* This function is planned for deprecation in a future version of the Glk
	spec where more CSS-like styling is implemented. See for more information:
	http://ifwiki.org/index.php/New_Glk_styles */

	UiMessage *msg = ui_message_new(UI_MESSAGE_MEASURE_STYLE, win);
	msg->uintval1 = styl;
	msg->uintval2 = hint;
	gint64 response = ui_message_queue_and_await(msg);
	if(response & ((gint64)1 << 32))
		return FALSE;
	if(result)
		*result = response;
	return TRUE;
}
