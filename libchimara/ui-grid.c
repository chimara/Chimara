#include <string.h>

#include <gtk/gtk.h>

#include "chimara-glk-private.h"
#include "event.h"
#include "magic.h"
#include "ui-misc.h"
#include "ui-style.h"
#include "ui-textwin.h"
#include "ui-window.h"
#include "window.h"

/* Internal function used to iterate over a style table, copying it */
static void
copy_tag_to_textbuffer(gpointer key, gpointer tag, gpointer target_table)
{
	gtk_text_tag_table_add(target_table, ui_text_tag_copy( GTK_TEXT_TAG(tag) ));
}

/* Internal function: initialize the default styles to a textgrid. */
static void
ui_grid_init_styles(ChimaraGlk *glk, GtkTextBuffer *screen)
{
	CHIMARA_GLK_USE_PRIVATE(glk, priv);

	/* Place the default text tags in the textbuffer's tag table */
	g_hash_table_foreach(priv->styles->text_grid, copy_tag_to_textbuffer, gtk_text_buffer_get_tag_table(screen));

	/* Copy the current text tags to the textbuffers's tag table */
	g_hash_table_foreach(priv->glk_styles->text_grid, copy_tag_to_textbuffer, gtk_text_buffer_get_tag_table(screen));

	/* Assign the 'default' tag the lowest priority */
	gtk_text_tag_set_priority( gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(screen), "default"), 0 );
}

/* Redirect the key press to the line input GtkEntry */
static gboolean
on_line_input_key_press_event(GtkWidget *widget, GdkEventKey *event, winid_t win)
{
	if(event->keyval == GDK_KEY_Up || event->keyval == GDK_KEY_KP_Up
		|| event->keyval == GDK_KEY_Down || event->keyval == GDK_KEY_KP_Down
		|| event->keyval == GDK_KEY_Left || event->keyval == GDK_KEY_KP_Left
		|| event->keyval == GDK_KEY_Right || event->keyval == GDK_KEY_KP_Right
		|| event->keyval == GDK_KEY_Tab || event->keyval == GDK_KEY_KP_Tab
		|| event->keyval == GDK_KEY_Page_Up || event->keyval == GDK_KEY_KP_Page_Up
		|| event->keyval == GDK_KEY_Page_Down || event->keyval == GDK_KEY_KP_Page_Down
		|| event->keyval == GDK_KEY_Home || event->keyval == GDK_KEY_KP_Home
		|| event->keyval == GDK_KEY_End || event->keyval == GDK_KEY_KP_End)
		return FALSE; /* Don't redirect these keys */
	gtk_widget_grab_focus(win->input_entry);
	gtk_editable_set_position(GTK_EDITABLE(win->input_entry), -1);
	gboolean retval = GDK_EVENT_STOP;
	g_signal_emit_by_name(win->input_entry, "key-press-event", event, &retval);
	return retval; /* Block this key event if the entry handled it */
}

/* Signal handler for mouse clicks in text grid windows */
static gboolean
on_grid_button_press(GtkWidget *widget, GdkEventButton *event, winid_t win)
{
	ChimaraGlk *glk = CHIMARA_GLK(gtk_widget_get_ancestor(win->widget, CHIMARA_TYPE_GLK));
	g_assert(glk);

	event_throw(glk, evtype_MouseInput, win,
		event->x / win->unit_width,
		event->y / win->unit_height);
	g_signal_handler_block(win->widget, win->button_press_event_handler);

	return GDK_EVENT_STOP;
}

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
	ui_grid_init_styles(glk, screen);

	/* Set up the appropriate text buffer or text grid font */
	PangoFontDescription *font = ui_style_get_current_font(glk, win->type);
	ui_window_override_font(win, win->widget, font);
	ui_calculate_zero_character_size(win->widget, font, &(win->unit_width), &(win->unit_height));
	pango_font_description_free(font);

	/* Create the cursor position mark */
	GtkTextIter begin;
	gtk_text_buffer_get_start_iter(screen, &begin);
	gtk_text_buffer_create_mark(screen, "cursor_position", &begin, TRUE);

	/* Connect signal handlers */
	win->char_input_keypress_handler = g_signal_connect(win->widget, "key-press-event", G_CALLBACK(ui_window_handle_char_input_key_press), win);
	g_signal_handler_block(win->widget, win->char_input_keypress_handler);
	win->line_input_keypress_handler = g_signal_connect(win->widget, "key-press-event", G_CALLBACK(on_line_input_key_press_event), win);
	g_signal_handler_block(win->widget, win->line_input_keypress_handler);
	win->shutdown_keypress_handler = g_signal_connect(win->widget, "key-press-event", G_CALLBACK(ui_window_handle_shutdown_key_press), win);
	g_signal_handler_block(win->widget, win->shutdown_keypress_handler);
	win->button_press_event_handler = g_signal_connect( win->widget, "button-press-event", G_CALLBACK(on_grid_button_press), win );
	g_signal_handler_block(win->widget, win->button_press_event_handler);
}

/* Prints @text at the current cursor position in the text grid */
void
ui_grid_print_string(winid_t win, const char *text)
{
	GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
	GtkTextMark *cursor = gtk_text_buffer_get_mark(buffer, "cursor_position");
	/* Number of characters to insert */
	long length = strlen(text);
	long chars_left = length;
	GtkTextIter start, insert;
	int start_offset;

	/* Get cursor position */
	gtk_text_buffer_get_iter_at_mark(buffer, &insert, cursor);

	g_mutex_lock(&win->lock);

	while(chars_left > 0 && !gtk_text_iter_is_end(&insert)) {
		/* Spaces available on this line */
		int available_space = win->width - gtk_text_iter_get_line_offset(&insert);

		GtkTextIter end = insert;
		if(chars_left <= available_space)
			gtk_text_iter_forward_chars(&end, chars_left);
		else
			gtk_text_iter_forward_to_line_end(&end);

		gtk_text_buffer_delete(buffer, &insert, &end);

		start_offset = gtk_text_iter_get_offset(&insert);
		gtk_text_buffer_insert(buffer, &insert, text + (length - chars_left), MIN(chars_left, available_space));
		gtk_text_buffer_get_iter_at_offset(buffer, &start, start_offset);
		ui_style_apply(win, &start, &insert);

		chars_left -= available_space;

		if(gtk_text_iter_get_line_offset(&insert) >= win->width)
			gtk_text_iter_forward_line(&insert);
	}

	g_mutex_unlock(&win->lock);

	gtk_text_buffer_move_mark(buffer, cursor, &insert);
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
	ui_style_apply(win, &start, &end);

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

/* Internal function: Retrieves the input of a TextGrid window and stores it in
 * the window buffer. Returns the number of characters written, suitable for
 * inclusion in a line input event. */
static int
ui_grid_finish_line_input(winid_t win, gboolean emit_signal)
{
	VALID_WINDOW(win, return 0);
	g_return_val_if_fail(win->type == wintype_TextGrid, 0);

	GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );

	gchar *text = g_strdup( gtk_entry_get_text(GTK_ENTRY(win->input_entry)) );
	/* Move the focus back into the text view */
	gtk_widget_grab_focus(win->widget);
	/* Remove entry widget from text view */
	/* Should be ok even though this is the widget's own signal handler */
	gtk_container_remove( GTK_CONTAINER(win->widget), GTK_WIDGET(win->input_entry) );
	win->input_entry = NULL;
	/* Delete the child anchor */
	GtkTextIter start, end;
	gtk_text_buffer_get_iter_at_child_anchor(buffer, &start, win->input_anchor);
	end = start;
	gtk_text_iter_forward_char(&end); /* Point after the child anchor */
	gtk_text_buffer_delete(buffer, &start, &end);
	win->input_anchor = NULL;

	gchar *spaces = g_strnfill(win->input_length - g_utf8_strlen(text, -1), ' ');
	gchar *text_to_insert = g_strconcat(text, spaces, NULL);
	g_free(spaces);
	gtk_text_buffer_insert(buffer, &start, text_to_insert, -1);
	g_free(text_to_insert);

	int chars_written = ui_textwin_finish_line_input(win, text, emit_signal);
	g_free(text);
	return chars_written;
}

/* Internal function: Callback for signal activate on the line input GtkEntry
in a text grid window. */
static void
on_input_entry_activate(GtkEntry *input_entry, winid_t win)
{
	g_signal_handler_block(win->widget, win->line_input_keypress_handler);

	int chars_written = ui_grid_finish_line_input(win, TRUE);
	ChimaraGlk *glk = CHIMARA_GLK(gtk_widget_get_ancestor(win->widget, CHIMARA_TYPE_GLK));
	event_throw(glk, evtype_LineInput, win, chars_written, 0);
}

/* Internal function: Callback for signal key-press-event on the line input
GtkEntry in a text grid window. */
static gboolean
on_input_entry_key_press_event(GtkEntry *input_entry, GdkEventKey *event, winid_t win)
{
	if(event->keyval == GDK_KEY_Up || event->keyval == GDK_KEY_KP_Up
		|| event->keyval == GDK_KEY_Down || event->keyval == GDK_KEY_KP_Down)
	{
		/* Prevent falling off the end of the history list */
		if( (event->keyval == GDK_KEY_Up || event->keyval == GDK_KEY_KP_Up)
			&& win->history_pos && win->history_pos->next == NULL)
			return GDK_EVENT_STOP;
		if( (event->keyval == GDK_KEY_Down || event->keyval == GDK_KEY_KP_Down)
			&& (win->history_pos == NULL || win->history_pos->prev == NULL) )
			return GDK_EVENT_STOP;

		if(win->history_pos == NULL) {
			const char *current_input = gtk_entry_get_text(input_entry);
			win->history = g_list_prepend(win->history, g_strdup(current_input));
			win->history_pos = win->history;
		}

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
		g_signal_handler_block(input_entry, win->line_input_entry_changed);
		gtk_entry_set_text(input_entry, win->history_pos->data);
		g_signal_handler_unblock(input_entry, win->line_input_entry_changed);
		return GDK_EVENT_STOP;
	} else if(g_slist_find(win->current_extra_line_terminators, GUINT_TO_POINTER(event->keyval))) {
		/* If this key was a line terminator, pretend we pressed enter */
		on_input_entry_activate(input_entry, win);
	}
	return GDK_EVENT_PROPAGATE;
}

static void
on_input_entry_changed(GtkEditable *editable, winid_t win)
{
	/* Set the history position to NULL and erase the text we were already editing */
	if(win->history_pos != NULL) {
		g_free(win->history->data);
		win->history = g_list_delete_link(win->history, win->history);
		win->history_pos = NULL;
	}
}

/* Request either latin-1 or unicode line input, in a text grid window @win. */
void
ui_grid_request_line_event(winid_t win, unsigned maxlen, gboolean insert, const char *inserttext)
{
	GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );

	GtkTextMark *cursor = gtk_text_buffer_get_mark(buffer, "cursor_position");
	GtkTextIter start_iter, end_iter;
	gtk_text_buffer_get_iter_at_mark(buffer, &start_iter, cursor);

	/* Determine the maximum length of the line input */
	gint cursorpos = gtk_text_iter_get_line_offset(&start_iter);
	/* Odd; the Glk spec says the maximum input length is
	windowwidth - 1 - cursorposition. I say no, because if cursorposition is
	zero, then the input should fill the whole line. FIXME??? */
	g_mutex_lock(&win->lock);
	win->input_length = MIN(win->width - cursorpos, win->line_input_buffer_max_len);
	g_mutex_unlock(&win->lock);
	end_iter = start_iter;
	gtk_text_iter_set_line_offset(&end_iter, cursorpos + win->input_length);

	/* If the buffer currently has a selection with one bound in the middle of
	the input field, then deselect it. Otherwise the input field gets trashed */
	GtkTextIter start_sel, end_sel;
	if( gtk_text_buffer_get_selection_bounds(buffer, &start_sel, &end_sel) )
	{
		if( gtk_text_iter_in_range(&start_sel, &start_iter, &end_iter) )
			gtk_text_buffer_place_cursor(buffer, &end_sel);
		if( gtk_text_iter_in_range(&end_sel, &start_iter, &end_iter) )
			gtk_text_buffer_place_cursor(buffer, &start_sel);
	}

	/* Erase the text currently in the input field and replace it with a GtkEntry */
	gtk_text_buffer_delete(buffer, &start_iter, &end_iter);
	win->input_anchor = gtk_text_buffer_create_child_anchor(buffer, &start_iter);
	win->input_entry = gtk_entry_new();

	/* Set the color of the entry, and make it as small as possible in order to
	fit with the text */
	gtk_entry_set_has_frame(GTK_ENTRY(win->input_entry), FALSE);
	GtkStyleContext *entry_style = gtk_widget_get_style_context(win->input_entry);
#ifdef GTK_STYLE_CLASS_FLAT /* COMPAT: available in 3.14 */
	gtk_style_context_add_class(entry_style, GTK_STYLE_CLASS_FLAT);
#endif /* GTK_STYLE_CLASS_FLAT */
	GtkCssProvider *css_provider = gtk_css_provider_new();
	gtk_css_provider_load_from_data(css_provider,
		".entry {"
		"    background-image: none;"
		"    background-color: #d3d7cf;"
		"    border: 0 none;"
		"    border-radius: 0;"
		"    outline: 0 none;"
		"    margin: 0;"
		"    padding: 0;"
		"}", -1, NULL);
	gtk_style_context_add_provider(entry_style, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	gtk_entry_set_max_length(GTK_ENTRY(win->input_entry), win->input_length);
	gtk_entry_set_width_chars(GTK_ENTRY(win->input_entry), win->input_length);

	/* Insert pre-entered text if needed */
	if(insert)
		gtk_entry_set_text(GTK_ENTRY(win->input_entry), inserttext);

	g_signal_connect(win->input_entry, "activate", G_CALLBACK(on_input_entry_activate), win);
	g_signal_connect(win->input_entry, "key-press-event", G_CALLBACK(on_input_entry_key_press_event), win);
	win->line_input_entry_changed = g_signal_connect(win->input_entry, "changed", G_CALLBACK(on_input_entry_changed), win);

	gtk_widget_show(win->input_entry);
	gtk_text_view_add_child_at_anchor(GTK_TEXT_VIEW(win->widget), win->input_entry, win->input_anchor);

	gtk_widget_grab_focus(win->input_entry);
}

/* Cancels a pending line input request on the text grid window @win.
 * Called from glk_cancel_line_event(). */
int
ui_grid_cancel_line_input(winid_t win)
{
	g_signal_handler_block(win->widget, win->line_input_keypress_handler);
	return ui_grid_finish_line_input(win, FALSE);
}

int
ui_grid_force_line_input(winid_t win, const char *text)
{
	/* Remove signal handlers so the line input doesn't get picked up again */
	g_signal_handler_block(win->widget, win->char_input_keypress_handler);

	/* Insert the forced input into the window */
	gtk_entry_set_text(GTK_ENTRY(win->input_entry), text);
	return ui_grid_finish_line_input(win, TRUE);
}
