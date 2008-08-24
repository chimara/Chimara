#include "event.h"
#include <string.h>

static GQueue *event_queue = NULL;
static GMutex *event_lock = NULL;
static GCond *event_queue_not_empty = NULL;
static GCond *event_queue_not_full = NULL;

/* Internal function: initialize the event_queue, locking and signalling 
objects. */
void
events_init()
{
	event_queue = g_queue_new();
	event_lock = g_mutex_new();
	event_queue_not_empty = g_cond_new();
	event_queue_not_full = g_cond_new();
}

/* Internal function: free the event queue and all the objects allocated in
events_init(). */
void
events_free()
{
	g_queue_foreach(event_queue, (GFunc)g_free, NULL);
	g_queue_free(event_queue);
	g_mutex_free(event_lock);
	g_cond_free(event_queue_not_empty);
	g_cond_free(event_queue_not_full);
}

/* Internal function: push an event onto the event queue. If the event queue is
full, wait for max three seconds and then drop the event. */
void
event_throw(glui32 type, winid_t win, glui32 val1, glui32 val2)
{
	GTimeVal timeout;
	g_get_current_time(&timeout);
	g_time_val_add(&timeout, 3000000); /* 3 Seconds */

	g_mutex_lock(event_lock);

	/* Wait for room in the event queue */
	if( g_queue_get_length(event_queue) >= EVENT_QUEUE_MAX_LENGTH )
		if( !g_cond_timed_wait(event_queue_not_full, event_lock, &timeout) ) 
		{
			/* Drop the event after 3 seconds */
			g_mutex_unlock(event_lock);
			return;
		}

	event_t *event = g_new0(event_t, 1);
	event->type = type;
	event->win = win;
	event->val1 = val1;
	event->val2 = val2;
	g_queue_push_head(event_queue, event);

	/* Signal that there is an event */
	g_cond_signal(event_queue_not_empty);

	g_mutex_unlock(event_lock);
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

	g_mutex_lock(event_lock);

	/* Wait for an event */
	if( g_queue_is_empty(event_queue) )
		g_cond_wait(event_queue_not_empty, event_lock);

	event_t *retrieved_event = g_queue_pop_tail(event_queue);
	if(retrieved_event == NULL)
	{
		g_mutex_unlock(event_lock);
		g_warning("%s: Retrieved NULL event from non-empty event queue", __func__);
		return;
	}
	memcpy(event, retrieved_event, sizeof(event_t));
	g_free(retrieved_event);

	/* Signal that the event queue is no longer full */
	g_cond_signal(event_queue_not_full);

	g_mutex_unlock(event_lock);

	/* Implementation-defined events */
	switch(event->type) {
		case EVENT_TYPE_QUIT:
			g_thread_exit(NULL);
	}
}
