#include <glib.h>
#include <gtk/gtk.h>

#include "glk.h"

/**
 * glk_exit:
 * 
 * Shuts down the Glk program. This function does not return.
 *
 * If you print some text to a window and then shut down your program, you can
 * assume that the player will be able to read it.
 *
 * <note><para>
 *  You should only shut down your program with glk_exit() or by returning from
 *  your glk_main() function. If you call the ANSI <function>exit()</function> 
 *  function, bad things may happen. This Glk library is designed for multiple 
 *  sessions, for example, and you would be cutting off all the sessions instead
 *  of just yours. You would also prevent final text from being visible to the 
 *  player.
 * </para></note>
 */
void
glk_exit(void)
{
	g_thread_exit(NULL);
}

