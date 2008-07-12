#ifndef STREAM_H
#define STREAM_H

#include <gtk/gtk.h>
#include "glk.h"
#include "window.h"

enum StreamType
{
	STREAM_TYPE_WINDOW,
	STREAM_TYPE_MEMORY,
	STREAM_TYPE_FILE,
	STREAM_TYPE_UNICODE_MEMORY,
	STREAM_TYPE_UNICODE_FILE,
};

struct glk_stream_struct
{
	GList* stream_list;

	glui32 rock;
	glui32 file_mode;
	glui32 read_count;
	glui32 write_count;
	enum StreamType stream_type;
	winid_t window;
	gchar *memory_buffer;
	glui32 *memory_buffer_unicode;
	glui32 buffer_len;
};


strid_t window_stream_new(winid_t window);

#endif
