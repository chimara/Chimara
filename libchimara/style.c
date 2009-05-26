#include "style.h"

extern ChimaraGlkPrivate *glk_data;

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

	gtk_text_buffer_create_tag(buffer, "normal", NULL);
	gtk_text_buffer_create_tag(buffer, "emphasized", "style", PANGO_STYLE_ITALIC, NULL);
	gtk_text_buffer_create_tag(buffer, "preformatted", "font-desc", glk_data->monospace_font_desc, NULL);
	gtk_text_buffer_create_tag(buffer, "header", "size-points", 16.0, "weight", PANGO_WEIGHT_BOLD, NULL);
	gtk_text_buffer_create_tag(buffer, "subheader", "size-points", 12.0, "weight", PANGO_WEIGHT_BOLD, NULL);
	gtk_text_buffer_create_tag(buffer, "alert", "foreground", "#aa0000", "weight", PANGO_WEIGHT_BOLD, NULL);
	gtk_text_buffer_create_tag(buffer, "note", "foreground", "#aaaa00", "weight", PANGO_WEIGHT_BOLD, NULL);
	gtk_text_buffer_create_tag(buffer, "block-quote", "justification", GTK_JUSTIFY_CENTER, "style", PANGO_STYLE_ITALIC, NULL);
	gtk_text_buffer_create_tag(buffer, "input", NULL);
	gtk_text_buffer_create_tag(buffer, "user1", NULL);
	gtk_text_buffer_create_tag(buffer, "user2", NULL);
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
	winid_t win = glk_window_iterate(NULL, NULL);
	while(win != NULL) {
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
