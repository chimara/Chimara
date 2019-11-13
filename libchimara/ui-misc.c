#include "config.h"

#include <stdint.h>

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

#include "chimara-glk.h"
#include "glk.h"
#include "magic.h"

/* Parses a glk color to a GdkRGBA */
void
glkcolor_to_gdkrgba(uint32_t val, GdkRGBA *color)
{
	color->red = ((val & 0xff0000) >> 16) / 255.0;
	color->green = ((val & 0x00ff00) >> 8) / 255.0;
	color->blue = (val & 0x0000ff) / 255.0;
	color->alpha = 1.0;
}

/* Determine the size of a "0" character in pixels. The size of a "0" character
is how Glk measures its text grid and text buffer window sizes. Returns width
and height of the character in *width and *height. */
void
ui_calculate_zero_character_size(GtkWidget *widget, PangoFontDescription *font, int *width, int *height)
{
	PangoLayout *zero = gtk_widget_create_pango_layout(widget, "0");
	pango_layout_set_font_description(zero, font);
	pango_layout_get_pixel_size(zero, width, height);
	g_object_unref(zero);
}

/* Internal ChimaraGlk method: prompt whether to overwrite the file
 * @display_name.
 * Returns the GTK_RESPONSE constant given by the dialog. */
int
ui_confirm_file_overwrite(ChimaraGlk *glk, const char *display_name)
{
	GtkWidget *toplevel = gtk_widget_get_toplevel(GTK_WIDGET(glk));
	GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(toplevel), 0,
		GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
		"File '%s' already exists. Overwrite?", display_name);
	int response = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return response;
}

/* Prompts the user for a file to open or save to.
 * Called as a result of glk_fileref_create_by_prompt(). */
char *
ui_prompt_for_file(ChimaraGlk *glk, unsigned usage, unsigned fmode, const char *current_dir)
{
	GtkWindow *toplevel = GTK_WINDOW( gtk_widget_get_toplevel( GTK_WIDGET(glk) ) );
	GtkWidget *chooser;

	switch(fmode) {
	case filemode_Read:
		chooser = gtk_file_chooser_dialog_new("Select a file to open", toplevel,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			_("_Cancel"), GTK_RESPONSE_CANCEL,
			_("_Open"), GTK_RESPONSE_ACCEPT,
			NULL);
		gtk_file_chooser_set_action(GTK_FILE_CHOOSER(chooser), GTK_FILE_CHOOSER_ACTION_OPEN);
		break;
	case filemode_Write:
		chooser = gtk_file_chooser_dialog_new("Select a file to save to", toplevel,
			GTK_FILE_CHOOSER_ACTION_SAVE,
			_("_Cancel"), GTK_RESPONSE_CANCEL,
			_("_Save"), GTK_RESPONSE_ACCEPT,
			NULL);
		gtk_file_chooser_set_action(GTK_FILE_CHOOSER(chooser), GTK_FILE_CHOOSER_ACTION_SAVE);
		gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(chooser), TRUE);
		break;
	case filemode_ReadWrite:
	case filemode_WriteAppend:
		chooser = gtk_file_chooser_dialog_new("Select a file to save to", toplevel,
			GTK_FILE_CHOOSER_ACTION_SAVE,
			_("_Cancel"), GTK_RESPONSE_CANCEL,
			_("_Save"), GTK_RESPONSE_ACCEPT,
			NULL);
		gtk_file_chooser_set_action(GTK_FILE_CHOOSER(chooser), GTK_FILE_CHOOSER_ACTION_SAVE);
		break;
	default:
		ILLEGAL_PARAM("Unknown file mode: %u", fmode);
		return NULL;
	}

	/* Set up a file filter with suggested extensions */
	GtkFileFilter *filter = gtk_file_filter_new();
	switch(usage & fileusage_TypeMask) {
	case fileusage_Data:
		gtk_file_filter_set_name(filter, _("Data files (*.glkdata)"));
		gtk_file_filter_add_pattern(filter, "*.glkdata");
		break;
	case fileusage_SavedGame:
		gtk_file_filter_set_name(filter, _("Saved games (*.glksave)"));
		gtk_file_filter_add_pattern(filter, "*.glksave");
		break;
	case fileusage_InputRecord:
		gtk_file_filter_set_name(filter, _("Text files (*.txt)"));
		gtk_file_filter_add_pattern(filter, "*.txt");
		break;
	case fileusage_Transcript:
		gtk_file_filter_set_name(filter, _("Transcript files (*.txt)"));
		gtk_file_filter_add_pattern(filter, "*.txt");
		break;
	default:
		ILLEGAL_PARAM("Unknown file usage: %u", usage);
		return NULL;
	}
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), filter);

	/* Add a "text mode" filter for text files */
	if((usage & fileusage_TypeMask) == fileusage_InputRecord || (usage & fileusage_TypeMask) == fileusage_Transcript) {
		filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, _("All text files"));
		gtk_file_filter_add_mime_type(filter, "text/plain");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), filter);
	}

	/* Add another non-restricted filter */
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, _("All files"));
	gtk_file_filter_add_pattern(filter, "*");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), filter);

	if(current_dir)
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), current_dir);

	if(gtk_dialog_run( GTK_DIALOG(chooser) ) != GTK_RESPONSE_ACCEPT) {
		gtk_widget_destroy(chooser);
		return NULL;
	}
	char *filename = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER(chooser) );
	gtk_widget_destroy(chooser);

	return filename;
}
