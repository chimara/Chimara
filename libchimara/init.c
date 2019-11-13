#include "config.h"

#include <glib.h>
#include <glib/gi18n-lib.h>
#ifdef HAVE_SOUND
#include <gst/gst.h>
#endif

static gboolean chimara_initialized = FALSE;

/* This function is called at every entry point of the library, to set up
gettext and GStreamer. It is NOT called from Glk functions. */
void
chimara_init(void)
{
	if( G_UNLIKELY(!chimara_initialized) )
	{
		/* Setup gettext */
		bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
		bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");

#ifdef HAVE_SOUND
		/* Make sure GStreamer has been initialized if it hasn't been already;
		in particular, if you want your program to parse GStreamer command line
		options then you should do it yourself, before gtk_init(). */

		if( !gst_is_initialized() )
			gst_init(NULL, NULL);
#endif
	}
}

