#include "window.h"
#include "chimara-glk-private.h"

extern ChimaraGlkPrivate *glk_data;

/**
 * glk_window_iterate:
 * @win: A window, or %NULL.
 * @rockptr: Return location for the next window's rock, or %NULL.
 *
 * Iterates over the list of windows; if @win is %NULL, it returns the first
 * window, otherwise the next window after @win. If there are no more, it
 * returns #NULL. The window's rock is stored in @rockptr. If you don't want
 * the rocks to be returned, you may set @rockptr to %NULL.
 *
 * The order in which windows are returned is arbitrary. The root window is
 * not necessarily first, nor is it necessarily last. The order may change
 * every time you create or destroy a window, invalidating the iteration.
 *
 * Returns: the next window, or %NULL if there are no more.
 */
winid_t
glk_window_iterate(winid_t win, glui32 *rockptr)
{
	GNode *retnode;
	
	if(win == NULL)
		retnode = glk_data->root_window;
	else
	{
		GNode *node = win->window_node;
		if( G_NODE_IS_LEAF(node) )
		{
			while(node && node->next == NULL)
				node = node->parent;
			if(node)
				retnode = node->next;
			else
				retnode = NULL;
		}
		else
			retnode = g_node_first_child(node);
	}
	winid_t retval = retnode? (winid_t)retnode->data : NULL;
		
	/* Store the window's rock in rockptr */
	if(retval && rockptr)
		*rockptr = glk_window_get_rock(retval);
		
	return retval;
}

/**
 * glk_window_get_rock:
 * @win: A window.
 * 
 * Returns @win's rock value. Pair windows always have rock 0; all other windows
 * have the rock value you created them with.
 *
 * Returns: A rock value.
 */
glui32
glk_window_get_rock(winid_t win)
{
	g_return_val_if_fail(win != NULL, 0);
	return win->rock;
}

/**
 * glk_window_get_type:
 * @win: A window.
 *
 * Returns @win's type, one of #wintype_Blank, #wintype_Pair,
 * #wintype_TextBuffer, #wintype_TextGrid, or #wintype_Graphics.
 *
 * Returns: The window's type.
 */
glui32
glk_window_get_type(winid_t win)
{
	g_return_val_if_fail(win != NULL, 0);
	return win->type;
}

/**
 * glk_window_get_parent:
 * @win: A window.
 *
 * Returns the window @win's parent window. If @win is the root window, this
 * returns %NULL, since the root window has no parent. Remember that the parent
 * of every window is a pair window; other window types are always childless.
 *
 * Returns: A window.
 */
winid_t
glk_window_get_parent(winid_t win)
{
	g_return_val_if_fail(win != NULL, NULL);
	/* Value will also be NULL if win is the root window */
	return (winid_t)win->window_node->parent->data;
}

/**
 * glk_window_get_sibling:
 * @win: A window.
 *
 * Returns the other child of the window @win's parent. If @win is the
 * root window, this returns %NULL.
 *
 * Returns: A window, or %NULL.
 */
winid_t
glk_window_get_sibling(winid_t win)
{
	g_return_val_if_fail(win != NULL, NULL);
	
	if(G_NODE_IS_ROOT(win->window_node))
		return NULL;
	if(win->window_node->next)
		return (winid_t)win->window_node->next;
	return (winid_t)win->window_node->prev;
}

/**
 * glk_window_get_root:
 * 
 * Returns the root window. If there are no windows, this returns #NULL.
 *
 * Returns: A window, or #NULL.
 */
winid_t
glk_window_get_root()
{
	if(glk_data->root_window == NULL)
		return NULL;
	return (winid_t)glk_data->root_window->data;
}

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
	if(split)
	{
		g_warning("glk_window_open: splitting of windows not implemented");
		return NULL;
	}

	if(glk_data->root_window != NULL)
	{
		g_warning("glk_window_open: there is already a window");
		return NULL;
	}
	
	gdk_threads_enter();
	
	/* We only create one window and don't support any more than that */
	winid_t win = g_new0(struct glk_window_struct, 1);
	glk_data->root_window = g_node_new(win);

	win->rock = rock;
	win->type = wintype;
    win->window_node = glk_data->root_window;

	switch(wintype)
	{
		case wintype_Blank:
		{
			/* A blank window will be a label without any text */
			GtkWidget *label = gtk_label_new("");
			gtk_widget_show(label);
			
			win->widget = label;
			win->frame = label;
			/* You can print to a blank window's stream, but it does nothing */
			win->window_stream = window_stream_new(win);
			win->echo_stream = NULL;
		}
			break;
			
		case wintype_TextBuffer:
		{
			GtkWidget *scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
			GtkWidget *textview = gtk_text_view_new();
			GtkTextBuffer *textbuffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(textview) );

			gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW(textview), GTK_WRAP_WORD_CHAR );
			gtk_text_view_set_editable( GTK_TEXT_VIEW(textview), FALSE );

			gtk_container_add( GTK_CONTAINER(scrolledwindow), textview );
			gtk_widget_show_all(scrolledwindow);

			win->widget = textview;
			win->frame = scrolledwindow;
			win->window_stream = window_stream_new(win);
			win->echo_stream = NULL;
			win->input_request_type = INPUT_REQUEST_NONE;
			win->line_input_buffer = NULL;
			win->line_input_buffer_unicode = NULL;

			/* Connect signal handlers */
			win->keypress_handler = g_signal_connect( G_OBJECT(textview), "key-press-event", G_CALLBACK(on_window_key_press_event), win );
			g_signal_handler_block( G_OBJECT(textview), win->keypress_handler );

			win->insert_text_handler = g_signal_connect_after( G_OBJECT(textbuffer), "insert-text", G_CALLBACK(after_window_insert_text), win );
			g_signal_handler_block( G_OBJECT(textbuffer), win->insert_text_handler );

			/* Create an editable tag to indicate uneditable parts of the window
			(for line input) */
			gtk_text_buffer_create_tag(textbuffer, "uneditable", "editable", FALSE, "editable-set", TRUE, NULL);

			/* Mark the position where the user will input text */
			GtkTextIter end;
			gtk_text_buffer_get_end_iter(textbuffer, &end);
			gtk_text_buffer_create_mark(textbuffer, "input_position", &end, TRUE);
		}
			break;
			
		default:
			gdk_threads_leave();
			g_warning("%s: unsupported window type", __func__);
			g_free(win);
			return NULL;
	}

    /* Put the frame widget into our container */
    gtk_widget_set_parent(win->frame, GTK_WIDGET(glk_data->self));
    gtk_widget_queue_resize(GTK_WIDGET(glk_data->self));

	gdk_threads_leave();

	return win;
}

/**
 * glk_window_close:
 * @win: Window to close.
 * @result: Pointer to a #stream_result_t in which to store the write count.
 *
 * Closes @win, which is pretty much exactly the opposite of opening a window.
 * It is legal to close all your windows, or to close the root window (which is
 * the same thing.) 
 *
 * The @result argument is filled with the output character count of the window
 * stream.
 */
void
glk_window_close(winid_t win, stream_result_t *result)
{
	g_return_if_fail(win != NULL);

	switch(win->type)
	{
		case wintype_TextBuffer:
			gtk_widget_destroy( gtk_widget_get_parent(win->widget) );
			/* TODO: Cancel all input requests */
			break;

		case wintype_Blank:
			gtk_widget_destroy(win->widget);
			break;
	}

	stream_close_common(win->window_stream, result);

	g_node_destroy(win->window_node);
	/* TODO: iterate over child windows, closing them */

	g_free(win);
}

/**
 * glk_window_clear:
 * @win: A window.
 *
 * Erases the window @win. The meaning of this depends on the window type.
 *
 * <itemizedlist>
 *  <listitem><para>
 *   Text buffer: This may do any number of things, such as delete all text in 
 *   the window, or print enough blank lines to scroll all text beyond 
 *   visibility, or insert a page-break marker which is treated specially by the
 *   display part of the library.
 *  </para></listitem>
 *  <listitem><para>
 *   Text grid: This will clear the window, filling all positions with blanks.
 *   The window cursor is moved to the top left corner (position 0,0).
 *  </para></listitem>
 *  <listitem><para>
 *   Graphics: Clears the entire window to its current background color.
 *  </para></listitem>
 *  <listitem><para>
 *   Other window types: No effect. 
 *  </para></listitem>
 * </itemizedlist>
 *
 * It is illegal to erase a window which has line input pending. 
 */
void
glk_window_clear(winid_t win)
{
	g_return_if_fail(win != NULL);
	g_return_if_fail(win->input_request_type != INPUT_REQUEST_LINE && win->input_request_type != INPUT_REQUEST_LINE_UNICODE);
	
	switch(win->type)
	{
		case wintype_Blank:
			/* do nothing */
			break;
			
		case wintype_TextBuffer:
			/* delete all text in the window */
		{
			gdk_threads_enter();

			GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
			GtkTextIter start, end;
			gtk_text_buffer_get_bounds(buffer, &start, &end);
			gtk_text_buffer_delete(buffer, &start, &end);

			gdk_threads_leave();
		}
			break;
			
		default:
			g_warning("glk_window_clear: unsupported window type");
	}
}

/**
 * glk_set_window:
 * @win: A window.
 *
 * Sets the current stream to @win's window stream. It is exactly equivalent to
 * <informalexample><programlisting>
 *  glk_stream_set_current(glk_window_get_stream(win))
 * </programlisting></informalexample>
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
 * Returns the stream which is associated with @win. Every window has a stream
 * which can be printed to, but this may not be useful, depending on the window
 * type. (For example, printing to a blank window's stream has no effect.)
 *
 * Returns: The window stream.
 */
strid_t glk_window_get_stream(winid_t win)
{
	g_return_val_if_fail(win != NULL, NULL);
	return win->window_stream;
}

/**
 * glk_window_set_echo_stream:
 * @win: A window.
 * @str: A stream to attach to the window, or %NULL.
 *
 * Attaches the stream @str to @win as a second stream. Any text printed to the
 * window is also echoed to this second stream, which is called the window's
 * "echo stream."
 *
 * Effectively, any call to glk_put_char() (or the other output commands) which
 * is directed to @win's window stream, is replicated to @win's echo stream.
 * This also goes for the style commands such as glk_set_style().
 *
 * Note that the echoing is one-way. You can still print text directly to the
 * echo stream, and it will go wherever the stream is bound, but it does not
 * back up and appear in the window.
 *
 * It is illegal to set a window's echo stream to be its own window stream,
 * which would create an infinite loop. It is similarly illegal to create a
 * longer loop (two or more windows echoing to each other.)
 *
 * You can reset a window to stop echoing by setting @str to %NULL.
 */
void
glk_window_set_echo_stream(winid_t win, strid_t str)
{
	g_return_if_fail(win != NULL);
	
	/* Test for an infinite loop */
	strid_t next = str;
	for(; next && next->type == STREAM_TYPE_WINDOW; next = next->window->echo_stream)
	{
		if(next == win->window_stream)
		{
			g_warning("%s: Infinite loop detected", __func__);
			win->echo_stream = NULL;
			return;
		}
	}
	
	win->echo_stream = str;
}

/**
 * glk_window_get_echo_stream:
 * @win: A window.
 *
 * Returns the echo stream of window @win. If the window has no echo stream (as
 * is initially the case) then this returns %NULL.
 *
 * Returns: A stream, or %NULL.
 */
strid_t
glk_window_get_echo_stream(winid_t win)
{
	g_return_val_if_fail(win != NULL, NULL);
	return win->echo_stream;
}

/**
 * glk_window_get_size:
 * @win: A window.
 * @widthptr: Pointer to a location to store the window's width, or %NULL.
 * @heightptr: Pointer to a location to store the window's height, or %NULL.
 *
 * Simply returns the actual size of the window, in its measurement system.
 * Either @widthptr or @heightptr can be %NULL, if you only want one 
 * measurement. (Or, in fact, both, if you want to waste time.)
 */
void
glk_window_get_size(winid_t win, glui32 *widthptr, glui32 *heightptr)
{
	g_return_if_fail(win != NULL);

	/* TODO: Write this function */
	/* For a text buffer window: Return the number of rows and columns which
	would be available _if_ the window was filled with "0" (zero) characters in
	the "normal" font. */
	if(widthptr != NULL) {
		*widthptr = 0;
	}

	if(heightptr != NULL) {
		*heightptr = 0;
	}
}

/**
 * glk_window_move_cursor:
 * @win: A text grid window.
 * @xpos: Horizontal cursor position.
 * @ypos: Vertical cursor position.
 * 
 * Sets the cursor position. If you move the cursor right past the end of a 
 * line, it wraps; the next character which is printed will appear at the
 * beginning of the next line.
 * 
 * If you move the cursor below the last line, or when the cursor reaches the
 * end of the last line, it goes "off the screen" and further output has no
 * effect. You must call glk_window_move_cursor() or glk_window_clear() to move
 * the cursor back into the visible region.
 * 
 * <note><para>
 *  Note that the arguments of glk_window_move_cursor() are <type>unsigned 
 *  int</type>s. This is okay, since there are no negative positions. If you try
 *  to pass a negative value, Glk will interpret it as a huge positive value,
 *  and it will wrap or go off the last line.
 * </para></note>
 *
 * <note><para>
 *  Also note that the output cursor is not necessarily visible. In particular,
 *  when you are requesting line or character input in a grid window, you cannot
 *  rely on the cursor position to prompt the player where input is indicated.
 *  You should print some character prompt at that spot -- a ">" character, for
 *  example.
 * </para></note>
 */
void
glk_window_move_cursor(winid_t win, glui32 xpos, glui32 ypos)
{
	g_return_if_fail(win != NULL);
	g_return_if_fail(win->type == wintype_TextGrid);
	/* TODO: write this function */
}
