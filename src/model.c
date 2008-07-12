#include "glk.h"

/* model.c: Model program for Glk API, version 0.5.
    Designed by Andrew Plotkin <erkyrath@eblong.com>
    http://www.eblong.com/zarf/glk/index.html
    This program is in the public domain.
*/

/* This is a simple model of a text adventure which uses the Glk API.
    It shows how to input a line of text, display results, maintain a
    status window, write to a transcript file, and so on. */

/* This is the cleanest possible form of a Glk program. It includes only
    "glk.h", and doesn't call any functions outside Glk at all. We even
    define our own str_eq() and str_len(), rather than relying on the
    standard libraries. */

/* We also define our own TRUE and FALSE and NULL. */
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif

static winid_t mainwin = NULL;

/* Forward declarations */
void glk_main(void);

/* The glk_main() function is called by the Glk system; it's the main entry
    point for your program. */
void glk_main(void)
{
    /* Open the main window. */
    mainwin = glk_window_open(0, 0, 0, wintype_TextBuffer, 1);
    if (!mainwin) {
        /* It's possible that the main window failed to open. There's
            nothing we can do without it, so exit. */
        return; 
    }
    
    /* Set the current output stream to print to it. */
    glk_set_window(mainwin);
    
    glk_put_string("Philip en Marijn zijn vet goed.\n");

	/* Bye bye */
	glk_exit();
}
