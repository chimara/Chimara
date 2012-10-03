#ifndef CHIMARA_NITFOL_PLUGIN_H
#define CHIMARA_NITFOL_PLUGIN_H

#include <glib.h>
#include <libpeas/peas.h>

G_BEGIN_DECLS

#define CHIMARA_TYPE_NITFOL_PLUGIN         (chimara_nitfol_plugin_get_type())
#define CHIMARA_NITFOL_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST((o), CHIMARA_TYPE_NITFOL_PLUGIN, ChimaraNitfolPlugin))
#define CHIMARA_NITFOL_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), CHIMARA_TYPE_NITFOL_PLUGIN, ChimaraNitfolPlugin))
#define CHIMARA_IS_NITFOL_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE((o), CHIMARA_TYPE_NITFOL_PLUGIN))
#define CHIMARA_IS_NITFOL_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), CHIMARA_TYPE_NITFOL_PLUGIN))
#define CHIMARA_NITFOL_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), CHIMARA_TYPE_NITFOL_PLUGIN, ChimaraNitfolPluginClass))

typedef struct _ChimaraNitfolPlugin       ChimaraNitfolPlugin;
typedef struct _ChimaraNitfolPluginClass  ChimaraNitfolPluginClass;

struct _ChimaraNitfolPlugin {
  PeasExtensionBase parent_instance;
};

struct _ChimaraNitfolPluginClass {
  PeasExtensionBaseClass parent_class;
};

GType chimara_nitfol_plugin_get_type(void) G_GNUC_CONST;
G_MODULE_EXPORT void peas_register_types (PeasObjectModule *module);

G_END_DECLS

#endif /* CHIMARA_NITFOL_PLUGIN_H */
