#include <sys/types.h>
#include <sys/stat.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>

/* Use a custom GSettings backend for our preferences file */
#define G_SETTINGS_ENABLE_BACKEND
#include <gio/gsettingsbackend.h>

#include "app.h"
#include "error.h"
#include "preferences.h"

typedef struct _ChimaraAppPrivate {
	GtkActionGroup *action_group;
} ChimaraAppPrivate;

#define CHIMARA_APP_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), CHIMARA_TYPE_APP, ChimaraAppPrivate))
#define CHIMARA_APP_USE_PRIVATE ChimaraAppPrivate *priv = CHIMARA_APP_PRIVATE(self)

G_DEFINE_TYPE(ChimaraApp, chimara_app, G_TYPE_OBJECT);

static void
chimara_app_finalize(GObject *self)
{
	CHIMARA_APP_USE_PRIVATE;
	g_object_unref(priv->action_group);
	
	/* Chain up */
	G_OBJECT_CLASS(chimara_app_parent_class)->finalize(self);
}

static void
chimara_app_class_init(ChimaraAppClass *klass)
{
	/* Override methods of parent classes */
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	//object_class->set_property = chimara_if_set_property;
	//object_class->get_property = chimara_if_get_property;
	object_class->finalize = chimara_app_finalize;
	
	/* Signals */

	/* Properties */

	/* Private data */
	g_type_class_add_private(klass, sizeof(ChimaraAppPrivate));
}

static GObject *
load_object(GtkBuilder *builder, const gchar *name)
{
	GObject *retval;
	if( (retval = gtk_builder_get_object(builder, name)) == NULL) {
		error_dialog(NULL, NULL, "Error while getting object '%s'", name);
		g_error("Error while getting object '%s'", name);
	}
	return retval;
}

static void
chimara_app_init(ChimaraApp *self)
{
	CHIMARA_APP_USE_PRIVATE;
	GError *error = NULL;

	/* Create configuration dir ~/.chimara */
	gchar *configdir = g_build_filename(g_get_home_dir(), ".chimara", NULL);
	if(!g_file_test(configdir, G_FILE_TEST_IS_DIR)
		&& g_mkdir(configdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
		g_error(_("Cannot create configuration directory ~/.chimara"));
	g_free(configdir);

	/* Initialize settings file; it can be overridden by a "chimara-config" file
	 in the current directory */
	gchar *keyfile;
	if(g_file_test("chimara-config", G_FILE_TEST_IS_REGULAR))
		keyfile = g_strdup("chimara-config");
	else
		keyfile = g_build_filename(g_get_home_dir(), ".chimara", "config", NULL);
	GSettingsBackend *backend = g_keyfile_settings_backend_new(keyfile, "/org/chimara-if/player/", NULL);
	self->prefs_settings = g_settings_new_with_backend("org.chimara-if.player.preferences", backend);
	self->state_settings = g_settings_new_with_backend("org.chimara-if.player.state", backend);
	g_free(keyfile);

	/* Build user interface */
	GtkBuilder *builder = gtk_builder_new();
	char *object_ids[] = {
		"app_group",
		"aboutwindow",
		"prefswindow",
		"available_interpreters",
		"interpreters",
		"style-list",
		NULL
	};
	
	if( !gtk_builder_add_objects_from_file(builder, PACKAGE_DATA_DIR "/chimara.ui", object_ids, &error) ) {
#ifdef DEBUG
		g_error_free(error);
		error = NULL;
		if( !gtk_builder_add_objects_from_file(builder, PACKAGE_SRC_DIR "/chimara.ui", object_ids, &error) ) {
#endif /* DEBUG */
			error_dialog(NULL, error, "Error while building interface: ");	
			return;
#ifdef DEBUG
		}
#endif /* DEBUG */
	}

	self->aboutwindow = GTK_WIDGET(load_object(builder, "aboutwindow"));
	self->prefswindow = GTK_WIDGET(load_object(builder, "prefswindow"));
	priv->action_group = GTK_ACTION_GROUP(load_object(builder, "app_group"));
	g_object_ref(priv->action_group);

	const gchar **ptr;
	GtkRecentFilter *filter = gtk_recent_filter_new();
	/* TODO: Use mimetypes and construct the filter dynamically depending on 
	what plugins are installed */
	const gchar *patterns[] = {
		"*.z[1-8]", "*.[zg]lb", "*.[zg]blorb", "*.ulx", "*.blb", "*.blorb", NULL
	};

	for(ptr = patterns; *ptr; ptr++)
		gtk_recent_filter_add_pattern(filter, *ptr);
	GtkRecentChooser *recent = GTK_RECENT_CHOOSER(load_object(builder, "recent"));
	gtk_recent_chooser_add_filter(recent, filter);

	/* Create preferences window */
	preferences_create(self, builder);

	gtk_builder_connect_signals(builder, self);

	g_object_unref(builder);
}

/* PUBLIC FUNCTIONS */

ChimaraApp *
chimara_app_get(void)
{
    static ChimaraApp *theapp = NULL;

    if(G_UNLIKELY(theapp == NULL))
    		theapp = CHIMARA_APP(g_object_new(CHIMARA_TYPE_APP, NULL));

    	return theapp;
}

GtkActionGroup *
chimara_app_get_action_group(ChimaraApp *self)
{
	CHIMARA_APP_USE_PRIVATE;
	return priv->action_group;
}

