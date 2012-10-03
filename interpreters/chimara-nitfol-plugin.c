#include <glib-object.h>
#include <libpeas/peas.h>
#include "chimara-nitfol-plugin.h"

G_DEFINE_DYNAMIC_TYPE(ChimaraNitfolPlugin, chimara_nitfol_plugin, PEAS_TYPE_EXTENSION_BASE);

G_MODULE_EXPORT void
peas_register_types(PeasObjectModule *module)
{
	chimara_nitfol_plugin_register_type(G_TYPE_MODULE(module));

	//peas_object_module_register_extension_type(module, PEAS_GTK_TYPE_CONFIGURABLE, CHIMARA_TYPE_NITFOL_PLUGIN);
}

static void
chimara_nitfol_plugin_init(ChimaraNitfolPlugin *self)
{
}

static void
chimara_nitfol_plugin_class_init(ChimaraNitfolPluginClass *klass)
{
}

static void
chimara_nitfol_plugin_class_finalize(ChimaraNitfolPluginClass *klass)
{
}