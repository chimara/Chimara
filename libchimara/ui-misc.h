#ifndef UI_MISC_H
#define UI_MISC_H

#include <stdint.h>

#include <gtk/gtk.h>

#include "chimara-glk.h"

G_GNUC_INTERNAL void glkcolor_to_gdkrgba(uint32_t val, GdkRGBA *color);
G_GNUC_INTERNAL void ui_calculate_zero_character_size(GtkWidget *widget, PangoFontDescription *font, int *width, int *height);
G_GNUC_INTERNAL int ui_confirm_file_overwrite(ChimaraGlk *glk, const char *display_name);
G_GNUC_INTERNAL char *ui_prompt_for_file(ChimaraGlk *glk, unsigned usage, unsigned fmode, const char *current_dir);
G_GNUC_INTERNAL GtkTextTag *ui_text_tag_copy(GtkTextTag *tag);

#endif /* UI_MISC_H */
