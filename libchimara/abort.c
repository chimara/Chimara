#include <glib.h>

#include "abort.h"
#include "chimara-glk-private.h"
#include "strio.h"
#include "ui-message.h"
#include "window.h"

extern GPrivate glk_data_key;

/**
 * glk_set_interrupt_handler:
 * @func: A pointer to an interrupt handler function.
 *
 * Sets @func to be the interrupt handler. @func should be a pointer to a
 * function which takes no argument and returns no result. If Glk receives an
 * interrupt, and you have set an interrupt handler, your handler will be
 * called, before the process is shut down.
 *
 * Initially there is no interrupt handler. You can reset to not having any by
 * calling `glk_set_interrupt_handler(NULL)`.
 *
 * If you call glk_set_interrupt_handler() with a new handler function while an
 * older one is set, the new one replaces the old one. Glk does not try to queue
 * interrupt handlers.
 *
 * You should not try to interact with the player in your interrupt handler. Do
 * not call glk_select() or glk_select_poll(). Anything you print to a window
 * may not be visible to the player.
 */
void
glk_set_interrupt_handler(void (*func)(void))
{
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
	glk_data->interrupt_handler = func;
}

/* Internal function: abort this Glk program, freeing resources and calling the
user's interrupt handler. */
static void
abort_glk(void)
{
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
	if(glk_data->interrupt_handler)
		(*(glk_data->interrupt_handler))();
	shutdown_glk_pre();
	shutdown_glk_post();
	/* If program is terminated by g_thread_exit() instead of returning from the
	 glk_main() function, then the line in glk_exit() where the "stopped" 
	 signal is emitted will not be reached. So we have to emit it here. */
	if(!glk_data->in_startup)
		g_signal_emit_by_name(glk_data->self, "stopped");
	g_thread_exit(NULL);
}

/* Internal function: check if the Glk program has been interrupted. */
void
check_for_abort(void)
{
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
	g_mutex_lock(&glk_data->abort_lock);
	if(glk_data->abort_signalled)
	{
		g_mutex_unlock(&glk_data->abort_lock);
		abort_glk();
	}
	g_mutex_unlock(&glk_data->abort_lock);
}

/* Internal function: shut down all requests and anything not necessary while
 showing the last displayed configuration of windows. */
void
shutdown_glk_pre(void)
{
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);

	/* Stop any timers */
	glk_request_timer_events(0);
	
	/* Cancel any pending input requests and flush all window buffers */
	winid_t win;
	for(win = glk_window_iterate(NULL, NULL); win; win = glk_window_iterate(win, NULL))
	{
		switch(win->input_request_type)
		{
			case INPUT_REQUEST_CHARACTER:
			case INPUT_REQUEST_CHARACTER_UNICODE:
				glk_cancel_char_event(win);
				break;
			case INPUT_REQUEST_LINE:
			case INPUT_REQUEST_LINE_UNICODE:
				glk_cancel_line_event(win, NULL);
				break;
			case INPUT_REQUEST_NONE:
			default:
				; /* TODO: Handle mouse and hyperlink requests */
		}
		
		flush_window_buffer(win);
	}
	
	/* Close any open resource files */
	if(glk_data->resource_map != NULL) {
		giblorb_destroy_map(glk_data->resource_map);
		glk_data->resource_map = NULL;
		glk_stream_close(glk_data->resource_file, NULL);
	}

	/* Wait for any pending window rearrange */
	ui_message_queue_and_await(ui_message_new(UI_MESSAGE_SYNC_ARRANGE, NULL));
}

/* Internal function: do any Glk-thread cleanup for shutting down the Glk library. */
void
shutdown_glk_post(void)
{
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);

	/* Free all opaque objects; can't iterate normally, because the objects are
	 being removed from the global iteration lists */
	if(glk_data->root_window)
		glk_window_close(glk_data->root_window->data, NULL);
	g_assert(glk_data->root_window == NULL);
	strid_t str;
	while( (str = glk_stream_iterate(NULL, NULL)) )
		glk_stream_close(str, NULL);
	frefid_t fref;
	while( (fref = glk_fileref_iterate(NULL, NULL)) )
		glk_fileref_destroy(fref);
	schanid_t sch;
	while( (sch = glk_schannel_iterate(NULL, NULL)) )
		glk_schannel_destroy(sch);

	/* Shut down the UI message queue */
	ui_message_queue_and_await(ui_message_new(UI_MESSAGE_SHUTDOWN, NULL));

	/* Empty the event queue */
	g_mutex_lock(&glk_data->event_lock);
	g_queue_foreach(glk_data->event_queue, (GFunc)g_free, NULL);
	g_queue_clear(glk_data->event_queue);
	g_mutex_unlock(&glk_data->event_lock);

	/* Reset the abort signaling mechanism */
	g_mutex_lock(&glk_data->abort_lock);
	glk_data->abort_signalled = FALSE;
	g_mutex_unlock(&glk_data->abort_lock);

	/* Unref input queues (they are not destroyed because the main thread stil holds a ref */
	g_async_queue_unref(glk_data->char_input_queue);
	g_async_queue_unref(glk_data->line_input_queue);

	/* Reset other stuff */
	glk_data->interrupt_handler = NULL;
	g_free(glk_data->current_dir);
	glk_data->current_dir = NULL;
	/* Remove the dispatch callbacks. However, if we are running under address
	 * sanitizer, leave them in place - this is an unfortunate hack that allows
	 * Glulxe to work twice in a row without unloading the plugin in between. */
#ifndef CHIMARA_ASAN_HACK
	glk_data->register_obj = NULL;
	glk_data->unregister_obj = NULL;
	glk_data->register_arr = NULL;
	glk_data->unregister_arr = NULL;
#endif
}

static gboolean
emit_waiting_signal(ChimaraGlk *glk)
{
	g_signal_emit_by_name(glk, "waiting");
	return G_SOURCE_REMOVE;
}

static gboolean
emit_stopped_signal(ChimaraGlk *glk)
{
	g_signal_emit_by_name(glk, "stopped");
	return G_SOURCE_REMOVE;
}

void
shutdown_glk_full(void)
{
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);

	shutdown_glk_pre();

	/* Find the biggest text buffer window */
	winid_t win, largewin = NULL;
	glui32 largearea = 0;
	for (win = glk_window_iterate(NULL, NULL); win; win = glk_window_iterate(win, NULL)) {
		if (win->type == wintype_TextBuffer) {
			glui32 w, h;
			if (!largewin) {
				largewin = win;
				glk_window_get_size(largewin, &w, &h);
				largearea = w * h;
			} else {
				glk_window_get_size(win, &w, &h);
				if(w * h > largearea) {
					largewin = win;
					largearea = w * h;
				}
			}
		}
	}
	if (largewin) {
		glk_set_window(largewin);
		glk_set_style(style_Alert);
		glk_put_string("\n");
		glk_put_string(glk_data->final_message);
		glk_put_string("\n");
		flush_window_buffer(largewin);
	}

	/* Wait for a keypress if any text grid or buffer windows are open */
	gboolean should_wait = FALSE;
	g_mutex_lock(&glk_data->shutdown_lock);
	for (win = glk_window_iterate(NULL, NULL); win; win = glk_window_iterate(win, NULL)) {
		if (win->type == wintype_TextGrid || win->type == wintype_TextBuffer) {
			g_signal_handler_unblock(win->widget, win->shutdown_keypress_handler);
			should_wait = TRUE;
		}
	}
	if (should_wait) {
		gdk_threads_add_idle((GSourceFunc)emit_waiting_signal, glk_data->self);
		if (glk_data->interactive)
			g_cond_wait(&glk_data->shutdown_key_pressed, &glk_data->shutdown_lock);
	}
	g_mutex_unlock(&glk_data->shutdown_lock);

	shutdown_glk_post();

	gdk_threads_add_idle((GSourceFunc)emit_stopped_signal, glk_data->self);
}
