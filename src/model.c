#include "glk.h"

static winid_t mainwin = NULL;

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
    
    unsigned char buffer[255];
    int i;
    for(i = 0; i < 255; i++)
    	buffer[i] = glk_char_to_upper(i + 1);
    
    glk_put_string("Philip en Marijn zijn vet goed.\n");
    glk_put_string(buffer);

	/* Bye bye */
	glk_exit();
}
