#include "config.h"

#include <string.h>

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

#include "chimara-glk.h"
#include "chimara-glk-private.h"
#include "event.h"
#include "magic.h"
#include "pager.h"
#include "ui-misc.h"
#include "ui-style.h"
#include "ui-textwin.h"
#include "ui-window.h"
#include "window.h"

/* Internal function: Callback for signal key-press-event on a text buffer
 * window. Used in character input. Blocked when not in use. */
static gboolean
on_char_input_key_press_event(GtkWidget *widget, GdkEventKey *event, winid_t win)
{
	gboolean retval = ui_window_handle_char_input_key_press(widget, event, win);
	if (retval == GDK_EVENT_STOP) {
		/* All text up to the input position is now regarded as being read by
		the user */
		pager_update(win);
	}
	return retval;
}

/* Internal function: Retrieves the input of a TextBuffer window and stores it
 * in the window buffer. Returns the number of characters written, suitable for
 * inclusion in a line input event. */
static int
ui_buffer_finish_line_input(winid_t win, gboolean emit_signal)
{
	VALID_WINDOW(win, return 0);
	g_return_val_if_fail(win->type == wintype_TextBuffer, 0);

	GtkTextIter start_iter, end_iter;

	GtkTextBuffer *window_buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
	GtkTextMark *input_position = gtk_text_buffer_get_mark(window_buffer, "input_position");
	gtk_text_buffer_get_iter_at_mark(window_buffer, &start_iter, input_position);
	gtk_text_buffer_get_end_iter(window_buffer, &end_iter);

	gchar *inserted_text = gtk_text_buffer_get_text(window_buffer, &start_iter, &end_iter, FALSE);

	/* If echoing is turned off, remove the text from the window */
	if(!win->echo_current_line_input)
		gtk_text_buffer_delete(window_buffer, &start_iter, &end_iter);

	/* Don't include the newline in the input */
	size_t inserted_len = strlen(inserted_text);
	if (inserted_len >= 1 && inserted_text[inserted_len] == '\n')
		inserted_text[inserted_len] = '\0';

	int chars_written = ui_textwin_finish_line_input(win, inserted_text, emit_signal);
	g_free(inserted_text);
	return chars_written;
}

static gboolean
on_line_input_key_press_event(GtkWidget *widget, GdkEventKey *event, winid_t win)
{
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(win->widget));
	GtkTextMark *input_position_mark = gtk_text_buffer_get_mark(buffer, "input_position");
	GtkTextIter input_position_iter, end_iter;

	gtk_text_buffer_get_iter_at_mark(buffer, &input_position_iter, input_position_mark);
	gtk_text_buffer_get_end_iter(buffer, &end_iter);

	/* Check whether the cursor is at the prompt or somewhere else in the text */
	GtkTextIter selection_start, selection_end;
	gtk_text_buffer_get_selection_bounds(buffer, &selection_start, &selection_end);
	if(gtk_text_iter_compare(&selection_start, &input_position_iter) < 0) {
		// Cursor is somewhere else in the text, place it at the end if the user starts typing
		if(event->keyval >= GDK_KEY_space && event->keyval <= GDK_KEY_asciitilde) {
			gtk_text_buffer_place_cursor(buffer, &end_iter);
		} else {
			// User is walking around, let him be.
			return GDK_EVENT_PROPAGATE;
		}
	}

	/* All text up to the input position is now regarded as being read by the user */
	pager_update(win);

	/* History up/down */
	if(event->keyval == GDK_KEY_Up || event->keyval == GDK_KEY_KP_Up
		|| event->keyval == GDK_KEY_Down || event->keyval == GDK_KEY_KP_Down)
	{
		/* Prevent falling off the end of the history list */
		if(win->history == NULL)
			return GDK_EVENT_STOP;
		if( (event->keyval == GDK_KEY_Up || event->keyval == GDK_KEY_KP_Up)
			&& win->history_pos && win->history_pos->next == NULL)
			return GDK_EVENT_STOP;
		if( (event->keyval == GDK_KEY_Down || event->keyval == GDK_KEY_KP_Down)
			&& (win->history_pos == NULL || win->history_pos->prev == NULL) )
			return GDK_EVENT_STOP;

		/* Erase any text that was already typed */
		if(win->history_pos == NULL) {
			gchar *current_input = gtk_text_buffer_get_text(buffer, &input_position_iter, &end_iter, FALSE);
			win->history = g_list_prepend(win->history, current_input);
			win->history_pos = win->history;
		}

		gtk_text_buffer_delete(buffer, &input_position_iter, &end_iter);

		if(event->keyval == GDK_KEY_Up || event->keyval == GDK_KEY_KP_Up) {
			if(win->history_pos)
				win->history_pos = g_list_next(win->history_pos);
			else
				win->history_pos = win->history;
		} else {
			/* down */
			win->history_pos = g_list_previous(win->history_pos);
		}

		/* Insert the history item into the window */
		gtk_text_buffer_get_end_iter(buffer, &end_iter);

		g_signal_handler_block(buffer, win->insert_text_handler);
		gtk_text_buffer_insert_with_tags_by_name(buffer, &end_iter, win->history_pos->data, -1, "default", "input", "glk-input", NULL);
		g_signal_handler_unblock(buffer, win->insert_text_handler);

		return GDK_EVENT_STOP;
	}

	/* Move to beginning/end of input field */
	else if(event->keyval == GDK_KEY_Home) {
		GtkTextIter input_iter;
		GtkTextMark *input_position = gtk_text_buffer_get_mark(buffer, "input_position");
		gtk_text_buffer_get_iter_at_mark(buffer, &input_iter, input_position);
		gtk_text_buffer_place_cursor(buffer, &input_iter);
		return GDK_EVENT_STOP;
	}
	else if(event->keyval == GDK_KEY_End) {
		gtk_text_buffer_place_cursor(buffer, &end_iter);
		return GDK_EVENT_STOP;
	}

	/* Handle the line terminators */
	else if(event->keyval == GDK_KEY_Return || event->keyval == GDK_KEY_KP_Enter
	   || g_slist_find(win->current_extra_line_terminators, GUINT_TO_POINTER(event->keyval)))
	{
		/* Remove signal handlers */
		g_signal_handler_block(buffer, win->insert_text_handler);
		g_signal_handler_block(win->widget, win->line_input_keypress_handler);

		/* Insert a newline (even if line input was terminated with a different key */
		gtk_text_buffer_get_end_iter(buffer, &end_iter);
		gtk_text_buffer_insert(buffer, &end_iter, "\n", 1);
		gtk_text_buffer_place_cursor(buffer, &end_iter);

		/* Make the window uneditable again and retrieve the text that was input */
		gtk_text_view_set_editable(GTK_TEXT_VIEW(win->widget), FALSE);

		int chars_written = ui_buffer_finish_line_input(win, TRUE);
		ChimaraGlk *glk = CHIMARA_GLK(gtk_widget_get_ancestor(win->widget, CHIMARA_TYPE_GLK));
		chimara_glk_push_event(glk, evtype_LineInput, win, chars_written, 0);
		return GDK_EVENT_STOP;
	}

	return GDK_EVENT_PROPAGATE;
}

/* Internal function: Callback for signal insert-text on a text buffer window.
Runs after the default handler has already inserted the text. */
static void
after_window_insert_text(GtkTextBuffer *textbuffer, GtkTextIter *location, char *text, int len, winid_t win)
{
	GtkTextBuffer *window_buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );

	/* Set the history position to NULL and erase the text we were already editing */
	if(win->history_pos != NULL) {
		g_free(win->history->data);
		win->history = g_list_delete_link(win->history, win->history);
		win->history_pos = NULL;
	}

	/* Apply the 'input' style to the text that was entered */
	GtkTextIter end_iter;
	gtk_text_buffer_get_end_iter(window_buffer, &end_iter);
	GtkTextIter input_iter;
	GtkTextMark *input_position = gtk_text_buffer_get_mark(window_buffer, "input_position");
	gtk_text_buffer_get_iter_at_mark(window_buffer, &input_iter, input_position);
	gtk_text_buffer_apply_tag_by_name(window_buffer, "default", &input_iter, &end_iter);
	gtk_text_buffer_apply_tag_by_name(window_buffer, "input", &input_iter, &end_iter);
	gtk_text_buffer_apply_tag_by_name(window_buffer, "glk-input", &input_iter, &end_iter);
}

/* Creates a text buffer window, filling in the fields of @win.
 * Called as a result of glk_window_open(). */
void
ui_buffer_create(winid_t win, ChimaraGlk *glk)
{
	win->frame = gtk_overlay_new();
	win->scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
	win->widget = gtk_text_view_new();
	win->pager = gtk_button_new_with_label(_("More"));
	GtkWidget *image = gtk_image_new_from_icon_name("go-down", GTK_ICON_SIZE_BUTTON);

	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(win->scrolledwindow), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );

	gtk_button_set_image( GTK_BUTTON(win->pager), image );
	gtk_widget_set_halign(win->pager, GTK_ALIGN_END);
	gtk_widget_set_valign(win->pager, GTK_ALIGN_END);
	gtk_widget_set_no_show_all(win->pager, TRUE);

	gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW(win->widget), GTK_WRAP_WORD_CHAR );
	gtk_text_view_set_editable( GTK_TEXT_VIEW(win->widget), FALSE );
	gtk_text_view_set_pixels_inside_wrap( GTK_TEXT_VIEW(win->widget), 3 );
	gtk_text_view_set_left_margin( GTK_TEXT_VIEW(win->widget), 20 );
	gtk_text_view_set_right_margin( GTK_TEXT_VIEW(win->widget), 20 );

	gtk_container_add( GTK_CONTAINER(win->scrolledwindow), win->widget );
	gtk_container_add( GTK_CONTAINER(win->frame), win->scrolledwindow );
	gtk_overlay_add_overlay( GTK_OVERLAY(win->frame), win->pager );
	gtk_widget_show_all(win->frame);

	/* Create the styles available to the window stream */
	GtkTextBuffer *screen = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
	chimara_glk_init_textbuffer_styles(glk, CHIMARA_GLK_TEXT_BUFFER, screen);

	/* Set up the appropriate text buffer or text grid font */
	PangoFontDescription *font = ui_style_get_current_font(glk, win->type);
	ui_window_override_font(win, win->widget, font);
	ui_calculate_zero_character_size(win->widget, font, &(win->unit_width), &(win->unit_height));
	pango_font_description_free(font);

	/* Pager */
	g_signal_connect_after( win->widget, "size-allocate", G_CALLBACK(pager_after_size_allocate), win );
	win->pager_keypress_handler = g_signal_connect( win->widget, "key-press-event", G_CALLBACK(pager_on_key_press_event), win );
	g_signal_handler_block(win->widget, win->pager_keypress_handler);
	GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment( GTK_SCROLLED_WINDOW(win->scrolledwindow) );
	win->pager_adjustment_handler = g_signal_connect_after(adj, "value-changed", G_CALLBACK(pager_after_adjustment_changed), win);
	g_signal_connect(win->pager, "clicked", G_CALLBACK(pager_on_clicked), win);

	/* Char and line input */
	win->char_input_keypress_handler = g_signal_connect( win->widget, "key-press-event", G_CALLBACK(on_char_input_key_press_event), win );
	g_signal_handler_block(win->widget, win->char_input_keypress_handler);
	win->line_input_keypress_handler = g_signal_connect( win->widget, "key-press-event", G_CALLBACK(on_line_input_key_press_event), win );
	g_signal_handler_block(win->widget, win->line_input_keypress_handler);
	win->insert_text_handler = g_signal_connect_after( screen, "insert-text", G_CALLBACK(after_window_insert_text), win );
	g_signal_handler_block(screen, win->insert_text_handler);

	/* Shutdown key press */
	win->shutdown_keypress_handler = g_signal_connect(win->widget, "key-press-event", G_CALLBACK(ui_window_handle_shutdown_key_press), glk);
	g_signal_handler_block(win->widget, win->shutdown_keypress_handler);

	/* Create an editable tag to indicate uneditable parts of the window
	(for line input) */
	gtk_text_buffer_create_tag(screen, "uneditable", "editable", FALSE, "editable-set", TRUE, NULL);

	/* Mark the position where the user will input text and the end mark */
	GtkTextIter end;
	gtk_text_buffer_get_end_iter(screen, &end);
	gtk_text_buffer_create_mark(screen, "input_position", &end, TRUE);
	gtk_text_buffer_create_mark(screen, "end_position", &end, FALSE);

	/* Create the pager position mark; it stands for the last character in the buffer
	 that has been on-screen */
	gtk_text_buffer_create_mark(screen, "pager_position", &end, TRUE);
}

/* Prints @text to the end of the text buffer */
void
ui_buffer_print_string(winid_t win, const char *text)
{
	GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
	GtkTextIter start, end;
	int start_offset;

	gtk_text_buffer_get_end_iter(buffer, &end);
	start_offset = gtk_text_iter_get_offset(&end);
	gtk_text_buffer_insert(buffer, &end, text, -1);
	gtk_text_buffer_get_iter_at_offset(buffer, &start, start_offset);
	ui_style_apply(win, &start, &end);

	ChimaraGlk *glk = CHIMARA_GLK(gtk_widget_get_ancestor(win->widget, CHIMARA_TYPE_GLK));
	g_assert(glk);
	g_signal_emit_by_name(glk, "text-buffer-output", win->rock, win->librock, text);
}

/* Clears the text buffer window @win by deleting all the text in it.
 * Called as a result of glk_window_clear(). */
void
ui_buffer_clear(winid_t win)
{
	GtkTextBuffer *screen = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
	GtkTextIter start, end;
	gtk_text_buffer_get_bounds(screen, &start, &end);
	gtk_text_buffer_delete(screen, &start, &end);
}

/* Request either latin-1 or unicode line input, in a text buffer window @win. */
void
ui_buffer_request_line_event(winid_t win, glui32 maxlen, gboolean insert, const char *inserttext)
{
	GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );

	/* Move the input_position mark to the end of the window_buffer */
	GtkTextMark *input_position = gtk_text_buffer_get_mark(buffer, "input_position");
	GtkTextIter end_iter;
	gtk_text_buffer_get_end_iter(buffer, &end_iter);
	gtk_text_buffer_move_mark(buffer, input_position, &end_iter);

	/* Set the entire contents of the window_buffer as uneditable
	 * (so input can only be entered at the end) */
	GtkTextIter start_iter;
	gtk_text_buffer_get_start_iter(buffer, &start_iter);
	gtk_text_buffer_remove_tag_by_name(buffer, "uneditable", &start_iter, &end_iter);
	gtk_text_buffer_apply_tag_by_name(buffer, "uneditable", &start_iter, &end_iter);

	/* Insert pre-entered text if needed */
	if(insert) {
		gtk_text_buffer_insert(buffer, &end_iter, inserttext, -1);
		gtk_text_buffer_get_end_iter(buffer, &end_iter); /* update after text insertion */
	}

	/* Apply the correct style to the input prompt */
	GtkTextIter input_iter;
	gtk_text_buffer_get_iter_at_mark(buffer, &input_iter, input_position);
	gtk_text_buffer_apply_tag_by_name(buffer, "default", &input_iter, &end_iter);
	gtk_text_buffer_apply_tag_by_name(buffer, "input", &input_iter, &end_iter);
	gtk_text_buffer_apply_tag_by_name(buffer, "glk-input", &input_iter, &end_iter);

	gtk_text_view_set_editable(GTK_TEXT_VIEW(win->widget), TRUE);

	g_signal_handler_unblock(buffer, win->insert_text_handler);
	gtk_widget_grab_focus(win->widget);
}

/* Cancels a pending line input event on a text buffer window @win.
 * Called as a result of glk_cancel_line_event(). */
int
ui_buffer_cancel_line_input(winid_t win)
{
	g_signal_handler_block(win->widget, win->line_input_keypress_handler);
	gtk_text_view_set_editable( GTK_TEXT_VIEW(win->widget), FALSE );
	GtkTextBuffer *screen = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
	g_signal_handler_block(screen, win->insert_text_handler);
	return ui_buffer_finish_line_input(win, FALSE);
}

int
ui_buffer_force_line_input(winid_t win, const char *text)
{
	GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
	GtkTextIter start, end;

	/* Remove signal handlers so the line input doesn't get picked up again */
	g_signal_handler_block(buffer, win->insert_text_handler);
	g_signal_handler_block(win->widget, win->line_input_keypress_handler);

	/* Erase any text that was already typed */
	GtkTextMark *input_position = gtk_text_buffer_get_mark(buffer, "input_position");
	gtk_text_buffer_get_iter_at_mark(buffer, &start, input_position);
	gtk_text_buffer_get_end_iter(buffer, &end);
	gtk_text_buffer_delete(buffer, &start, &end);

	/* Make the window uneditable again */
	gtk_text_view_set_editable(GTK_TEXT_VIEW(win->widget), FALSE);

	/* Insert the forced input into the window */
	if(win->echo_current_line_input) {
		gtk_text_buffer_get_end_iter(buffer, &end);
		char *text_to_insert = g_strconcat(text, "\n", NULL);
		gtk_text_buffer_insert_with_tags_by_name(buffer, &end, text_to_insert, -1, "default", "input", "glk-input", NULL);
	}

	return ui_buffer_finish_line_input(win, TRUE);
}

/* Adds an image, stored in @pixbuf, to a text buffer window @win.
 * The @alignment parameter, an imagealign_ constant, specifies how to
 * vertically align the image relative to the text.
 * Called as a result of glk_image_draw(). */
void
ui_buffer_draw_image(winid_t win, GdkPixbuf *pixbuf, glui32 alignment)
{
	GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
	GtkTextIter end, start;
	gtk_text_buffer_get_end_iter(buffer, &end);

	gtk_text_buffer_insert_pixbuf(buffer, &end, pixbuf);
	start = end;
	gtk_text_iter_forward_char(&end);

	gint height = 0;
	switch(alignment) {
	case imagealign_InlineDown:
		height -= win->unit_height;
		break;
	case imagealign_InlineCenter:
		height = -win->unit_height / 2;
		break;
	case imagealign_InlineUp:
	default:
		height = 0;
	}

	if(height != 0) {
		GtkTextTag *tag = gtk_text_buffer_create_tag(buffer, NULL, "rise", PANGO_SCALE * (-height), NULL);
		gtk_text_buffer_apply_tag(buffer, tag, &start, &end);
	}
}
