#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <glib.h>
#include <gtk/gtk.h>
#include <libchimara/chimara-glk.h>

G_GNUC_INTERNAL void preferences_create(GtkBuilder *builder, ChimaraGlk *glk);

#endif
