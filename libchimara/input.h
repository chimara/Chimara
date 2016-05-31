#ifndef INPUT_H
#define INPUT_H

#include <glib.h>

#include "glk.h"

G_GNUC_INTERNAL glui32 keyval_to_glk_keycode(guint keyval, gboolean unicode);
G_GNUC_INTERNAL void force_char_input_from_queue(winid_t win, event_t *event);
G_GNUC_INTERNAL void force_line_input_from_queue(winid_t win, event_t *event);
G_GNUC_INTERNAL gboolean is_valid_line_terminator(glui32 keycode);

#endif
