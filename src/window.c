#include "window.h"

/* Global list of all windows */
static GNode *root_window = NULL;

winid_t
glk_window_open(winid_t split, glui32 method, glui32 size, glui32 wintype, glui32 rock)
{
	extern GtkBuilder *builder;

	if(split)
	{
		g_warning("glk_window_open: splitting of windows not implemented");
		return NULL;
	}

	if(root_window != NULL)
	{
		g_warning("glk_window_open: there is already a window");
		return NULL;
	}

	winid_t new_window = g_new0(struct glk_window_struct, 1);
	root_window = g_node_new(new_window);

	new_window->rock = rock;
	new_window->window_type = wintype;

	GtkBox *vbox = GTK_BOX( gtk_builder_get_object(builder, "vbox") );			
	if(vbox == NULL)
	{
		error_dialog(NULL, NULL, "Could not find vbox");
		return NULL;
	}

	switch(wintype)
	{
		case wintype_TextBuffer:
		{
			GtkWidget *scroll_window = gtk_scrolled_window_new(NULL, NULL);
			GtkWidget *window = gtk_text_view_new();
			gtk_container_add( GTK_CONTAINER(scroll_window), window );
			gtk_box_pack_end(vbox, scroll_window, TRUE, TRUE, 0);
			gtk_widget_show_all(scroll_window);

			new_window->widget = window;
			new_window->window_stream = window_stream_new(new_window);
			new_window->echo_stream = NULL;
			new_window->input_request_type = INPUT_REQUEST_NONE;
			new_window->line_input_buffer = NULL;
			new_window->line_input_buffer_unicode = NULL;
		}

			break;
		default:
			g_warning("glk_window_open: unsupported window type");
			g_free(new_window);
			return NULL;
	}

	new_window->window_node = root_window;

	return new_window;
}

void
glk_set_window(winid_t window)
{
	glk_stream_set_current( glk_window_get_stream(window) );
}

strid_t glk_window_get_stream(winid_t window)
{
	return window->window_stream;
}
