#include <gtk/gtk.h>
#include <config.h>
#include "util.h"
#include "error.h"

const char *
get_data_file_path(const char *filename)
{
	char *path = g_build_filename(PACKAGE_DATA_DIR, filename);
	if(g_file_test(path, G_FILE_TEST_EXISTS))
		return path;
#ifdef DEBUG
	g_free(path);
	path = g_build_filename(PACKAGE_SRC_DIR, filename);
	if(g_file_test(path, G_FILE_TEST_EXISTS))
		return path;
#endif /* DEBUG */
	g_error("Could not find data file: %s", filename);
}

GtkBuilder *
new_builder_with_objects(char **object_ids)
{
	GError *error = NULL;
	GtkBuilder *builder = gtk_builder_new();
	
	if( !gtk_builder_add_objects_from_file(builder, PACKAGE_DATA_DIR "/chimara.ui", object_ids, &error) ) {
#ifdef DEBUG
		g_error_free(error);
		error = NULL;
		if( !gtk_builder_add_objects_from_file(builder, PACKAGE_SRC_DIR "/chimara.ui", object_ids, &error) )
#endif /* DEBUG */
			goto fail;
	}
	return builder;

fail:
	error_dialog(NULL, error, _("Error while building interface: "));
	return NULL;
}

GObject *
load_object(GtkBuilder *builder, const char *name)
{
	GObject *retval;
	if( (retval = gtk_builder_get_object(builder, name)) == NULL) {
		error_dialog(NULL, NULL, "Error while getting object '%s'", name);
		g_error("Error while getting object '%s'", name);
	}
	return retval;
}

GtkUIManager *
new_ui_manager(const char *filename)
{
	GError *error = NULL;
	GtkUIManager *uimanager = gtk_ui_manager_new();
	char *path = g_build_filename(PACKAGE_DATA_DIR, filename, NULL);
	
	if( !gtk_ui_manager_add_ui_from_file(uimanager, path, &error) ) {
#ifdef DEBUG
		g_free(path);
		path = g_build_filename(PACKAGE_SRC_DIR, filename, NULL);
		g_error_free(error);
		error = NULL;
		if( !gtk_ui_manager_add_ui_from_file(uimanager, path, &error) )
#endif /* DEBUG */
			goto fail;
	}
	return uimanager;

fail:
	error_dialog(NULL, error, _("Error while building interface: "));
	return NULL;
}

