#ifndef STRIO_H
#define STRIO_H

#include <glib.h>

#include "glk.h"

G_GNUC_INTERNAL void flush_window_buffer(winid_t win);
G_GNUC_INTERNAL void queue_flush_window_buffer(winid_t win);

#endif
