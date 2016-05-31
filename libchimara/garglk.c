#include <glib.h>
#include <glib/gprintf.h>

#include "chimara-glk-private.h"
#include "fileref.h"
#include "glk.h"
#include "magic.h"
#include "stream.h"
#include "ui-message.h"
#include "window.h"

extern GPrivate glk_data_key;

void ui_window_set_reverse_video(winid_t win, gboolean reverse);

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
 * |[<!--language="C"-->
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
 * |[<!--language="C"-->
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
 * |[<!--language="C"-->
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
	WARNING("Not implemented");
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

	WARNING("Not implemented");
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
	
	WARNING("Not implemented");
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
	g_debug("garglk_set_zcolors_stream(str->rock=%d, fg=%08X, bg=%08X)", str->rock, fg, bg);

	VALID_STREAM(str, return);
	g_return_if_fail(str->window != NULL);

	UiMessage *msg = ui_message_new(UI_MESSAGE_SET_ZCOLORS, str->window);
	msg->uintval1 = fg;
	msg->uintval2 = bg;
	ui_message_queue(msg);
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
	g_debug("garglk_set_reversevideo_stream(str->rock=%d, reverse=%d)", str->rock, reverse);

	VALID_STREAM(str, return);

	winid_t win = str->window;
	g_return_if_fail(win != NULL);
	g_return_if_fail(win->type != wintype_TextBuffer || win->type != wintype_TextGrid);

	UiMessage *msg = ui_message_new(UI_MESSAGE_SET_REVERSE_VIDEO, win);
	msg->boolval = reverse;
	ui_message_queue(msg);
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
