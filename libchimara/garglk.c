#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <libchimara/glk.h>
#include "chimara-glk-private.h"
#include "stream.h"
#include "fileref.h"
#include "style.h"
#include "garglk.h"

#define ZCOLOR_NAME_TEMPLATE "zcolor:%s/%s"

extern GPrivate glk_data_key;

/**
 * garglk_fileref_get_name:
 * @fref: A file reference.
 *
 * Gets the actual disk filename that @fref refers to, in the platform's
 * native filename encoding. The string is owned by @fref and must not be
 * changed or freed.
 *
 * Returns: a string in filename encoding.
 */
char * 
garglk_fileref_get_name(frefid_t fref)
{
	VALID_FILEREF(fref, return NULL);
	return fref->filename;
}

/**
 * garglk_set_program_name:
 * @name: Name of the Glk program that is running.
 *
 * This function is used to let the library know the name of the currently
 * running Glk program, in case it wants to display this information somewhere
 * &mdash; for example, in the title bar of a window. A typical use of this
 * function would be:
 * |[
 * garglk_set_program_name("SuperGlkFrotz 0.1");
 * ]|
 */
void 
garglk_set_program_name(const char *name)
{
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
	glk_data->program_name = g_strdup(name);
	g_object_notify(G_OBJECT(glk_data->self), "program-name");
}

/**
 * garglk_set_program_info:
 * @info: Information about the Glk program that is running.
 *
 * This function is used to provide the library with additional information
 * about the currently running Glk program, in case it wants to display this
 * information somewhere &mdash; for example, in an About box. A typical use of
 * this function would be:
 * |[
 * garglk_set_program_info("SuperGlkFrotz, version 0.1\n"
 *     "Original Frotz by Stefan Jokisch\n"
 *     "Unix port by Jim Dunleavy and David Griffith\n"
 *     "Glk port by Tor Andersson\n"
 *     "Animation, networking, and evil AI by Sven Metcalfe");
 * ]|
 */
void 
garglk_set_program_info(const char *info)
{
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
	glk_data->program_info = g_strdup(info);
	g_object_notify(G_OBJECT(glk_data->self), "program-info");
}

/**
 * garglk_set_story_name:
 * @name: Name of the story that the Glk program is currently interpreting.
 *
 * If the Glk program running is an interactive fiction interpreter, then this
 * function can be used to let the library know the name of the story currently
 * loaded in the interpreter, in case it wants to display this information
 * anywhere &mdash; for example, in the title bar of a window. A typical use of
 * this function would be:
 * |[
 * garglk_set_story_name("Lighan Ses Lion, el Zarf");
 * ]|
 */
void 
garglk_set_story_name(const char *name)
{
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
	glk_data->story_name = g_strdup(name);
	g_object_notify(G_OBJECT(glk_data->self), "story-name");
}

/**
 * garglk_set_story_title:
 * @title: Title bar text for the currently running story.
 *
 * This function is a hint to the library to put @title in the title bar of the
 * window that the Glk program is running in. It overrides
 * garglk_set_program_name() and garglk_set_story_name(), if they were displayed
 * in the title bar, although they may still be displayed somewhere else.
 *
 * <warning><para>This function is not currently implemented.</para></warning>
 */
void
garglk_set_story_title(const char *title)
{
	WARNING(_("Not implemented"));
}

/**
 * garglk_unput_string:
 * @str: a null-terminated string.
 *
 * Removes @str from the end of the current stream, if indeed it is there. The
 * stream's write count is decreased accordingly, and the stream's echo stream
 * is also modified, if it has one.
 *
 * <warning><para>This function is not currently implemented.</para></warning>
 */
void 
garglk_unput_string(char *str)
{
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
	g_return_if_fail(glk_data->current_stream != NULL);

	WARNING(_("Not implemented"));
}

/**
 * garglk_unput_string_uni:
 * @str: a zero-terminated array of Unicode code points.
 *
 * Like garglk_unput_string(), but for Unicode streams.
 *
 * <warning><para>This function is not currently implemented.</para></warning>
 */
void 
garglk_unput_string_uni(glui32 *str)
{
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
	g_return_if_fail(glk_data->current_stream != NULL);
	
	WARNING(_("Not implemented"));
}

/**
 * garglk_set_zcolors_stream:
 * @str: a stream.
 * @fg: a 24-bit foreground color.
 * @bg: a 24-bit background color.
 *
 * This function changes the foreground color of @str to @fg and the background
 * color to @bg. @fg and @bg are encoded the same way as described in
 * %stylehint_TextColor. See garglk_set_zcolors() for more information.
 */
void
garglk_set_zcolors_stream(strid_t str, glui32 fg, glui32 bg)
{
#ifdef DEBUG_STYLES
	g_printf("garglk_set_zcolors_stream(str->rock=%d, fg=%08X, bg=%08X)\n", str->rock, fg, bg);
#endif

	VALID_STREAM(str, return);
	g_return_if_fail(str->window != NULL);

	winid_t window = str->window;

	gdk_threads_enter();

	GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(window->widget) );
	GtkTextTagTable *tags = gtk_text_buffer_get_tag_table(buffer);
	GdkRGBA fore, back;
	GdkRGBA *fore_pointer = NULL;
	GdkRGBA *back_pointer = NULL;
	gchar *fore_name;
	gchar *back_name;

	switch(fg) {
	case zcolor_Transparent:
	case zcolor_Cursor:
		WARNING(_("zcolor_Transparent, zcolor_Cursor not implemented"));
		// Fallthrough to default
	case zcolor_Default:
		fore_name = g_strdup("default");
		break;
	case zcolor_Current:
	{
		if(window->zcolor) {
			// Get the current foreground color
			GdkRGBA *current_color;
			g_object_get(window->zcolor, "foreground-rgba", &current_color, NULL);
			fore_name = gdk_rgba_to_string(current_color);

			// Copy the color and use it
			fore.red = current_color->red;
			fore.green = current_color->green;
			fore.blue = current_color->blue;
			fore_pointer = &fore;
		} else {
			fore_name = g_strdup("default");
		}
		break;
	}
	default:
		glkcolor_to_gdkrgba(fg, &fore);
		fore_pointer = &fore;
		fore_name = gdk_rgba_to_string(&fore);
	}

	switch(bg) {
	case zcolor_Transparent:
	case zcolor_Cursor:
		WARNING(_("zcolor_Transparent, zcolor_Cursor not implemented"));
		// Fallthrough to default
	case zcolor_Default:
		back_name = g_strdup("default");
		break;
	case zcolor_Current:
	{
		if(window->zcolor) {
			// Get the current background color
			GdkRGBA *current_color;
			g_object_get(window->zcolor, "background-rgba", &current_color, NULL);
			back_name = gdk_rgba_to_string(current_color);

			// Copy the color and use it
			back.red = current_color->red;
			back.green = current_color->green;
			back.blue = current_color->blue;
			back_pointer = &back;
		} else {
			back_name = g_strdup("default");
		}
		break;
	}
	default:
		glkcolor_to_gdkrgba(bg, &back);
		back_pointer = &back;
		back_name = gdk_rgba_to_string(&back);
	}

	if(fore_pointer == NULL && back_pointer == NULL) {
		// NULL value means to ignore the zcolor property altogether
		window->zcolor = NULL;
	} else {
		char *name = g_strdup_printf(ZCOLOR_NAME_TEMPLATE, fore_name, back_name);
		g_free(fore_name);
		g_free(back_name);

		// See if we have used this color combination before
		GtkTextTag *tag = gtk_text_tag_table_lookup(tags, name);

		if(tag == NULL) {
			// Create a new texttag with the specified colors
			tag = gtk_text_buffer_create_tag(
				buffer,
				name,
				"foreground-rgba", fore_pointer,
				"foreground-set", fore_pointer != NULL,
				"background-rgba", back_pointer,
				"background-set", back_pointer != NULL,
				NULL
			);
		}

		// From now on, text will be drawn in the specified colors
		window->zcolor = tag;

		// Update the reversed version if necessary
		if(str->window->zcolor_reversed) {
			gint reversed = GPOINTER_TO_INT( g_object_get_data( G_OBJECT(str->window->zcolor_reversed), "reverse-color" ) );

			gdk_threads_leave();
			garglk_set_reversevideo_stream(str, reversed != 0);
			gdk_threads_enter();
		}
	
	}

	gdk_threads_leave();
}

/**
 * garglk_set_zcolors:
 * @fg: a 24-bit foreground color.
 * @bg: a 24-bit background color.
 *
 * Glk works with styles, not specific colors. This is not quite compatible with
 * the Z-machine, so this Glk extension implements Z-machine style colors.
 *
 * This function changes the foreground color of the current stream to @fg and 
 * the background color to @bg. @fg and @bg are encoded the same way as
 * described in %stylehint_TextColor.
 */
void 
garglk_set_zcolors(glui32 fg, glui32 bg)
{
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
	g_return_if_fail(glk_data->current_stream != NULL);

	garglk_set_zcolors_stream(glk_data->current_stream, fg, bg);
}

/**
 * garglk_set_reversevideo_stream:
 * @str: a stream.
 * @reverse: nonzero for reverse colors, zero for normal colors.
 *
 * If @reverse is not zero, uses the foreground color of @str as its background
 * and vice versa. If @reverse is zero, changes the colors of @str back to
 * normal.
 */
void
garglk_set_reversevideo_stream(strid_t str, glui32 reverse)
{
#ifdef DEBUG_STYLES
	g_printf("garglk_set_reversevideo_stream(str->rock=%d, reverse=%d)\n", str->rock, reverse);
#endif

	VALID_STREAM(str, return);
	g_return_if_fail(str->window != NULL);
	g_return_if_fail(str->window->type != wintype_TextBuffer || str->window->type != wintype_TextGrid);

	// Determine the current colors
	
	// If all fails, use black/white
	// FIXME: Use system theme here
	GdkRGBA foreground, background;
	gdk_rgba_parse(&foreground, "black");
	gdk_rgba_parse(&background, "white");
	GdkRGBA *current_foreground = &foreground;
	GdkRGBA *current_background = &background;

	gdk_threads_enter();

	style_stream_colors(str, &current_foreground, &current_background);

	if(reverse) {
		GdkRGBA *temp = current_foreground;
		current_foreground = current_background;
		current_background = temp;
	}

	// Name the color
	char *foreground_name = gdk_rgba_to_string(current_foreground);
	char *background_name = gdk_rgba_to_string(current_background);
	char *name = g_strdup_printf(ZCOLOR_NAME_TEMPLATE, foreground_name, background_name);
	g_free(foreground_name);
	g_free(background_name);

	// Create a tag for the new colors if it doesn't exist yet
	GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(str->window->widget) );	
	GtkTextTagTable *tags = gtk_text_buffer_get_tag_table(buffer);
	GtkTextTag *tag = gtk_text_tag_table_lookup(tags, name);
	if(tag == NULL) {
		tag = gtk_text_buffer_create_tag(
			buffer,
			name,
			"foreground-rgba", current_foreground,
			"foreground-set", TRUE,
			"background-rgba", current_background,
			"background-set", TRUE,
			NULL
		);
		g_object_set_data( G_OBJECT(tag), "reverse-color", GINT_TO_POINTER(reverse) );
	}

	// From now on, text will be drawn in the specified colors
	str->window->zcolor_reversed = tag;

	// Update the background of the gtktextview to correspond with the current background color
	if(current_background != NULL) {
		gtk_widget_override_background_color(str->window->widget, GTK_STATE_FLAG_NORMAL, current_background);
	}

	gdk_threads_leave();
}

/**
 * garglk_set_reversevideo:
 * @reverse: nonzero for reverse colors, zero for normal colors.
 *
 * If @reverse is not zero, uses the foreground color of the current stream as
 * its background and vice versa. If @reverse is zero, changes the colors of the
 * current stream back to normal.
 */
void 
garglk_set_reversevideo(glui32 reverse)
{
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
	g_return_if_fail(glk_data->current_stream != NULL);
	g_return_if_fail(glk_data->current_stream->window != NULL);

	garglk_set_reversevideo_stream(glk_data->current_stream, reverse);
}
