#ifndef STREAM_H
#define STREAM_H

#include <gtk/gtk.h>
#include "glk.h"
#include "window.h"

enum StreamType
{
	STREAM_TYPE_WINDOW,
	STREAM_TYPE_MEMORY,
	STREAM_TYPE_FILE
};

struct glk_stream_struct
{
	glui32 rock;
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
	/* For memory and file streams */
	gboolean unicode;
	/* Specific to memory streams */
	gchar *buffer;
	glui32 *ubuffer;
	glui32 mark;
	glui32 buflen;
	/* Specific to file streams */
	FILE *file_pointer;
	gboolean binary;
	gchar *filename; /* Displayable filename in UTF-8 for error handling */
};

strid_t window_stream_new(winid_t window);
void stream_close_common(strid_t str, stream_result_t *result);
#endif
