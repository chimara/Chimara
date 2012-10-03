#include <glib-object.h>
#include <libpeas/peas.h>
#include "chimara-frotz-plugin.h"

G_DEFINE_DYNAMIC_TYPE(ChimaraFrotzPlugin, chimara_frotz_plugin, PEAS_TYPE_EXTENSION_BASE);

G_MODULE_EXPORT void
peas_register_types(PeasObjectModule *module)
{
	chimara_frotz_plugin_register_type(G_TYPE_MODULE(module));

	//peas_object_module_register_extension_type(module, PEAS_GTK_TYPE_CONFIGURABLE, CHIMARA_TYPE_BOCFEL_PLUGIN);
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