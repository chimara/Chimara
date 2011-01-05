#include <libchimara/glk.h>
#include <stdio.h>

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
	
	schanid_t sc = glk_schannel_create(0);
	if(!sc) {
		fprintf(stderr, "Could not create sound channel.\n");
		return;
	}
	
	glk_schannel_set_volume(sc, 0x10000);
	glk_schannel_set_volume(sc, 0x08000);
	glk_schannel_set_volume(sc, 0x04000);
	glk_schannel_set_volume(sc, 0x00000);
	glk_schannel_set_volume(sc, 0xA0000); /* max supported volume */
	glk_schannel_set_volume(sc, 0xB0000); /* should be coerced */
	glk_schannel_set_volume(sc, 0x10000);
	
	glk_schannel_destroy(sc);
}
