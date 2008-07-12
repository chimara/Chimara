#ifndef WINDOW_H
#define WINDOW_H

#include <gtk/gtk.h>
#include "glk.h"

#include "stream.h"
#include "error.h"

enum InputRequestType
{
	INPUT_REQUEST_NONE,
	INPUT_REQUEST_CHARACTER,
	INPUT_REQUEST_CHARACTER_UNICODE,
	INPUT_REQUEST_LINE,
	INPUT_REQUEST_LINE_UNICODE
};

struct glk_window_struct
{
	GNode *window_node;

	glui32 rock;
	glui32 window_type;
	GtkWidget *widget;
	strid_t window_stream;
	strid_t echo_stream;
	enum InputRequestType input_request_type;
	gchar *line_input_buffer;
	glui32 *line_input_buffer_unicode;
	glui32 line_input_buffer_max_len;
};

#endif
