#include <gtk/gtk.h>

#include "chimara-glk.h"
#include "glk.h"
#include "input.h"
#include "mouse.h"
#include "strio.h"
#include "style.h"
#include "ui-window.h"
#include "window.h"

/* Creates a text grid window, filling in the fields of @win.
 * Called as a result of glk_window_open(). */
void
ui_grid_create(winid_t win, ChimaraGlk *glk)
{
	win->widget = win->frame = gtk_text_view_new();
	gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW(win->widget), GTK_WRAP_NONE );
	gtk_text_view_set_editable( GTK_TEXT_VIEW(win->widget), FALSE );
	gtk_widget_show(win->widget);

	/* Create the styles available to the window stream */
	GtkTextBuffer *screen = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
	ui_style_init_text_grid(glk, screen);

	/* Set up the appropriate text buffer or text grid font */
	PangoFontDescription *font = ui_style_get_current_font(glk, win->type);
	gtk_widget_override_font(win->widget, font);
	calculate_zero_character_size(win->widget, font, &(win->unit_width), &(win->unit_height));
	pango_font_description_free(font);

	/* Create the cursor position mark */
	GtkTextIter begin;
	gtk_text_buffer_get_start_iter(screen, &begin);
	gtk_text_buffer_create_mark(screen, "cursor_position", &begin, TRUE);

	/* Connect signal handlers */
	win->char_input_keypress_handler = g_signal_connect(win->widget, "key-press-event", G_CALLBACK(on_char_input_key_press_event), win);
	g_signal_handler_block(win->widget, win->char_input_keypress_handler);
	win->line_input_keypress_handler = g_signal_connect(win->widget, "key-press-event", G_CALLBACK(on_line_input_key_press_event), win);
	g_signal_handler_block(win->widget, win->line_input_keypress_handler);
	win->shutdown_keypress_handler = g_signal_connect(win->widget, "key-press-event", G_CALLBACK(on_shutdown_key_press_event), win);
	g_signal_handler_block(win->widget, win->shutdown_keypress_handler);
	win->button_press_event_handler = g_signal_connect( win->widget, "button-press-event", G_CALLBACK(on_window_button_press), win );
	g_signal_handler_block(win->widget, win->button_press_event_handler);
}

/* Clears the text grid window @win by filling it with blanks.
 * Called as a result of glk_window_clear(). */
void
ui_grid_clear(winid_t win)
{
	/* Manually put newlines at the end of each row of characters in the buffer;
	manual newlines make resizing the window's grid easier. */
	g_mutex_lock(&win->lock);
	char *blanks = g_strnfill(win->width, ' ');
	char **blanklines = g_new0(char *, win->height + 1);
	int count;
	for(count = 0; count < win->height; count++)
		blanklines[count] = blanks;
	blanklines[win->height] = NULL;
	g_mutex_unlock(&win->lock);
	char *text = g_strjoinv("\n", blanklines);
	g_free(blanklines); /* not g_strfreev() */
	g_free(blanks);

	GtkTextBuffer *screen = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
	gtk_text_buffer_set_text(screen, text, -1);
	g_free(text);

	GtkTextIter start, end;
	gtk_text_buffer_get_start_iter(screen, &start);
	gtk_text_buffer_get_end_iter(screen, &end);
	style_apply(win, &start, &end);

	gtk_text_buffer_move_mark_by_name(screen, "cursor_position", &start);
}

/* Moves the output cursor to coordinates @xpos, @ypos within the text grid
 * window @win.
 * @xpos and @ypos must be valid coordinates.
 * Called as a result of glk_window_move_cursor(). */
void
ui_grid_move_cursor(winid_t win, unsigned xpos, unsigned ypos)
{
	GtkTextBuffer *screen = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
	GtkTextIter newpos;
	/* There must actually be a character at xpos, or the following function will choke */
	gtk_text_buffer_get_iter_at_line_offset(screen, &newpos, ypos, xpos);
	gtk_text_buffer_move_mark_by_name(screen, "cursor_position", &newpos);
}

/* Moves the output cursor to the next row and the first column within the text
 * grid window @win.
 * Not called explicitly as the result of any Glk function, but happens when a
 * newline character is printed to a text grid window's window stream, for
 * example. */
void
ui_grid_newline_cursor(winid_t win)
{
	/* Move cursor position forward to the next line */
	GtkTextIter cursor_pos;
	GtkTextView *textview = GTK_TEXT_VIEW(win->widget);
	GtkTextBuffer *screen = gtk_text_view_get_buffer(textview);
	GtkTextMark *cursor_mark = gtk_text_buffer_get_mark(screen, "cursor_position");

	gtk_text_buffer_get_iter_at_mark(screen, &cursor_pos, cursor_mark);
	gtk_text_view_forward_display_line(textview, &cursor_pos);
	gtk_text_view_backward_display_line_start(textview, &cursor_pos);
	gtk_text_buffer_move_mark(screen, cursor_mark, &cursor_pos);
}

/* Cancels a pending line input request on the text grid window @win.
 * Called from glk_cancel_line_event(). */
int
ui_grid_cancel_line_input(winid_t win)
{
	g_signal_handler_block(win->widget, win->line_input_keypress_handler);
	return ui_grid_finish_line_input(win, FALSE);
}
