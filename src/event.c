#include "event.h"
#include "glk.h"
#include <string.h>

#include "chimara-glk-private.h"

extern ChimaraGlkPrivate *glk_data;

#define EVENT_TIMEOUT_MICROSECONDS (3000000)

/* Internal function: push an event onto the event queue. If the event queue is
full, wait for max three seconds and then drop the event. If the event queue is
NULL, i.e. freed, then fail silently. */
void
event_throw(glui32 type, winid_t win, glui32 val1, glui32 val2)
{
	if(!glk_data->event_queue)
		return;

	GTimeVal timeout;
	g_get_current_time(&timeout);
	g_time_val_add(&timeout, EVENT_TIMEOUT_MICROSECONDS);

	g_mutex_lock(glk_data->event_lock);

	/* Wait for room in the event queue */
	while( g_queue_get_length(glk_data->event_queue) >= EVENT_QUEUE_MAX_LENGTH )
		if( !g_cond_timed_wait(glk_data->event_queue_not_full, glk_data->event_lock, &timeout) ) 
		{
			/* Drop the event after 3 seconds */
			g_mutex_unlock(glk_data->event_lock);
			return;
		}

	event_t *event = g_new0(event_t, 1);
	event->type = type;
	event->win = win;
	event->val1 = val1;
	event->val2 = val2;
	g_queue_push_head(glk_data->event_queue, event);

	/* Signal that there is an event */
	g_cond_signal(glk_data->event_queue_not_empty);

	g_mutex_unlock(glk_data->event_lock);
}

/**
 * glk_select:
 * @event: Pointer to an #event_t.
 *
 * Causes the program to wait for an event, and then store it in the structure
 * pointed to by @event. Unlike most Glk functions that take pointers, the
 * argument of glk_select() may not be %NULL.
 *
 * Most of the time, you only get the events that you request. However, there
 * are some events which can arrive at any time. This is why you must always
 * call glk_select() in a loop, and continue the loop until you get the event
 * you really want.
 */
void
glk_select(event_t *event)
{
	g_return_if_fail(event != NULL);

	g_mutex_lock(glk_data->event_lock);

	/* Wait for an event */
	while( g_queue_is_empty(glk_data->event_queue) )
		g_cond_wait(glk_data->event_queue_not_empty, glk_data->event_lock);

	event_t *retrieved_event = g_queue_pop_tail(glk_data->event_queue);
	if(retrieved_event == NULL)
	{
		g_mutex_unlock(glk_data->event_lock);
		g_warning("%s: Retrieved NULL event from non-empty event queue", __func__);
		return;
	}
	memcpy(event, retrieved_event, sizeof(event_t));
	g_free(retrieved_event);

	/* Signal that the event queue is no longer full */
	g_cond_signal(glk_data->event_queue_not_full);

	g_mutex_unlock(glk_data->event_lock);
	
	/* Check for interrupt */
	glk_tick();
	
	/* If an abort event was generated, the thread should have exited by now */
	g_assert(event->type != evtype_Abort);
}

