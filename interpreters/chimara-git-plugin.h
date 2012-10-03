#ifndef CHIMARA_GIT_PLUGIN_H
#define CHIMARA_GIT_PLUGIN_H

#include <glib.h>
#include <libpeas/peas.h>

G_BEGIN_DECLS

#define CHIMARA_TYPE_GIT_PLUGIN         (chimara_git_plugin_get_type())
#define CHIMARA_GIT_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST((o), CHIMARA_TYPE_GIT_PLUGIN, ChimaraGitPlugin))
#define CHIMARA_GIT_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), CHIMARA_TYPE_GIT_PLUGIN, ChimaraGitPlugin))
#define CHIMARA_IS_GIT_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE((o), CHIMARA_TYPE_GIT_PLUGIN))
#define CHIMARA_IS_GIT_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), CHIMARA_TYPE_GIT_PLUGIN))
#define CHIMARA_GIT_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), CHIMARA_TYPE_GIT_PLUGIN, ChimaraGitPluginClass))

typedef struct _ChimaraGitPlugin       ChimaraGitPlugin;
typedef struct _ChimaraGitPluginClass  ChimaraGitPluginClass;

struct _ChimaraGitPlugin {
  PeasExtensionBase parent_instance;
};

struct _ChimaraGitPluginClass {
  PeasExtensionBaseClass parent_class;
};

GType chimara_git_plugin_get_type(void) G_GNUC_CONST;
G_MODULE_EXPORT void peas_register_types (PeasObjectModule *module);

G_END_DECLS

#endif /* CHIMARA_GIT_PLUGIN_H */
