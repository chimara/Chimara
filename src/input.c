#include "input.h"

/** glk_request_char_event:
 * @win: A window to request char events from Request
 *
 * Request input of a Latin-1 character or special key. A window cannot have requests
 * for both character and line input at the same time. Nor can it have requests
 * for character input of both types (Latin-1 and Unicode). It is illegal to
 * call glk_request_char_event() if the window already has a pending request
 * for either character or line input. 
 */
void
glk_request_char_event(winid_t win)
{
	g_return_if_fail(win);
	g_return_if_fail(win->input_request_type == INPUT_REQUEST_NONE);
	g_return_if_fail(win->window_type != wintype_TextBuffer || win->window_type != wintype_TextGrid);

	win->input_request_type = INPUT_REQUEST_CHARACTER;
	g_signal_handler_unblock( G_OBJECT(win->widget), win->keypress_handler );
}

/** glk_request_char_event_uni:
 * @win: A window to request char events from Request
 *
 * Request input of a Unicode character or special key. A window cannot have requests
 * for both character and line input at the same time. Nor can it have requests
 * for character input of both types (Latin-1 and Unicode). It is illegal to
 * call glk_request_char_event_uni() if the window already has a pending request
 * for either character or line input. 
 */
void
glk_request_char_event_uni(winid_t win)
{
	g_return_if_fail(win);
	g_return_if_fail(win->input_request_type == INPUT_REQUEST_NONE);
	g_return_if_fail(win->window_type != wintype_TextBuffer || win->window_type != wintype_TextGrid);

	win->input_request_type = INPUT_REQUEST_CHARACTER_UNICODE;
	g_signal_handler_unblock( G_OBJECT(win->widget), win->keypress_handler );
}

void
glk_request_line_event(winid_t win, char* buf, glui32 maxlen, glui32 initlen)
{
	GtkTextBuffer *window_buffer;

	g_return_if_fail(win);
	g_return_if_fail(win->input_request_type == INPUT_REQUEST_NONE);
	g_return_if_fail(win->window_type != wintype_TextBuffer || win->window_type != wintype_TextGrid);

	window_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(win->widget));

	win->input_request_type = INPUT_REQUEST_LINE;
	win->line_input_buffer = buf;
	win->line_input_buffer_max_len = maxlen;

	/* Move the input_position mark to the end of the window_buffer */
	GtkTextMark *input_position = gtk_text_buffer_get_mark(window_buffer, "input_position");
	GtkTextIter end_iter;
	gtk_text_buffer_get_end_iter(window_buffer, &end_iter);
	gtk_text_buffer_move_mark(window_buffer, input_position, &end_iter);

	/* Set the entire contents of the window_buffer as uneditable
	 * (so input can only be entered at the end) */
	GtkTextIter start_iter;
	gtk_text_buffer_get_start_iter(window_buffer, &start_iter);
	gtk_text_buffer_remove_tag_by_name(window_buffer, "uneditable", &start_iter, &end_iter);
	gtk_text_buffer_apply_tag_by_name(window_buffer, "uneditable", &start_iter, &end_iter);

	if(initlen > 0) {
		gtk_text_buffer_insert(window_buffer, &end_iter, buf, initlen);
	}

	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(win->widget), input_position);
	g_signal_handler_unblock( G_OBJECT(window_buffer), win->insert_text_handler );
	gtk_text_view_set_editable(GTK_TEXT_VIEW(win->widget), TRUE);
}

void
glk_request_line_event_uni(winid_t win, glui32 *buf, glui32 maxlen, glui32 initlen)
{
	GtkTextBuffer *window_buffer;

	g_return_if_fail(win);
	g_return_if_fail(win->input_request_type == INPUT_REQUEST_NONE);
	g_return_if_fail(win->window_type != wintype_TextBuffer || win->window_type != wintype_TextGrid);

	window_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(win->widget));

	win->input_request_type = INPUT_REQUEST_LINE_UNICODE;
	win->line_input_buffer_unicode = buf;
	win->line_input_buffer_max_len = maxlen;

	/* Move the input_position mark to the end of the window_buffer */
	GtkTextMark *input_position = gtk_text_buffer_get_mark(window_buffer, "input_position");
	GtkTextIter end_iter;
	gtk_text_buffer_get_end_iter(window_buffer, &end_iter);
	gtk_text_buffer_move_mark(window_buffer, input_position, &end_iter);

	/* Set the entire contents of the window_buffer as uneditable
	 * (so input can only be entered at the end) */
	GtkTextIter start_iter;
	gtk_text_buffer_get_start_iter(window_buffer, &start_iter);
	gtk_text_buffer_remove_tag_by_name(window_buffer, "uneditable", &start_iter, &end_iter);
	gtk_text_buffer_apply_tag_by_name(window_buffer, "uneditable", &start_iter, &end_iter);

	if(initlen > 0) {
		GError *error = NULL;
		gchar *utf8;
		utf8 = g_ucs4_to_utf8(buf, initlen, NULL, NULL, &error);
			
		if(utf8 == NULL)
		{
			error_dialog(NULL, error, "Error during unicode->utf8 conversion: ");
			return;
		}

		gtk_text_buffer_insert(window_buffer, &end_iter, utf8, -1);
		g_free(utf8);
	}

	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(win->widget), input_position);
	g_signal_handler_unblock( G_OBJECT(window_buffer), win->insert_text_handler );
	gtk_text_view_set_editable(GTK_TEXT_VIEW(win->widget), TRUE);
}



gboolean
on_window_key_press_event(GtkWidget *widget, GdkEventKey *event, winid_t window)
{
	if(window->input_request_type != INPUT_REQUEST_CHARACTER && 
		window->input_request_type != INPUT_REQUEST_CHARACTER_UNICODE) {
		return FALSE;
	}

	int keycode;

	switch(event->keyval) {
		case GDK_Up:
		case GDK_KP_Up: keycode = keycode_Up; break;
		case GDK_Down: 
		case GDK_KP_Down: keycode = keycode_Down; break;
		case GDK_Left:
		case GDK_KP_Left: keycode = keycode_Left; break;
		case GDK_Right:
		case GDK_KP_Right: keycode = keycode_Right; break;
		case GDK_Return:
		case GDK_KP_Enter: keycode = keycode_Return; break;
		case GDK_Delete:
		case GDK_BackSpace:
		case GDK_KP_Delete: keycode = keycode_Delete; break;
		case GDK_Escape: keycode = keycode_Escape; break;
		case GDK_Tab: keycode = keycode_Tab; break;
		case GDK_Page_Up:
		case GDK_KP_Page_Up: keycode = keycode_PageUp; break;
		case GDK_Page_Down:
		case GDK_KP_Page_Down: keycode = keycode_PageDown; break;
		case GDK_Home:
		case GDK_KP_Home: keycode = keycode_Home; break;
		case GDK_End:
		case GDK_KP_End: keycode = keycode_End; break;
		case GDK_F1: keycode = keycode_Func1; break;
		case GDK_F2: keycode = keycode_Func2; break;
		case GDK_F3: keycode = keycode_Func3; break;
		case GDK_F4: keycode = keycode_Func4; break;
		case GDK_F5: keycode = keycode_Func5; break;
		case GDK_F6: keycode = keycode_Func6; break;
		case GDK_F7: keycode = keycode_Func7; break;
		case GDK_F8: keycode = keycode_Func8; break;
		case GDK_F9: keycode = keycode_Func9; break;
		case GDK_F10: keycode = keycode_Func10; break;
		case GDK_F11: keycode = keycode_Func11; break;
		case GDK_F12: keycode = keycode_Func12; break;
		default:
			keycode = gdk_keyval_to_unicode(event->keyval);
	}

	if(window->input_request_type == INPUT_REQUEST_CHARACTER) {
		if(keycode >= 1 || keycode <= 255) {
			event_throw(evtype_CharInput, window, keycode, 0);
		} else {
			event_throw(evtype_CharInput, window, keycode_Unknown, 0);
		}
	} else {
		if(keycode == 0) {
			event_throw(evtype_CharInput, window, keycode_Unknown, 0);
		} else {
			event_throw(evtype_CharInput, window, keycode, 0);
		}
	}

	/* Only one keypress will be handled */
	window->input_request_type = INPUT_REQUEST_NONE;
	g_signal_handler_block( G_OBJECT(window->widget), window->keypress_handler );

	return TRUE;
}

void
on_window_insert_text(GtkTextBuffer *textbuffer, GtkTextIter *location, gchar *text, gint len, gpointer user_data) 
{
	gchar *newline_pos = strchr(text, '\n');
	if(newline_pos != NULL) {
		printf("position: %d\n", newline_pos-text);
		*newline_pos = 'a';
	}
}

void
after_window_insert_text(GtkTextBuffer *textbuffer, GtkTextIter *location, gchar *text, gint len, winid_t window) 
{
	if( strchr(text, '\n') != NULL) { 
		/* Make the window uneditable again and remove signal handlers */
		GtkTextBuffer *window_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(window->widget));
		gtk_text_view_set_editable(GTK_TEXT_VIEW(window->widget), FALSE);
		g_signal_handler_block(window_buffer, window->insert_text_handler);

		/* Retrieve the text that was input */
		GtkTextMark *input_position = gtk_text_buffer_get_mark(window_buffer, "input_position");
		GtkTextIter start_iter;
		GtkTextIter end_iter;
		gtk_text_buffer_get_iter_at_mark(window_buffer, &start_iter, input_position);
		gtk_text_buffer_get_end_iter(window_buffer, &end_iter);
		gchar *inserted_text = gtk_text_buffer_get_text(window_buffer, &start_iter, &end_iter, FALSE);


		/* Convert the string from UTF-8 to Latin-1 or Unicode */
		if(window->input_request_type == INPUT_REQUEST_LINE) {
			GError *error = NULL;
			gchar *latin1;
			gsize bytes_written;
			latin1 = g_convert_with_fallback(inserted_text, -1, "ISO-8859-1", "UTF-8", "?", NULL, &bytes_written, &error);
			g_free(inserted_text);
			
			if(latin1 == NULL)
			{
				error_dialog(NULL, error, "Error during utf8->latin1 conversion: ");
				event_throw(evtype_LineInput, window, 0, 0);
				return;
			}

			/* Place input in the echo stream */
			if(window->echo_stream != NULL) 
				glk_put_string_stream(window->echo_stream, latin1);

			/* Copy the string (but not the NULL at the end) */
			memcpy(window->line_input_buffer, latin1, MIN(window->line_input_buffer_max_len, bytes_written-1));
			g_free(latin1);
			event_throw(evtype_LineInput, window, MIN(window->line_input_buffer_max_len, bytes_written-1), 0);
		}
		else if(window->input_request_type == INPUT_REQUEST_LINE_UNICODE) {
			gunichar *unicode;
			glong items_written;
			unicode = g_utf8_to_ucs4_fast(inserted_text, -1, &items_written);
			g_free(inserted_text);
			
			if(unicode == NULL)
			{
				error_dialog(NULL, NULL, "Error during utf8->unicode conversion");
				event_throw(evtype_LineInput, window, 0, 0);
				return;
			}

			/* Place input in the echo stream */
			// TODO: fixme
			// if(window->echo_stream != NULL) 
			// 	glk_put_string_stream_uni(window->echo_stream, unicode);

			/* Copy the string (but not the NULL at the end) */
			memcpy(window->line_input_buffer_unicode, unicode, MIN(window->line_input_buffer_max_len, items_written)*sizeof(gunichar));
			g_free(unicode);
			event_throw(evtype_LineInput, window, MIN(window->line_input_buffer_max_len, items_written), 0);
		}
		else {
			g_warning("%s: Wrong input request type.", __func__);
		}

		window->input_request_type = INPUT_REQUEST_NONE;
	}
}
