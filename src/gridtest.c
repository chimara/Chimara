#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "glk.h"

void glk_main(void)
{
    event_t ev;
    winid_t mainwin = glk_window_open(0, 0, 0, wintype_TextGrid, 0);
    if(!mainwin)
        return;
    
    glk_set_window(mainwin);
    glk_put_string("Philip en Marijn zijn vet goed.\n");
    glk_put_string("A veeeeeeeeeeeeeeeeeeeeeeeeeeeery looooooooooooooooooooooooong striiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiing.\n");
    
    int count;
    for(count = 0; count < 30; count++)
        glk_put_string("I want to write past the end of this text buffer! ");
    
    glui32 width, height;
    glk_window_get_size(mainwin, &width, &height);
    fprintf(stderr, "\nWidth: %d\nHeight: %d\nPress a key in the window, not in the terminal.\n", width, height);
    glk_request_char_event(mainwin);
    while(1) {
        glk_select(&ev);
        if(ev.type == evtype_CharInput)
            break;
    }
    
    glk_window_move_cursor(mainwin, 15, 15);
    glk_put_string(". . ");
    glk_window_move_cursor(mainwin, 15, 16);
    glk_put_string(" . .");
    glk_window_move_cursor(mainwin, 15, 17);
    glk_put_string(". . ");
    glk_window_move_cursor(mainwin, 15, 18);
    glk_put_string(" . .");
    fprintf(stderr, "Cursor location test.\nPress another key.\n");
    glk_request_char_event(mainwin);
    while(1) {
        glk_select(&ev);
        if(ev.type == evtype_CharInput)
            break;
    }
    
    char *buffer = calloc(256, sizeof(char));
    assert(buffer);
    
    fprintf(stderr, "Line input field until end of line\n");
    glk_window_move_cursor(mainwin, 10, 20);
    glk_request_line_event(mainwin, buffer, 256, 0);
    while(1) {
        glk_select(&ev);
        if(ev.type == evtype_LineInput)
            break;
    }
    
    fprintf(stderr, "Now edit your previous line input\n");
    glk_window_move_cursor(mainwin, 10, 22);
    glk_request_line_event(mainwin, buffer, 256, strlen(buffer));
    while(1) {
        glk_select(&ev);
        if(ev.type == evtype_LineInput)
            break;
    }
    
	char *text = calloc(ev.val1 + 1, sizeof(char));
	assert(text);
	strncpy(text, buffer, ev.val1);
	text[ev.val1] = '\0';
    fprintf(stderr, "Your string was: '%s'.\nPress another key to clear the window and exit.\n", text);
	free(text);
    glk_request_char_event(mainwin);
    while(1) {
        glk_select(&ev);
        if(ev.type == evtype_CharInput)
            break;
    }
    
    glk_window_clear(mainwin);
    free(buffer);
}
