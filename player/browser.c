#include <glib-object.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include "browser.h"
#include "app.h"
#include "util.h"

typedef struct _ChimaraBrowserPrivate {
	int dummy;
} ChimaraBrowserPrivate;

#define CHIMARA_BROWSER_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), CHIMARA_TYPE_BROWSER, ChimaraBrowserPrivate))
#define CHIMARA_BROWSER_USE_PRIVATE ChimaraBrowserPrivate *priv = CHIMARA_BROWSER_PRIVATE(self)

G_DEFINE_TYPE(ChimaraBrowser, chimara_browser, GTK_TYPE_WINDOW);

static void
chimara_browser_finalize(GObject *self)
{
	/* Chain up */
	G_OBJECT_CLASS(chimara_browser_parent_class)->finalize(self);
}

static void
chimara_browser_class_init(ChimaraBrowserClass *klass)
{
	/* Override methods of parent classes */
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	//object_class->set_property = chimara_if_set_property;
	//object_class->get_property = chimara_if_get_property;
	object_class->finalize = chimara_browser_finalize;
	
	/* Signals */

	/* Properties */

	/* Private data */
	g_type_class_add_private(klass, sizeof(ChimaraBrowserPrivate));
}

static void
chimara_browser_init(ChimaraBrowser *self)
{
	ChimaraApp *theapp = chimara_app_get();
	GError *error = NULL;

	/* Set own properties */
	g_object_set(self,
		"title", _("Chimara"),
		NULL);

	GtkUIManager *uimanager = new_ui_manager("browser.menus");

	gtk_ui_manager_insert_action_group(uimanager, chimara_app_get_action_group(theapp), 0);
	GtkWidget *menubar = gtk_ui_manager_get_widget(uimanager, "/browser_menu");
	gtk_container_add(GTK_CONTAINER(self), menubar);
}

/* PUBLIC FUNCTIONS */
GtkWidget *
chimara_browser_new(void)
{
	return GTK_WIDGET(g_object_new(CHIMARA_TYPE_BROWSER,
		"type", GTK_WINDOW_TOPLEVEL,
		NULL));
}

