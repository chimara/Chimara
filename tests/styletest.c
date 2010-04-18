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
	glk_put_string("int finish_text_grid_line_input(winid_t win, gboolean\n		 11 static void cancel_old_input_request(winid_t win);\n	 12 \n		  13 /* Internal function: code common to both flavors of char ev\n				 14 void\n				  15 request_char_event_common(winid_t win, gboolean unicode)\n				   16 {\n				    17     VALID_WINDOW(win, return);\n					 18     g_return_if_fail(win->type != wintype_TextBuffer || win-\n					  19 \n					   20     cancel_old_input_request(win);\n					    21 \n						 22     flush_window_buffer(win);\n						  23 \n						   24     ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key\n						    25 \n							 26     win->input_request_type = unicode? INPUT_REQUEST_CHARACT\n							  27     g_signal_handler_unblock( win->widget, win->char_input_k\n							   28 \n							    29     gdk_threads_enter();\n								 30 \n								  31     /*\n								   32     if(win->type == wintype_TextBuffer)\n								    33     {\n									 34         GtkTextBuffer *buffer = gtk_text_view_get_buffer( GT\n									  35         GtkTextIter iter;\n									   36         gtk_text_buffer_get_end_iter(buffer, &iter);\n									    37         gtk_text_buffer_place_cursor(buffer, &iter);\n										 38         gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(win\n										  39         // Why doesn't this always work?? \n										   40     } */\n		   41 \n		    42     gtk_widget_grab_focus( GTK_WIDGET(win->widget) );\n	  43     gdk_threads_leave();\n	   44 \n					    47 }\n						 48 \n						  49 /**\n						   50  * glk_request_char_event:\n						    51  * @win: A window to request char events from.\n							 52  *\n							  53  * Request input of a Latin-1 character or special key. A wi\n							   54  * requests for both character and line input at the same ti\n							    55  * requests for character input of both types (Latin-1 and U\n								 56  * illegal to call glk_request_char_event() if the window al\n								  57  * request for either character or\n");
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
