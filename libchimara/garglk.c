#include <libchimara/glk.h>
#include "chimara-glk-private.h"

extern GPrivate *glk_data_key;

void
apply_reverse_color(GtkTextTag *tag, gpointer data)
{
	g_object_set_data( G_OBJECT(tag), "reverse_color", data );
}

void 
garglk_set_reversevideo(glui32 reverse)
{
	printf("set_reversevideo(%d)\n", reverse);
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	g_return_if_fail(glk_data->current_stream != NULL);
	g_return_if_fail(glk_data->current_stream->window != NULL);

	GtkTextTagTable *tags = gtk_text_buffer_get_tag_table( GTK_TEXT_BUFFER(glk_data->current_stream->window->widget) );
	gtk_text_tag_table_foreach( tags, apply_reverse_color, GINT_TO_POINTER(reverse) );
}

