#include <gtk/gtk.h>
#include <pango/pango.h>

#include "chimara-glk.h"
#include "chimara-glk-private.h"
#include "glk.h"
#include "pager.h"
#include "strio.h"
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

/* Determine the size of a "0" character in pixels. The size of a "0" character
is how Glk measures its text grid and text buffer window sizes. Returns width
and height of the character in *width and *height. */
void
calculate_zero_character_size(GtkWidget *widget, PangoFontDescription *font, int *width, int *height)
{
	PangoLayout *zero = gtk_widget_create_pango_layout(widget, "0");
	pango_layout_set_font_description(zero, font);
	pango_layout_get_pixel_size(zero, width, height);
	g_object_unref(zero);
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

void
ui_window_set_style(winid_t win, unsigned styl)
{
	win->style_tagname = (char *) chimara_glk_get_tag_name(styl);
	win->glk_style_tagname = (char *) chimara_glk_get_glk_tag_name(styl);
}
