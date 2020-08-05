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

static const char *
enum_value_get_nick(GType enum_type, unsigned value)
{
	GEnumClass *enum_class = g_type_class_ref(enum_type);
	GEnumValue *enum_value = g_enum_get_value(enum_class, value);
	const char *retval = enum_value->value_nick;
	g_type_class_unref(enum_class);
	return retval;
}

static char *
pango_font_description_to_css(PangoFontDescription *font)
{
	GString *builder = g_string_new("*{");
	PangoFontMask mask = pango_font_description_get_set_fields(font);

	if (mask & PANGO_FONT_MASK_FAMILY)
		g_string_append_printf(builder, "font-family: %s;",
			pango_font_description_get_family(font));

	if (mask & PANGO_FONT_MASK_SIZE) {
		int size = pango_font_description_get_size(font) / PANGO_SCALE;
		const char *unit = pango_font_description_get_size_is_absolute(font)? "pt" : "px";
		g_string_append_printf(builder, "font-size: %d%s;", size, unit);
	}

	if (mask & PANGO_FONT_MASK_STYLE) {
		PangoStyle style = pango_font_description_get_style(font);
		g_string_append_printf(builder, "font-style: %s;",
			enum_value_get_nick(PANGO_TYPE_STYLE, style));
	}

	if (mask & PANGO_FONT_MASK_VARIANT) {
		PangoVariant variant = pango_font_description_get_variant(font);
		g_string_append_printf(builder, "font-variant: %s;",
			enum_value_get_nick(PANGO_TYPE_VARIANT, variant));
	}

	if (mask & PANGO_FONT_MASK_WEIGHT) {
		PangoWeight weight = pango_font_description_get_weight(font);
		unsigned val;
		/* CSS weights do not quite correspond to Pango weights */
		switch (weight) {
		case PANGO_WEIGHT_THIN:
		case PANGO_WEIGHT_ULTRALIGHT:
		case PANGO_WEIGHT_LIGHT:
		case PANGO_WEIGHT_NORMAL:
		case PANGO_WEIGHT_MEDIUM:
		case PANGO_WEIGHT_SEMIBOLD:
		case PANGO_WEIGHT_BOLD:
		case PANGO_WEIGHT_ULTRABOLD:
		case PANGO_WEIGHT_HEAVY:
			val = weight;
			break;
		case PANGO_WEIGHT_SEMILIGHT:
		case PANGO_WEIGHT_BOOK:
			val = 400;
			break;
		case PANGO_WEIGHT_ULTRAHEAVY:
			val = 900;
			break;
		default:
			val = CLAMP(weight - (weight % 100), 100, 900);
		}
		g_string_append_printf(builder, "font-weight: %u;", val);
	}

	if (mask & PANGO_FONT_MASK_STRETCH) {
		PangoStretch stretch = pango_font_description_get_stretch(font);
		g_string_append_printf(builder, "font-stretch: %s;",
			enum_value_get_nick(PANGO_TYPE_STRETCH, stretch));
	}

	g_string_append_c(builder, '}');
	return g_string_free(builder, FALSE);
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

	char *css = pango_font_description_to_css(font);

	GError *error = NULL;
	if (!gtk_css_provider_load_from_data(win->font_override, css, -1, &error)) {
		char *font_string = pango_font_description_to_string(font);
		g_critical("Error overriding font to %s: %s", font_string, error->message);
		g_free(font_string);
		g_clear_error(&error);
	}

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

/* Callback for signal key-press-event while waiting for shutdown. */
gboolean
ui_window_handle_shutdown_key_press(GtkWidget *widget, GdkEventKey *event, winid_t win)
{
	ChimaraGlk *glk = CHIMARA_GLK(gtk_widget_get_ancestor(widget, CHIMARA_TYPE_GLK));
	chimara_glk_clear_shutdown(glk);
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
	chimara_glk_push_event(glk, evtype_CharInput, win, keycode, 0);
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
