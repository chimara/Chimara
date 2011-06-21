#ifndef __UTIL_H__
#define __UTIL_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

const char *get_data_file_path(const char *filename);
GtkBuilder *new_builder_with_objects(char **object_ids);
GObject *load_object(GtkBuilder *builder, const char *name);
GtkUIManager *new_ui_manager(const char *filename);

G_END_DECLS

#endif /* __UTIL_H__ */
