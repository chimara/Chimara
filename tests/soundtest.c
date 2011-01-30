#include <libchimara/glk.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

void
glk_main(void)
{
	if(!glk_gestalt(gestalt_Sound, 0)) {
		fprintf(stderr, "Sound not supported.\n");
		return;
	}
	if(!glk_gestalt(gestalt_SoundVolume, 0)) {
		fprintf(stderr, "Sound volume not supported.\n");
		return;
	}
	if(!glk_gestalt(gestalt_SoundNotify, 0)) {
		fprintf(stderr, "Sound notification not supported.\n");
		return;
	}
	
	schanid_t sc = glk_schannel_create(0);
	if(!sc) {
		fprintf(stderr, "Could not create sound channel.\n");
		return;
	}

	/* Open the main window. */
    winid_t mainwin = glk_window_open(0, 0, 0, wintype_TextBuffer, 1);
    if (!mainwin) {
        /* It's possible that the main window failed to open. There's
            nothing we can do without it, so exit. */
        return;
    }
	glk_set_window(mainwin);
	glk_put_string("Copy a sound file to the current directory and rename it "
	    "to SND3. Supported formats: AIFF, OGG, MOD, S3M, IT, XM. Type 'play' "
	    "to play it.\n");

	char buffer[1024];
	int len;
	int finish = 0;
	int repeat = 1;

	event_t ev;
	while(!finish) {
		glk_put_string("\nprompt> ");
		glk_request_line_event(mainwin, buffer, 1024, 0);
		glk_select(&ev);
		printf("Received event:\n");
		printf("Type: %d\n", ev.type);
		printf("Win: ");
		if(ev.win)
			printf( "%d\n", glk_window_get_rock(ev.win) );
		else
			printf("NULL\n");
		printf("Var1: %d\n", ev.val1);
		printf("Var2: %d\n", ev.val2);
		switch(ev.type) {
			case evtype_LineInput:
				// Null-terminate string
				len = ev.val1;
				buffer[len] = '\0';

				if(strcmp(buffer, "quit") == 0) {
					glk_put_string("That's all, folks.\n");
					finish = 1;
				} else if(strcmp(buffer, "play") == 0) {
					glk_put_string("Playing sound.\n");
					if(!glk_schannel_play_ext(sc, 3, repeat, 1)) {
						fprintf(stderr, "Could not start sound channel.\n");
						finish = 1;
					}
				} else if(strcmp(buffer, "stop") == 0) {
					glk_put_string("Stopping sound.\n");
					glk_schannel_stop(sc);
				} else if(strcmp(buffer, "repeat") == 0) {
					glk_put_string("Setting repeat to ");
					if(repeat == 1) {
						glk_put_string("TWICE.\n");
						repeat = 2;
					} else if(repeat == 2) {
						glk_put_string("INFINITE.\n");
						repeat = -1;
					} else if(repeat == -1) {
						glk_put_string("DON'T PLAY.\n");
						repeat = 0;
					} else if(repeat == 0) {
						glk_put_string("ONCE.\n");
						repeat = 1;
					}
				} else if(strcmp(buffer, "help") == 0) {
					glk_put_string("Type PLAY or REPEAT or STOP or QUIT.\n");
				}
				break;
			case evtype_SoundNotify:
				glk_cancel_line_event(mainwin, NULL);
				glk_put_string("\nGot sound notify event!\n");
				break;
			default:
				;
		}
	}

	glk_schannel_stop(sc);
	glk_schannel_destroy(sc);
}
