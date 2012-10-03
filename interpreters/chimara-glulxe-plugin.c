#include <glib-object.h>
#include <libpeas/peas.h>
#include "chimara-glulxe-plugin.h"

G_DEFINE_DYNAMIC_TYPE(ChimaraGlulxePlugin, chimara_glulxe_plugin, PEAS_TYPE_EXTENSION_BASE);

G_MODULE_EXPORT void
peas_register_types(PeasObjectModule *module)
{
	chimara_glulxe_plugin_register_type(G_TYPE_MODULE(module));

	//peas_object_module_register_extension_type(module, PEAS_GTK_TYPE_CONFIGURABLE, CHIMARA_TYPE_GLULXE_PLUGIN);
}

static void
chimara_glulxe_plugin_init(ChimaraGlulxePlugin *self)
{
}

static void
chimara_glulxe_plugin_class_init(ChimaraGlulxePluginClass *klass)
{
}

static void
chimara_glulxe_plugin_class_finalize(ChimaraGlulxePluginClass *klass)
{
}