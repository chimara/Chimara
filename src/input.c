#include "input.h"

/** glk_request_char_event:
 * @win: A window to request char events from.
 *
 * Request input of a Latin-1 character or special key. A window cannot have 
 * requests for both character and line input at the same time. Nor can it have
 * requests for character input of both types (Latin-1 and Unicode). It is
 * illegal to call glk_request_char_event() if the window already has a pending
 * request for either character or line input. 
 */
void
glk_request_char_event(winid_t win)
{
	g_return_if_fail(win);
	g_return_if_fail(win->input_request_type == INPUT_REQUEST_NONE);
	g_return_if_fail(win->type != wintype_TextBuffer || win->type != wintype_TextGrid);

	win->input_request_type = INPUT_REQUEST_CHARACTER;
	g_signal_handler_unblock( G_OBJECT(win->widget), win->keypress_handler );
}

/** glk_request_char_event_uni:
 * @win: A window to request char events from.
 *
 * Request input of a Unicode character or special key. See 
 * glk_request_char_event().
 */
void
glk_request_char_event_uni(winid_t win)
{
	g_return_if_fail(win);
	g_return_if_fail(win->input_request_type == INPUT_REQUEST_NONE);
	g_return_if_fail(win->type != wintype_TextBuffer || win->type != wintype_TextGrid);

	win->input_request_type = INPUT_REQUEST_CHARACTER_UNICODE;
	g_signal_handler_unblock( G_OBJECT(win->widget), win->keypress_handler );
}

/* Internal function: Request either latin-1 or unicode line input. */
void
request_line_event_common(winid_t win, gboolean insert, gchar *inserttext)
{
	GtkTextBuffer *window_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(win->widget));

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
	
	/* Insert pre-entered text if needed */
	if(insert)
		gtk_text_buffer_insert(window_buffer, &end_iter, inserttext, -1);
	
	/* Scroll to input point */
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(win->widget), input_position);
	g_signal_handler_unblock( G_OBJECT(window_buffer), win->insert_text_handler );
	gtk_text_view_set_editable(GTK_TEXT_VIEW(win->widget), TRUE);
}

/**
 * glk_request_line_event:
 * @win: A text buffer or text grid window to request line input on.
 * @buf: A buffer of at least @maxlen bytes.
 * @maxlen: Length of the buffer.
 * @initlen: The number of characters in @buf to pre-enter.
 *
 * Requests input of a line of Latin-1 characters. A window cannot have requests
 * for both character and line input at the same time. Nor can it have requests
 * for line input of both types (Latin-1 and Unicode). It is illegal to call
 * glk_request_line_event() if the window already has a pending request for
 * either character or line input.
 * 
 * The @buf argument is a pointer to space where the line input will be stored.
 * (This may not be %NULL.) @maxlen is the length of this space, in bytes; the
 * library will not accept more characters than this. If @initlen is nonzero,
 * then the first @initlen bytes of @buf will be entered as pre-existing input
 * -- just as if the player had typed them himself. (The player can continue
 * composing after this pre-entered input, or delete it or edit as usual.)
 * 
 * The contents of the buffer are undefined until the input is completed (either
 * by a line input event, or glk_cancel_line_event(). The library may or may not
 * fill in the buffer as the player composes, while the input is still pending;
 * it is illegal to change the contents of the buffer yourself. 
 */
void
glk_request_line_event(winid_t win, char* buf, glui32 maxlen, glui32 initlen)
{
	g_return_if_fail(win);
	g_return_if_fail(buf);
	g_return_if_fail(win->input_request_type == INPUT_REQUEST_NONE);
	g_return_if_fail(win->type != wintype_TextBuffer || win->type != wintype_TextGrid);

	win->input_request_type = INPUT_REQUEST_LINE;
	win->line_input_buffer = buf;
	win->line_input_buffer_max_len = maxlen;

	gchar *inserttext = (initlen > 0)? g_strndup(buf, initlen) : g_strdup("");
	request_line_event_common(win, (initlen > 0), inserttext);
	g_free(inserttext);
}

/**
 * glk_request_line_event_uni:
 * @win: A text buffer or text grid window to request line input on.
 * @buf: A buffer of at least @maxlen characters.
 * @maxlen: Length of the buffer.
 * @initlen: The number of characters in @buf to pre-enter.
 *
 * Request input of a line of Unicode characters. This works the same as
 * glk_request_line_event(), except the result is stored in an array of
 * <type>glui32</type> values instead of an array of characters, and the values
 * may be any valid Unicode code points.
 * 
 * The result will be in Unicode Normalization Form C. This basically means that
 * composite characters will be single characters where possible, instead of
 * sequences of base and combining marks. See 
 * <ulink url="http://www.unicode.org/reports/tr15/">Unicode Standard Annex #15
 * </ulink> for the details.
 */
void
glk_request_line_event_uni(winid_t win, glui32 *buf, glui32 maxlen, glui32 initlen)
{
	g_return_if_fail(win);
	g_return_if_fail(buf);
	g_return_if_fail(win->input_request_type == INPUT_REQUEST_NONE);
	g_return_if_fail(win->type != wintype_TextBuffer || win->type != wintype_TextGrid);

	win->input_request_type = INPUT_REQUEST_LINE_UNICODE;
	win->line_input_buffer_unicode = buf;
	win->line_input_buffer_max_len = maxlen;

	gchar *utf8;
	if(initlen > 0) {
		GError *error = NULL;
		utf8 = g_ucs4_to_utf8(buf, initlen, NULL, NULL, &error);
			
		if(utf8 == NULL)
		{
			g_warning("Error during unicode->utf8 conversion: %s", error->message);
			return;
		}
	}
	else
		utf8 = g_strdup("");

	request_line_event_common(win, (initlen > 0), utf8);		
	g_free(utf8);
}

/* Internal function: Callback for signal key-press-event on a text buffer 
window. */
gboolean
on_window_key_press_event(GtkWidget *widget, GdkEventKey *event, winid_t win)
{
	if(win->input_request_type != INPUT_REQUEST_CHARACTER && 
		win->input_request_type != INPUT_REQUEST_CHARACTER_UNICODE)
		return FALSE;

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
		case GDK_Linefeed:
		case GDK_Return:
		case GDK_KP_Enter: keycode = keycode_Return; break;
		case GDK_Delete:
		case GDK_BackSpace:
		case GDK_KP_Delete: keycode = keycode_Delete; break;
		case GDK_Escape: keycode = keycode_Escape; break;
		case GDK_Tab: 
		case GDK_KP_Tab: keycode = keycode_Tab; break;
		case GDK_Page_Up:
		case GDK_KP_Page_Up: keycode = keycode_PageUp; break;
		case GDK_Page_Down:
		case GDK_KP_Page_Down: keycode = keycode_PageDown; break;
		case GDK_Home:
		case GDK_KP_Home: keycode = keycode_Home; break;
		case GDK_End:
		case GDK_KP_End: keycode = keycode_End; break;
		case GDK_F1: 
		case GDK_KP_F1: keycode = keycode_Func1; break;
		case GDK_F2: 
		case GDK_KP_F2: keycode = keycode_Func2; break;
		case GDK_F3: 
		case GDK_KP_F3: keycode = keycode_Func3; break;
		case GDK_F4: 
		case GDK_KP_F4: keycode = keycode_Func4; break;
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
			/* If keycode is 0, then keyval was not recognized; also return
			unknown if Latin-1 input was requested and the character is not in
			Latin-1 */
			if(keycode == 0 || (win->input_request_type == INPUT_REQUEST_CHARACTER && keycode > 255))
				keycode = keycode_Unknown;	
	}

	event_throw(evtype_CharInput, win, keycode, 0);
	
	/* Only one keypress will be handled */
	win->input_request_type = INPUT_REQUEST_NONE;
	g_signal_handler_block( G_OBJECT(win->widget), win->keypress_handler );

	return TRUE;
}

/* Internal function: Callback for signal insert-text on a text buffer window.
Not used; cannot modify the text buffer from within the callback?? Segfault! */
/*void
on_window_insert_text(GtkTextBuffer *textbuffer, GtkTextIter *location, gchar *text, gint len, gpointer user_data) 
{
	gchar *newline_pos = strchr(text, '\n');
	if(newline_pos != NULL) {
		printf("position: %d\n", newline_pos-text);
		*newline_pos = 'a';
	}
}*/

/* Internal function: Callback for signal insert-text on a text buffer window.
Runs after the default handler has already inserted the text.*/
void
after_window_insert_text(GtkTextBuffer *textbuffer, GtkTextIter *location, gchar *text, gint len, winid_t win) 
{
	if( strchr(text, '\n') != NULL ) 
	{ 
		/* Make the window uneditable again and remove signal handlers */
		GtkTextBuffer *window_buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
		gtk_text_view_set_editable(GTK_TEXT_VIEW(win->widget), FALSE);
		g_signal_handler_block(window_buffer, win->insert_text_handler);

		/* Retrieve the text that was input */
		GtkTextMark *input_position = gtk_text_buffer_get_mark(window_buffer, "input_position");
		GtkTextIter start_iter;
		GtkTextIter end_iter;
		gtk_text_buffer_get_iter_at_mark(window_buffer, &start_iter, input_position);
		gtk_text_buffer_get_end_iter(window_buffer, &end_iter);
		gchar *inserted_text = gtk_text_buffer_get_text(window_buffer, &start_iter, &end_iter, FALSE);

		/* Convert the string from UTF-8 to Latin-1 or Unicode */
		if(win->input_request_type == INPUT_REQUEST_LINE) 
		{
			GError *error = NULL;
			gchar *latin1;
			gsize bytes_written;
			latin1 = g_convert_with_fallback(inserted_text, -1, "ISO-8859-1", "UTF-8", "?", NULL, &bytes_written, &error);
			g_free(inserted_text);
			
			if(latin1 == NULL)
			{
				g_warning("Error during utf8->latin1 conversion: %s", error->message);
				event_throw(evtype_LineInput, win, 0, 0);
				return;
			}

			/* Place input in the echo stream */
			if(win->echo_stream != NULL) 
				glk_put_string_stream(win->echo_stream, latin1);

			/* Copy the string (but not the NULL at the end) */
			int copycount = MIN(win->line_input_buffer_max_len, bytes_written - 1);
			memcpy(win->line_input_buffer, latin1, copycount);
			g_free(latin1);
			event_throw(evtype_LineInput, win, copycount, 0);
		}
		else if(win->input_request_type == INPUT_REQUEST_LINE_UNICODE) 
		{
			gunichar *unicode;
			glong items_written;
			unicode = g_utf8_to_ucs4_fast(inserted_text, -1, &items_written);
			g_free(inserted_text);
			
			if(unicode == NULL)
			{
				g_warning("Error during utf8->unicode conversion");
				event_throw(evtype_LineInput, win, 0, 0);
				return;
			}

			/* Place input in the echo stream */
			/* TODO: fixme
			if(win->echo_stream != NULL) 
				glk_put_string_stream_uni(window->echo_stream, unicode);*/

			/* Copy the string (but not the NULL at the end) */
			int copycount = MIN(win->line_input_buffer_max_len, items_written);
			memcpy(win->line_input_buffer_unicode, unicode, copycount * sizeof(gunichar));
			g_free(unicode);
			event_throw(evtype_LineInput, win, copycount, 0);
		}
		else 
			g_warning("%s: Wrong input request type.", __func__);

		win->input_request_type = INPUT_REQUEST_NONE;
	}
}
