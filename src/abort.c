#include "event.h"
#include <glib.h>
#include <gtk/gtk.h>

static GMutex *abort_lock = NULL;
static gboolean abort_signalled = FALSE;
static void (*interrupt_handler)(void) = NULL;

/* Internal function: initialize the interrupt handling system. */
void
interrupt_init()
{
	abort_lock = g_mutex_new();
}

/* Internal function: free the resources allocated in interrupt_init(). */
void
interrupt_free()
{
	g_mutex_lock(abort_lock);
	/* Make sure no other thread is busy with this */
	g_mutex_unlock(abort_lock);
	g_mutex_free(abort_lock);
	abort_lock = NULL;
}

/**
 * glk_set_interrupt_handler:
 * @func: A pointer to a function which takes no argument and returns no result.
 *
 * Specifies an interrupt handler function for cleaning up critical resources.
 * If Glk receives an interrupt, and you have set an interrupt handler, your
 * handler will be called, before the process is shut down.
 * 
 * Initially there is no interrupt handler. You can reset to not having any by
 * calling glk_set_interrupt_handler(%NULL).
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
	interrupt_handler = func;
}

/* Internal function: Free all Glk resources. */
void
cleanup()
{
	events_free();
	interrupt_free();
}

/* Internal function: abort this Glk program, freeing resources and calling the
user's interrupt handler. */
void
abort_glk()
{
	if(interrupt_handler)
		(*interrupt_handler)();
	cleanup();
	g_thread_exit(NULL);
}

/* Internal function: Signal this Glk thread to abort. Does nothing if the abort
mutex has already been freed. (That means the thread already ended.) */
void
signal_abort()
{
	if(abort_lock) {
		g_mutex_lock(abort_lock);
		abort_signalled = TRUE;
		g_mutex_unlock(abort_lock);
		/* Stop blocking on the event queue condition */
		event_throw(evtype_Abort, NULL, 0, 0);
	}
}

/* Internal function: check if the Glk program has been interrupted. */
void
check_for_abort()
{
	g_mutex_lock(abort_lock);
	if(abort_signalled) 
	{
		g_mutex_unlock(abort_lock);
		abort_glk();
	}
	g_mutex_unlock(abort_lock);
}


