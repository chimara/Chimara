#ifndef STREAM_H
#define STREAM_H

#include <stdio.h>

#include <glib.h>

#include "glk.h"
#include "gi_dispa.h"

enum StreamType
{
	STREAM_TYPE_WINDOW,
	STREAM_TYPE_MEMORY,
	STREAM_TYPE_FILE,
	STREAM_TYPE_RESOURCE
};

struct glk_stream_struct
{
	/*< private >*/
	glui32 magic, rock;
	gidispatch_rock_t disprock;
	/* Pointer to the list node in the global stream list that contains this
	stream */
	GList* stream_list;
	/* Stream parameters */
	glui32 file_mode;
	glui32 read_count;
	glui32 write_count;
	enum StreamType type;
	/* Specific to window stream: the window this stream is connected to */
	winid_t window;
	/* For memory, file, and resource streams */
	gboolean unicode;
	/* For file and resource streams */
	gboolean binary;
	/* For memory and resource streams */
	char *buffer;
	glui32 *ubuffer;
	glui32 mark;
	glui32 endmark;
	glui32 buflen;
	/* Specific to memory streams */
	gidispatch_rock_t buffer_rock;
	/* Specific to file streams */
	FILE *file_pointer;
	gchar *filename; /* Displayable filename in UTF-8 for error handling */
	glui32 lastop; /* 0, filemode_Write, or filemode_Read */

	gchar *style; /* Name of the current style */
	gchar *glk_style; /* Name of the current glk style override */
	gboolean hyperlink_mode; /* When turned on, text written to the stream will be a hyperlink */
};

G_GNUC_INTERNAL strid_t file_stream_new(frefid_t fileref, glui32 fmode, glui32 rock, gboolean unicode);
G_GNUC_INTERNAL strid_t stream_new_common(glui32 rock);
G_GNUC_INTERNAL void stream_close_common(strid_t str, stream_result_t *result);

#endif
