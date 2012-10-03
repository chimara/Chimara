#ifndef CHIMARA_BOCFEL_PLUGIN_H
#define CHIMARA_BOCFEL_PLUGIN_H

#include <glib.h>
#include <libpeas/peas.h>

G_BEGIN_DECLS

#define CHIMARA_TYPE_BOCFEL_PLUGIN         (chimara_bocfel_plugin_get_type())
#define CHIMARA_BOCFEL_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST((o), CHIMARA_TYPE_BOCFEL_PLUGIN, ChimaraBocfelPlugin))
#define CHIMARA_BOCFEL_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), CHIMARA_TYPE_BOCFEL_PLUGIN, ChimaraBocfelPlugin))
#define CHIMARA_IS_BOCFEL_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE((o), CHIMARA_TYPE_BOCFEL_PLUGIN))
#define CHIMARA_IS_BOCFEL_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), CHIMARA_TYPE_BOCFEL_PLUGIN))
#define CHIMARA_BOCFEL_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), CHIMARA_TYPE_BOCFEL_PLUGIN, ChimaraBocfelPluginClass))

typedef struct _ChimaraBocfelPlugin       ChimaraBocfelPlugin;
typedef struct _ChimaraBocfelPluginClass  ChimaraBocfelPluginClass;

struct _ChimaraBocfelPlugin {
  PeasExtensionBase parent_instance;
};

struct _ChimaraBocfelPluginClass {
  PeasExtensionBaseClass parent_class;
};

GType chimara_bocfel_plugin_get_type(void) G_GNUC_CONST;
G_MODULE_EXPORT void peas_register_types (PeasObjectModule *module);

G_END_DECLS

#endif /* CHIMARA_BOCFEL_PLUGIN_H */
