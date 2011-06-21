#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <glib.h>
#include <gtk/gtk.h>
#include "app.h"

G_BEGIN_DECLS

#define CHIMARA_TYPE_PREFS            (chimara_prefs_get_type())
#define CHIMARA_PREFS(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), CHIMARA_TYPE_PREFS, ChimaraPrefs))
#define CHIMARA_PREFS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), CHIMARA_TYPE_PREFS, ChimaraPrefsClass))
#define CHIMARA_IS_PREFS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), CHIMARA_TYPE_PREFS))
#define CHIMARA_IS_PREFS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), CHIMARA_TYPE_PREFS))
#define CHIMARA_PREFS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), CHIMARA_TYPE_PREFS, ChimaraPrefsClass))

typedef struct _ChimaraPrefs {
	GtkDialog parent_instance;
	
	/* Public pointers */
} ChimaraPrefs;

typedef struct _ChimaraPrefsClass {
	GtkDialogClass parent_class;
} ChimaraPrefsClass;

GType chimara_prefs_get_type(void) G_GNUC_CONST;
GtkWidget *chimara_prefs_new(void);
G_GNUC_INTERNAL void preferences_create(ChimaraApp *theapp, GtkBuilder *builder);

G_END_DECLS

#endif
