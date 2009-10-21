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

typedef enum _ChimaraIFFlags {
	CHIMARA_IF_PIRACY_MODE = 1 << 0,
	CHIMARA_IF_TANDY_BIT = 1 << 1,
	CHIMARA_IF_EXPAND_ABBREVIATIONS = 1 << 2,
	CHIMARA_IF_IGNORE_ERRORS = 1 << 3,
	CHIMARA_IF_TYPO_CORRECTION = 1 << 4
} ChimaraIFFlags;

typedef struct _ChimaraIFPrivate {
	ChimaraIFInterpreter preferred_interpreter[CHIMARA_IF_NUM_FORMATS];
	ChimaraIFFormat format;
	ChimaraIFInterpreter interpreter;
	ChimaraIFFlags flags;
	ChimaraIFZmachineVersion interpreter_number;
	gint random_seed;
	gboolean random_seed_set;
	/* Holding buffers for input and response */
	gchar *input;
	GString *response;
} ChimaraIFPrivate;

#define CHIMARA_IF_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), CHIMARA_TYPE_IF, ChimaraIFPrivate))
#define CHIMARA_IF_USE_PRIVATE(o, n) ChimaraIFPrivate *n = CHIMARA_IF_PRIVATE(o)

enum {
	PROP_0,
	PROP_PIRACY_MODE,
	PROP_TANDY_BIT,
	PROP_EXPAND_ABBREVIATIONS,
	PROP_IGNORE_ERRORS,
	PROP_TYPO_CORRECTION,
	PROP_INTERPRETER_NUMBER,
	PROP_RANDOM_SEED,
	PROP_RANDOM_SEED_SET
};

enum {
	COMMAND,
	LAST_SIGNAL
};

static guint chimara_if_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE(ChimaraIF, chimara_if, CHIMARA_TYPE_GLK);

static void
chimara_if_waiting(ChimaraGlk *glk)
{
	CHIMARA_IF_USE_PRIVATE(glk, priv);

	gchar *response = g_string_free(priv->response, FALSE);
	priv->response = g_string_new("");
	
	g_signal_emit_by_name(glk, "command", priv->input, response);
	
	g_free(priv->input);
	g_free(response);
	priv->input = NULL;
}

static void
chimara_if_stopped(ChimaraGlk *glk)
{
	CHIMARA_IF_USE_PRIVATE(glk, priv);
	
	if(priv->input || priv->response->len > 0)
		chimara_if_waiting(glk); /* Send one last command signal */
	
	priv->format = CHIMARA_IF_FORMAT_NONE;
	priv->interpreter = CHIMARA_IF_INTERPRETER_NONE;
}

static void
chimara_if_line_input(ChimaraGlk *glk, guint32 win_rock, gchar *input)
{
	CHIMARA_IF_USE_PRIVATE(glk, priv);
	g_assert(priv->input == NULL);
	priv->input = g_strdup(input);
}

static void
chimara_if_text_buffer_output(ChimaraGlk *glk, guint32 win_rock, gchar *output)
{
	CHIMARA_IF_USE_PRIVATE(glk, priv);
	g_string_append(priv->response, output);
}

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
	priv->format = CHIMARA_IF_FORMAT_NONE;
	priv->interpreter = CHIMARA_IF_INTERPRETER_NONE;
	priv->flags = CHIMARA_IF_TYPO_CORRECTION;
	priv->interpreter_number = CHIMARA_IF_ZMACHINE_DEFAULT;
	priv->random_seed_set = FALSE;
	priv->input = NULL;
	priv->response = g_string_new("");
	
	/* Connect to signals of ChimaraGlk parent */
	g_signal_connect(self, "stopped", G_CALLBACK(chimara_if_stopped), NULL);
	g_signal_connect(self, "waiting", G_CALLBACK(chimara_if_waiting), NULL);
	g_signal_connect(self, "line-input", G_CALLBACK(chimara_if_line_input), NULL);
	g_signal_connect(self, "text-buffer-output", G_CALLBACK(chimara_if_text_buffer_output), NULL);
}

#define PROCESS_FLAG(flags, flag, val) (flags) = (val)? (flags) | (flag) : (flags) & ~(flag)

static void
chimara_if_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	CHIMARA_IF_USE_PRIVATE(object, priv);
    switch(prop_id)
    {
    	case PROP_PIRACY_MODE:
    		PROCESS_FLAG(priv->flags, CHIMARA_IF_PIRACY_MODE, g_value_get_boolean(value));
    		break;
    	case PROP_TANDY_BIT:
    		PROCESS_FLAG(priv->flags, CHIMARA_IF_TANDY_BIT, g_value_get_boolean(value));
    		break;
    	case PROP_EXPAND_ABBREVIATIONS:
    		PROCESS_FLAG(priv->flags, CHIMARA_IF_EXPAND_ABBREVIATIONS, g_value_get_boolean(value));
    		break;
    	case PROP_IGNORE_ERRORS:
    		PROCESS_FLAG(priv->flags, CHIMARA_IF_IGNORE_ERRORS, g_value_get_boolean(value));
    		break;
    	case PROP_TYPO_CORRECTION:
    		PROCESS_FLAG(priv->flags, CHIMARA_IF_TYPO_CORRECTION, g_value_get_boolean(value));
    		break;
    	case PROP_INTERPRETER_NUMBER:
    		priv->interpreter_number = g_value_get_uint(value);
    		break;
    	case PROP_RANDOM_SEED:
    		priv->random_seed = g_value_get_int(value);
    		break;
    	case PROP_RANDOM_SEED_SET:
    		priv->random_seed_set = g_value_get_boolean(value);
    		break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
chimara_if_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	CHIMARA_IF_USE_PRIVATE(object, priv);
    switch(prop_id)
    {
    	case PROP_PIRACY_MODE:
    		g_value_set_boolean(value, priv->flags & CHIMARA_IF_PIRACY_MODE);
    		break;
    	case PROP_TANDY_BIT:
    		g_value_set_boolean(value, priv->flags & CHIMARA_IF_TANDY_BIT);
    		break;
    	case PROP_EXPAND_ABBREVIATIONS:
    		g_value_set_boolean(value, priv->flags & CHIMARA_IF_EXPAND_ABBREVIATIONS);
    		break;
    	case PROP_IGNORE_ERRORS:
    		g_value_set_boolean(value, priv->flags & CHIMARA_IF_IGNORE_ERRORS);
    		break;
    	case PROP_TYPO_CORRECTION:
    		g_value_set_boolean(value, priv->flags & CHIMARA_IF_TYPO_CORRECTION);
    		break;
    	case PROP_INTERPRETER_NUMBER:
    		g_value_set_uint(value, priv->interpreter_number);
    		break;
    	case PROP_RANDOM_SEED:
    		g_value_set_int(value, priv->random_seed);
    		break;
    	case PROP_RANDOM_SEED_SET:
    		g_value_set_boolean(value, priv->random_seed_set);
    		break;
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

/* G_PARAM_STATIC_STRINGS only appeared in GTK 2.13.0 */
#ifndef G_PARAM_STATIC_STRINGS
#define G_PARAM_STATIC_STRINGS (G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB)
#endif

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
	g_object_class_install_property(object_class, PROP_PIRACY_MODE,
		g_param_spec_boolean("piracy-mode", _("Piracy mode"), 
		_("Pretend the game is pirated"), FALSE,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_LAX_VALIDATION | G_PARAM_STATIC_STRINGS));
	/**
	 * ChimaraIF:tandy-bit:
	 * 
	 * Some early Infocom games were sold by the Tandy Corporation. Setting this
	 * property to %TRUE changes the wording of some Version 3 Infocom games 
	 * slightly, so as to be less offensive. See <ulink 
	 * url="http://www.ifarchive.org/if-archive/infocom/info/tandy_bits.html">
	 * http://www.ifarchive.org/if-archive/infocom/info/tandy_bits.html</ulink>.
	 * 
	 * Only works on Z-machine interpreters.
	 */
	g_object_class_install_property(object_class, PROP_TANDY_BIT,
		g_param_spec_boolean("tandy-bit", _("Tandy bit"), 
		_("Censor certain Infocom games"), FALSE, 
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_LAX_VALIDATION | G_PARAM_STATIC_STRINGS));
	
	g_object_class_install_property(object_class, PROP_EXPAND_ABBREVIATIONS,
		g_param_spec_boolean("expand-abbreviations", _("Expand abbreviations"),
		_("Expand abbreviations such as X for EXAMINE"), FALSE,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_LAX_VALIDATION | G_PARAM_STATIC_STRINGS));
	
	g_object_class_install_property(object_class, PROP_IGNORE_ERRORS,
		g_param_spec_boolean("ignore-errors", _("Ignore errors"), 
		_("Do not warn the user about Z-machine errors"), FALSE,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_LAX_VALIDATION | G_PARAM_STATIC_STRINGS));
	
	g_object_class_install_property(object_class, PROP_TYPO_CORRECTION,
		g_param_spec_boolean("typo-correction", _("Typo correction"),
		_("Try to remedy typos if the interpreter supports it"), TRUE,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_LAX_VALIDATION | G_PARAM_STATIC_STRINGS));
	
	g_object_class_install_property(object_class, PROP_INTERPRETER_NUMBER,
		g_param_spec_uint("interpreter-number", _("Interpreter number"),
		_("Platform the Z-machine should pretend it is running on"), 
		CHIMARA_IF_ZMACHINE_DEFAULT, CHIMARA_IF_ZMACHINE_MAXVAL, CHIMARA_IF_ZMACHINE_DEFAULT,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_LAX_VALIDATION | G_PARAM_STATIC_STRINGS));
	
	g_object_class_install_property(object_class, PROP_RANDOM_SEED,
		g_param_spec_int("random-seed", _("Random seed"),
		_("Seed for the random number generator"), G_MININT, G_MAXINT, 0,
		G_PARAM_READWRITE | G_PARAM_LAX_VALIDATION | G_PARAM_STATIC_STRINGS));
		
	g_object_class_install_property(object_class, PROP_RANDOM_SEED_SET,
		g_param_spec_boolean("random-seed-set", _("Random seed set"),
		_("Whether the seed for the random number generator should be set manually"), FALSE,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_LAX_VALIDATION | G_PARAM_STATIC_STRINGS));

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
	gchar *pluginfile = g_strconcat(plugin_names[interpreter], "." G_MODULE_SUFFIX, NULL);

	/* If there is a plugin in the source tree, use that */
	gchar *pluginpath = g_build_filename("..", "interpreters", plugin_names[interpreter], LT_OBJDIR, pluginfile, NULL);
	if( !g_file_test(pluginpath, G_FILE_TEST_EXISTS) ) 
	{
		g_free(pluginpath);
		pluginpath = g_build_filename(PLUGINDIR, pluginfile, NULL);
		if( !g_file_test(pluginpath, G_FILE_TEST_EXISTS) ) 
		{
			g_free(pluginpath);
			g_free(pluginfile);
			g_set_error(error, CHIMARA_ERROR, CHIMARA_PLUGIN_NOT_FOUND, _("No appropriate %s interpreter plugin was found"), interpreter_names[interpreter]);
			return FALSE;
		}
	}
	g_free(pluginfile);

	/* Decide what arguments to pass to the interpreters; currently only the
	Z-machine interpreters accept command line arguments other than the game */
	GSList *args = NULL;
	gchar *terpnumstr = NULL, *randomstr = NULL;
	args = g_slist_prepend(args, pluginpath);
	args = g_slist_prepend(args, gamefile);
	switch(interpreter)
	{
		case CHIMARA_IF_INTERPRETER_FROTZ:
			if(priv->flags & CHIMARA_IF_PIRACY_MODE)
				args = g_slist_prepend(args, "-P");
			if(priv->flags & CHIMARA_IF_TANDY_BIT)
				args = g_slist_prepend(args, "-t");
			if(priv->flags & CHIMARA_IF_EXPAND_ABBREVIATIONS)
				args = g_slist_prepend(args, "-x");
			if(priv->flags & CHIMARA_IF_IGNORE_ERRORS)
				args = g_slist_prepend(args, "-i");
			if(priv->interpreter_number != CHIMARA_IF_ZMACHINE_DEFAULT)
			{
				terpnumstr = g_strdup_printf("-I%u", priv->interpreter_number);
				args = g_slist_prepend(args, terpnumstr);
			}
			if(priv->random_seed_set)
			{
				randomstr = g_strdup_printf("-s%d", priv->random_seed);
				args = g_slist_prepend(args, randomstr);
			}
			break;
		case CHIMARA_IF_INTERPRETER_NITFOL:
			if(priv->flags & CHIMARA_IF_PIRACY_MODE)
				args = g_slist_prepend(args, "-pirate");
			if(priv->flags & CHIMARA_IF_TANDY_BIT)
				args = g_slist_prepend(args, "-tandy");
			if(!(priv->flags & CHIMARA_IF_EXPAND_ABBREVIATIONS))
				args = g_slist_prepend(args, "-no-expand");
			if(priv->flags & CHIMARA_IF_IGNORE_ERRORS)
				args = g_slist_prepend(args, "-ignore");
			if(!(priv->flags & CHIMARA_IF_TYPO_CORRECTION))
				args = g_slist_prepend(args, "-no-spell");
			if(priv->interpreter_number != CHIMARA_IF_ZMACHINE_DEFAULT)
			{
				terpnumstr = g_strdup_printf("-terpnum%u", priv->interpreter_number);
				args = g_slist_prepend(args, terpnumstr);
			}
			if(priv->random_seed_set)
			{
				randomstr = g_strdup_printf("-random%d", priv->random_seed);
				args = g_slist_prepend(args, randomstr);
			}
			break;
		default:
			;
	}
	
	/* Allocate argv to hold the arguments */
	int argc = g_slist_length(args);
	args = g_slist_prepend(args, NULL);
	char **argv = g_new0(char *, argc + 1);
	
	/* Fill argv */
	args = g_slist_reverse(args);
	int count;
	GSList *ptr;
	for(count = 0, ptr = args; ptr; count++, ptr = g_slist_next(ptr))
		argv[count] = ptr->data;
	
	gboolean retval = chimara_glk_run(CHIMARA_GLK(self), pluginpath, argc, argv, error);
	g_free(argv);
	if(terpnumstr)
		g_free(terpnumstr);
	if(randomstr)
		g_free(randomstr);
	g_free(pluginpath);
	
	/* Set current format and interpreter if plugin was started successfully */
	if(retval)
	{
		priv->format = format;
		priv->interpreter = interpreter;
	}
	return retval;
}
