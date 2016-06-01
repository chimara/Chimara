#ifndef RESOURCE_H
#define RESOURCE_H

#include <glib.h>

#include "glk.h"
#include "gi_blorb.h"

#ifdef DEBUG
G_GNUC_INTERNAL void giblorb_print_contents(giblorb_map_t *map);
#endif

G_GNUC_INTERNAL const char * giblorb_get_error_message(giblorb_err_t err);

#endif
