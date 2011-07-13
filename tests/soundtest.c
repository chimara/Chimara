#include <libchimara/glk.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define NUM_CHANNELS 2

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
	
	schanid_t sc[NUM_CHANNELS];
	int count;
	for(count = 0; count < NUM_CHANNELS; count++) {
		sc[count] = glk_schannel_create(count);
		if(!sc[count]) {
			fprintf(stderr, "Could not create sound channel number %d.\n", count);
			return;
		}
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
	    "to play it.\n\n"
		"If you want to test multi-sound playing, copy another sound file and "
		"rename it to SND4 as well. You can't stop it, so make it a short "
		"sound effect.\n");

	char buffer[1024];
	int len;
	int finish = 0;
	int repeat = 1;
	int ramp = 0;

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
					if(!glk_schannel_play_ext(sc[0], 3, repeat, 1)) {
						fprintf(stderr, "Could not start sound channel.\n");
						finish = 1;
					}
				} else if(strcmp(buffer, "stop") == 0) {
					glk_put_string("Stopping sound.\n");
					glk_schannel_stop(sc[0]);
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
				} else if(strcmp(buffer, "pause") == 0) {
					glk_put_string("Pausing channel.\n");
					glk_schannel_pause(sc[0]);
				} else if(strcmp(buffer, "unpause") == 0) {
					glk_put_string("Unpausing channel.\n");
					glk_schannel_unpause(sc[0]);
				} else if(strcmp(buffer, "ramp") == 0) {
					glk_put_string("Ramping volume to ");
					if(ramp == 0) {
						glk_put_string("HALF.\n");
						glk_schannel_set_volume_ext(sc[0], 0x8000, 3000, 42);
						ramp = 1;
					} else if(ramp == 1) {
						glk_put_string("FULL.\n");
						glk_schannel_set_volume_ext(sc[0], 0x10000, 3000, 69);
						ramp = 0;
					}
				} else if(strcmp(buffer, "multi") == 0) {
					glk_put_string("Playing two sounds. (These will not repeat.)\n");
					glui32 sounds[NUM_CHANNELS] = { 3, 4 };
					if(glk_schannel_play_multi(sc, NUM_CHANNELS, sounds, NUM_CHANNELS, 1) < 2) {
						fprintf(stderr, "Tried to start %d sounds, but not all were successful.", NUM_CHANNELS);
						finish = 1;
					}
				} else if(strcmp(buffer, "help") == 0) {
					glk_put_string("Type PLAY or MULTI or REPEAT or PAUSE or UNPAUSE or RAMP or STOP or QUIT.\n");
				}
				break;
			case evtype_SoundNotify:
				glk_cancel_line_event(mainwin, NULL);
				glk_put_string("\nGot sound notify event!\n");
				break;
			case evtype_VolumeNotify:
				glk_cancel_line_event(mainwin, NULL);
				glk_put_string("\nGot volume notify event!\n");
				break;
			default:
				;
		}
	}

	for(count = 0; count < NUM_CHANNELS; count++) {
		glk_schannel_stop(sc[count]);
		glk_schannel_destroy(sc[count]);
	}
}
