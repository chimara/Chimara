#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <libchimara/glk.h>
#include <libchimara/garglk.h>

void glk_main(void)
{
    event_t ev;
    winid_t mainwin = glk_window_open(0, 0, 0, wintype_TextGrid, 0);
    if(!mainwin)
        return;
    
    glk_set_window(mainwin);
    glui32 width, height, x, y;
    glk_window_get_size(mainwin, &width, &height);
    if(height < 4 || width < 22)
    {
    	glk_put_string("Window not big enough");
    	glk_exit();
    }

    char *buffer = calloc(256, sizeof(char));
    assert(buffer);
    
	garglk_set_reversevideo(1);
	for(y=0; y<height; y++) {
		snprintf(buffer, 256, "%02d\n", y);
		glk_put_string(buffer);
	}
	garglk_set_reversevideo(0);

	glk_set_style(style_Emphasized);
	for(y=0; y<height; y++) {
		snprintf(buffer, 256, "%02d", y);
		glk_window_move_cursor(mainwin, (y+1)*width-2, 0);
		glk_put_string(buffer);
	}
	glk_set_style(style_Normal);

    glk_window_move_cursor(mainwin, 2, 0);
	glk_put_string("+");
    glk_window_move_cursor(mainwin, width-3, 0);
	glk_put_string("+");
    glk_window_move_cursor(mainwin, 2, height-1);
	glk_put_string("+");
    glk_window_move_cursor(mainwin, width-3, height-1);
	glk_put_string("+");

    x = width / 2 - 10;
    y = height / 2;
    
    glk_window_move_cursor(mainwin, x, y - 1);
    glk_put_string("Enter text, or 'quit'");
    glk_window_move_cursor(mainwin, x, y);
    glk_request_line_event(mainwin, buffer, 21, 0);
    while(strncmp(buffer, "quit", 4)) 
    {
        glk_select(&ev);
        if(ev.type == evtype_LineInput)
        {
        	glk_window_move_cursor(mainwin, x, y + 1);
        	glk_put_string("                     ");
        	glk_window_move_cursor(mainwin, x, y + 1);
        	glk_put_buffer(buffer, ev.val1);
        	glk_window_move_cursor(mainwin, x, y);
    		glk_request_line_event(mainwin, buffer, 21, 0);
        }
    }
    glk_cancel_line_event(mainwin, NULL);
    glk_window_clear(mainwin);
    free(buffer);
}
