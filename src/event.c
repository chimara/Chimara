#include "event.h"

static GQueue *event_queue = NULL;
static GMutex *event_lock = NULL;
static GCond *event_queue_not_empty = NULL;
static GCond *event_queue_not_full = NULL;

void
events_init()
{
	event_queue = g_queue_new();
	event_lock = g_mutex_new();
	event_queue_not_empty = g_cond_new();
	event_queue_not_full = g_cond_new();
}

static void
event_free(gpointer data, gpointer user_data)
{
	g_free(data);
}

void
events_free()
{
	g_queue_foreach(event_queue, event_free, NULL);
	g_queue_free(event_queue);
	g_mutex_free(event_lock);
	g_cond_free(event_queue_not_empty);
	g_cond_free(event_queue_not_full);
}

void
event_throw(glui32 type, winid_t win, glui32 val1, glui32 val2)
{
	GTimeVal timeout;
	g_get_current_time(&timeout);
	g_time_val_add(&timeout, 3000000); /* 3 Seconds */

	/* Wait for room in the event queue */
	g_mutex_lock(event_lock);
	if( g_queue_get_length(event_queue) >= EVENT_QUEUE_MAX_LENGTH ) {
		if( !g_cond_timed_wait(event_queue_not_full, event_lock, &timeout) ) {
			/* Drop the event */
			g_mutex_unlock(event_lock);
			return;
		}
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

void
glk_select(event_t *event)
{
	event_t *retrieved_event;

	g_return_if_fail(event != NULL);

	g_mutex_lock(event_lock);

	/* Wait for an event */
	if( g_queue_is_empty(event_queue) ) {
		g_cond_wait(event_queue_not_empty, event_lock);
	}

	retrieved_event = g_queue_pop_tail(event_queue);
	g_return_if_fail(retrieved_event != NULL);

	event->type = retrieved_event->type;
	event->win = retrieved_event->win;
	event->val1 = retrieved_event->val1;
	event->val2 = retrieved_event->val2;

	g_free(retrieved_event);

	/* Signal that the event queue is no longer full */
	g_cond_signal(event_queue_not_full);

	g_mutex_unlock(event_lock);

	/* Implementation defined events */
	switch(event->type) {
		case EVENT_TYPE_QUIT:
			g_thread_exit(NULL);
	}
}
