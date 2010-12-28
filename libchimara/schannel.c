#include <config.h>
#include <glib.h>
#include <libchimara/glk.h>
#ifdef GSTREAMER_SOUND
#include <gst/gst.h>
#endif
#include "magic.h"
#include "schannel.h"
#include "chimara-glk-private.h"
#include "gi_dispa.h"

extern GPrivate *glk_data_key;

/**
 * glk_schannel_create:
 * @rock: The rock value to give the new sound channel.
 *
 * This creates a sound channel, about as you'd expect.
 *
 * Remember that it is possible that the library will be unable to create a new
 * channel, in which case glk_schannel_create() will return %NULL.
 *
 * Returns: A new sound channel, or %NULL.
 */
schanid_t 
glk_schannel_create(glui32 rock)
{
#ifdef GSTREAMER_SOUND
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);

	schanid_t s = g_new0(struct glk_schannel_struct, 1);
	s->magic = MAGIC_SCHANNEL;
	s->rock = rock;
	if(glk_data->register_obj)
		s->disprock = (*glk_data->register_obj)(s, gidisp_Class_Schannel);

	/* Add it to the global sound channel list */
	glk_data->schannel_list = g_list_prepend(glk_data->schannel_list, s);
	s->schannel_list = glk_data->schannel_list;

	/* Create a GStreamer pipeline for the sound channel */
	gchar *pipeline_name = g_strdup_printf("pipeline-%p", s);
	s->pipeline = gst_pipeline_new(pipeline_name);
	g_free(pipeline_name);

	/* Create GStreamer elements to put in the pipeline */
	s->source = gst_element_factory_make("filesrc", NULL);
	s->filter = gst_element_factory_make("identity", NULL);
	s->sink = gst_element_factory_make("autoaudiosink", NULL);
	if(!s->source || !s->filter || !s->sink) {
		WARNING("Could not create one or more GStreamer elements");
		goto fail;
	}
		
	gst_bin_add_many(GST_BIN(s->pipeline), s->source, s->filter, s->sink, NULL);
	if(!gst_element_link_many(s->source, s->filter, s->sink, NULL)) {
		WARNING("Could not link GStreamer elements");
		goto fail;
	}
	
	return s;

fail:
	glk_schannel_destroy(s);
	return NULL;
#else
	return NULL;
#endif /* GSTREAMER_SOUND */
}

/**
 * glk_schannel_destroy:
 * @chan: The sound channel to destroy.
 *
 * Destroys the channel. If the channel is playing a sound, the sound stops 
 * immediately (with no notification event).
 */
void 
glk_schannel_destroy(schanid_t chan)
{
	VALID_SCHANNEL(chan, return);

	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	
	glk_data->schannel_list = g_list_delete_link(glk_data->schannel_list, chan->schannel_list);

	if(glk_data->unregister_obj)
	{
		(*glk_data->unregister_obj)(chan, gidisp_Class_Schannel, chan->disprock);
		chan->disprock.ptr = NULL;
	}
	
	if(chan->pipeline)
		g_object_unref(chan->pipeline);
	
	chan->magic = MAGIC_FREE;
	g_free(chan);
}

/**
 * glk_schannel_iterate:
 * @chan: A sound channel, or %NULL.
 * @rockptr: Return location for the next sound channel's rock, or %NULL.
 *
 * This function can be used to iterate through the list of all open channels.
 * See <link linkend="chimara-Iterating-Through-Opaque-Objects">Iterating 
 * Through Opaque Objects</link>.
 *
 * As that section describes, the order in which channels are returned is 
 * arbitrary.
 *
 * Returns: the next sound channel, or %NULL if there are no more.
 */
schanid_t 
glk_schannel_iterate(schanid_t chan, glui32 *rockptr)
{
	VALID_SCHANNEL_OR_NULL(chan, return NULL);

	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	GList *retnode;
	
	if(chan == NULL)
		retnode = glk_data->schannel_list;
	else
		retnode = chan->schannel_list->next;
	schanid_t retval = retnode? (schanid_t)retnode->data : NULL;
		
	/* Store the sound channel's rock in rockptr */
	if(retval && rockptr)
		*rockptr = glk_schannel_get_rock(retval);
		
	return retval;
}

/**
 * glk_schannel_get_rock:
 * @chan: A sound channel.
 * 
 * Retrieves the channel's rock value. See <link 
 * linkend="chimara-Rocks">Rocks</link>.
 *
 * Returns: A rock value.
 */
glui32 
glk_schannel_get_rock(schanid_t chan)
{
	VALID_SCHANNEL(chan, return 0);
	return chan->rock;
}

/**
 * glk_schannel_play:
 * @chan: Channel to play the sound in.
 * @snd: Resource number of the sound to play.
 *
 * Begins playing the given sound on the channel. If the channel was already
 * playing a sound (even the same one), the old sound is stopped (with no
 * notification event.
 *
 * This returns 1 if the sound actually started playing, and 0 if there was any
 * problem.
 * <note><para>
 *   The most obvious problem is if there is no sound resource with the given
 *   identifier. But other problems can occur. For example, the MOD-playing 
 *   facility in a library might be unable to handle two MODs at the same time,
 *   in which case playing a MOD resource would fail if one was already playing.
 * </para></note>
 *
 * <warning><para>This function is not implemented yet.</para></warning>
 *
 * Returns: 1 on success, 0 on failure.
 */
glui32 
glk_schannel_play(schanid_t chan, glui32 snd)
{
	return glk_schannel_play_ext(chan, snd, 1, 0);
}

/**
 * glk_schannel_play_ext:
 * @chan: Channel to play the sound in.
 * @snd: Resource number of the sound to play.
 * @repeats: Number of times to repeat the sound.
 * @notify: If nonzero, requests a notification when the sound is finished.
 *
 * This works the same as glk_schannel_play(), but lets you specify additional 
 * options. <code>glk_schannel_play(chan, snd)</code> is exactly equivalent to 
 * <code>glk_schannel_play_ext(chan, snd, 1, 0)</code>.
 * 
 * The @repeats value is the number of times the sound should be repeated. A 
 * repeat value of -1 (or rather 0xFFFFFFFF) means that the sound should repeat 
 * forever. A repeat value of 0 means that the sound will not be played at all; 
 * nothing happens. (Although a previous sound on the channel will be stopped, 
 * and the function will return 1.)
 * 
 * The @notify value should be nonzero in order to request a sound notification
 * event. If you do this, when the sound is completed, you will get an event 
 * with type %evtype_SoundNotify. The @window will be %NULL, @val1 will be the 
 * sound's resource id, and @val2 will be the nonzero value you passed as 
 * @notify.
 * 
 * If you request sound notification, and the repeat value is greater than one, 
 * you will get the event only after the last repetition. If the repeat value is
 * 0 or -1, you will never get a notification event at all. Similarly, if the 
 * sound is stopped or interrupted, or if the channel is destroyed while the 
 * sound is playing, there will be no notification event.
 *
 * Not all libraries support sound notification. You should test the
 * %gestalt_SoundNotify selector before you rely on it; see <link
 * linkend="chimara-Testing-for-Sound-Capabilities">Testing for Sound 
 * Capabilities</link>.
 *
 * <warning><para>This function is not implemented yet.</para></warning>
 * 
 * Returns: 1 on success, 0 on failure.
 */
glui32 
glk_schannel_play_ext(schanid_t chan, glui32 snd, glui32 repeats, glui32 notify)
{
	VALID_SCHANNEL(chan, return 0);
	return 0;
}

/**
 * glk_schannel_stop:
 * @chan: Channel to silence.
 *
 * Stops any sound playing in the channel. No notification event is generated,
 * even if you requested one. If no sound is playing, this has no effect.
 *
 * <warning><para>This function is not implemented yet.</para></warning>
 */
void 
glk_schannel_stop(schanid_t chan)
{
	VALID_SCHANNEL(chan, return);
}

/**
 * glk_schannel_set_volume:
 * @chan: Channel to set the volume of.
 * @vol: Integer representing the volume; 0x10000 is 100&percnt;.
 *
 * Sets the volume in the channel. When you create a channel, it has full 
 * volume, represented by the value 0x10000. Half volume would be 0x8000, 
 * three-quarters volume would be 0xC000, and so on. A volume of zero represents
 * silence, although the sound is still considered to be playing.
 *
 * You can call this function between sounds, or while a sound is playing. The 
 * effect is immediate.
 * 
 * You can overdrive the volume of a channel by setting a volume greater than 
 * 0x10000. However, this is not recommended; the library may be unable to 
 * increase the volume past full, or the sound may become distorted. You should 
 * always create sound resources with the maximum volume you will need, and then
 * call glk_schannel_set_volume() to reduce the volume when appropriate.
 *
 * Not all libraries support this function. You should test the
 * %gestalt_SoundVolume selector before you rely on it; see <link
 * linkend="chimara-Testing-for-Sound-Capabilities">Testing for Sound
 * Capabilities</link>.
 *
 * <warning><para>This function is not implemented yet.</para></warning>
 */
void 
glk_schannel_set_volume(schanid_t chan, glui32 vol)
{
	VALID_SCHANNEL(chan, return);
}

/**
 * glk_sound_load_hint:
 * @snd: Resource number of a sound.
 * @flag: Nonzero to tell the library to load the sound, zero to tell the
 * library to unload it.
 *
 * This gives the library a hint about whether the given sound should be loaded
 * or not. If the @flag is nonzero, the library may preload the sound or do
 * other initialization, so that glk_schannel_play() will be faster. If the
 * @flag is zero, the library may release memory or other resources associated
 * with the sound. Calling this function is always optional, and it has no
 * effect on what the library actually plays.
 *
 * <warning><para>This function is not implemented yet.</para></warning>
 */
void 
glk_sound_load_hint(glui32 snd, glui32 flag)
{
}
