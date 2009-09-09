#include <stdio.h>
#include <libchimara/glk.h>

static winid_t mainwin = NULL;

void sayit(void)
{
	fprintf(stderr, "I'm the interrupt handler!\n");
}

void glk_main(void)
{
    /* Open the main window. */
    mainwin = glk_window_open(0, 0, 0, wintype_TextBuffer, 1);
    if (!mainwin) {
        /* It's possible that the main window failed to open. There's
            nothing we can do without it, so exit. */
        return; 
    }
	
    glui32 buffer[1024];
    int i;
    for(i = 0; i < 512; i++) {
    	buffer[i * 2] = i + 33;
		buffer[i * 2 + 1] = 32;
	}
    
/*    frefid_t f = glk_fileref_create_temp(fileusage_BinaryMode, 0);
    if(f) 
    {
		strid_t s = glk_stream_open_file(f, filemode_ReadWrite, 0);*/
		glui32 membuf[512];
		strid_t s = glk_stream_open_memory_uni(membuf, 512, filemode_ReadWrite, 0);
		glk_stream_set_current(s);
		
		glk_put_char_uni('X');
		glk_put_string("Philip en Marijn zijn vet goed.\n");
		glk_put_buffer_uni(buffer, 1024);

		glk_stream_set_position(s, 0, seekmode_Start);
		glk_set_window(mainwin);
		glk_put_char_uni( glk_get_char_stream_uni(s) );
		glk_put_char('\n');
		printf("Line read: %d\n", glk_get_line_stream_uni(s, buffer, 1024) );
		printf("string[5] = %X\n", buffer[5]);
		glk_put_string_uni(buffer);
		int count = glk_get_buffer_stream_uni(s, buffer, 1024);
		printf("Buffer read: %d\n", count);
		glk_put_string("\n---SOME CHARACTERS---\n");
		glk_put_buffer_uni(buffer, count);
		glk_put_string("\n---THE SAME CHARACTERS IN UPPERCASE---\n");
		int newcount = glk_buffer_to_upper_case_uni(buffer, 1024, 1024);
		glk_put_buffer_uni(buffer, newcount);
		
		stream_result_t result;
		glk_stream_close(s, &result);
		
		fprintf(stderr, "Read count: %d\nWrite count: %d\n", result.readcount, result.writecount);
/*		glk_fileref_destroy(f);
	}*/

	glk_set_interrupt_handler(&sayit);

	event_t ev;
	while(1) {
		glk_put_string("\nprompt> ");
		glk_request_line_event_uni(mainwin, buffer, 1024, 0);
		glk_select(&ev);
		switch(ev.type) {
			default:
				printf("Received event:\n");
				printf("Type: %d\n", ev.type);
				printf("Win: %d\n", glk_window_get_rock(ev.win) );
				printf("Var1: %d\n", ev.val1);
				printf("Var2: %d\n", ev.val2);
		}
	}
	
	/* Bye bye */
	glk_exit();
}
