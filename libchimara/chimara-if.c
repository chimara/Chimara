#include <errno.h>
#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include <config.h>
#include <glib/gi18n-lib.h>
#include "chimara-if.h"
#include "chimara-glk.h"
#include "chimara-marshallers.h"
#include "init.h"

static gboolean supported_formats[CHIMARA_IF_NUM_FORMATS][CHIMARA_IF_NUM_INTERPRETERS] = {
	/* Frotz Nitfol Glulxe Git */
	{ TRUE,  TRUE,  FALSE, FALSE }, /* Z5 */
	{ TRUE,  TRUE,  FALSE, FALSE }, /* Z6 */
	{ TRUE,  TRUE,  FALSE, FALSE }, /* Z8 */
	{ TRUE,  TRUE,  FALSE, FALSE }, /* Zblorb */
	{ FALSE, FALSE, TRUE,  TRUE  }, /* Glulx */
	{ FALSE, FALSE, TRUE,  TRUE  }  /* Gblorb */
};
static gchar *format_names[CHIMARA_IF_NUM_FORMATS] = {
	N_("Z-code version 5"),
	N_("Z-code version 6"),
	N_("Z-code version 8"),
	N_("Blorbed Z-code"),
	N_("Glulx"),
	N_("Blorbed Glulx")
};
static gchar *interpreter_names[CHIMARA_IF_NUM_INTERPRETERS] = {
	N_("Frotz"), N_("Nitfol"), N_("Glulxe"), N_("Git")
};
static gchar *plugin_names[CHIMARA_IF_NUM_INTERPRETERS] = {
	"frotz", "nitfol", "glulxe", "git"
};

typedef struct _ChimaraIFPrivate {
	ChimaraIFInterpreter preferred_interpreter[CHIMARA_IF_NUM_FORMATS];
} ChimaraIFPrivate;

#define CHIMARA_IF_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), CHIMARA_TYPE_IF, ChimaraIFPrivate))
#define CHIMARA_IF_USE_PRIVATE(o, n) ChimaraIFPrivate *n = CHIMARA_IF_PRIVATE(o)

enum {
	PROP_0
};

enum {
	COMMAND,

	LAST_SIGNAL
};

static guint chimara_if_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE(ChimaraIF, chimara_if, CHIMARA_TYPE_GLK);

static void
chimara_if_init(ChimaraIF *self)
{
	CHIMARA_IF_USE_PRIVATE(self, priv);
	priv->preferred_interpreter[CHIMARA_IF_FORMAT_Z5] = CHIMARA_IF_INTERPRETER_FROTZ;
	priv->preferred_interpreter[CHIMARA_IF_FORMAT_Z6] = CHIMARA_IF_INTERPRETER_FROTZ;
	priv->preferred_interpreter[CHIMARA_IF_FORMAT_Z8] = CHIMARA_IF_INTERPRETER_FROTZ;
	priv->preferred_interpreter[CHIMARA_IF_FORMAT_Z_BLORB] = CHIMARA_IF_INTERPRETER_FROTZ;
	priv->preferred_interpreter[CHIMARA_IF_FORMAT_GLULX] = CHIMARA_IF_INTERPRETER_GLULXE;
	priv->preferred_interpreter[CHIMARA_IF_FORMAT_GLULX_BLORB] = CHIMARA_IF_INTERPRETER_GLULXE;
}

static void
chimara_if_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
    switch(prop_id)
    {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
chimara_if_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
    switch(prop_id)
    {

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
chimara_if_finalize(GObject *object)
{
    G_OBJECT_CLASS(chimara_if_parent_class)->finalize(object);
}

static void
chimara_if_command(ChimaraIF *self, gchar *input, gchar *response)
{
	/* Default signal handler */
}

static void
chimara_if_class_init(ChimaraIFClass *klass)
{
	/* Override methods of parent classes */
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->set_property = chimara_if_set_property;
	object_class->get_property = chimara_if_get_property;
	object_class->finalize = chimara_if_finalize;

	/* Signals */
	klass->command = chimara_if_command;
	/* Gtk-Doc for command */
	chimara_if_signals[COMMAND] = g_signal_new("command",
		G_OBJECT_CLASS_TYPE(klass), 0,
		G_STRUCT_OFFSET(ChimaraIFClass, command), NULL, NULL,
		chimara_marshal_VOID__STRING_STRING,
		G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_STRING);

	/* Properties */
	/* Gtk-Doc for property */
	/* g_object_class_install_property(object_class, PROPERTY, ...); */

	/* Private data */
	g_type_class_add_private(klass, sizeof(ChimaraIFPrivate));
}

/* PUBLIC FUNCTIONS */

/**
 * chimara_if_new:
 *
 * Creates and initializes a new #ChimaraIF widget.
 *
 * Return value: a #ChimaraIF widget, with a floating reference.
 */
GtkWidget *
chimara_if_new(void)
{
	/* This is a library entry point; initialize the library */
	chimara_init();
    return GTK_WIDGET(g_object_new(CHIMARA_TYPE_IF, NULL));
}

void
chimara_if_set_preferred_interpreter(ChimaraIF *self, ChimaraIFFormat format, ChimaraIFInterpreter interpreter)
{
	g_return_if_fail(self && CHIMARA_IS_IF(self));
	g_return_if_fail(format < CHIMARA_IF_NUM_FORMATS);
	g_return_if_fail(format < CHIMARA_IF_NUM_INTERPRETERS);

	CHIMARA_IF_USE_PRIVATE(self, priv);

	if(supported_formats[format][interpreter])
		priv->preferred_interpreter[format] = interpreter;
	else
		g_warning("Format '%s' is not supported by interpreter '%s'", format_names[format], interpreter_names[interpreter]);
}

ChimaraIFInterpreter
chimara_if_get_preferred_interpreter(ChimaraIF *self, ChimaraIFFormat format)
{
	g_return_val_if_fail(self && CHIMARA_IS_IF(self), -1);
	g_return_val_if_fail(format < CHIMARA_IF_NUM_FORMATS, -1);
	CHIMARA_IF_USE_PRIVATE(self, priv);
	return priv->preferred_interpreter[format];
}

/* Opens a '.la' file and finds the name of the plugin library. g_free() the
 * string when done. */
static gchar *
find_dlname(const gchar *pluginfile, GError **error)
{
	/* Find the name of the shared library */
	gchar *dlname;
	FILE *plugin = fopen(pluginfile, "r");
	if(!plugin)
	{
		g_set_error(error, G_FILE_ERROR, errno, "Error opening '%s': %s", pluginfile, g_strerror(errno));
		return NULL;
	}
	gchar *line = NULL;
	size_t buflen;
	ssize_t length;
	while((length = getline(&line, &buflen, plugin)) != -1)
	{	
		if(g_str_has_prefix(line, "dlname='"))
		{
			dlname = g_strndup(line + 8, length - 10);
			break;
		}
	}
	free(line);
	if(!dlname)
	{
		g_set_error(error, G_FILE_ERROR, errno, "Error reading '%s': %s", pluginfile, g_strerror(errno));
		return NULL;
	}
	if(fclose(plugin))
	{
		g_set_error(error, G_FILE_ERROR, errno, "Error closing '%s': %s", pluginfile, g_strerror(errno));
		return NULL;
	}
	return dlname;
}

gboolean 
chimara_if_run_game(ChimaraIF *self, gchar *gamefile, GError **error)
{
	g_return_val_if_fail(self && CHIMARA_IS_IF(self), FALSE);
	g_return_val_if_fail(gamefile, FALSE);
	
	CHIMARA_IF_USE_PRIVATE(self, priv);

	/* Find out what format the game is */
	/* TODO: Look inside the file instead of just looking at the extension */
	ChimaraIFFormat format = CHIMARA_IF_FORMAT_Z5;
	if(g_str_has_suffix(gamefile, ".z5"))
		format = CHIMARA_IF_FORMAT_Z5;
	else if(g_str_has_suffix(gamefile, ".z6"))
		format = CHIMARA_IF_FORMAT_Z6;
	else if(g_str_has_suffix(gamefile, ".z8"))
		format = CHIMARA_IF_FORMAT_Z8;
	else if(g_str_has_suffix(gamefile, ".zlb") || g_str_has_suffix(gamefile, ".zblorb"))
		format = CHIMARA_IF_FORMAT_Z_BLORB;
	else if(g_str_has_suffix(gamefile, ".ulx"))
		format = CHIMARA_IF_FORMAT_GLULX;
	else if(g_str_has_suffix(gamefile, ".blb") || g_str_has_suffix(gamefile, ".blorb") || g_str_has_suffix(gamefile, ".glb") || g_str_has_suffix(gamefile, ".gblorb"))
		format = CHIMARA_IF_FORMAT_GLULX_BLORB;
	
	/* Now decide what interpreter to use */
	ChimaraIFInterpreter interpreter = priv->preferred_interpreter[format];
	gchar *libtoolfile = g_strconcat(plugin_names[interpreter], ".la", NULL);
#ifndef RELEASE
	/* If there is a plugin in the source tree, use that */
	gboolean use_installed = FALSE;
	gchar *libtoolpath = g_build_filename("..", "interpreters", plugin_names[interpreter], libtoolfile, NULL);
	if( !g_file_test(libtoolpath, G_FILE_TEST_EXISTS) ) 
	{
		g_free(libtoolpath);
#endif
		gchar *libtoolpath = g_build_filename(PLUGINDIR, libtoolfile, NULL);
		if( !g_file_test(libtoolpath, G_FILE_TEST_EXISTS) ) 
		{
			g_free(libtoolpath);
			g_free(libtoolfile);
			g_warning("Cannot open %s plugin file", interpreter_names[interpreter]);
			/* TODO: set error */
			return FALSE;
		}
#ifndef RELEASE
		use_installed = TRUE;
	}
#endif
	g_free(libtoolfile);
	
	gchar *dlname = find_dlname(libtoolpath, error);
	g_free(libtoolpath);
	if(!dlname)
		return FALSE;
	gchar *pluginpath;
#ifndef RELEASE
	if(use_installed)
#endif
		pluginpath = g_build_filename(PLUGINDIR, dlname, NULL);
#ifndef RELEASE
	else
		pluginpath = g_build_filename("..", "interpreters", plugin_names[interpreter], LT_OBJDIR, dlname, NULL);
#endif
	char *argv[3] = { pluginpath, gamefile, NULL };
	gboolean retval = chimara_glk_run(CHIMARA_GLK(self), pluginpath, 2, argv, error);
	g_free(dlname);
	return retval;
}
