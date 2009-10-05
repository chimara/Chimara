#include <glib.h>
#include <libchimara/glk.h>

schanid_t 
glk_schannel_create(glui32 rock)
{
	return NULL;
}

void 
glk_schannel_destroy(schanid_t chan)
{
}

schanid_t 
glk_schannel_iterate(schanid_t chan, glui32 *rockptr)
{
	return NULL;
}

glui32 
glk_schannel_get_rock(schanid_t chan)
{
	return 0;
}

glui32 
glk_schannel_play(schanid_t chan, glui32 snd)
{
	return 0;
}

glui32 
glk_schannel_play_ext(schanid_t chan, glui32 snd, glui32 repeats, glui32 notify)
{
	return 0;
}

void 
glk_schannel_stop(schanid_t chan)
{
}

void 
glk_schannel_set_volume(schanid_t chan, glui32 vol)
{
}

void glk_sound_load_hint(glui32 snd, glui32 flag)
{
}
