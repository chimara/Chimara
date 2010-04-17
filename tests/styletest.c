#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <libchimara/glk.h>

void print_help();
void do_style_test();
void do_link_test();
void do_mouse_test();

winid_t mainwin;
winid_t statuswin;

void glk_main(void)
{
	char stringbuffer[128];
    event_t ev;
    mainwin = glk_window_open(0, 0, 0, wintype_TextBuffer, 0);
    if(!mainwin)
        return;

	statuswin = glk_window_open(mainwin, winmethod_Above | winmethod_Fixed, 3, wintype_TextGrid, 1);
    
    glk_set_window(mainwin);
    
    char *buffer = calloc(256, sizeof(char));
    assert(buffer);
    
    glk_put_string("Welcome to the style test\n");

    glk_request_line_event(mainwin, buffer, 255, 0);
    while(strncmp(buffer, "quit", 4)) 
    {
        glk_select(&ev);
        if(ev.type == evtype_LineInput)
        {
			if( !strncmp(buffer, "help", 4) ) {
				print_help();
			}
			else if( !strncmp(buffer, "style", 4) ) {
				do_style_test();
			}
			else if( !strncmp(buffer, "link", 4) ) {
				do_link_test();
			}
			else if( !strncmp(buffer, "mouse", 4) ) {
				do_mouse_test();
			}
			else {
				glk_put_string("Huh?\n");
			}
    		glk_request_line_event(mainwin, buffer, 255, 0);
        }
		else if(ev.type == evtype_Hyperlink)
		{
			glk_cancel_line_event(mainwin, NULL);
			snprintf(stringbuffer, 128, "Link %d was clicked\n", ev.val1);
			glk_put_string(stringbuffer);
    		glk_request_line_event(mainwin, buffer, 255, 0);
		}
		else if(ev.type == evtype_MouseInput)
		{
			glk_cancel_line_event(mainwin, NULL);
			snprintf(stringbuffer, 128, "Mouse click: x=%d, y=%d\n", ev.val1, ev.val2);
			glk_put_string(stringbuffer);
    		glk_request_line_event(mainwin, buffer, 255, 0);
		}
    }

    glk_cancel_line_event(mainwin, NULL);
    glk_window_clear(mainwin);
    free(buffer);
}

void
do_style_test() {
	glk_set_style(style_Normal);
	glk_put_string("Normal\n");

	glk_set_style(style_Emphasized);
	glk_put_string("Emphasized\n");

	glk_set_style(style_Preformatted);
	glk_put_string("Preformatted\n");

	glk_set_style(style_Header);
	glk_put_string("Header\n");

	glk_set_style(style_Subheader);
	glk_put_string("Subheader\n");

	glk_set_style(style_Alert);
	glk_put_string("Alert\n");

	glk_set_style(style_Note);
	glk_put_string("Note\n");

	glk_set_style(style_BlockQuote);
	glk_put_string("BlockQuote\n");

	glk_set_style(style_Input);
	glk_put_string("Input\n");

	glk_set_style(style_User1);
	glk_put_string("User1\n");

	glk_set_style(style_User2);
	glk_put_string("User2\n");

	glk_set_style(style_Normal);
}

void
do_link_test() {
    glk_set_window(mainwin);
	glk_set_hyperlink(1);
	glk_put_string("This is link 1\n");
	glk_set_hyperlink(2);
	glk_put_string("This is link 2\n");
	glk_set_hyperlink(0);

    glk_set_window(statuswin);
	glk_set_hyperlink(3);
	glk_window_move_cursor(statuswin, 0, 0);
	glk_put_string("This is link 3\n");
	glk_set_hyperlink(4);
	glk_window_move_cursor(statuswin, 0, 1);
	glk_put_string("This is link 4\n");
	glk_set_hyperlink(0);

	glk_request_hyperlink_event(mainwin);
	glk_request_hyperlink_event(statuswin);

    glk_set_window(mainwin);
}

void
do_mouse_test() {
    glk_set_window(statuswin);
	glk_window_move_cursor(statuswin, 0, 0);
	glk_put_string("Click me......\n");
	glk_request_mouse_event(statuswin);
	glk_set_window(mainwin);
}

void
print_help() {
	glk_put_string("The following commands are supported:\n - help (this help text)\n - style (perform style test)\n - link (perform hyperlink test)\n - mouse (perform mouse test)\n - quit (quit the program)\n");
}
