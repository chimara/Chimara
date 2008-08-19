#include <glib.h>
#include <gtk/gtk.h>

#include "glk.h"

/**
 * glk_exit:
 * 
 * End the Glk program. As far as the client program is concerned, this
 * function does not return.
 */
void
glk_exit(void)
{
	g_thread_exit(NULL);
}

/*
void
glk_select(event_t *event)
{
	gtk_main_iteration();
}
*/


