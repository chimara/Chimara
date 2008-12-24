#include <gtk/gtk.h>

#include "glk.h"
#include "abort.h"
#include "chimara-glk.h"
#include "chimara-glk-private.h"

ChimaraGlkPrivate *glk_data = NULL;

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
    g_signal_emit_by_name(glk_data->self, "stopped");
    glk_data = NULL;
	g_thread_exit(NULL);
}

/**
 * glk_tick:
 *
 * Many platforms have some annoying thing that has to be done every so often,
 * or the gnurrs come from the voodvork out and eat your computer.
 * 
 * Well, not really. But you should call glk_tick() every so often, just in
 * case. It may be necessary to yield time to other applications in a
 * cooperative-multitasking OS, or to check for player interrupts in an infinite
 * loop.
 * 
 * This call is fast; in fact, on average, it does nothing at all. So you can
 * call it often. (In a virtual machine interpreter, once per opcode is
 * appropriate. In a program with lots of computation, pick a comparable rate.)
 * 
 * glk_tick() does not try to update the screen, or check for player input, or
 * any other interface task. For that, you should call glk_select() or 
 * glk_select_poll().
 * 
 * Basically, you must ensure there's some fixed upper bound on the amount of
 * computation that can occur before a glk_tick() (or glk_select()) occurs. In a
 * VM interpreter, where the VM code might contain an infinite loop, this is
 * critical. In a C program, you can often eyeball it.
 */
void
glk_tick()
{
	check_for_abort();
	
	/* Do one iteration of the main loop if there are any events */
	gdk_threads_enter();
	if(gtk_events_pending())
		gtk_main_iteration();
	gdk_threads_leave();
}
