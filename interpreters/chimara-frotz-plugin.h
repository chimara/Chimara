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

/**
 * ChimaraFrotzDebugFlags:
 * @CHIMARA_FROTZ_DEBUG_NONE: No debugging messages
 * @CHIMARA_FROTZ_DEBUG_ATTRIBUTE_SETTING: Print a debug message whenever a
 * Z-machine object attribute is set or cleared.
 * @CHIMARA_FROTZ_DEBUG_ATTRIBUTE_TESTING: Print a debug message whenever a
 * Z-machine object attribute is tested.
 * @CHIMARA_FROTZ_DEBUG_OBJECT_MOVEMENT: Print a debug message whenever a
 * Z-machine object is inserted into or removed from another object.
 * @CHIMARA_FROTZ_DEBUG_OBJECT_LOCATING: Print a debug message whenever the
 * location of a Z-machine object is checked.
 *
 * Controls what debugging messages Frotz should print. See the
 * :debug-messages property.
 */
typedef enum {
	CHIMARA_FROTZ_DEBUG_NONE = 0,
	CHIMARA_FROTZ_DEBUG_ATTRIBUTE_SETTING = 1 << 0,
	CHIMARA_FROTZ_DEBUG_ATTRIBUTE_TESTING = 1 << 1,
	CHIMARA_FROTZ_DEBUG_OBJECT_MOVEMENT = 1 << 2,
	CHIMARA_FROTZ_DEBUG_OBJECT_LOCATING = 1 << 3,
} ChimaraFrotzDebugFlags;

GType chimara_frotz_plugin_get_type(void) G_GNUC_CONST;
G_MODULE_EXPORT void peas_register_types (PeasObjectModule *module);

G_END_DECLS

#endif /* CHIMARA_FROTZ_PLUGIN_H */
