#include <string.h>

#include <glib.h>

#include "charset.h"
#include "chimara-glk-private.h"
#include "garglk.h"
#include "magic.h"
#include "ui-message.h"
#include "window.h"

extern GPrivate glk_data_key;

/* Internal function: cancels any pending input requests on the window and
 * presents a warning if not INPUT_REQUEST_NONE */
static void
cancel_old_input_request(winid_t win)
{
	switch(win->input_request_type) {
	case INPUT_REQUEST_NONE:
		break; /* All is well */
	case INPUT_REQUEST_CHARACTER:
	case INPUT_REQUEST_CHARACTER_UNICODE:
		glk_cancel_char_event(win);
		WARNING("Cancelling pending char event");
		break;
	case INPUT_REQUEST_LINE:
	case INPUT_REQUEST_LINE_UNICODE:
		glk_cancel_line_event(win, NULL);
		WARNING("Cancelling pending line event");
		break;
	default:
		WARNING("Could not cancel pending input request: unknown input request");
	}
}

/* Internal function: code common to both flavors of char event request */
static void
request_char_event_common(winid_t win, gboolean unicode)
{
	VALID_WINDOW(win, return);
	g_return_if_fail(win->type != wintype_TextBuffer || win->type != wintype_TextGrid);

	cancel_old_input_request(win);

	UiMessage *msg = ui_message_new(UI_MESSAGE_REQUEST_CHAR_INPUT, win);
	msg->boolval = unicode;
	ui_message_queue(msg);
}

/**
 * glk_request_char_event:
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
	request_char_event_common(win, FALSE);
}

/**
 * glk_request_char_event_uni:
 * @win: A window to request char events from.
 *
 * Request input of a Unicode character or special key. See
 * glk_request_char_event().
 */
void
glk_request_char_event_uni(winid_t win)
{
	request_char_event_common(win, TRUE);
}

/**
 * glk_cancel_char_event:
 * @win: A window to cancel the latest char event request on.
 *
 * This cancels a pending request for character input. (Either Latin-1 or
 * Unicode.) For convenience, it is legal to call glk_cancel_char_event() even
 * if there is no charcter input request on that window. Glk will ignore the
 * call in this case.
 */
void
glk_cancel_char_event(winid_t win)
{
	VALID_WINDOW(win, return);
	g_return_if_fail(win->type != wintype_TextBuffer || win->type != wintype_TextGrid);

	ui_message_queue(ui_message_new(UI_MESSAGE_CANCEL_CHAR_INPUT, win));
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
 * &mdash; just as if the player had typed them himself. (The player can continue
 * composing after this pre-entered input, or delete it or edit as usual.)
 *
 * The contents of the buffer are undefined until the input is completed (either
 * by a line input event, or glk_cancel_line_event(). The library may or may not
 * fill in the buffer as the player composes, while the input is still pending;
 * it is illegal to change the contents of the buffer yourself.
 */
void
glk_request_line_event(winid_t win, char *buf, glui32 maxlen, glui32 initlen)
{
	VALID_WINDOW(win, return);
	g_return_if_fail(buf);
	g_return_if_fail(win->type != wintype_TextBuffer || win->type != wintype_TextGrid);
	g_return_if_fail(initlen <= maxlen);

	cancel_old_input_request(win);

	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);

	/* Register the buffer */
	if(glk_data->register_arr)
        win->buffer_rock = (*glk_data->register_arr)(buf, maxlen, "&+#!Cn");

	win->input_request_type = INPUT_REQUEST_LINE;
	win->last_line_input_was_unicode = false;
	win->line_input_buffer = buf;
	win->line_input_buffer_max_len = maxlen;
	win->echo_current_line_input = win->echo_line_input;
	win->current_extra_line_terminators = g_slist_copy(win->extra_line_terminators);

	gchar *inserttext = (initlen > 0)? g_strndup(buf, initlen) : g_strdup("");

	UiMessage *msg = ui_message_new(UI_MESSAGE_REQUEST_LINE_INPUT, win);
	msg->uintval1 = maxlen;
	msg->boolval = initlen > 0;
	msg->strval = inserttext;
	ui_message_queue(msg);
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
 * #glui32 values instead of an array of characters, and the values may be any
 * valid Unicode code points.
 *
 * If possible, the library should return fully composed Unicode characters,
 * rather than strings of base and composition characters.
 *
 * <note><para>
 *   Fully-composed characters are the norm for Unicode text, so an
 *   implementation that ignores this issue will probably produce the right
 *   result. However, the game may not want to rely on that. Another factor is
 *   that case-folding can (occasionally) produce non-normalized text.
 *   Therefore, to cover all its bases, a game should call
 *   glk_buffer_to_lower_case_uni(), followed by
 *   glk_buffer_canon_normalize_uni(), before parsing.
 * </para></note>
 *
 * <note><para>
 *   Earlier versions of this spec said that line input must always be in
 *   Unicode Normalization Form C. However, this has not been universally
 *   implemented. It is also somewhat redundant, for the results noted above.
 *   Therefore, we now merely recommend that line input be fully composed. The
 *   game is ultimately responsible for all case-folding and normalization. See
 *   <link linkend="chimara-Unicode-String-Normalization">Unicode String
 *   Normalization</link>.
 * </para></note>
 */
void
glk_request_line_event_uni(winid_t win, glui32 *buf, glui32 maxlen, glui32 initlen)
{
	VALID_WINDOW(win, return);
	g_return_if_fail(buf);
	g_return_if_fail(win->type != wintype_TextBuffer || win->type != wintype_TextGrid);
	g_return_if_fail(initlen <= maxlen);

	cancel_old_input_request(win);
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);

	/* Register the buffer */
	if(glk_data->register_arr)
        win->buffer_rock = (*glk_data->register_arr)(buf, maxlen, "&+#!Iu");

	win->input_request_type = INPUT_REQUEST_LINE_UNICODE;
	win->last_line_input_was_unicode = true;
	win->line_input_buffer_unicode = buf;
	win->line_input_buffer_max_len = maxlen;
	win->echo_current_line_input = win->echo_line_input;
	win->current_extra_line_terminators = g_slist_copy(win->extra_line_terminators);

	gchar *utf8;
	if(initlen > 0) {
		utf8 = convert_ucs4_to_utf8(buf, initlen);
		if(utf8 == NULL)
			return;
	}
	else
		utf8 = g_strdup("");

	UiMessage *msg = ui_message_new(UI_MESSAGE_REQUEST_LINE_INPUT, win);
	msg->uintval1 = maxlen;
	msg->boolval = initlen > 0;
	msg->strval = utf8;
	ui_message_queue(msg);
}

/**
 * glk_cancel_line_event:
 * @win: A text buffer or text grid window to cancel line input on.
 * @event: Will be filled in if the user had already input something.
 *
 * This cancels a pending request for line input. (Either Latin-1 or Unicode.)
 *
 * The event pointed to by the event argument will be filled in as if the
 * player had hit <keycap>enter</keycap>, and the input composed so far will be
 * stored in the buffer; see below. If you do not care about this information,
 * pass %NULL as the @event argument. (The buffer will still be filled.)
 *
 * For convenience, it is legal to call glk_cancel_line_event() even if there
 * is no line input request on that window. The event type will be set to
 * %evtype_None in this case.
 */
void
glk_cancel_line_event(winid_t win, event_t *event)
{
	VALID_WINDOW(win, return);
	g_return_if_fail(win->type != wintype_TextBuffer || win->type != wintype_TextGrid);

	if(event != NULL) {
		event->type = evtype_None;
		event->win = win;
		event->val1 = 0;
		event->val2 = 0;
	}

	if(win->input_request_type != INPUT_REQUEST_LINE && win->input_request_type != INPUT_REQUEST_LINE_UNICODE)
		return;

	UiMessage *msg = ui_message_new(UI_MESSAGE_CANCEL_LINE_INPUT, win);
	int chars_written = ui_message_queue_and_await(msg);

	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
	if(glk_data->unregister_arr)
	{
		if(win->input_request_type == INPUT_REQUEST_LINE_UNICODE)
			(*glk_data->unregister_arr)(win->line_input_buffer_unicode, win->line_input_buffer_max_len, "&+#!Iu", win->buffer_rock);
		else
        	(*glk_data->unregister_arr)(win->line_input_buffer, win->line_input_buffer_max_len, "&+#!Cn", win->buffer_rock);
    }

	if(event != NULL) {
		event->type = evtype_LineInput;
		event->val1 = chars_written;
	}
}

glui32
keyval_to_glk_keycode(guint keyval, gboolean unicode)
{
	glui32 keycode;
	switch(keyval) {
		case GDK_KEY_Up:
		case GDK_KEY_KP_Up: return keycode_Up;
		case GDK_KEY_Down:
		case GDK_KEY_KP_Down: return keycode_Down;
		case GDK_KEY_Left:
		case GDK_KEY_KP_Left: return keycode_Left;
		case GDK_KEY_Right:
		case GDK_KEY_KP_Right: return keycode_Right;
		case GDK_KEY_Linefeed:
		case GDK_KEY_Return:
		case GDK_KEY_KP_Enter: return keycode_Return;
		case GDK_KEY_Delete:
		case GDK_KEY_BackSpace:
		case GDK_KEY_KP_Delete: return keycode_Delete;
		case GDK_KEY_Escape: return keycode_Escape;
		case GDK_KEY_Tab:
		case GDK_KEY_KP_Tab: return keycode_Tab;
		case GDK_KEY_Page_Up:
		case GDK_KEY_KP_Page_Up: return keycode_PageUp;
		case GDK_KEY_Page_Down:
		case GDK_KEY_KP_Page_Down: return keycode_PageDown;
		case GDK_KEY_Home:
		case GDK_KEY_KP_Home: return keycode_Home;
		case GDK_KEY_End:
		case GDK_KEY_KP_End: return keycode_End;
		case GDK_KEY_F1:
		case GDK_KEY_KP_F1: return keycode_Func1;
		case GDK_KEY_F2:
		case GDK_KEY_KP_F2: return keycode_Func2;
		case GDK_KEY_F3:
		case GDK_KEY_KP_F3: return keycode_Func3;
		case GDK_KEY_F4:
		case GDK_KEY_KP_F4: return keycode_Func4;
		case GDK_KEY_F5: return keycode_Func5;
		case GDK_KEY_F6: return keycode_Func6;
		case GDK_KEY_F7: return keycode_Func7;
		case GDK_KEY_F8: return keycode_Func8;
		case GDK_KEY_F9: return keycode_Func9;
		case GDK_KEY_F10: return keycode_Func10;
		case GDK_KEY_F11: return keycode_Func11;
		case GDK_KEY_F12: return keycode_Func12;
		default:
			keycode = gdk_keyval_to_unicode(keyval);
			/* If keycode is 0, then keyval was not recognized; also return
			unknown if Latin-1 input was requested and the character is not in
			Latin-1 */
			if(keycode == 0 || (!unicode && keycode > 255))
				return keycode_Unknown;
			return keycode;
	}
}

void
force_char_input_from_queue(winid_t win, event_t *event)
{
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
	guint keyval = GPOINTER_TO_UINT(g_async_queue_pop(glk_data->char_input_queue));
    bool is_unicode = win->input_request_type == INPUT_REQUEST_CHARACTER_UNICODE;

	glk_cancel_char_event(win);

	UiMessage *msg = ui_message_new(UI_MESSAGE_FORCE_CHAR_INPUT, win);
	msg->uintval1 = keyval;
	ui_message_queue(msg);

	event->type = evtype_CharInput;
	event->win = win;
	event->val1 = keyval_to_glk_keycode(keyval, is_unicode);
	event->val2 = 0;
}

void
force_line_input_from_queue(winid_t win, event_t *event)
{
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
	const gchar *text = g_async_queue_pop(glk_data->line_input_queue);

	UiMessage *msg = ui_message_new(UI_MESSAGE_FORCE_LINE_INPUT, win);
	msg->strval = g_strdup(text);
	glui32 chars_written = ui_message_queue_and_await(msg);

	event->type = evtype_LineInput;
	event->win = win;
	event->val1 = chars_written;
	event->val2 = 0;
}

/**
 * glk_set_echo_line_event:
 * @win: The window in which to change the echoing behavior.
 * @val: Zero to turn off echoing, nonzero for normal echoing.
 *
 * Normally, after line input is completed or cancelled in a buffer window, the
 * library ensures that the complete input line (or its latest state, after
 * cancelling) is displayed at the end of the buffer, followed by a newline.
 * This call allows you to suppress this behavior. If the @val argument is zero,
 * all <emphasis>subsequent</emphasis> line input requests in the given window
 * will leave the buffer unchanged after the input is completed or cancelled;
 * the player's input will not be printed. If @val is nonzero, subsequent input
 * requests will have the normal printing behavior.
 *
 * <note><para>
 *   Note that this feature is unrelated to the window's echo stream.
 * </para></note>
 *
 * If you turn off line input echoing, you can reproduce the standard input
 * behavior by following each line input event (or line input cancellation) by
 * printing the input line, in the Input style, followed by a newline in the
 * original style.
 *
 * The glk_set_echo_line_event() does not affect a pending line input request.
 * It also has no effect in non-buffer windows.
 * <note><para>
 *   In a grid window, the game can overwrite the input area at will, so there
 *   is no need for this distinction.
 * </para></note>
 *
 * Not all libraries support this feature. You can test for it with
 * %gestalt_LineInputEcho.
 */
void
glk_set_echo_line_event(winid_t win, glui32 val)
{
	VALID_WINDOW(win, return);

	if(win->type != wintype_TextBuffer)
		return; /* Quietly do nothing */

	win->echo_line_input = val? TRUE : FALSE;
}

/* Internal function to convert from a Glk keycode to a GDK keyval. */
static unsigned
keycode_to_gdk_keyval(glui32 keycode)
{
	switch (keycode)
	{
		case keycode_Left:
			return GDK_KEY_Left;
		case keycode_Right:
			return GDK_KEY_Right;
		case keycode_Up:
			return GDK_KEY_Up;
		case keycode_Down:
			return GDK_KEY_Down;
		case keycode_Return:
			return GDK_KEY_Return;
		case keycode_Delete:
			return GDK_KEY_Delete;
		case keycode_Escape:
			return GDK_KEY_Escape;
		case keycode_Tab:
			return GDK_KEY_Tab;
		case keycode_PageUp:
			return GDK_KEY_Page_Up;
		case keycode_PageDown:
			return GDK_KEY_Page_Down;
		case keycode_Home:
			return GDK_KEY_Home;
		case keycode_End:
			return GDK_KEY_End;
		case keycode_Func1:
			return GDK_KEY_F1;
		case keycode_Func2:
			return GDK_KEY_F2;
		case keycode_Func3:
			return GDK_KEY_F3;
		case keycode_Func4:
			return GDK_KEY_F4;
		case keycode_Func5:
			return GDK_KEY_F5;
		case keycode_Func6:
			return GDK_KEY_F6;
		case keycode_Func7:
			return GDK_KEY_F7;
		case keycode_Func8:
			return GDK_KEY_F8;
		case keycode_Func9:
			return GDK_KEY_F9;
		case keycode_Func10:
			return GDK_KEY_F10;
		case keycode_Func11:
			return GDK_KEY_F11;
		case keycode_Func12:
			return GDK_KEY_F12;
		case keycode_Erase:
			return GDK_KEY_BackSpace;
	}
	unsigned keyval = gdk_unicode_to_keyval(keycode);
	if(keyval < 0x01000000) /* magic number meaning illegal unicode point */
		return keyval;
	return 0;
}

/* Internal function to decide whether @keycode is a valid line terminator. */
gboolean
is_valid_line_terminator(glui32 keycode)
{
	switch(keycode) {
		case keycode_Escape:
		case keycode_Func1:
		case keycode_Func2:
		case keycode_Func3:
		case keycode_Func4:
		case keycode_Func5:
		case keycode_Func6:
		case keycode_Func7:
		case keycode_Func8:
		case keycode_Func9:
		case keycode_Func10:
		case keycode_Func11:
		case keycode_Func12:
			return TRUE;
	}
	return FALSE;
}

/**
 * glk_set_terminators_line_event:
 * @win: The window for which to set the line input terminator keys.
 * @keycodes: An array of `keycode_` constants, of length @count.
 * @count: The array length of @keycodes.
 *
 * It is possible to request that other keystrokes complete line input as well.
 * (This allows a game to intercept function keys or other special keys during
 * line input.) To do this, call glk_set_terminators_line_event(), and pass an
 * array of @count keycodes.
 * These must all be special keycodes (see [Character
 * Input][chimara-Character-Input]).
 * Do not include regular printable characters in the array, nor %keycode_Return
 * (which represents the default <keycap>enter</keycap> key and will always be
 * recognized). To return to the default behavior, pass a %NULL or empty array.
 *
 * The glk_set_terminators_line_event() affects <emphasis>subsequent</emphasis>
 * line input requests in the given window. It does not affect a pending line
 * input request.
 *
 * <note><para>
 *   This distinction makes life easier for interpreters that set up UI
 *   callbacks only at the start of input.
 * </para></note>
 *
 * A library may not support this feature; if it does, it may not support all
 * special keys as terminators. (Some keystrokes are reserved for OS or
 * interpreter control.) You can test for this with %gestalt_LineTerminators and
 * %gestalt_LineTerminatorKey.
 */
void
glk_set_terminators_line_event(winid_t win, glui32 *keycodes, glui32 count)
{
	VALID_WINDOW(win, return);

	g_slist_free(win->extra_line_terminators);
	win->extra_line_terminators = NULL;

	if(keycodes == NULL || count == 0)
		return;

	int i;
	for(i = 0; i < count; i++)
	{
		unsigned key = keycode_to_gdk_keyval(keycodes[i]);
		if(is_valid_line_terminator(keycodes[i]))
			win->extra_line_terminators = g_slist_prepend(win->extra_line_terminators, GUINT_TO_POINTER(key));
		else
		   WARNING_S("Ignoring invalid line terminator", gdk_keyval_name(key));
	}
}
