#include "config.h"

#include <glib.h>
#ifdef HAVE_SOUND
#include <gst/gst.h>
#endif

#include "chimara-glk-private.h"
#include "event.h"
#include "glk.h"
#include "gi_dispa.h"
#include "magic.h"
#include "resource.h"
#include "schannel.h"

#define VOLUME_TIMER_RESOLUTION 1.0 /* In milliseconds */

#ifdef HAVE_SOUND
#define OGG_MIMETYPE "audio/ogg"
#endif

extern GPrivate glk_data_key;

#ifdef HAVE_SOUND
/* Stop any currently playing sound on this channel, and remove any
 format-specific GStreamer elements from the channel. */
static void
clean_up_after_playing_sound(schanid_t chan)
{
	if(!gst_element_set_state(chan->pipeline, GST_STATE_NULL))
		WARNING("Could not set GstElement state to NULL");
	if(chan->source)
	{
		gst_bin_remove(GST_BIN(chan->pipeline), chan->source);
		chan->source = NULL;
	}
	if(chan->demux)
	{
		gst_bin_remove(GST_BIN(chan->pipeline), chan->demux);
		chan->demux = NULL;
	}
	if(chan->decode)
	{
		gst_bin_remove(GST_BIN(chan->pipeline), chan->decode);
		chan->decode = NULL;
	}
}

/* This signal is thrown whenever the GStreamer pipeline generates a message.
 Most messages are harmless. */
static void
on_pipeline_message(GstBus *bus, GstMessage *message, schanid_t s)
{
	/* g_printerr("Got %s message\n", GST_MESSAGE_TYPE_NAME(message)); */

	GError *err;
	gchar *debug_message;
	
	switch(GST_MESSAGE_TYPE(message)) {
	case GST_MESSAGE_ERROR: 
	{
		gst_message_parse_error(message, &err, &debug_message);
		IO_WARNING("GStreamer error", err->message, debug_message);
		g_error_free(err);
		g_free(debug_message);
		clean_up_after_playing_sound(s);
	}
		break;
	case GST_MESSAGE_WARNING:
	{
		gst_message_parse_warning(message, &err, &debug_message);
		IO_WARNING("GStreamer warning", err->message, debug_message);
		g_error_free(err);
		g_free(debug_message);
	}
		break;
	case GST_MESSAGE_INFO:
	{
		gst_message_parse_info(message, &err, &debug_message);
		g_message("GStreamer info \"%s\": %s", err->message, debug_message);
		g_error_free(err);
		g_free(debug_message);
	}
		break;
	case GST_MESSAGE_EOS: /* End of stream */
		/* Decrease repeats if not set to forever */
		if(s->repeats != (glui32)-1)
			s->repeats--;
		if(s->repeats > 0) {
			if(!gst_element_seek_simple(s->pipeline, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT, 0)) {
				WARNING("Could not execute GStreamer seek");
				clean_up_after_playing_sound(s);
			}
		} else {
			clean_up_after_playing_sound(s);
			/* Sound ended normally, send a notification if requested */
			if(s->notify)
				chimara_glk_push_event(s->glk, evtype_SoundNotify, NULL, s->resource, s->notify);
		}
		break;
	default:
		/* unhandled message */
		break;
	}
}

/* This signal is thrown when the OGG demuxer element has decided what kind of
 outputs it will output. We connect the decoder element dynamically. */
static void
on_ogg_demuxer_pad_added(GstElement *demux, GstPad *pad, schanid_t s)
{
	GstPad *sinkpad;
	
	/* We can now link this pad with the vorbis-decoder sink pad */
	sinkpad = gst_element_get_static_pad(s->decode, "sink");
	if(gst_pad_link(pad, sinkpad) != GST_PAD_LINK_OK)
		WARNING("Could not link OGG demuxer with Vorbis decoder");
	gst_object_unref(sinkpad);
}

/* This signal is thrown when the typefinder element has found the type of its
 input. Now that we know what kind of input stream we have, we can connect the
 proper demuxer/decoder elements. */
static void
on_type_found(GstElement *typefind, guint probability, GstCaps *caps, schanid_t s)
{
	gchar *type = gst_caps_to_string(caps);
	if(strcmp(type, OGG_MIMETYPE) == 0) {
		s->demux = gst_element_factory_make("oggdemux", NULL);
		s->decode = gst_element_factory_make("vorbisdec", NULL);
		if(!s->demux || !s->decode) {
			WARNING("Could not create one or more GStreamer elements");
			goto finally;
		}
		gst_bin_add_many(GST_BIN(s->pipeline), s->demux, s->decode, NULL);
		if(!gst_element_link(s->typefind, s->demux) || !gst_element_link(s->decode, s->convert)) {
			WARNING("Could not link GStreamer elements");
			goto finally;
		}
		/* We link the demuxer and decoder together dynamically, since the
		 demuxer doesn't know what source pads it will have until it starts
		 demuxing the stream */
		g_signal_connect(s->demux, "pad-added", G_CALLBACK(on_ogg_demuxer_pad_added), s);
	} else if(strcmp(type, "audio/x-aiff") == 0) {
		s->decode = gst_element_factory_make("aiffparse", NULL);
		if(!s->decode) {
			WARNING("Could not create 'aiffparse' GStreamer element");
			goto finally;
		}
		gst_bin_add(GST_BIN(s->pipeline), s->decode);
		if(!gst_element_link_many(s->typefind, s->decode, s->convert, NULL)) {
			WARNING("Could not link GStreamer elements");
			goto finally;
		}
	} else if(g_str_has_prefix(type, "audio/x-mod")) {
		/* "audio/x-mod, type=(string)s3m" has been observed */
		s->decode = gst_element_factory_make("modplug", NULL);
		if(!s->decode) {
			WARNING("Could not create 'modplug' GStreamer element");
			goto finally;
		}
		gst_bin_add(GST_BIN(s->pipeline), s->decode);
		if(!gst_element_link_many(s->typefind, s->decode, s->convert, NULL)) {
			WARNING("Could not link GStreamer elements");
			goto finally;
		}
	} else {
		WARNING_S("Unexpected audio type in blorb", type);
	}

	/* This is necessary in case this handler occurs in the middle of a state
	change */
	gst_element_sync_state_with_parent(s->decode);
	if(s->demux != NULL)
		gst_element_sync_state_with_parent(s->demux);

finally:
	g_free(type);
}

/* Load a sound resource into a GInputStream, by whatever method */
static GInputStream *
load_resource_into_giostream(glui32 snd)
{
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
	GInputStream *retval;

	if(glk_data->resource_map == NULL) {
		if(glk_data->resource_load_callback == NULL) {
			WARNING("No resource map has been loaded yet");
			return NULL;
		}
		char *filename = glk_data->resource_load_callback(CHIMARA_RESOURCE_SOUND, snd, glk_data->resource_load_callback_data);
		if(filename == NULL) {
			WARNING("Error loading resource from alternative location");
			return NULL;
		}

		GError *err = NULL;
		GFile *file = g_file_new_for_path(filename);
		retval = G_INPUT_STREAM(g_file_read(file, NULL, &err));
		if(retval == NULL)
			IO_WARNING("Error loading resource from file", filename, err->message);
		g_free(filename);
		g_object_unref(file);
	} else {
		giblorb_result_t resource;
		giblorb_err_t result = giblorb_load_resource(glk_data->resource_map, giblorb_method_Memory, &resource, giblorb_ID_Snd, snd);
		if(result != giblorb_err_None) {
			WARNING_S( "Error loading resource", giblorb_get_error_message(result) );
			return NULL;
		}
		retval = g_memory_input_stream_new_from_data(resource.data.ptr, resource.length, NULL);
	}
	return retval;
}
#endif  /* HAVE_SOUND */

/**
 * glk_schannel_create:
 * @rock: The rock value to give the new sound channel.
 *
 * This creates a sound channel, about as you'd expect.
 *
 * Remember that it is possible that the library will be unable to create a new
 * channel, in which case glk_schannel_create() will return %NULL.
 *
 * When you create a channel using glk_schannel_create(), it has full volume,
 * represented by the value 0x10000. Half volume would be 0x8000, three-quarters
 * volume would be 0xC000, and so on. A volume of zero represents silence.
 *
 * You can overdrive the volume of a channel by setting a volume greater than 
 * 0x10000. However, this is not recommended; the library may be unable to 
 * increase the volume past full, or the sound may become distorted. You should 
 * always create sound resources with the maximum volume you will need, and then
 * reduce the volume when appropriate using the channel-volume calls.
 *
 * <note><para>
 *   Mathematically, these volume changes should be taken as linear
 *   multiplication of a waveform represented as linear samples. As I
 *   understand it, linear PCM encodes the sound pressure, and therefore a
 *   volume of 0x8000 should represent a 6 dB drop.
 * </para></note>
 *
 * Returns: A new sound channel, or %NULL.
 */
schanid_t 
glk_schannel_create(glui32 rock)
{
	return glk_schannel_create_ext(rock, 0x10000);
}

/**
 * glk_schannel_create_ext:
 * @rock: The rock value to give the new sound channel.
 * @volume: Integer representing the volume; 0x10000 is 100&percnt;.
 *
 * The glk_schannel_create_ext() call lets you create a channel with the volume
 * already set at a given level.
 *
 * Not all libraries support glk_schannel_create_ext(). You should test the
 * %gestalt_Sound2 selector before you rely on it; see [Testing for Sound
 * Capabilities][chimara-Testing-for-Sound-Capabilities].
 *
 * Returns: A new sound channel, or %NULL.
 */
schanid_t
glk_schannel_create_ext(glui32 rock, glui32 volume)
{
#ifdef HAVE_SOUND
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);

	schanid_t s = g_slice_new0(struct glk_schannel_struct);
	s->magic = MAGIC_SCHANNEL;
	s->rock = rock;
	if(glk_data->register_obj)
		s->disprock = (*glk_data->register_obj)(s, gidisp_Class_Schannel);

	/* Add it to the global sound channel list */
	glk_data->schannel_list = g_list_prepend(glk_data->schannel_list, s);
	s->schannel_list = glk_data->schannel_list;

	/* Add a pointer to the ChimaraGlk widget, for convenience */
	s->glk = glk_data->self;

	/* Create a GStreamer pipeline for the sound channel */
	gchar *pipeline_name = g_strdup_printf("pipeline-%p", s);
	s->pipeline = gst_pipeline_new(pipeline_name);
	g_free(pipeline_name);

	/* Watch for messages from the pipeline */
	GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(s->pipeline));
	gst_bus_add_signal_watch(bus);
	g_signal_connect(bus, "message", G_CALLBACK(on_pipeline_message), s);
	gst_object_unref(bus);

	/* Create GStreamer elements to put in the pipeline */
	s->typefind = gst_element_factory_make("typefind", NULL);
	s->convert = gst_element_factory_make("audioconvert", NULL);
	s->filter = gst_element_factory_make("volume", NULL);
	s->sink = gst_element_factory_make("autoaudiosink", NULL);
	if(!s->typefind || !s->convert || !s->filter || !s->sink) {
		WARNING("Could not create one or more GStreamer elements");
		goto fail;
	}

	/* Set the initial volume */
	glk_schannel_set_volume(s, volume);

	/* Put the elements in the pipeline and link as many together as we can
	 without knowing the type of the audio stream */
	gst_bin_add_many(GST_BIN(s->pipeline), s->typefind, s->convert, s->filter, s->sink, NULL);

	/* Link elements: ??? -> Converter -> Volume filter -> Sink */
	if(!gst_element_link_many(s->convert, s->filter, s->sink, NULL)) {
		WARNING("Could not link GStreamer elements");
		goto fail;
	}
	g_signal_connect(s->typefind, "have-type", G_CALLBACK(on_type_found), s);
	
	return s;

fail:
	glk_schannel_destroy(s);
	return NULL;
#else
	return NULL;
#endif  /* HAVE_SOUND */
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

#ifdef HAVE_SOUND
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);

	if(!gst_element_set_state(chan->pipeline, GST_STATE_NULL))
		WARNING("Could not set GstElement state to NULL");
	
	glk_data->schannel_list = g_list_delete_link(glk_data->schannel_list, chan->schannel_list);

	if(glk_data->unregister_obj)
	{
		(*glk_data->unregister_obj)(chan, gidisp_Class_Schannel, chan->disprock);
		chan->disprock.ptr = NULL;
	}

	/* This also frees all the objects inside the pipeline */
	if(chan->pipeline)
		gst_object_unref(chan->pipeline);
	
	chan->magic = MAGIC_FREE;
	g_slice_free(struct glk_schannel_struct, chan);
#endif  /* HAVE_SOUND */
}

/**
 * glk_schannel_iterate:
 * @chan: A sound channel, or %NULL.
 * @rockptr: Return location for the next sound channel's rock, or %NULL.
 *
 * This function can be used to iterate through the list of all open channels.
 * See [Iterating through Opaque
 * Objects][chimara-Iterating-Through-Opaque-Objects].
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

#ifdef HAVE_SOUND
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
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
#else
	return NULL;
#endif  /* HAVE_SOUND */
}

/**
 * glk_schannel_get_rock:
 * @chan: A sound channel.
 * 
 * Retrieves the channel's rock value.
 * See [Rocks][chimara-Rocks].
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
 * options. `glk_schannel_play(chan, snd)` is exactly equivalent to
 * `glk_schannel_play_ext(chan, snd, 1, 0)`.
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
 * %gestalt_Sound2 selector before you rely on it; see [Testing for Sound
 * Capabilities][chimara-Testing-for-Sound-Capabilities].
 *
 * Note that you can play a sound on a channel whose volume is zero. This has
 * no audible result, unless you later change the volume; but it produces
 * notifications as usual. You can also play a sound on a paused channel; the
 * sound is paused immediately, and does not progress.
 * 
 * Returns: 1 on success, 0 on failure.
 */
glui32 
glk_schannel_play_ext(schanid_t chan, glui32 snd, glui32 repeats, glui32 notify)
{
	VALID_SCHANNEL(chan, return 0);
#ifdef HAVE_SOUND
	/* Stop the previous sound */
	clean_up_after_playing_sound(chan);

	/* Don't play if repeats = 0 */
	if(repeats == 0) {
		chan->repeats = 0;
		return 1;
	}

	GInputStream *stream = load_resource_into_giostream(snd);
	if(stream == NULL)
		return 0;

	chan->source = gst_element_factory_make("giostreamsrc", NULL);
	g_object_set(chan->source, "stream", stream, NULL);
	g_object_unref(stream); /* Now owned by GStreamer element */
	gst_bin_add(GST_BIN(chan->pipeline), chan->source);
	if(!gst_element_link(chan->source, chan->typefind)) {
		WARNING("Could not link GStreamer elements");
		clean_up_after_playing_sound(chan);
		return 0;
	}

	chan->repeats = repeats;
	chan->resource = snd;
	chan->notify = notify;
	
	/* Play the sound; unless the channel is paused, then pause it instead */
	if(!gst_element_set_state(chan->pipeline, chan->paused? GST_STATE_PAUSED : GST_STATE_PLAYING)) {
		WARNING_S("Could not set GstElement state to", chan->paused? "PAUSED" : "PLAYING");
		clean_up_after_playing_sound(chan);
		return 0;
	}
	return 1;
#else
	return 0;
#endif  /* HAVE_SOUND */
}

/**
 * glk_schannel_play_multi:
 * @chanarray: Array of #schanid_t structures.
 * @chancount: Length of @chanarray.
 * @sndarray: Array of sound resource numbers.
 * @soundcount: Length of @sndarray, must be equal to @chanarray.
 * @notify: If nonzero, request a notification when each sound finishes.
 *
 * This works the same as glk_schannel_play_ext(), except that you can specify
 * more than one sound. The channel references and sound resource numbers are
 * given as two arrays, which must be the same length. The @notify argument
 * applies to all the sounds; the repeats value for all the sounds is 1.
 * 
 * All the sounds will begin at exactly the same time.
 * 
 * This returns the number of sounds that began playing correctly. (This will be
 * a number from 0 to @soundcount.)
 *
 * <note><para>
 *   If the @notify argument is nonzero, you will get a separate sound
 *   notification event as each sound finishes. They will all have the same
 *   @val2 value.
 * </para></note>
 * <note><para>
 *   Note that you have to supply @chancount and @soundcount as separate
 *   arguments, even though they are required to be the same. This is an awkward
 *   consequence of the way array arguments are dispatched in Glulx.
 * </para></note>
 * 
 * Returns: The number of sounds that started playing correctly.
 */
glui32
glk_schannel_play_multi(schanid_t *chanarray, glui32 chancount, glui32 *sndarray, glui32 soundcount, glui32 notify)
{
	g_return_val_if_fail(chancount == soundcount, 0);
	g_return_val_if_fail(chanarray != NULL || chancount == 0, 0);
	g_return_val_if_fail(sndarray != NULL || soundcount == 0, 0);

	int count;
	for(count = 0; count < chancount; count++)
		VALID_SCHANNEL(chanarray[count], return 0);

#ifdef HAVE_SOUND
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
	if(!glk_data->resource_map && !glk_data->resource_load_callback) {
		WARNING("No resource map has been loaded yet");
		return 0;
	}

	/* We keep an array of sounds to skip if any of them have errors */
	gboolean *skiparray = g_new0(gboolean, chancount);

	/* Set up all the channels one by one */
	for(count = 0; count < chancount; count++) {
		/* Stop the previous sound */
		clean_up_after_playing_sound(chanarray[count]);

		GInputStream *stream = load_resource_into_giostream(sndarray[count]);
		if(stream == NULL) {
			skiparray[count] = TRUE;
			continue;
		}

		chanarray[count]->source = gst_element_factory_make("giostreamsrc", NULL);
		g_object_set(chanarray[count]->source, "stream", stream, NULL);
		g_object_unref(stream); /* Now owned by GStreamer element */
		gst_bin_add(GST_BIN(chanarray[count]->pipeline), chanarray[count]->source);
		if(!gst_element_link(chanarray[count]->source, chanarray[count]->typefind)) {
			WARNING("Could not link GStreamer elements");
			clean_up_after_playing_sound(chanarray[count]);
		}

		chanarray[count]->repeats = 1;
		chanarray[count]->resource = sndarray[count];
		chanarray[count]->notify = notify;
	}

	/* Start all the sounds as close to each other as possible. */
	/* FIXME: Is there a way to start them exactly at the same time? */
	glui32 successes = 0;
	for(count = 0; count < chancount; count++) {
		if(skiparray[count])
			continue;
		/* Play the sound; unless the channel is paused, then pause it instead */
		if(!gst_element_set_state(chanarray[count]->pipeline, chanarray[count]->paused? GST_STATE_PAUSED : GST_STATE_PLAYING)) {
			WARNING_S("Could not set GstElement state to", chanarray[count]->paused? "PAUSED" : "PLAYING");
			skiparray[count] = TRUE;
			clean_up_after_playing_sound(chanarray[count]);
			continue;
		}
		successes++;
	}
	g_free(skiparray);
	return successes;
#else
	return 0;
#endif  /* HAVE_SOUND */
}

/**
 * glk_schannel_stop:
 * @chan: Channel to silence.
 *
 * Stops any sound playing in the channel. No notification event is generated,
 * even if you requested one. If no sound is playing, this has no effect.
 */
void 
glk_schannel_stop(schanid_t chan)
{
	VALID_SCHANNEL(chan, return);
#ifdef HAVE_SOUND
	clean_up_after_playing_sound(chan);
#endif
}

/**
 * glk_schannel_pause:
 * @chan: Channel to pause.
 *
 * Pause any sound playing in the channel. This does not generate any
 * notification events. If the channel is already paused, this does nothing.
 * 
 * New sounds started in a paused channel are paused immediately.
 * 
 * A volume change in progress is <emphasis>not</emphasis> paused, and may
 * proceed to completion, generating a notification if appropriate.
 */
void
glk_schannel_pause(schanid_t chan)
{
	VALID_SCHANNEL(chan, return);

	if(chan->paused)
		return; /* Silently do nothing */

	/* Mark the channel as paused even if there is no sound playing yet */
	chan->paused = TRUE;

#ifdef HAVE_SOUND
	GstState state;
	if(gst_element_get_state(chan->pipeline, &state, NULL, GST_CLOCK_TIME_NONE) != GST_STATE_CHANGE_SUCCESS) {
		WARNING("Could not get GstElement state");
		return;
	}
	if(state != GST_STATE_PLAYING)
		return; /* Silently do nothing if no sound is playing */

	if(!gst_element_set_state(chan->pipeline, GST_STATE_PAUSED)) {
		WARNING("Could not set GstElement state to PAUSED");
		return;
	}
#endif  /* HAVE_SOUND */
}

/**
 * glk_schannel_unpause:
 * @chan: Channel to unpause.
 *
 * Unpause the channel. Any paused sounds begin playing where they left off. If
 * the channel is not already paused, this does nothing.
 *
 * <note><para>
 *   This means, for example, that you can pause a channel that is currently
 *   not playing any sounds. If you then add a sound to the channel, it will
 *   not start playing; it will be paused at its beginning. If you later
 *   unpaise the channel, the sound will commence.
 * </para></note>
 */
void
glk_schannel_unpause(schanid_t chan)
{
	VALID_SCHANNEL(chan, return);

	if(!chan->paused)
		return; /* Silently do nothing */

	/* Mark the channel as not paused in any case */
	chan->paused = FALSE;

#ifdef HAVE_SOUND
	GstState state;
	if(gst_element_get_state(chan->pipeline, &state, NULL, GST_CLOCK_TIME_NONE) != GST_STATE_CHANGE_SUCCESS) {
		WARNING("Could not get GstElement state");
		return;
	}
	if(state != GST_STATE_PAUSED)
		return; /* Silently do nothing */

	if(!gst_element_set_state(chan->pipeline, GST_STATE_PLAYING)) {
		WARNING("Could not set GstElement state to PLAYING");
		return;
	}
#endif  /* HAVE_SOUND */
}

/**
 * glk_schannel_set_volume:
 * @chan: Channel to set the volume of.
 * @vol: Integer representing the volume; 0x10000 is 100&percnt;.
 *
 * Sets the volume in the channel, from 0 (silence) to 0x10000 (full volume).
 * Again, you can overdrive the volume by setting a value greater than 0x10000,
 * but this is not recommended.
 *
 * The glk_schannel_set_volume() function does not include duration and notify
 * values. Both are assumed to be zero: immediate change, no notification.
 *
 * You can call this function between sounds, or while a sound is playing.
 * However, a zero-duration change while a sound is playing may produce
 * unpleasant clicks.
 * 
 * At most one volume change can be occurring on a sound channel at any time.
 * If you call this function while a previous volume change is in progress, the
 * previous change is interrupted.
 *
 * Not all libraries support this function. You should test the
 * %gestalt_SoundVolume selector before you rely on it; see [Testing for Sound
 * Capabilities][chimara-Testing-for-Sound-Capabilities].
 *
 * > # Chimara #
 * > Chimara supports volumes from 0 to 1000&percnt;, that is, values of @vol up
 * > to 0xA0000.
 */
void 
glk_schannel_set_volume(schanid_t chan, glui32 vol)
{
	glk_schannel_set_volume_ext(chan, vol, 0, 0);
}

#ifdef HAVE_SOUND
static double
volume_glk_to_gstreamer(glui32 volume_glk)
{
	return CLAMP(((double)volume_glk / 0x10000), 0.0, 10.0);
}

static gboolean
volume_change_timeout(schanid_t chan)
{
	GTimeVal now;
	g_get_current_time(&now);

	if(now.tv_sec >= chan->target_time_sec && now.tv_usec >= chan->target_time_usec) {
		/* We're done - make sure the volume is at the requested level */
		g_object_set(chan->filter, "volume", chan->target_volume, NULL);

		if(chan->volume_notify)
			chimara_glk_push_event(chan->glk, evtype_VolumeNotify, NULL, 0, chan->volume_notify);

		chan->volume_timer_id = 0;
		return FALSE;
	}

	/* Calculate the appropriate step every time - a busy system may delay or
	 * drop	timer ticks */
	double time_left_msec = (chan->target_time_sec - now.tv_sec) * 1000.0
		+ (chan->target_time_usec - now.tv_usec) / 1000.0;
	double steps_left = time_left_msec / VOLUME_TIMER_RESOLUTION;
	double current_volume;
	g_object_get(chan->filter, "volume", &current_volume, NULL);
	double volume_step = (chan->target_volume - current_volume) / steps_left;

	g_object_set(chan->filter, "volume", current_volume + volume_step, NULL);

	return TRUE;
}
#endif  /* HAVE_SOUND */

/**
 * glk_schannel_set_volume_ext:
 * @chan: Channel to set the volume of.
 * @vol: Integer representing the volume; 0x10000 is 100&percnt;.
 * @duration: Length of volume change in milliseconds, or 0 for immediate.
 * @notify: If nonzero, requests a notification when the volume change finishes.
 *
 * Sets the volume in the channel, from 0 (silence) to 0x10000 (full volume).
 * Again, you can overdrive the volume by setting a value greater than 0x10000,
 * but this is not recommended.
 *
 * If the @duration is zero, the change is immediate. Otherwise, the change
 * begins immediately, and occurs smoothly over the next @duration milliseconds.
 *
 * The @notify value should be nonzero in order to request a volume notification
 * event. If you do this, when the volume change is completed, you will get an
 * event with type #evtype_VolumeNotify. The window will be %NULL, @val1 will be
 * zero, and @val2 will be the nonzero value you passed as @notify.
 *
 * You can call this function between sounds, or while a sound is playing.
 * However, a zero-duration change while a sound is playing may produce
 * unpleasant clicks.
 *
 * At most one volume change can be occurring on a sound channel at any time. If
 * you call this function while a previous volume change is in progress, the
 * previous change is interrupted. The beginning point of the new volume change
 * should be wherever the previous volume change was interrupted (rather than
 * the previous change's beginning or ending point).
 *
 * Not all libraries support these functions. You should test the appropriate
 * gestalt selectors before you rely on them; see "Testing for Sound
 * Capabilities".
 */
void
glk_schannel_set_volume_ext(schanid_t chan, glui32 vol, glui32 duration, glui32 notify)
{
	VALID_SCHANNEL(chan, return);
	/* Silently ignore out-of-range volume values */

#ifdef HAVE_SOUND
	/* Interrupt a previous volume change */
	if(chan->volume_timer_id > 0)
		g_source_remove(chan->volume_timer_id);
	
	double target_volume = volume_glk_to_gstreamer(vol);

	if(duration == 0) {
		g_object_set(chan->filter, "volume", target_volume, NULL);

		if(notify != 0)
			chimara_glk_push_event(chan->glk, evtype_VolumeNotify, NULL, 0, notify);

		return;
	}

	GTimeVal target_time;
	g_get_current_time(&target_time);
	g_time_val_add(&target_time, (long)duration * 1000);

	chan->target_volume = target_volume;
	chan->target_time_sec = target_time.tv_sec;
	chan->target_time_usec = target_time.tv_usec;
	chan->volume_notify = notify;

	/* Set up a timer for the volume */
	chan->volume_timer_id = g_timeout_add(VOLUME_TIMER_RESOLUTION, (GSourceFunc)volume_change_timeout, chan);
#endif  /* HAVE_SOUND */
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
 */
void 
glk_sound_load_hint(glui32 snd, glui32 flag)
{
#ifdef HAVE_SOUND
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
	giblorb_result_t resource;
	giblorb_err_t result;

	/* Sound load hints only work for Blorb resource maps */
	if(!glk_data->resource_map)
		return;

	if(flag) {
		/* The sound load hint simply loads the resource from the resource map;
		 loading a chunk more than once does nothing */
		result = giblorb_load_resource(glk_data->resource_map, giblorb_method_Memory, &resource, giblorb_ID_Snd, snd);
		if(result != giblorb_err_None) {
			WARNING_S( "Error loading resource", giblorb_get_error_message(result) );
			return;
		}
	} else {
		/* Get the Blorb chunk number by loading the resource with
		 method_DontLoad, then unload that chunk - has no effect if the chunk
		 isn't loaded */
		result = giblorb_load_resource(glk_data->resource_map, giblorb_method_DontLoad, &resource, giblorb_ID_Snd, snd);
		if(result != giblorb_err_None) {
			WARNING_S( "Error loading resource", giblorb_get_error_message(result) );
			return;
		}
		result = giblorb_unload_chunk(glk_data->resource_map, resource.chunknum);
		if(result != giblorb_err_None) {
			WARNING_S( "Error unloading chunk", giblorb_get_error_message(result) );
			return;
		}
	}
#endif  /* HAVE_SOUND */
}
