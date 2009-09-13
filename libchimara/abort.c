#include "event.h"
#include <glib.h>
#include <gtk/gtk.h>

#include "chimara-glk-private.h"

extern GPrivate *glk_data_key;

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
 * calling <code>#glk_set_interrupt_handler(%NULL)</code>.
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
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	glk_data->interrupt_handler = func;
}

/* Internal function: abort this Glk program, freeing resources and calling the
user's interrupt handler. */
static void
abort_glk()
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	if(glk_data->interrupt_handler)
		(*(glk_data->interrupt_handler))();
	g_signal_emit_by_name(glk_data->self, "stopped");
	g_thread_exit(NULL);
}

/* Internal function: check if the Glk program has been interrupted. */
void
check_for_abort()
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	g_mutex_lock(glk_data->abort_lock);
	if(glk_data->abort_signalled) 
	{
		g_mutex_unlock(glk_data->abort_lock);
		abort_glk();
	}
	g_mutex_unlock(glk_data->abort_lock);
}
