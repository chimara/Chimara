#include <math.h>
#include <string.h>

#include <gtk/gtk.h>

#include "chimara-glk.h"
#include "chimara-glk-private.h"
#include "glk.h"
#include "magic.h"
#include "stream.h"
#include "ui-misc.h"
#include "window.h"

/* Determine the current colors used to render the text for a given stream.
 * This can be set in a number of places */
static void
style_cascade_colors(GtkTextTag *tag, GtkTextTag *glk_tag, GtkTextTag *default_tag, GdkRGBA **foreground, GdkRGBA **background)
{
	gboolean foreground_set = FALSE;
	gboolean background_set = FALSE;
	gint reverse_color;
	GdkRGBA *fg = NULL;
	GdkRGBA *bg = NULL;

	// Default color
	reverse_color = GPOINTER_TO_INT( g_object_get_data(G_OBJECT(default_tag), "reverse-color") );
	g_object_get(default_tag, "foreground-set", &foreground_set, "background-set", &background_set, NULL);
	if(foreground_set)
		g_object_get(default_tag, "foreground-rgba", reverse_color ? &bg : &fg, NULL);
	if(background_set)
		g_object_get(default_tag, "background-rgba", reverse_color ? &fg : &bg, NULL);

	// Player defined color
	reverse_color = GPOINTER_TO_INT( g_object_get_data(G_OBJECT(tag), "reverse-color") );
	g_object_get(tag, "foreground-set", &foreground_set, "background-set", &background_set, NULL);
	if(foreground_set) {
		GdkRGBA **dest = reverse_color ? &bg : &fg;
		if (*dest != NULL)
			gdk_rgba_free(*dest);
		g_object_get(tag, "foreground-rgba", dest, NULL);
	}
	if(background_set) {
		GdkRGBA **dest = reverse_color ? &fg : &bg;
		if (*dest != NULL)
			gdk_rgba_free(*dest);
		g_object_get(tag, "background-rgba", dest, NULL);
	}

	// GLK defined color
	reverse_color = GPOINTER_TO_INT( g_object_get_data(G_OBJECT(glk_tag), "reverse-color") );
	g_object_get(glk_tag, "foreground-set", &foreground_set, "background-set", &background_set, NULL);
	if(foreground_set) {
		GdkRGBA **dest = reverse_color ? &bg : &fg;
		if (*dest != NULL)
			gdk_rgba_free(*dest);
		g_object_get(glk_tag, "foreground-rgba", dest, NULL);
	}
	if(background_set) {
		GdkRGBA **dest = reverse_color ? &fg : &bg;
		if (*dest != NULL)
			gdk_rgba_free(*dest);
		g_object_get(glk_tag, "background-rgba", dest, NULL);
	}

	*foreground = fg;
	*background = bg;
}

/* Internal function: changes a GTK tag to correspond with the given style. */
static void
ui_style_apply_hint_to_tag(ChimaraGlk *glk, GtkTextTag *tag, unsigned wintype, unsigned styl, unsigned hint, int val)
{
	g_return_if_fail(tag != NULL);

	GObject *tag_object = G_OBJECT(tag);

	int reverse_color = GPOINTER_TO_INT( g_object_get_data(tag_object, "reverse-color") );
	int i = 0;
	GdkRGBA color;

	switch(hint) {
	case stylehint_Indentation:
		g_object_set(tag_object, "left-margin", 5 * val, "left-margin-set", TRUE, NULL);
		g_object_set(tag_object, "right-margin", 5 * val, "right-margin-set", TRUE, NULL);
		break;

	case stylehint_ParaIndentation:
		g_object_set(tag_object, "indent", 5 * val, "indent-set", TRUE, NULL);
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
			double scale = PANGO_SCALE_MEDIUM;
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
		char *font_family;
		gboolean family_set;

		if(wintype != wintype_TextBuffer) {
		   	if(val)
				WARNING("Style hint 'proportional' only supported on text buffers.");

			break;
		}

		GtkTextTag *font_tag = chimara_glk_get_tag(glk, CHIMARA_GLK_TEXT_BUFFER, val? "default" : "preformatted");
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
		/* Determine the current colors. If all fails, use black/white.
		 * FIXME: Use system theme here */
		GdkRGBA foreground, background;
		gdk_rgba_parse(&foreground, "black");
		gdk_rgba_parse(&background, "white");
		GdkRGBA *current_foreground = &foreground;
		GdkRGBA *current_background = &background;

		ChimaraGlkWindowType chimara_wintype = wintype == wintype_TextBuffer? CHIMARA_GLK_TEXT_BUFFER : CHIMARA_GLK_TEXT_GRID;
		GtkTextTag *default_tag = chimara_glk_get_tag(glk, chimara_wintype, "default");
		GtkTextTag *base_tag = chimara_glk_get_tag(glk, chimara_wintype, chimara_glk_get_tag_name(styl));
		style_cascade_colors(base_tag, tag, default_tag, &current_foreground, &current_background);

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
			NULL);

		g_object_set_data( tag_object, "reverse-color", GINT_TO_POINTER(val != 0) );
		break;
	}

	default:
		WARNING("Unknown style hint");
	}
}

/* Sets a style hint @hint to @val on the global style @styl for windows of type
 * @wintype.
 * Called as a result of glk_stylehint_set(). */
void
ui_style_set_hint(ChimaraGlk *glk, unsigned wintype, unsigned styl, unsigned hint, int val)
{
	GtkTextTag *to_change;
	if(wintype == wintype_TextBuffer || wintype == wintype_AllTypes) {
		to_change = chimara_glk_get_glk_tag(glk, CHIMARA_GLK_TEXT_BUFFER, chimara_glk_get_glk_tag_name(styl));
		ui_style_apply_hint_to_tag(glk, to_change, wintype_TextBuffer, styl, hint, val);
	}

	if(wintype == wintype_TextGrid || wintype == wintype_AllTypes) {
		to_change = chimara_glk_get_glk_tag(glk, CHIMARA_GLK_TEXT_GRID, chimara_glk_get_glk_tag_name(styl));
		ui_style_apply_hint_to_tag(glk, to_change, wintype_TextGrid, styl, hint, val);
	}
}

/* Internal function: parses a GdkRGBA to a glk color */
static uint32_t
gdkrgba_to_glkcolor(GdkRGBA *color)
{
	g_return_val_if_fail(color != NULL, 0);
	return (uint32_t) ((int)(color->red * 255) << 16
	                 | (int)(color->green * 255) << 8
	                 | (int)(color->blue * 255));
}

/* Internal function: queries a text tag for the value of a given style hint */
static int
ui_style_query_tag(ChimaraGlk *glk, GtkTextTag *tag, unsigned wintype, unsigned hint)
{
	int intval;
	double doubleval;
	GdkRGBA *colval;

	g_return_val_if_fail(tag != NULL, 0);

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
		return (int)round(log(doubleval) / log(1.2));

	case stylehint_Oblique:
		g_object_get(tag, "style", &intval , NULL);
		return intval == PANGO_STYLE_ITALIC ? 1 : 0;

	case stylehint_Proportional:
		/* Use pango_font_family_is_monospace()? */
	{
		char *font_family, *query_font_family;
		ChimaraGlkWindowType chimara_wintype = wintype == wintype_TextBuffer? CHIMARA_GLK_TEXT_BUFFER : CHIMARA_GLK_TEXT_GRID;
		GtkTextTag *font_tag = chimara_glk_get_tag(glk, chimara_wintype, "preformatted");
		g_object_get(font_tag, "family", &font_family, NULL);
		g_object_get(tag, "family", &query_font_family, NULL);
		int retval = strcmp(font_family, query_font_family)? 0 : 1;
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

/* Removes a style hint @hint from the global style @styl for windows of type
 * @wintype.
 * Called as a result of glk_stylehint_clear(). */
void
ui_style_clear_hint(ChimaraGlk *glk, unsigned wintype, unsigned styl, unsigned hint)
{
	GtkTextTag *tag;

	if(wintype == wintype_TextBuffer || wintype == wintype_AllTypes) {
		tag = chimara_glk_get_glk_tag(glk, CHIMARA_GLK_TEXT_BUFFER, chimara_glk_get_glk_tag_name(styl));
		if(tag) {
			glsi32 val = ui_style_query_tag(glk, tag, wintype_TextBuffer, hint);
			ui_style_apply_hint_to_tag(glk, tag, wintype_TextBuffer, styl, hint, val);
		}
	}

	if(wintype == wintype_TextGrid || wintype == wintype_AllTypes) {
		tag = chimara_glk_get_glk_tag(glk, CHIMARA_GLK_TEXT_GRID, chimara_glk_get_glk_tag_name(styl));
		if(tag) {
			glsi32 val = ui_style_query_tag(glk, tag, wintype_TextGrid, hint);
			ui_style_apply_hint_to_tag(glk, tag, wintype_TextGrid, styl, hint, val);
		}
	}
}

/* Queries the current value of style hint @hint for global style @styl on text
 * grid or text buffer window @win.
 * Returns the style hint value in the lower 32 bits of the return value, or
 * 1 << 32 if no value could be found.
 * Called as a result of glk_style_measure(). */
int64_t
ui_window_measure_style(winid_t win, ChimaraGlk *glk, unsigned styl, unsigned hint)
{
	GtkTextTag *tag;
	int64_t result;

	switch(win->type) {
	case wintype_TextBuffer:
		tag = chimara_glk_get_glk_tag(glk, CHIMARA_GLK_TEXT_BUFFER, chimara_glk_get_glk_tag_name(styl));
		result = ui_style_query_tag(glk, tag, win->type, hint);
		break;
	case wintype_TextGrid:
		tag = chimara_glk_get_glk_tag(glk, CHIMARA_GLK_TEXT_GRID, chimara_glk_get_glk_tag_name(styl));
		result = ui_style_query_tag(glk, tag, win->type, hint);
	default:
		return (int64_t)1 << 32;
	}

	return result; /* Must fit within 32 bits */
}

/* Internal function copying the attributes of a text tag to a pango attribute list */
static void
text_tag_to_attr_list(GtkTextTag *tag, PangoAttrList *list)
{
	gboolean set;
	GdkRGBA *foreground, *background;
	char *string;
	PangoFontDescription *font_desc;
	gboolean strikethrough;
	PangoUnderline underline;

	g_object_get(tag,
		"foreground-set", &set,
		"foreground-rgba", &foreground,
		NULL);
	if(set) {
		pango_attr_list_insert(list,
			pango_attr_foreground_new(foreground->red, foreground->green, foreground->blue));
	}
	gdk_rgba_free(foreground);

	g_object_get(tag,
		"background-set", &set,
		"background-rgba", &background,
		NULL);
	if(set) {
		pango_attr_list_insert(list,
			pango_attr_background_new(background->red, background->green, background->blue));
	}
	gdk_rgba_free(background);

	g_object_get(tag,
		"language-set", &set,
		"language", &string,
		NULL);
	if(set) {
		pango_attr_list_insert(list,
			pango_attr_language_new( pango_language_from_string(string) ));
	}
	g_free(string);

	/* Font description updates the following properties simultaniously:
	 * family, style, weight, variant, stretch, size.
	 * FIXME: Except it doesn't really.
	 */
	g_object_get(tag, "font-desc", &font_desc, NULL);
	pango_attr_list_insert(list, pango_attr_font_desc_new(font_desc));
	pango_font_description_free(font_desc);

	g_object_get(tag,
		"strikethrough-set", &set,
		"strikethrough", &strikethrough,
		NULL);
	if(set) {
		pango_attr_list_insert(list,
			pango_attr_strikethrough_new(strikethrough));
	}

	g_object_get(tag,
		"underline-set", &set,
		"underline", &underline,
		NULL);
	if(set) {
		pango_attr_list_insert(list, pango_attr_underline_new(underline));
	}
}

/* Internal function returning the current default font for a window type
 * This can be used later for size calculations. Only wintype_TextGrid and
 * wintype_TextBuffer are supported for now. Free return value with
 * pango_font_description_free(). */
PangoFontDescription *
ui_style_get_current_font(ChimaraGlk *glk, unsigned wintype)
{
	PangoFontDescription *font;
	ChimaraGlkWindowType chimara_wintype;

	switch(wintype) {
	case wintype_TextGrid:
		chimara_wintype = CHIMARA_GLK_TEXT_BUFFER;
		font = pango_font_description_from_string("Monospace");
		break;
	case wintype_TextBuffer:
		chimara_wintype = CHIMARA_GLK_TEXT_GRID;
		font = pango_font_description_from_string("Serif");
		break;
	default:
		return NULL;
	}

	PangoAttrList *list = pango_attr_list_new();

	text_tag_to_attr_list(chimara_glk_get_tag(glk, chimara_wintype, "default"), list);
	PangoAttrIterator *it = pango_attr_list_get_iterator(list);
	pango_attr_iterator_get_font(it, font, NULL, NULL);
	pango_attr_iterator_destroy(it);

	text_tag_to_attr_list(chimara_glk_get_tag(glk, chimara_wintype, "normal"), list);
	it = pango_attr_list_get_iterator(list);
	pango_attr_iterator_get_font(it, font, NULL, NULL);
	pango_attr_iterator_destroy(it);

	text_tag_to_attr_list(chimara_glk_get_glk_tag(glk, chimara_wintype, "glk-normal"), list );
	it = pango_attr_list_get_iterator(list);
	pango_attr_iterator_get_font(it, font, NULL, NULL);
	pango_attr_iterator_destroy(it);

	/* Make a copy of the family, preventing it's destruction at the end of this function. */
	pango_font_description_set_family( font, pango_font_description_get_family(font) );

	pango_attr_list_unref(list);

	return font;
}

/* Determine the current colors used to render the text for a given window.
 * This can be set in a number of places */
void
ui_style_get_window_colors(winid_t win, GdkRGBA **foreground, GdkRGBA **background)
{
	VALID_WINDOW(win, return);
	g_return_if_fail(win->type != wintype_TextBuffer || win->type != wintype_TextGrid);

	GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
	GtkTextTagTable *tags = gtk_text_buffer_get_tag_table(buffer);
	GtkTextTag* default_tag = gtk_text_tag_table_lookup(tags, "default");
	GtkTextTag *tag = gtk_text_tag_table_lookup(tags, win->style_tagname);
	GtkTextTag *glk_tag = gtk_text_tag_table_lookup(tags, win->glk_style_tagname);

	style_cascade_colors(tag, glk_tag, default_tag, foreground, background);

	gboolean foreground_set, background_set;

	// Windows can have zcolors defined
	if(win->zcolor) {
		g_object_get(win->zcolor,
			"foreground-set", &foreground_set,
			"background-set", &background_set,
			NULL);
		if(foreground_set)
			g_object_get(win->zcolor, "foreground-rgba", foreground, NULL);
		if(background_set)
			g_object_get(win->zcolor, "background-rgba", background, NULL);
	}
}

/* Apply styles to a segment of text in a GtkTextBuffer, combining multiple
 * GtkTextTags.
 */
void
ui_style_apply(winid_t win, GtkTextIter *start, GtkTextIter *end)
{
	GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
	GtkTextTagTable *tags = gtk_text_buffer_get_tag_table(buffer);

	GtkTextTag *default_tag = gtk_text_tag_table_lookup(tags, "default");
	GtkTextTag *style_tag = gtk_text_tag_table_lookup(tags, win->style_tagname);
	GtkTextTag *glk_style_tag = gtk_text_tag_table_lookup(tags, win->glk_style_tagname);

	/* Player's style overrides */
	gtk_text_buffer_apply_tag(buffer, style_tag, start, end);

	/* Glk program's style overrides */
	gtk_text_buffer_apply_tag(buffer, glk_style_tag, start, end);

	/* Default style */
	gtk_text_buffer_apply_tag(buffer, default_tag, start, end);

	/* Link style overrides */
	if(win->window_stream->hyperlink_mode) {
		GtkTextTag *link_style_tag = gtk_text_tag_table_lookup(tags, "hyperlink");
		GtkTextTag *link_tag = win->current_hyperlink->tag;
		gtk_text_buffer_apply_tag(buffer, link_style_tag, start, end);
		gtk_text_buffer_apply_tag(buffer, link_tag, start, end);
	}

	/* Glk program's style overrides using garglk_set_zcolors() */
	if(win->zcolor != NULL)
		gtk_text_buffer_apply_tag(buffer, win->zcolor, start, end);

	/* Glk program's style overrides using garglk_set_reversevideo() */
	if(win->zcolor_reversed != NULL)
		gtk_text_buffer_apply_tag(buffer, win->zcolor_reversed, start, end);
}
