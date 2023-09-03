#include <glib.h>

#include "abort.h"

G_GNUC_INTERNAL GPrivate glk_data_key = G_PRIVATE_INIT(NULL);

/**
 * glk_exit:
 * 
 * If you want to shut down your program in the middle of your `glk_main()`
 * function, you can call glk_exit().
 *
 * This function does not return.
 *
 * If you print some text to a window and then shut down your program, you can
 * assume that the player will be able to read it. Most likely the Glk library
 * will give a “`Hit any key to exit`” prompt.
 * (There are other possiblities, however.
 * A terminal-window version of Glk might simply exit and leave the last screen
 * state visible in the terminal window.)
 *
 * <note><para>
 * You should only shut down your program with glk_exit() or by returning from
 * your <function>glk_main()</function> function. If you call the ANSI 
 * <function>exit()</function> function, bad things may happen. Some versions of
 * the Glk library may be designed for multiple sessions, for example, and you
 * would be cutting off all the sessions instead of just yours. You would 
 * probably also prevent final text from being visible to the player.
 * </para></note>
 *
 * > # Chimara #
 * > If there are any windows open at the time glk_exit() is called, then
 * > Chimara will leave them open.
 * > This way, the final text remains visible.
 * > Note that bad things most definitely <emphasis>will</emphasis> happen if
 * > you use the ANSI `exit()`.
 */
void
glk_exit(void)
{
	shutdown_glk_full();
	g_thread_exit(NULL);
}

/**
 * glk_tick:
 *
 * Carries out platform-dependent actions such as yielding time to the operating
 * system and checking for interrupts. glk_tick() should be called every so 
 * often when there is a long interval between calls of glk_select() or 
 * glk_select_poll(). This call is fast; in fact, on average, it does nothing at
 * all. So you can call it often.
 *
 * <note><para>
 *   In a virtual machine interpreter, once per opcode is appropriate. A more
 *   parsimonious approach would be once per branch and function call opcode;
 *   this guarantees it will be called inside loops. In a program with lots of
 *   computation, pick a comparable rate.
 * </para></note>
 * 
 * glk_tick() does not try to update the screen, or check for player input, or
 * any other interface task. For that, you should call glk_select() or 
 * glk_select_poll().
 * See [Events][chimara-Events].
 *
 * <note>
 *   <para>Captious critics have pointed out that in the sample program
 *   <filename>model.c</filename>, I do not call glk_tick() at all. This is
 *   because <filename>model.c</filename> has no heavy loops. It does a bit of
 *   work for each command, and then cycles back to the top of the event loop.
 *   The glk_select() call, of course, blocks waiting for input, so it does all
 *   the yielding and interrupt-checking one could imagine.
 *   </para>
 *   <para>Basically, you must ensure there's some fixed upper bound on the
 *   amount of computation that can occur before a glk_tick() (or glk_select())
 *   occurs. In a VM interpreter, where the VM code might contain an infinite
 *   loop, this is critical. In a C program, you can often eyeball it.
 *   </para>
 *   <para>But the next version of <filename>model.c</filename> will have a
 *   glk_tick() in the ornate printing loop of 
 *   <function>verb_yada&lpar;&rpar;</function>. Just to make the point.
 *   </para>
 * </note>
 */
void
glk_tick()
{
	check_for_abort();
	g_thread_yield();
}

