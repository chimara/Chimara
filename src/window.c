#include "window.h"

/* Global tree of all windows */
static GNode *root_window = NULL;

/**
 * glk_window_open:
 * @split: The window to split to create the new window. Must be 0 if there
 * are no windows yet.
 * @method: Position of the new window and method of size computation. One of
 * #winmethod_Above, #winmethod_Below, #winmethod_Left, or #winmethod_Right
 * OR'ed with #winmethod_Fixed or #winmethod_Proportional. If @wintype is
 * #wintype_Blank, then #winmethod_Fixed is not allowed.
 * @size: Size of the new window, in percentage points if @method is
 * #winmethod_Proportional, otherwise in characters if @wintype is 
 * #wintype_TextBuffer or #wintype_TextGrid, or pixels if @wintype is
 * #wintype_Graphics.
 * @wintype: Type of the new window. One of #wintype_Blank, #wintype_TextGrid,
 * #wintype_TextBuffer, or #wintype_Graphics.
 * @rock: The new window's rock value.
 *
 * If there are no windows, create a new root window. @split must be 0, and
 * @method and @size are ignored. Otherwise, split window @split into two, with
 * position, size, and type specified by @method, @size, and @wintype. See the
 * Glk documentation for the window placement algorithm.
 *
 * Returns: the new window.
 */
winid_t
glk_window_open(winid_t split, glui32 method, glui32 size, glui32 wintype, 
                glui32 rock)
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
	/* We only create one window and don't support any more than that */
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
			/* We need to put these declarations inside their own scope */
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

/**
 * glk_set_window:
 * @win: A window.
 *
 * Sets the current stream to @win's window stream.
 */
void
glk_set_window(winid_t win)
{
	glk_stream_set_current( glk_window_get_stream(win) );
}

/**
 * glk_window_get_stream:
 * @win: A window.
 *
 * Gets the stream associated with @win.
 *
 * Returns: The window stream.
 */
strid_t glk_window_get_stream(winid_t win)
{
	return win->window_stream;
}

