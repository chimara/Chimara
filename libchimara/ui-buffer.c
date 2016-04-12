#include "config.h"

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

#include "glk.h"
#include "input.h"
#include "pager.h"
#include "strio.h"
#include "style.h"
#include "ui-window.h"
#include "window.h"

/* Creates a text buffer window, filling in the fields of @win.
 * Called as a result of glk_window_open(). */
void
ui_buffer_create(winid_t win, ChimaraGlk *glk)
{
	win->frame = gtk_overlay_new();
	win->scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
	win->widget = gtk_text_view_new();
	win->pager = gtk_button_new_with_label(_("More"));
	GtkWidget *image = gtk_image_new_from_icon_name("go-down", GTK_ICON_SIZE_BUTTON);

	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(win->scrolledwindow), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );

	gtk_button_set_image( GTK_BUTTON(win->pager), image );
	gtk_widget_set_halign(win->pager, GTK_ALIGN_END);
	gtk_widget_set_valign(win->pager, GTK_ALIGN_END);
	gtk_widget_set_no_show_all(win->pager, TRUE);

	gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW(win->widget), GTK_WRAP_WORD_CHAR );
	gtk_text_view_set_editable( GTK_TEXT_VIEW(win->widget), FALSE );
	gtk_text_view_set_pixels_inside_wrap( GTK_TEXT_VIEW(win->widget), 3 );
	gtk_text_view_set_left_margin( GTK_TEXT_VIEW(win->widget), 20 );
	gtk_text_view_set_right_margin( GTK_TEXT_VIEW(win->widget), 20 );

	gtk_container_add( GTK_CONTAINER(win->scrolledwindow), win->widget );
	gtk_container_add( GTK_CONTAINER(win->frame), win->scrolledwindow );
	gtk_overlay_add_overlay( GTK_OVERLAY(win->frame), win->pager );
	gtk_widget_show_all(win->frame);

	/* Create the styles available to the window stream */
	GtkTextBuffer *screen = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
	ui_style_init_text_buffer(glk, screen);

	/* Set up the appropriate text buffer or text grid font */
	PangoFontDescription *font = ui_style_get_current_font(glk, win->type);
	gtk_widget_override_font(win->widget, font);
	calculate_zero_character_size(win->widget, font, &(win->unit_width), &(win->unit_height));
	pango_font_description_free(font);

	/* Pager */
	g_signal_connect_after( win->widget, "size-allocate", G_CALLBACK(pager_after_size_allocate), win );
	win->pager_keypress_handler = g_signal_connect( win->widget, "key-press-event", G_CALLBACK(pager_on_key_press_event), win );
	g_signal_handler_block(win->widget, win->pager_keypress_handler);
	GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment( GTK_SCROLLED_WINDOW(win->scrolledwindow) );
	win->pager_adjustment_handler = g_signal_connect_after(adj, "value-changed", G_CALLBACK(pager_after_adjustment_changed), win);
	g_signal_connect(win->pager, "clicked", G_CALLBACK(pager_on_clicked), win);

	/* Char and line input */
	win->char_input_keypress_handler = g_signal_connect( win->widget, "key-press-event", G_CALLBACK(on_char_input_key_press_event), win );
	g_signal_handler_block(win->widget, win->char_input_keypress_handler);
	win->line_input_keypress_handler = g_signal_connect( win->widget, "key-press-event", G_CALLBACK(on_line_input_key_press_event), win );
	g_signal_handler_block(win->widget, win->line_input_keypress_handler);
	win->insert_text_handler = g_signal_connect_after( screen, "insert-text", G_CALLBACK(after_window_insert_text), win );
	g_signal_handler_block(screen, win->insert_text_handler);

	/* Shutdown key press */
	win->shutdown_keypress_handler = g_signal_connect( win->widget, "key-press-event", G_CALLBACK(on_shutdown_key_press_event), win );
	g_signal_handler_block(win->widget, win->shutdown_keypress_handler);

	/* Create an editable tag to indicate uneditable parts of the window
	(for line input) */
	gtk_text_buffer_create_tag(screen, "uneditable", "editable", FALSE, "editable-set", TRUE, NULL);

	/* Mark the position where the user will input text and the end mark */
	GtkTextIter end;
	gtk_text_buffer_get_end_iter(screen, &end);
	gtk_text_buffer_create_mark(screen, "input_position", &end, TRUE);
	gtk_text_buffer_create_mark(screen, "end_position", &end, FALSE);

	/* Create the pager position mark; it stands for the last character in the buffer
	 that has been on-screen */
	gtk_text_buffer_create_mark(screen, "pager_position", &end, TRUE);
}

/* Clears the text buffer window @win by deleting all the text in it.
 * Called as a result of glk_window_clear(). */
void
ui_buffer_clear(winid_t win)
{
	GtkTextBuffer *screen = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
	GtkTextIter start, end;
	gtk_text_buffer_get_bounds(screen, &start, &end);
	gtk_text_buffer_delete(screen, &start, &end);
}

/* Cancels a pending line input event on a text buffer window @win.
 * Called as a result of glk_cancel_line_event(). */
int
ui_buffer_cancel_line_input(winid_t win)
{
	g_signal_handler_block(win->widget, win->line_input_keypress_handler);
	gtk_text_view_set_editable( GTK_TEXT_VIEW(win->widget), FALSE );
	GtkTextBuffer *screen = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
	g_signal_handler_block(screen, win->insert_text_handler);
	return ui_buffer_finish_line_input(win, FALSE);
}

/* Adds an image, stored in @pixbuf, to a text buffer window @win.
 * The @alignment parameter, an imagealign_ constant, specifies how to
 * vertically align the image relative to the text.
 * Called as a result of glk_image_draw(). */
void
ui_buffer_draw_image(winid_t win, GdkPixbuf *pixbuf, glui32 alignment)
{
	GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
	GtkTextIter end, start;
	gtk_text_buffer_get_end_iter(buffer, &end);

	gtk_text_buffer_insert_pixbuf(buffer, &end, pixbuf);
	start = end;
	gtk_text_iter_forward_char(&end);

	gint height = 0;
	switch(alignment) {
	case imagealign_InlineDown:
		height -= win->unit_height;
		break;
	case imagealign_InlineCenter:
		height = -win->unit_height / 2;
		break;
	case imagealign_InlineUp:
	default:
		height = 0;
	}

	if(height != 0) {
		GtkTextTag *tag = gtk_text_buffer_create_tag(buffer, NULL, "rise", PANGO_SCALE * (-height), NULL);
		gtk_text_buffer_apply_tag(buffer, tag, &start, &end);
	}
}
