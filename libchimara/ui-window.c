#include <gtk/gtk.h>

#include "chimara-glk.h"
#include "chimara-glk-private.h"
#include "event.h"
#include "glk.h"
#include "input.h"
#include "ui-buffer.h"
#include "ui-graphics.h"
#include "ui-grid.h"
#include "ui-window.h"
#include "window.h"

/* Queues a size reallocation for the entire window hierarchy. If
 * @suppress_next_arrange_event is TRUE, an evtype_Arrange event will not be
 * sent back to the Glk thread as a result of this resize. */
void
ui_window_arrange(ChimaraGlk *glk, gboolean suppress_next_arrange_event)
{
	CHIMARA_GLK_USE_PRIVATE(glk, priv);
	priv->needs_rearrange = TRUE;
	priv->ignore_next_arrange_event = suppress_next_arrange_event;
	gtk_widget_queue_resize(GTK_WIDGET(priv->self));
}

/* Creates the widgets for and fills in the fields of @win.
 * Called as a result of glk_window_open(). */
void
ui_window_create(winid_t win, ChimaraGlk *glk)
{
	win->style_tagname = "normal";
	win->glk_style_tagname = "normal";

	switch(win->type) {
	case wintype_Blank:
		/* A blank window will be a label without any text */
		win->widget = win->frame = gtk_label_new("");
		gtk_widget_show(win->widget);
		break;

	case wintype_TextGrid:
		ui_grid_create(win, glk);
		break;

	case wintype_TextBuffer:
		ui_buffer_create(win, glk);
		break;

	case wintype_Graphics:
		ui_graphics_create(win);
		break;

	default:
		g_assert_not_reached();  /* This should not be called if type is wrong */
	}

	/* Set the minimum size to "as small as possible" so it doesn't depend on
	the size of the window contents */
	gtk_widget_set_size_request(win->widget, 0, 0);
	gtk_widget_set_size_request(win->frame, 0, 0);

	/* Set the window as a child of the Glk widget */
	gtk_widget_set_parent(win->frame, GTK_WIDGET(glk));
}

void
ui_window_clear(winid_t win)
{
	if(win->type == wintype_TextBuffer)
		ui_buffer_clear(win);
	else if(win->type == wintype_TextGrid)
		ui_grid_clear(win);
	else if(win->type == wintype_Graphics)
		ui_graphics_clear(win);
}

void
ui_window_override_font(winid_t win, GtkWidget *widget, PangoFontDescription *font)
{
	GtkStyleContext *style = gtk_widget_get_style_context(widget);

	if (win->font_override) {
		gtk_style_context_remove_provider(style, GTK_STYLE_PROVIDER(win->font_override));
	} else {
		win->font_override = gtk_css_provider_new();
	}

	char *font_string = pango_font_description_to_string(font);
	char *css = g_strdup_printf("*{ font: %s; }", font_string);

	GError *error = NULL;
	if (!gtk_css_provider_load_from_data(win->font_override, css, -1, &error)) {
		g_critical("Error overriding font to %s: %s", font_string, error->message);
		g_clear_error(&error);
	}

	g_free(font_string);
	g_free(css);

	gtk_style_context_add_provider(style, GTK_STYLE_PROVIDER(win->font_override),
		GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

void
ui_window_override_background_color(winid_t win, GtkWidget *widget, GdkRGBA *color)
{
	GtkStyleContext *style = gtk_widget_get_style_context(widget);

	if (win->background_override) {
		gtk_style_context_remove_provider(style, GTK_STYLE_PROVIDER(win->background_override));
	} else {
		win->background_override = gtk_css_provider_new();
	}

	char *color_string = gdk_rgba_to_string(color);
	char *css = g_strdup_printf("*{ background-color: %s; }", color_string);

	GError *error = NULL;
	if (!gtk_css_provider_load_from_data(win->background_override, css, -1, &error)) {
		g_critical("Error overriding background color to %s: %s", color_string, error->message);
		g_clear_error(&error);
	}

	g_free(color_string);
	g_free(css);

	gtk_style_context_add_provider(style, GTK_STYLE_PROVIDER(win->background_override),
		GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

/* Helper function: Turn off shutdown key-press-event signal handler */
static gboolean
turn_off_handler(GNode *node)
{
	winid_t win = node->data;
	g_signal_handler_block(win->widget, win->shutdown_keypress_handler);
	return FALSE; /* don't stop */
}

/* Callback for signal key-press-event while waiting for shutdown. */
gboolean
ui_window_handle_shutdown_key_press(GtkWidget *widget, GdkEventKey *event, winid_t win)
{
	ChimaraGlk *glk = CHIMARA_GLK(gtk_widget_get_ancestor(widget, CHIMARA_TYPE_GLK));
	g_assert(glk);
	CHIMARA_GLK_USE_PRIVATE(glk, priv);

	/* Turn off all the signal handlers */
	if(priv->root_window)
		g_node_traverse(priv->root_window, G_IN_ORDER, G_TRAVERSE_LEAVES, -1, (GNodeTraverseFunc)turn_off_handler, NULL);

	/* Signal the Glk library that it can shut everything down now */
	g_mutex_lock(&priv->shutdown_lock);
	g_cond_signal(&priv->shutdown_key_pressed);
	g_mutex_unlock(&priv->shutdown_lock);

	return GDK_EVENT_STOP;
}

void
ui_window_request_char_input(ChimaraGlk *glk, winid_t win, gboolean unicode)
{
	if(win->type == wintype_TextBuffer) {
		/* Move the input_position mark to the end of the window_buffer */
		GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
		GtkTextMark *input_position = gtk_text_buffer_get_mark(buffer, "input_position");
		GtkTextIter end_iter;
		gtk_text_buffer_get_end_iter(buffer, &end_iter);
		gtk_text_buffer_move_mark(buffer, input_position, &end_iter);
	}

	win->input_request_type = unicode? INPUT_REQUEST_CHARACTER_UNICODE : INPUT_REQUEST_CHARACTER;
	g_signal_handler_unblock( win->widget, win->char_input_keypress_handler );

	gtk_widget_grab_focus( GTK_WIDGET(win->widget) );

	/* Emit the "waiting" signal to let listeners know we are ready for input */
	g_signal_emit_by_name(glk, "waiting");
}

/* Internal function: General callback for signal key-press-event on a text
 * buffer or text grid window. Used in character input on both text buffers and
 * grids. Blocked when not in use. */
gboolean
ui_window_handle_char_input_key_press(GtkWidget *widget, GdkEventKey *event, winid_t win) {
	/* Ignore modifier keys, otherwise the char input will already trigger on
	the shift key when the user tries to type a capital letter */
	if(event->is_modifier)
		return GDK_EVENT_PROPAGATE;

	glui32 keycode = keyval_to_glk_keycode(event->keyval, win->input_request_type == INPUT_REQUEST_CHARACTER_UNICODE);

	ChimaraGlk *glk = CHIMARA_GLK(gtk_widget_get_ancestor(widget, CHIMARA_TYPE_GLK));
	g_assert(glk);
	event_throw(glk, evtype_CharInput, win, keycode, 0);
	g_signal_emit_by_name(glk, "char-input", win->rock, win->librock, event->keyval);

	/* Only one keypress will be handled */
	win->input_request_type = INPUT_REQUEST_NONE;
	g_signal_handler_block(win->widget, win->char_input_keypress_handler);

	return GDK_EVENT_STOP;
}

void
ui_window_cancel_char_input(winid_t win)
{
	if(win->input_request_type == INPUT_REQUEST_CHARACTER || win->input_request_type == INPUT_REQUEST_CHARACTER_UNICODE)
	{
		win->input_request_type = INPUT_REQUEST_NONE;
		g_signal_handler_block(win->widget, win->char_input_keypress_handler);
	}
}

void
ui_window_force_char_input(winid_t win, ChimaraGlk *glk, unsigned keyval)
{
	g_signal_emit_by_name(glk, "char-input", win->rock, win->librock, keyval);
}
