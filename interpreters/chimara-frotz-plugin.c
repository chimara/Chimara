#include <glib-object.h>
#include <gtk/gtk.h>
#include <libpeas/peas.h>
#include <libpeas-gtk/peas-gtk.h>
#include "chimara-frotz-plugin.h"

static void chimara_frotz_plugin_configurable_init(PeasGtkConfigurableInterface *);
static GtkWidget *chimara_frotz_plugin_create_configure_widget(PeasGtkConfigurable *);

G_DEFINE_DYNAMIC_TYPE_EXTENDED(ChimaraFrotzPlugin, chimara_frotz_plugin, PEAS_TYPE_EXTENSION_BASE, 0,
	G_IMPLEMENT_INTERFACE_DYNAMIC(PEAS_GTK_TYPE_CONFIGURABLE, chimara_frotz_plugin_configurable_init));

G_MODULE_EXPORT void
peas_register_types(PeasObjectModule *module)
{
	chimara_frotz_plugin_register_type(G_TYPE_MODULE(module));
	peas_object_module_register_extension_type(module, PEAS_GTK_TYPE_CONFIGURABLE, CHIMARA_TYPE_FROTZ_PLUGIN);
}

static void
chimara_frotz_plugin_init(ChimaraFrotzPlugin *self)
{
}

static void
chimara_frotz_plugin_class_init(ChimaraFrotzPluginClass *klass)
{
}

static void
chimara_frotz_plugin_class_finalize(ChimaraFrotzPluginClass *klass)
{
}

static void
chimara_frotz_plugin_configurable_init(PeasGtkConfigurableInterface *iface)
{
	iface->create_configure_widget = chimara_frotz_plugin_create_configure_widget;
}

static GtkWidget *
chimara_frotz_plugin_create_configure_widget(PeasGtkConfigurable *self)
{
	return gtk_label_new("Configure Widget");
}
