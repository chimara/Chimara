#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <glib.h>
#include <gtk/gtk.h>
#include "app.h"

G_GNUC_INTERNAL void preferences_create(ChimaraApp *theapp, GtkBuilder *builder);

#endif
