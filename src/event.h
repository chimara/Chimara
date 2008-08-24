#ifndef EVENT_H
#define EVENT_H

#include <glib.h>
#include "glk.h"

#define EVENT_QUEUE_MAX_LENGTH (100)

/* Implementation-defined events */
#define EVENT_TYPE_QUIT (-1)

void events_init();
void events_free();

void get_event_lock();
void release_event_lock();

void event_throw(glui32 type, winid_t win, glui32 val1, glui32 val2);

#endif
