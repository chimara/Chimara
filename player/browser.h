#ifndef __BROWSER_H__
#define __BROWSER_H__

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CHIMARA_TYPE_BROWSER            (chimara_browser_get_type())
#define CHIMARA_BROWSER(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), CHIMARA_TYPE_BROWSER, ChimaraBrowser))
#define CHIMARA_BROWSER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), CHIMARA_TYPE_BROWSER, ChimaraBrowserClass))
#define CHIMARA_IS_BROWSER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), CHIMARA_TYPE_BROWSER))
#define CHIMARA_IS_BROWSER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), CHIMARA_TYPE_BROWSER))
#define CHIMARA_BROWSER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), CHIMARA_TYPE_BROWSER, ChimaraBrowserClass))

typedef struct _ChimaraBrowser {
	GtkWindow parent_instance;
	
	/* Public pointers */
} ChimaraBrowser;

typedef struct _ChimaraBrowserClass {
	GtkWindowClass parent_class;
} ChimaraBrowserClass;

GType chimara_browser_get_type(void) G_GNUC_CONST;
GtkWidget *chimara_browser_new(void);

G_END_DECLS

#endif /* __BROWSER_H__ */
