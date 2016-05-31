#ifndef UI_TEXTWIN_H
#define UI_TEXTWIN_H

#include <glib.h>

#include "chimara-glk.h"
#include "glk.h"

G_GNUC_INTERNAL void ui_textwin_print_string(winid_t win, const char *text);
G_GNUC_INTERNAL void ui_textwin_request_line_input(ChimaraGlk *glk, winid_t win, glui32 maxlen, gboolean insert, const char *inserttext);
G_GNUC_INTERNAL int ui_textwin_finish_line_input(winid_t win, const char *inserted_text, gboolean emit_signal);
G_GNUC_INTERNAL int ui_textwin_cancel_line_input(winid_t win);
G_GNUC_INTERNAL int ui_textwin_force_line_input(winid_t win, const char *text);
G_GNUC_INTERNAL void ui_textwin_set_hyperlink(winid_t win, unsigned linkval);
G_GNUC_INTERNAL void ui_textwin_request_hyperlink_input(winid_t win);
G_GNUC_INTERNAL void ui_textwin_cancel_hyperlink_input(winid_t win);
G_GNUC_INTERNAL void ui_textwin_set_style(winid_t win, unsigned styl);
G_GNUC_INTERNAL void ui_textwin_set_zcolors(winid_t window, unsigned fg, unsigned bg);
G_GNUC_INTERNAL void ui_textwin_set_reverse_video(winid_t win, gboolean reverse);

#endif /* UI_TEXTWIN_H */
