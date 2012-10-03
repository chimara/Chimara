#include <glib-object.h>
#include <libpeas/peas.h>
#include "chimara-git-plugin.h"

G_DEFINE_DYNAMIC_TYPE(ChimaraGitPlugin, chimara_git_plugin, PEAS_TYPE_EXTENSION_BASE);

G_MODULE_EXPORT void
peas_register_types(PeasObjectModule *module)
{
	chimara_git_plugin_register_type(G_TYPE_MODULE(module));

	//peas_object_module_register_extension_type(module, PEAS_GTK_TYPE_CONFIGURABLE, CHIMARA_TYPE_GIT_PLUGIN);
}

static void
chimara_git_plugin_init(ChimaraGitPlugin *self)
{
}

static void
chimara_git_plugin_class_init(ChimaraGitPluginClass *klass)
{
}

static void
chimara_git_plugin_class_finalize(ChimaraGitPluginClass *klass)
{
}