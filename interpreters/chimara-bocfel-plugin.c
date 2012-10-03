#include <glib-object.h>
#include <libpeas/peas.h>
#include "chimara-bocfel-plugin.h"

G_DEFINE_DYNAMIC_TYPE(ChimaraBocfelPlugin, chimara_bocfel_plugin, PEAS_TYPE_EXTENSION_BASE);

G_MODULE_EXPORT void
peas_register_types(PeasObjectModule *module)
{
	chimara_bocfel_plugin_register_type(G_TYPE_MODULE(module));

	//peas_object_module_register_extension_type(module, PEAS_GTK_TYPE_CONFIGURABLE, CHIMARA_TYPE_BOCFEL_PLUGIN);
}

static void
chimara_bocfel_plugin_init(ChimaraBocfelPlugin *self)
{
}

static void
chimara_bocfel_plugin_class_init(ChimaraBocfelPluginClass *klass)
{
}

static void
chimara_bocfel_plugin_class_finalize(ChimaraBocfelPluginClass *klass)
{
}