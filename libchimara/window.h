#ifndef WINDOW_H
#define WINDOW_H

#include <gtk/gtk.h>

#include "glk.h"
#include "gi_dispa.h"

enum InputRequestType
{
	INPUT_REQUEST_NONE,
	INPUT_REQUEST_CHARACTER,
	INPUT_REQUEST_CHARACTER_UNICODE,
	INPUT_REQUEST_LINE,
	INPUT_REQUEST_LINE_UNICODE
};

struct hyperlink {
	uint32_t value;
	GtkTextTag *tag;
	unsigned long event_handler;
	winid_t window;
};
typedef struct hyperlink hyperlink_t;

struct glk_window_struct
{
	/*< private >*/

	/* The following fields are constant and must not be changed after creating
	the window */
	glui32 magic, rock;
	char *librock; /* "library rock" - unique string identifier */
	gidispatch_rock_t disprock;
	glui32 type;

	/* The following fields should only be accessed by the Glk thread */
	/* Streams associated with the window */
	strid_t window_stream;  /* returned by get_window_stream() */
	strid_t echo_stream;    /* returned by get_echo_stream() */

	/* These fields may be accessed by both the Glk thread and the UI thread. Any
	access must be protected by locking the @lock mutex. */

	GMutex lock;
	/* Width and height of the window, in characters */
	glui32 width;     /* returned by get_size(), needed in ui_window_print_string() */
	glui32 height;    /* ditto */
	/* Width and height of the window's size units, in pixels */
	int unit_width;   /* ditto */
	int unit_height;  /* ditto */

	/* The window tree may be accessed by both the Glk thread and the UI thread,
	but must be protected by locking the library's arrange_lock. */

	/* Pointer to the node in the global tree that contains this window */
	GNode *window_node;      /* returned by get_parent(), get_sibling(), etc. */
	/* Window split data (pair windows only) */
	winid_t key_window;      /* returned by get_arrangement(), needed in size_allocate() */
	glui32 split_method;     /* ditto */
	glui32 constraint_size;  /* ditto */

	/* Below here, the fields should only be accessed by the UI thread */

	/* "widget" is the actual widget with the window's functionality */
	GtkWidget *widget;
	/* "frame" is the widget that is the child of the ChimaraGlk container, such 
	as a scroll window. It may be the same as "widget". */
	GtkWidget *frame;
	/* In text buffer windows, the scrolled window and the pager are extra
	widgets that are neither "widget" nor "frame" */
	GtkWidget *scrolledwindow;
	GtkWidget *pager;
	/* Input request stuff */
	enum InputRequestType input_request_type;
	gchar *line_input_buffer;
	glui32 *line_input_buffer_unicode;
	glui32 line_input_buffer_max_len;
	gidispatch_rock_t buffer_rock;
	gboolean mouse_input_requested;
	GList *history;
	GList *history_pos;
	GSList *extra_line_terminators;
	GSList *current_extra_line_terminators;
	/* Line input echoing (text buffers only) */
	gboolean echo_line_input;
	gboolean echo_current_line_input;
	/* Line input field (text grids only) */
	glui32 input_length;
	GtkTextChildAnchor *input_anchor;
	GtkWidget *input_entry;
	gulong line_input_entry_changed;
	/* Signal handlers */
	gulong char_input_keypress_handler;
	gulong line_input_keypress_handler;
	gulong insert_text_handler;
	gulong tag_event_handler;
	gulong shutdown_keypress_handler;
	gulong button_press_event_handler;
	gulong size_allocate_handler;
	gulong pager_keypress_handler;
	gulong pager_adjustment_handler;
	/* Window buffer (text buffers and grids only) */
	GString *buffer;
	GtkTextTag *zcolor;
	GtkTextTag *zcolor_reversed;
	char *style_tagname;  /* Name of the current style */
	char *glk_style_tagname;  /* Name of the current glk style override */
	GtkCssProvider *font_override;
	GtkCssProvider *background_override;
	/* Hyperlinks */
	GHashTable *hyperlinks;
	struct hyperlink *current_hyperlink;
	gboolean hyperlink_event_requested;
	/* Graphics */
	glui32 background_color;
	cairo_surface_t *backing_store;
	/* Pager (textbuffer only) */
	gboolean currently_paging;
};

#endif
