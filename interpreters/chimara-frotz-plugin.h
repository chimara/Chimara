#ifndef CHIMARA_FROTZ_PLUGIN_H
#define CHIMARA_FROTZ_PLUGIN_H

#include <glib.h>
#include <libpeas/peas.h>

G_BEGIN_DECLS

#define CHIMARA_TYPE_FROTZ_PLUGIN         (chimara_frotz_plugin_get_type())
#define CHIMARA_FROTZ_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST((o), CHIMARA_TYPE_FROTZ_PLUGIN, ChimaraFrotzPlugin))
#define CHIMARA_FROTZ_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), CHIMARA_TYPE_FROTZ_PLUGIN, ChimaraFrotzPlugin))
#define CHIMARA_IS_FROTZ_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE((o), CHIMARA_TYPE_FROTZ_PLUGIN))
#define CHIMARA_IS_FROTZ_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), CHIMARA_TYPE_FROTZ_PLUGIN))
#define CHIMARA_FROTZ_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), CHIMARA_TYPE_FROTZ_PLUGIN, ChimaraFrotzPluginClass))

typedef struct _ChimaraFrotzPlugin       ChimaraFrotzPlugin;
typedef struct _ChimaraFrotzPluginClass  ChimaraFrotzPluginClass;

struct _ChimaraFrotzPlugin {
  PeasExtensionBase parent_instance;
};

struct _ChimaraFrotzPluginClass {
  PeasExtensionBaseClass parent_class;
};

GType chimara_frotz_plugin_get_type(void) G_GNUC_CONST;
G_MODULE_EXPORT void peas_register_types (PeasObjectModule *module);

G_END_DECLS

#endif /* CHIMARA_FROTZ_PLUGIN_H */
