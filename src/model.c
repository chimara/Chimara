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
    
    unsigned char buffer[256];
    int i;
    for(i = 0; i < 256; i++)
    	buffer[i] = glk_char_to_upper(i);
    
    glk_put_string("Philip en Marijn zijn vet goed.\n");
    glk_put_buffer((gchar *)buffer, 256);
    
    frefid_t f = 
    	glk_fileref_create_by_prompt(fileusage_TextMode, filemode_Write, 0);
    if(f) {
		if( glk_fileref_does_file_exist(f) )
			glk_put_string("\n\nFile exists!\n");
		else
			glk_put_string("\n\nFile does not exist!\n");
		glk_fileref_destroy(f);
	} else
		glk_put_string("\n\nCancel was clicked!\n");
	
	/* Bye bye */
	glk_exit();
}
