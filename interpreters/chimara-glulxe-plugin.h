#ifndef CHIMARA_GLULXE_PLUGIN_H
#define CHIMARA_GLULXE_PLUGIN_H

#include <glib.h>
#include <libpeas/peas.h>

G_BEGIN_DECLS

#define CHIMARA_TYPE_GLULXE_PLUGIN         (chimara_glulxe_plugin_get_type())
#define CHIMARA_GLULXE_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST((o), CHIMARA_TYPE_GLULXE_PLUGIN, ChimaraGlulxePlugin))
#define CHIMARA_GLULXE_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), CHIMARA_TYPE_GLULXE_PLUGIN, ChimaraGlulxePlugin))
#define CHIMARA_IS_GLULXE_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE((o), CHIMARA_TYPE_GLULXE_PLUGIN))
#define CHIMARA_IS_GLULXE_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), CHIMARA_TYPE_GLULXE_PLUGIN))
#define CHIMARA_GLULXE_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), CHIMARA_TYPE_GLULXE_PLUGIN, ChimaraGlulxePluginClass))

typedef struct _ChimaraGlulxePlugin       ChimaraGlulxePlugin;
typedef struct _ChimaraGlulxePluginClass  ChimaraGlulxePluginClass;

struct _ChimaraGlulxePlugin {
  PeasExtensionBase parent_instance;
};

struct _ChimaraGlulxePluginClass {
  PeasExtensionBaseClass parent_class;
};

GType chimara_glulxe_plugin_get_type(void) G_GNUC_CONST;
G_MODULE_EXPORT void peas_register_types (PeasObjectModule *module);

G_END_DECLS

#endif /* CHIMARA_GLULXE_PLUGIN_H */
