#include <stdbool.h>

#include <gtk/gtk.h>

#include "chimara-glk-private.h"
#include "strio.h"
#include "ui-buffer.h"
#include "ui-graphics.h"
#include "ui-grid.h"
#include "ui-message.h"
#include "ui-misc.h"
#include "ui-style.h"
#include "ui-textwin.h"
#include "ui-window.h"
#include "window.h"

extern GPrivate glk_data_key;

struct SyncArrangeCallbackData {
	UiMessage *msg;
	unsigned handler_id;
};

#ifdef DEBUG_MESSAGES

static const char *desc[] = {
	"print string",
	"create window",
	"unparent widget",
	"arrange",
	"silent arrange",
	"sync arrange",
	"clear window",
	"move cursor",
	"grid newline",
	"set style",
	"set zcolors",
	"set reverse video",
	"set stylehint",
	"clear stylehint",
	"measure style",
	"file prompt",
	"confirm file overwrite",
	"request char input",
	"cancel char input",
	"force char input",
	"request line input",
	"cancel line input",
	"force line input",
	"set hyperlink",
	"request hyperlink input",
	"cancel hyperlink input",
	"graphics draw image",
	"graphics fill rect",
	"buffer draw image",
	"shutdown"
};

static void
debug_ui_message(UiMessage *msg, gboolean queueing)
{
	g_message("%s %s, win=%p", queueing? "Queueing" : "Processing",
		desc[msg->type], msg->win);
}

#endif /* DEBUG_MESSAGES */

/* Creates a new message of @type to pass to the UI thread.
 * @win is the Glk window it refers to, though that may be NULL.
 * You own the return value, which must be freed with ui_message_free(),
 * although if you actually pass the message to the UI thread then you will
 * transfer your ownership of the message as well. */
UiMessage *
ui_message_new(UiMessageType type, winid_t win)
{
	UiMessage *msg = g_slice_new0(UiMessage);
	msg->type = type;
	msg->win = win;
	msg->is_waiting = false;
	return msg;
}

/* Frees a message created with ui_message_new(). */
void
ui_message_free(UiMessage *msg)
{
	g_free(msg->strval);
	g_clear_pointer(&msg->response, g_variant_unref);
	g_slice_free(UiMessage, msg);
}

/* Sends @msg to the UI thread.
 * This function must be called from the Glk thread.
 * Takes over your ownership of @msg until it is received on the other side by
 * the UI thread, which will free it.
 * After queueing a message it is illegal to access it from the Glk thread. */
void
ui_message_queue(UiMessage *msg)
{
	/* Some messages imply flushing the window buffer first, since they affect
	how or where following text is printed. Also the window buffer is flushed
	before any input is requested. */
	if (msg->type == UI_MESSAGE_MOVE_CURSOR ||
	    msg->type == UI_MESSAGE_GRID_NEWLINE ||
		msg->type == UI_MESSAGE_SET_STYLE ||
		msg->type == UI_MESSAGE_SET_ZCOLORS ||
		msg->type == UI_MESSAGE_SET_REVERSE_VIDEO ||
		msg->type == UI_MESSAGE_REQUEST_CHAR_INPUT ||
		msg->type == UI_MESSAGE_REQUEST_LINE_INPUT ||
		msg->type == UI_MESSAGE_SET_HYPERLINK ||
		msg->type == UI_MESSAGE_REQUEST_HYPERLINK_INPUT ||
		msg->type == UI_MESSAGE_BUFFER_DRAW_IMAGE)
		queue_flush_window_buffer(msg->win);
	else if (msg->type == UI_MESSAGE_CLEAR_WINDOW &&
	    (msg->win->type == wintype_TextBuffer || msg->win->type == wintype_TextGrid))
	    queue_flush_window_buffer(msg->win);

#ifdef DEBUG_MESSAGES
	debug_ui_message(msg, TRUE);
#endif

	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
	g_async_queue_push(glk_data->ui_message_queue, msg);
}

/* Helper function: queues @msg, waits for response as a GVariant */
static void
queue_and_await_response(UiMessage *msg)
{
	g_mutex_init(&msg->lock);
	g_cond_init(&msg->sign);
	msg->response = NULL;
	msg->is_waiting = true;

	ui_message_queue(msg);

	g_mutex_lock(&msg->lock);
	while(msg->response == NULL)
		g_cond_wait(&msg->sign, &msg->lock);
	g_mutex_unlock(&msg->lock);

	g_mutex_clear(&msg->lock);
	g_cond_clear(&msg->sign);
}

/* Sends @msg to the UI thread and blocks on a response in the form of a 64-bit
 * signed integer.
 * (That data type can hold a number of flags as well as a glui32 or glsi32, if
 * necessary.)
 * This function must be called from the Glk thread.
 * It is illegal to send a message with this function that isn't supposed to
 * send back a 64-bit signed integer response.
 * Takes over your ownership of @msg just as ui_message_queue() does.
 */
gint64
ui_message_queue_and_await(UiMessage *msg)
{
	queue_and_await_response(msg);
	int retval = g_variant_get_int64(msg->response);
	ui_message_free(msg);
	return retval;
}

/* Sends @msg to the UI thread and blocks on a response in the form of a string.
 * Takes over your ownership of @msg just as ui_message_queue() does.
 * This function must be called from the Glk thread.
 * It is illegal to send a message with this function that isn't supposed to
 * send back a string response.
 * You gain ownership of the return value, which may be %NULL, and must be freed
 * with g_free(). */
char *
ui_message_queue_and_await_string(UiMessage *msg)
{
	queue_and_await_response(msg);
	char *retval;
	g_variant_get(msg->response, "ms", &retval);
	ui_message_free(msg);
	return retval;
}

/* Sends an answer back to the Glk thread in response to @msg, in the form of a
 * 64-bit signed integer @response.
 * (See ui_message_queue_and_await().)
 * This function must be called from the UI thread. */
static void
ui_message_respond(UiMessage *msg, gint64 response)
{
	g_mutex_lock(&msg->lock);
	msg->response = g_variant_new_int64(response);
	g_variant_ref_sink(msg->response);
	g_cond_signal(&msg->sign);
	g_mutex_unlock(&msg->lock);
}

/* Sends an answer back to the Glk thread in response to @msg, in the form of a
 * string @response.
 * (See ui_message_queue_and_await_string().)
 * This function must be called from the UI thread.
 * The string @response must be allocated (not a string literal) and you must
 * own it.
 * This function takes over your ownership of @reponse. */
static void
ui_message_respond_string(UiMessage *msg, char *response)
{
	g_mutex_lock(&msg->lock);
	msg->response = g_variant_new_maybe(G_VARIANT_TYPE_STRING,
		response != NULL ? g_variant_new_take_string(response) : NULL);
	g_variant_ref_sink(msg->response);
	g_cond_signal(&msg->sign);
	g_mutex_unlock(&msg->lock);
}

/* Callback for responding to UI_MESSAGE_SYNC_ARRANGE below */
static void
respond_after_size_allocate(ChimaraGlk *glk, GtkAllocation *allocation, struct SyncArrangeCallbackData *data)
{
	/* If no response is being awaited, we can free the message at the end of
	 * this function. Otherwise, the other thread will free it after the
	 * response has been received. */
	bool should_free = !data->msg->is_waiting;

	ui_message_respond(data->msg, 1);

	if (should_free)
		ui_message_free(data->msg);

	g_signal_handler_disconnect(glk, data->handler_id);
	g_slice_free(struct SyncArrangeCallbackData, data);
}

void
ui_message_perform(ChimaraGlk *glk, UiMessage *msg)
{
#ifdef DEBUG_MESSAGES
	debug_ui_message(msg, FALSE);
#endif

	/* If no response is being awaited, we can free the message at the end of
	 * this function. Otherwise, the other thread will free it after the
	 * response has been received. */
	bool should_free = !msg->is_waiting;

	switch(msg->type) {
	case UI_MESSAGE_PRINT_STRING:
		ui_textwin_print_string(msg->win, msg->strval);
		ui_message_respond(msg, 1);
		break;
	case UI_MESSAGE_CREATE_WINDOW:
		ui_window_create(msg->win, glk);
		ui_message_respond(msg, 1);
		break;
	case UI_MESSAGE_UNPARENT_WIDGET:
		gtk_widget_unparent(GTK_WIDGET(msg->ptrval));
		break;
	case UI_MESSAGE_ARRANGE:
		chimara_glk_queue_arrange(glk, FALSE);
		break;
	case UI_MESSAGE_ARRANGE_SILENTLY:
		chimara_glk_queue_arrange(glk, TRUE);
		break;
	case UI_MESSAGE_SYNC_ARRANGE:
		if (!chimara_glk_needs_rearrange(glk)) {
			ui_message_respond(msg, 1);
			break;
		}

		struct SyncArrangeCallbackData *data = g_slice_new0(struct SyncArrangeCallbackData);
		data->msg = msg;
		data->handler_id = g_signal_connect_after(glk, "size-allocate",
			G_CALLBACK(respond_after_size_allocate), data);

		return;  /* not break, msg is freed in the callback! */
	case UI_MESSAGE_CLEAR_WINDOW:
		ui_window_clear(msg->win);
		break;
	case UI_MESSAGE_MOVE_CURSOR:
		ui_grid_move_cursor(msg->win, msg->uintval1, msg->uintval2);
		break;
	case UI_MESSAGE_GRID_NEWLINE:
		ui_grid_newline_cursor(msg->win);
		break;
	case UI_MESSAGE_SET_STYLE:
		ui_textwin_set_style(msg->win, msg->uintval1);
		break;
	case UI_MESSAGE_SET_ZCOLORS:
		ui_textwin_set_zcolors(msg->win, msg->uintval1, msg->uintval2);
		break;
	case UI_MESSAGE_SET_REVERSE_VIDEO:
		ui_textwin_set_reverse_video(msg->win, msg->boolval);
		break;
	case UI_MESSAGE_SET_STYLEHINT:
		ui_style_set_hint(glk, msg->uintval1, msg->uintval2, msg->uintval3, msg->intval);
		break;
	case UI_MESSAGE_CLEAR_STYLEHINT:
		ui_style_clear_hint(glk, msg->uintval1, msg->uintval2, msg->uintval3);
		break;
	case UI_MESSAGE_MEASURE_STYLE:
		ui_message_respond(msg, ui_window_measure_style(msg->win, glk, msg->uintval1, msg->uintval2));
		break;
	case UI_MESSAGE_FILE_PROMPT:
		ui_message_respond_string(msg, ui_prompt_for_file(glk, msg->uintval1, msg->uintval2, msg->strval));
		break;
	case UI_MESSAGE_CONFIRM_FILE_OVERWRITE:
		ui_message_respond(msg, ui_confirm_file_overwrite(glk, msg->strval));
		break;
	case UI_MESSAGE_REQUEST_CHAR_INPUT:
		ui_window_request_char_input(glk, msg->win, msg->boolval);
		break;
	case UI_MESSAGE_CANCEL_CHAR_INPUT:
		ui_window_cancel_char_input(msg->win);
		break;
	case UI_MESSAGE_FORCE_CHAR_INPUT:
		ui_window_force_char_input(msg->win, glk, msg->uintval1);
		break;
	case UI_MESSAGE_REQUEST_LINE_INPUT:
		ui_textwin_request_line_input(glk, msg->win, msg->uintval1, msg->boolval, msg->strval);
		break;
	case UI_MESSAGE_CANCEL_LINE_INPUT:
		ui_message_respond(msg, ui_textwin_cancel_line_input(msg->win));
		break;
	case UI_MESSAGE_FORCE_LINE_INPUT:
		ui_message_respond(msg, ui_textwin_force_line_input(msg->win, msg->strval));
		break;
	case UI_MESSAGE_SET_HYPERLINK:
		ui_textwin_set_hyperlink(msg->win, msg->uintval1);
		break;
	case UI_MESSAGE_REQUEST_HYPERLINK_INPUT:
		ui_textwin_request_hyperlink_input(msg->win);
		break;
	case UI_MESSAGE_CANCEL_HYPERLINK_INPUT:
		ui_textwin_cancel_hyperlink_input(msg->win);
		break;
	case UI_MESSAGE_GRAPHICS_DRAW_IMAGE:
		ui_graphics_draw_image(msg->win, GDK_PIXBUF(msg->ptrval), msg->x, msg->y);
		break;
	case UI_MESSAGE_GRAPHICS_FILL_RECT:
		ui_graphics_fill_rect(msg->win, msg->uintval1, msg->x, msg->y, msg->uintval2, msg->uintval3);
		break;
	case UI_MESSAGE_BUFFER_DRAW_IMAGE:
		ui_buffer_draw_image(msg->win, GDK_PIXBUF(msg->ptrval), msg->uintval1);
		break;
	case UI_MESSAGE_SHUTDOWN:
		chimara_glk_stop_processing_queue(glk);
		ui_message_respond(msg, 1);
		break;
	}

	if (should_free)
		ui_message_free(msg);
}
