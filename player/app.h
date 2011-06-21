#ifndef __APP_H__
#define __APP_H__

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CHIMARA_TYPE_APP            (chimara_app_get_type())
#define CHIMARA_APP(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), CHIMARA_TYPE_APP, ChimaraApp))
#define CHIMARA_APP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), CHIMARA_TYPE_APP, ChimaraAppClass))
#define CHIMARA_IS_APP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), CHIMARA_TYPE_APP))
#define CHIMARA_IS_APP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), CHIMARA_TYPE_APP))
#define CHIMARA_APP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), CHIMARA_TYPE_APP, ChimaraAppClass))

typedef struct _ChimaraApp {
	GObject parent_instance;
	
	/* Public pointers */
	GtkWidget *browser_window;
	GtkWidget *aboutwindow;
	GtkWidget *prefswindow;
	/* Public settings */
	GSettings *prefs_settings;
	GSettings *state_settings;
} ChimaraApp;

typedef struct _ChimaraAppClass {
	GObjectClass parent_class;
} ChimaraAppClass;

GType chimara_app_get_type(void) G_GNUC_CONST;
ChimaraApp *chimara_app_get(void);
GtkActionGroup *chimara_app_get_action_group(ChimaraApp *self);

G_END_DECLS

#endif /* __APP_H__ */
