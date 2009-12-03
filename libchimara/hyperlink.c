#include "hyperlink.h"
#include "chimara-glk-private.h"
#include "magic.h"

extern GPrivate *glk_data_key;

/**
 * glk_set_hyperlink:
 * @linkval: Set to nonzero to initiate hyperlink mode. Set to zero to disengage.
 *
 * Use this function to create hyperlinks in a textbuffer. It sets the current stream
 * to hyperlink mode, after which text will become a hyperlink until hyperlink mode
 * is turned off. If the current stream does not write to a textbuffer window, this function
 * does nothing.
 *
 * You can request hyperlink events with glk_request_hyperlink_event() to react
 * to clicks on the link.
 */
void 
glk_set_hyperlink(glui32 linkval)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	g_return_if_fail(glk_data->current_stream != NULL);
	glk_set_hyperlink_stream(glk_data->current_stream, linkval);
}

/**
 * glk_set_hyperlink_stream:
 * @str: The stream to set the hyperlink mode on.
 * @linkval: Set to nonzero to initiate hyperlink mode. Set to zero to disengage.
 *
 * Use this function to create hyperlinks in a textbuffer. It sets a stream to a textbuffer
 * window to hyperlink mode, after which text will become a hyperlink until hyperlink mode
 * is turned off. Calling this function on a stream that does not write to a textbuffer does
 * nothing.
 *
 * You can request hyperlink events with glk_request_hyperlink_event() to react
 * to clicks on the link.
 */
void 
glk_set_hyperlink_stream(strid_t str, glui32 linkval)
{
	g_return_if_fail(str != NULL);
	g_return_if_fail(str->window != NULL);
	g_return_if_fail(str->window->type == wintype_TextBuffer);

	flush_window_buffer(str->window);

	if(linkval == 0) {
		/* Turn off hyperlink mode */
		str->hyperlink_mode = FALSE;
		str->window->current_hyperlink = NULL;
		return;
	}

	/* Check whether a tag with the needed value already exists */
	hyperlink_t *new_hyperlink = g_hash_table_lookup(str->window->hyperlinks, &linkval);
	if(new_hyperlink == NULL) {
		/* Create a new hyperlink with the requested value */
		new_hyperlink = g_new0(struct hyperlink, 1);
		new_hyperlink->value = linkval;
		new_hyperlink->tag = gtk_text_tag_new(NULL);
		new_hyperlink->event_handler = g_signal_connect( new_hyperlink->tag, "event", G_CALLBACK(on_hyperlink_clicked), new_hyperlink );
		g_signal_handler_block(new_hyperlink->tag, new_hyperlink->event_handler);
		new_hyperlink->window = str->window;

		/* Add the new tag to the tag table of the textbuffer */
		GtkTextBuffer *textbuffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(str->window->widget) );
		GtkTextTagTable *tags = gtk_text_buffer_get_tag_table(textbuffer);
		gtk_text_tag_table_add(tags, new_hyperlink->tag);

		printf("inserting link %d\n", linkval);

		gint *linkval_pointer = g_new0(gint, 1);
		*linkval_pointer = linkval;
		g_hash_table_insert(str->window->hyperlinks, linkval_pointer, new_hyperlink);
	}

	str->hyperlink_mode = TRUE;
	str->window->current_hyperlink = new_hyperlink;
}

/* Internal function used to iterate over all the hyperlinks, unblocking the event handler */
void
hyperlink_unblock_event_handler(gpointer key, gpointer value, gpointer user_data)
{
	hyperlink_t *link = (hyperlink_t *) value;
	g_signal_handler_unblock(link->tag, link->event_handler);
	printf("unblocking link %d\n", link->value);
}

/* Internal function used to iterate over all the hyperlinks, blocking the event handler */
void
hyperlink_block_event_handler(gpointer key, gpointer value, gpointer user_data)
{
	hyperlink_t *link = (hyperlink_t *) value;
	g_signal_handler_block(link->tag, link->event_handler);
}

void 
glk_request_hyperlink_event(winid_t win)
{
	VALID_WINDOW(win, return);
	g_return_if_fail(win != NULL);
	g_return_if_fail(win->type != wintype_TextBuffer || win->type != wintype_TextGrid);

	g_hash_table_foreach(win->hyperlinks, hyperlink_unblock_event_handler, NULL);

}

void 
glk_cancel_hyperlink_event(winid_t win)
{
	VALID_WINDOW(win, return);
	g_return_if_fail(win != NULL);
	g_return_if_fail(win->type != wintype_TextBuffer || win->type != wintype_TextGrid);

	g_hash_table_foreach(win->hyperlinks, hyperlink_block_event_handler, NULL);
}

gboolean
on_hyperlink_clicked(GtkTextTag *tag, GObject *object, GdkEvent *event, GtkTextIter *iter, hyperlink_t *link)
{
	ChimaraGlk *glk = CHIMARA_GLK(gtk_widget_get_ancestor(link->window->widget, CHIMARA_TYPE_GLK));
	g_assert(glk);

	if(event->type == GDK_BUTTON_PRESS) {
		event_throw(glk, evtype_Hyperlink, link->window, link->value, 0);
	}

	return FALSE;
}
