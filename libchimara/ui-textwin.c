#include <string.h>

#include <gtk/gtk.h>

#include "charset.h"
#include "chimara-glk.h"
#include "chimara-glk-private.h"
#include "event.h"
#include "glk.h"
#include "garglk.h"
#include "magic.h"
#include "ui-buffer.h"
#include "ui-grid.h"
#include "ui-misc.h"
#include "ui-style.h"
#include "ui-textwin.h"
#include "ui-window.h"
#include "window.h"

#define ZCOLOR_NAME_TEMPLATE "zcolor:%s/%s"

void
ui_textwin_print_string(winid_t win, const char *text)
{
	switch(win->type) {
	case wintype_TextBuffer:
		ui_buffer_print_string(win, text);
		break;
	case wintype_TextGrid:
		ui_grid_print_string(win, text);
		break;
	}
}

void
ui_textwin_request_line_input(ChimaraGlk *glk, winid_t win, glui32 maxlen, gboolean insert, const char *inserttext)
{
	switch(win->type)
	{
	case wintype_TextBuffer:
		ui_buffer_request_line_event(win, maxlen, insert, inserttext);
		break;
	case wintype_TextGrid:
		ui_grid_request_line_event(win, maxlen, insert, inserttext);
		break;
	}
	g_signal_handler_unblock(win->widget, win->line_input_keypress_handler);

	/* Emit the "waiting" signal to let listeners know we are ready for input */
	g_signal_emit_by_name(glk, "waiting");
}

/* Internal function: finish handling a line input request, for both text grid
and text buffer windows. */
int
ui_textwin_finish_line_input(winid_t win, const char *inserted_text, gboolean emit_signal)
{
	int copycount = 0;

    /* Convert the string from UTF-8 to Latin-1 or Unicode */
    if(win->input_request_type == INPUT_REQUEST_LINE)
    {
        size_t bytes_written;
        char *latin1 = convert_utf8_to_latin1(inserted_text, &bytes_written);

        if(latin1 == NULL)
            return 0;

        /* Place input in the echo stream */
        if(win->echo_stream != NULL)
            glk_put_string_stream(win->echo_stream, latin1);

        /* Copy the string (bytes_written does not include the NULL at the end) */
        copycount = MIN(win->line_input_buffer_max_len, bytes_written);
        memcpy(win->line_input_buffer, latin1, copycount);
        g_free(latin1);
    }
    else if(win->input_request_type == INPUT_REQUEST_LINE_UNICODE)
    {
        long items_written;
        gunichar *unicode = convert_utf8_to_ucs4(inserted_text, &items_written);

        if(unicode == NULL)
            return 0;

        /* Place input in the echo stream */
        if(win->echo_stream != NULL)
            glk_put_string_stream_uni(win->echo_stream, unicode);

        /* Copy the string (but not the NULL at the end) */
        copycount = MIN(win->line_input_buffer_max_len, items_written);
        memcpy(win->line_input_buffer_unicode, unicode, copycount * sizeof(gunichar));
        g_free(unicode);
    }
    else
        WARNING("Wrong input request type");

    win->input_request_type = INPUT_REQUEST_NONE;

	if(emit_signal)
	{
		ChimaraGlk *glk = CHIMARA_GLK(gtk_widget_get_ancestor(win->widget, CHIMARA_TYPE_GLK));
		g_assert(glk);
		g_signal_emit_by_name(glk, "line-input", win->rock, win->librock, inserted_text);
	}

	/* Add the text to the window input history */
	if(win->history_pos != NULL)
	{
		g_free(win->history->data);
		win->history = g_list_delete_link(win->history, win->history);
	}
	if(*inserted_text != 0)
		win->history = g_list_prepend(win->history, g_strdup(inserted_text));

	win->history_pos = NULL;

	return copycount;
}

int
ui_textwin_cancel_line_input(winid_t win)
{
	if(win->type == wintype_TextBuffer)
		return ui_buffer_cancel_line_input(win);
	else if(win->type == wintype_TextGrid)
		return ui_grid_cancel_line_input(win);
	return 0;
}

int
ui_textwin_force_line_input(winid_t win, const char *text)
{
	if(win->type == wintype_TextBuffer)
		return ui_buffer_force_line_input(win, text);
	else if(win->type == wintype_TextGrid)
		return ui_grid_force_line_input(win, text);
	return 0;
}

/* Internal function used to iterate over all the hyperlinks, blocking the event
 * handler */
static void
hyperlink_block_event_handler(gpointer key, gpointer value, gpointer user_data)
{
	hyperlink_t *link = (hyperlink_t *) value;
	g_signal_handler_block(link->tag, link->event_handler);
}

/* Signal handler for hyperlink click; sends hyperlink event to Glk thread */
static gboolean
on_hyperlink_clicked(GtkTextTag *tag, GObject *object, GdkEvent *event, GtkTextIter *iter, hyperlink_t *link)
{
	ChimaraGlk *glk = CHIMARA_GLK(gtk_widget_get_ancestor(link->window->widget, CHIMARA_TYPE_GLK));
	g_assert(glk);

	if(event->type == GDK_BUTTON_PRESS) {
		link->window->hyperlink_event_requested = FALSE;
		g_hash_table_foreach(link->window->hyperlinks, hyperlink_block_event_handler, NULL);
		chimara_glk_push_event(glk, evtype_Hyperlink, link->window, link->value, 0);
	}

	return FALSE;
}

/* Sets the text grid or text buffer window @win to output hyperlink text which
 * returns @linkval when clicked.
 * Called as a result of glk_set_hyperlink_stream() and glk_set_hyperlink(). */
void
ui_textwin_set_hyperlink(winid_t win, unsigned linkval)
{
	if(linkval == 0) {
		/* Turn off hyperlink mode */
		win->current_hyperlink = NULL;
		return;
	}

	/* Check whether a tag with the needed value already exists */
	hyperlink_t *new_hyperlink = g_hash_table_lookup(win->hyperlinks, &linkval);
	if(new_hyperlink == NULL) {
		/* Create a new hyperlink with the requested value */
		new_hyperlink = g_new0(struct hyperlink, 1);
		new_hyperlink->value = linkval;

		new_hyperlink->tag = gtk_text_tag_new(NULL);
		new_hyperlink->event_handler = g_signal_connect( new_hyperlink->tag, "event", G_CALLBACK(on_hyperlink_clicked), new_hyperlink );
		if(!win->hyperlink_event_requested)
			g_signal_handler_block(new_hyperlink->tag, new_hyperlink->event_handler);
		new_hyperlink->window = win;

		/* Add the new tag to the tag table of the textbuffer */
		GtkTextBuffer *textbuffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
		GtkTextTagTable *tags = gtk_text_buffer_get_tag_table(textbuffer);
		gtk_text_tag_table_add(tags, new_hyperlink->tag);

		gint *linkval_pointer = g_new0(gint, 1);
		*linkval_pointer = linkval;
		g_hash_table_insert(win->hyperlinks, linkval_pointer, new_hyperlink);
	}

	win->current_hyperlink = new_hyperlink;
}

/* Internal function used to iterate over all the hyperlinks, unblocking the
 * event handler */
static void
hyperlink_unblock_event_handler(gpointer key, gpointer value, gpointer user_data)
{
	hyperlink_t *link = (hyperlink_t *) value;
	g_signal_handler_unblock(link->tag, link->event_handler);
}

/* Requests hyperlink input on @win. Called as a result of
 * glk_request_hyperlink_event(). */
void
ui_textwin_request_hyperlink_input(winid_t win)
{
	if(win->hyperlink_event_requested) {
		WARNING("Tried to request a hyperlink event on a window that already had a hyperlink request");
		return;
	}

	win->hyperlink_event_requested = TRUE;
	g_hash_table_foreach(win->hyperlinks, hyperlink_unblock_event_handler, NULL);

}

/* Cancels hyperlink input on @win. Called as a result of
 * glk_cancel_hyperlink_event(). */
void
ui_textwin_cancel_hyperlink_input(winid_t win)
{
	if(!win->hyperlink_event_requested) {
		WARNING("Tried to cancel a nonexistent hyperlink request");
		return;
	}

	win->hyperlink_event_requested = FALSE;
	g_hash_table_foreach(win->hyperlinks, hyperlink_block_event_handler, NULL);
}

void
ui_textwin_set_style(winid_t win, unsigned styl)
{
	win->style_tagname = (char *) chimara_glk_get_tag_name(styl);
	win->glk_style_tagname = (char *) chimara_glk_get_glk_tag_name(styl);
}

/* Sets the foreground and background of the text buffer or text grid window
 * @window to the Z-machine colors @fg and @bg, respectively.
 * Called as a result of garglk_set_zcolors_stream() and
 * garglk_set_zcolors(). */
void
ui_textwin_set_zcolors(winid_t win, unsigned fg, unsigned bg)
{
	GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
	GtkTextTagTable *tags = gtk_text_buffer_get_tag_table(buffer);
	GdkRGBA fore, back;
	GdkRGBA *fore_pointer = NULL;
	GdkRGBA *back_pointer = NULL;
	char *fore_name;
	char *back_name;

	switch(fg) {
	case zcolor_Transparent:
	case zcolor_Cursor:
		WARNING("zcolor_Transparent, zcolor_Cursor not implemented");
		// Fallthrough to default
	case zcolor_Default:
		fore_name = g_strdup("default");
		break;
	case zcolor_Current:
	{
		if(win->zcolor) {
			// Get the current foreground color
			GdkRGBA *current_color;
			g_object_get(win->zcolor, "foreground-rgba", &current_color, NULL);
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
		WARNING("zcolor_Transparent, zcolor_Cursor not implemented");
		// Fallthrough to default
	case zcolor_Default:
		back_name = g_strdup("default");
		break;
	case zcolor_Current:
	{
		if(win->zcolor) {
			// Get the current background color
			GdkRGBA *current_color;
			g_object_get(win->zcolor, "background-rgba", &current_color, NULL);
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
		win->zcolor = NULL;
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
		win->zcolor = tag;

		// Update the reversed version if necessary
		if(win->zcolor_reversed) {
			int reversed = GPOINTER_TO_INT( g_object_get_data( G_OBJECT(win->zcolor_reversed), "reverse-color" ) );
			ui_textwin_set_reverse_video(win, reversed != 0);
		}
	}
}

/* Sets reverse video on the text buffer or text grid window @win if @reverse is
 * TRUE.
 * If @reverse is FALSE, sets the colors back to normal.
 * Called as a result of garglk_set_reversevideo_stream() and
 * garglk_set_reversevideo(). */
void
ui_textwin_set_reverse_video(winid_t win, gboolean reverse)
{
	// Determine the current colors

	// If all fails, use black/white
	// FIXME: Use system theme here
	GdkRGBA foreground, background;
	gdk_rgba_parse(&foreground, "black");
	gdk_rgba_parse(&background, "white");
	GdkRGBA *current_foreground = &foreground;
	GdkRGBA *current_background = &background;

	ui_style_get_window_colors(win, &current_foreground, &current_background);

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
	GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
	GtkTextTagTable *tags = gtk_text_buffer_get_tag_table(buffer);
	GtkTextTag *tag = gtk_text_tag_table_lookup(tags, name);
	if(tag == NULL) {
		tag = gtk_text_buffer_create_tag(buffer, name,
			"foreground-rgba", current_foreground,
			"foreground-set", TRUE,
			"background-rgba", current_background,
			"background-set", TRUE,
			NULL);
		g_object_set_data( G_OBJECT(tag), "reverse-color", GINT_TO_POINTER(reverse) );
	}

	// From now on, text will be drawn in the specified colors
	win->zcolor_reversed = tag;

	// Update the background of the gtktextview to correspond with the current background color
	if(current_background != NULL) {
		ui_window_override_background_color(win, win->widget, current_background);
	}
}
