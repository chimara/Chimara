#ifndef EVENT_H
#define EVENT_H

#include <glib.h>
#include "glk.h"
#include "chimara-glk.h"

#define EVENT_QUEUE_MAX_LENGTH (100)
#define evtype_Abort (-1)

G_GNUC_INTERNAL void event_throw(ChimaraGlk *glk, glui32 type, winid_t win, glui32 val1, glui32 val2);

#endif
