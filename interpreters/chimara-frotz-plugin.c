#include <glib-object.h>
#include <config.h>
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <libpeas/peas.h>
#include <libpeas-gtk/peas-gtk.h>
#include "chimara-frotz-plugin.h"
#include "frotz/frotz.h"

typedef struct _ChimaraFrotzPluginPrivate {
	int random_seed;
	gboolean random_seed_set;
	gboolean tandy_bit;
} ChimaraFrotzPluginPrivate;

#define CHIMARA_FROTZ_PLUGIN_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), CHIMARA_TYPE_FROTZ_PLUGIN, ChimaraFrotzPluginPrivate))
#define CHIMARA_FROTZ_PLUGIN_USE_PRIVATE ChimaraFrotzPluginPrivate *priv = CHIMARA_FROTZ_PLUGIN_PRIVATE(self)

static void chimara_frotz_plugin_configurable_init(PeasGtkConfigurableInterface *);
static GtkWidget *chimara_frotz_plugin_create_configure_widget(PeasGtkConfigurable *);

G_DEFINE_DYNAMIC_TYPE_EXTENDED(ChimaraFrotzPlugin, chimara_frotz_plugin, PEAS_TYPE_EXTENSION_BASE, 0,
	G_IMPLEMENT_INTERFACE_DYNAMIC(PEAS_GTK_TYPE_CONFIGURABLE, chimara_frotz_plugin_configurable_init));

enum {
	PROP_0,
	PROP_DEBUG_MESSAGES,
	PROP_IGNORE_ERRORS,
	PROP_PIRACY_MODE,
	PROP_QUETZAL_SAVE_FORMAT,
	PROP_TANDY_BIT,
	PROP_EXPAND_ABBREVIATIONS,
	PROP_RANDOM_SEED,
	PROP_RANDOM_SEED_SET,
	PROP_TRANSCRIPT_COLUMNS,
	PROP_UNDO_SLOTS
};

G_MODULE_EXPORT void
peas_register_types(PeasObjectModule *module)
{
	chimara_frotz_plugin_register_type(G_TYPE_MODULE(module));
	peas_object_module_register_extension_type(module, PEAS_GTK_TYPE_CONFIGURABLE, CHIMARA_TYPE_FROTZ_PLUGIN);
}

static void
chimara_frotz_plugin_init(ChimaraFrotzPlugin *self)
{
	CHIMARA_FROTZ_PLUGIN_USE_PRIVATE;
	priv->random_seed_set = FALSE;
	priv->tandy_bit = FALSE;
}

#define PROCESS_FLAG(name) ((flags & (name))? 1 : 0)

static void
chimara_frotz_plugin_set_property(GObject *self, unsigned prop_id, const GValue *value, GParamSpec *pspec)
{
	CHIMARA_FROTZ_PLUGIN_USE_PRIVATE;

	switch(prop_id) {
		case PROP_DEBUG_MESSAGES:
		{
			unsigned flags = g_value_get_uint(value);
			option_attribute_assignment = PROCESS_FLAG(CHIMARA_FROTZ_DEBUG_ATTRIBUTE_SETTING);
			option_attribute_testing = PROCESS_FLAG(CHIMARA_FROTZ_DEBUG_ATTRIBUTE_TESTING);
			option_object_movement = PROCESS_FLAG(CHIMARA_FROTZ_DEBUG_OBJECT_MOVEMENT);
			option_object_locating = PROCESS_FLAG(CHIMARA_FROTZ_DEBUG_OBJECT_LOCATING);
			g_object_notify(self, "debug-messages");
			break;
		}
		case PROP_IGNORE_ERRORS:
			option_ignore_errors = g_value_get_boolean(value);
			g_object_notify(self, "ignore-errors");
			break;
		case PROP_PIRACY_MODE:
			option_piracy = g_value_get_boolean(value);
			g_object_notify(self, "piracy-mode");
			break;
		case PROP_QUETZAL_SAVE_FORMAT:
			option_save_quetzal = g_value_get_boolean(value);
			g_object_notify(self, "quetzal-save-format");
			break;
		case PROP_TANDY_BIT:
			priv->tandy_bit = g_value_get_boolean(value);
			g_object_notify(self, "tandy-bit");
			break;
		case PROP_EXPAND_ABBREVIATIONS:
			option_expand_abbreviations = g_value_get_boolean(value);
			g_object_notify(self, "expand-abbreviations");
			break;
		case PROP_RANDOM_SEED:
			priv->random_seed = g_value_get_int(value);
			g_object_notify(self, "random-seed");
			break;
		case PROP_RANDOM_SEED_SET:
			priv->random_seed_set = g_value_get_boolean(value);
			g_object_notify(self, "random-seed-set");
			break;
		case PROP_TRANSCRIPT_COLUMNS:
			option_script_cols = g_value_get_uint(value);
			g_object_notify(self, "transcript-columns");
			break;
		case PROP_UNDO_SLOTS:
			option_undo_slots = g_value_get_uint(value);
			g_object_notify(self, "undo-slots");
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(self, prop_id, pspec);
	}
}

#undef PROCESS_FLAG

static void
chimara_frotz_plugin_get_property(GObject *self, unsigned prop_id, GValue *value, GParamSpec *pspec)
{
	CHIMARA_FROTZ_PLUGIN_USE_PRIVATE;

	switch(prop_id) {
		case PROP_DEBUG_MESSAGES:
		{
			unsigned flags = option_attribute_assignment << 0
				| option_attribute_testing << 1
				| option_object_movement << 2
				| option_object_locating << 3;
			g_value_set_uint(value, flags);
			break;
		}
		case PROP_IGNORE_ERRORS:
			g_value_set_boolean(value, option_ignore_errors);
			break;
		case PROP_PIRACY_MODE:
			g_value_set_boolean(value, option_piracy);
			break;
		case PROP_QUETZAL_SAVE_FORMAT:
			g_value_set_boolean(value, option_save_quetzal);
			break;
		case PROP_TANDY_BIT:
			g_value_set_boolean(value, priv->tandy_bit);
			break;
		case PROP_EXPAND_ABBREVIATIONS:
			g_value_set_boolean(value, option_expand_abbreviations);
			break;
		case PROP_RANDOM_SEED:
			g_value_set_int(value, priv->random_seed);
			break;
		case PROP_RANDOM_SEED_SET:
			g_value_set_boolean(value, priv->random_seed_set);
			break;
		case PROP_TRANSCRIPT_COLUMNS:
			g_value_set_uint(value, option_script_cols);
			break;
		case PROP_UNDO_SLOTS:
			g_value_set_uint(value, option_undo_slots);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(self, prop_id, pspec);
	}
}

static void
chimara_frotz_plugin_class_init(ChimaraFrotzPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->set_property = chimara_frotz_plugin_set_property;
	object_class->get_property = chimara_frotz_plugin_get_property;

	/* Private data */
	g_type_class_add_private(klass, sizeof(ChimaraFrotzPluginPrivate));

	/* Properties */
	/**
	 * ChimaraFrotzPlugin:debug-messages:
	 *
	 * Set of flags to control which debugging messages, if any, Frotz prints
	 * while interpreting the story. See #ChimaraFrotzDebugFlags.
	 */
	/* TODO: register a flags type and use g_param_spec_flags() */
	g_object_class_install_property(object_class, PROP_DEBUG_MESSAGES,
		g_param_spec_uint("debug-messages", _("Kinds of debugging messages"),
			_("Control which kinds of debugging messages to print"),
			0, 255, CHIMARA_FROTZ_DEBUG_NONE,
			G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_LAX_VALIDATION | G_PARAM_STATIC_STRINGS));
	/**
	 * ChimaraFrotzPlugin:ignore-errors:
	 *
	 * Setting this property to %TRUE will cause the interpreter to ignore
	 * fatal Z-machine runtime errors.
	 */
	g_object_class_install_property(object_class, PROP_IGNORE_ERRORS,
		g_param_spec_boolean("ignore-errors", _("Ignore errors"),
		_("Do not warn the user about fatal Z-machine errors"), FALSE,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_LAX_VALIDATION | G_PARAM_STATIC_STRINGS));
	/**
	 * ChimaraFrotzPlugin:piracy-mode:
	 *
	 * The Z-machine specification defines a facility for games to ask the
	 * interpreter they are running on whether this copy of the game is pirated.
	 * How the interpreter is supposed to magically determine that it is running
	 * pirate software is unclear, and so the majority of games and interpreters
	 * ignore this feature. Set this property to %TRUE if you want the
	 * interpreter to pretend it has detected a pirated game.
	 */
	g_object_class_install_property(object_class, PROP_PIRACY_MODE,
		g_param_spec_boolean("piracy-mode", _("Piracy mode"),
		_("Pretend the game is pirated"), FALSE,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_LAX_VALIDATION | G_PARAM_STATIC_STRINGS));
	/**
	 * ChimaraFrotzPlugin:quetzal-save-format:
	 *
	 * If set to %TRUE, use the newer-style Quetzal format for saved games.
	 * (This is the default.)
	 */
	g_object_class_install_property(object_class, PROP_QUETZAL_SAVE_FORMAT,
		g_param_spec_boolean("quetzal-save-format", _("Use Quetzal save format"),
			_("Use the Quetzal format for saved games"), TRUE,
			G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_LAX_VALIDATION | G_PARAM_STATIC_STRINGS));
	/**
	 * ChimaraFrotzPlugin:tandy-bit:
	 *
	 * Some early Infocom games were sold by the Tandy Corporation. Setting this
	 * property to %TRUE changes the wording of some Version 3 Infocom games
	 * slightly, so as to be less offensive. See <ulink
	 * url="http://www.ifarchive.org/if-archive/infocom/info/tandy_bits.html">
	 * http://www.ifarchive.org/if-archive/infocom/info/tandy_bits.html</ulink>.
	 *
	 * Only affects Z-machine interpreters.
	 */
	g_object_class_install_property(object_class, PROP_TANDY_BIT,
		g_param_spec_boolean("tandy-bit", _("Tandy bit"),
		_("Censor certain Infocom games"), FALSE,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_LAX_VALIDATION | G_PARAM_STATIC_STRINGS));
	/**
	 * ChimaraIF:expand-abbreviations:
	 *
	 * Most Z-machine games, in particular ones compiled with the Inform
	 * library, support the following one-letter abbreviations:
	 * <simplelist>
	 * <member>D &mdash; Down</member>
	 * <member>E &mdash; East</member>
	 * <member>G &mdash; aGain</member>
	 * <member>I &mdash; Inventory</member>
	 * <member>L &mdash; Look</member>
	 * <member>N &mdash; North</member>
	 * <member>O &mdash; Oops</member>
	 * <member>Q &mdash; Quit</member>
	 * <member>S &mdash; South</member>
	 * <member>U &mdash; Up</member>
	 * <member>W &mdash; West</member>
	 * <member>X &mdash; eXamine</member>
	 * <member>Y &mdash; Yes</member>
	 * <member>Z &mdash; wait (ZZZZ...)</member>
	 * </simplelist>
	 * Some early Infocom games might not recognize these abbreviations.
	 * However, Frotz can expand G, X, and Z regardless of what the game
	 * recognizes. Setting this property to %TRUE will cause Frotz to expand
	 * these abbreviations to the full words before passing the commands on to
	 * the game.
	 */
	g_object_class_install_property(object_class, PROP_EXPAND_ABBREVIATIONS,
		g_param_spec_boolean("expand-abbreviations", _("Expand abbreviations"),
		_("Expand abbreviations such as X for EXAMINE"), FALSE,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_LAX_VALIDATION | G_PARAM_STATIC_STRINGS));
	/**
	 * ChimaraFrotzPlugin:random-seed:
	 *
	 * If the #ChimaraFrotzPlugin:random-seed-set property is %TRUE, then the
	 * interpreter will use the value of this property as a seed for the random
	 * number generator. Use this feature to duplicate sequences of random
	 * numbers for testing games.
	 *
	 * Note that the value -1 will cause Frotz to pick an arbitrary seed even
	 * when #ChimaraFrotzPlugin:random-seed-set is %TRUE.
	 */
	g_object_class_install_property(object_class, PROP_RANDOM_SEED,
		g_param_spec_int("random-seed", _("Random seed"),
		_("Seed for the random number generator"), G_MININT, G_MAXINT, 0,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_LAX_VALIDATION | G_PARAM_STATIC_STRINGS));
	/**
	 * ChimaraFrotzPlugin:random-seed-set:
	 *
	 * Whether to use or ignore the #ChimaraFrotzPlugin:random-seed property.
	 */
	g_object_class_install_property(object_class, PROP_RANDOM_SEED_SET,
		g_param_spec_boolean("random-seed-set", _("Random seed set"),
		_("Whether the seed for the random number generator should be set manually"), FALSE,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_LAX_VALIDATION | G_PARAM_STATIC_STRINGS));
	/**
	 * ChimaraFrotzPlugin:transcript-columns:
	 *
	 * How many columns to make the transcript output.
	 */
	g_object_class_install_property(object_class, PROP_TRANSCRIPT_COLUMNS,
		g_param_spec_uint("transcript-columns", _("Transcript columns"),
			_("Number of columns for transcript output"),
			0, G_MAXUINT, 80,
			G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_LAX_VALIDATION | G_PARAM_STATIC_STRINGS));
	/**
	 * ChimaraFrotzPlugin:undo-slots:
	 *
	 * How many slots to reserve for multiple Undo commands.
	 */
	g_object_class_install_property(object_class, PROP_UNDO_SLOTS,
		g_param_spec_uint("undo-slots", _("Undo slots"),
			_("Number of slots to reserve for multiple undo"),
			0, MAX_UNDO_SLOTS, MAX_UNDO_SLOTS,
			G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_LAX_VALIDATION | G_PARAM_STATIC_STRINGS));
}

static void
chimara_frotz_plugin_class_finalize(ChimaraFrotzPluginClass *klass)
{
}

static void
chimara_frotz_plugin_configurable_init(PeasGtkConfigurableInterface *iface)
{
	iface->create_configure_widget = chimara_frotz_plugin_create_configure_widget;
}

/* Helper function to transform flags value to boolean; @data contains the
GINT_TO_POINTER()'ed but position (0=LSB). */
static gboolean
debug_message_flags_transform_to(GBinding *binding, const GValue *source, GValue *target, gpointer data)
{
	int bit_shift = GPOINTER_TO_INT(data);
	unsigned flags = g_value_get_uint(source);
	g_value_set_boolean(target, flags & (1 << bit_shift));
	return TRUE; /* success */
}

/* Reverse of debug_message_flags_transform_to(). */
static gboolean
debug_message_flags_transform_from(GBinding *binding, const GValue *source, GValue *target, gpointer data)
{
	int bit_shift = GPOINTER_TO_INT(data);
	unsigned flags = g_value_get_uint(target);
	int new_value = g_value_get_boolean(source)? 1 : 0;
	g_value_set_uint(target, flags & (new_value << bit_shift));
	return TRUE; /* success */
}

static GtkWidget *
chimara_frotz_plugin_create_configure_widget(PeasGtkConfigurable *self)
{
	GError *error = NULL;
	const char *datadir = peas_plugin_info_get_data_dir(peas_engine_get_plugin_info(peas_engine_get_default(), "frotz"));
	char *glade_file = g_build_filename(datadir, "chimara-frotz-plugin.glade", NULL);
	GtkBuilder *builder = gtk_builder_new();
	if(!gtk_builder_add_from_file(builder, glade_file, &error)) {
		g_free(glade_file);
		g_critical("Error building Frotz configuration dialog: %s\n", error->message);
		return NULL;
	}
	g_free(glade_file);
	GObject *retval = gtk_builder_get_object(builder, "frotz-configure-widget");

	/* Bind GUI widget properties to this plugin's configuration properties */
	g_object_bind_property_full(self, "debug-messages",
		gtk_builder_get_object(builder, "attribute-setting-button"), "active",
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL,
		debug_message_flags_transform_to,
		debug_message_flags_transform_from,
		GINT_TO_POINTER(0), NULL);
	g_object_bind_property_full(self, "debug-messages",
		gtk_builder_get_object(builder, "attribute-testing-button"), "active",
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL,
		debug_message_flags_transform_to,
		debug_message_flags_transform_from,
		GINT_TO_POINTER(1), NULL);
	g_object_bind_property_full(self, "debug-messages",
		gtk_builder_get_object(builder, "object-movement-button"), "active",
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL,
		debug_message_flags_transform_to,
		debug_message_flags_transform_from,
		GINT_TO_POINTER(2), NULL);
	g_object_bind_property_full(self, "debug-messages",
		gtk_builder_get_object(builder, "object-location-button"), "active",
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL,
		debug_message_flags_transform_to,
		debug_message_flags_transform_from,
		GINT_TO_POINTER(3), NULL);
	g_object_bind_property(self, "ignore-errors",
		gtk_builder_get_object(builder, "ignore-errors-button"), "active",
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(self, "piracy-mode",
		gtk_builder_get_object(builder, "piracy-button"), "active",
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(self, "quetzal-save-format",
		gtk_builder_get_object(builder, "quetzal-button"), "active",
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(self, "tandy-bit",
		gtk_builder_get_object(builder, "tandy-button"), "active",
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(self, "expand-abbreviations",
		gtk_builder_get_object(builder, "expand-abbreviations-button"), "active",
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(self, "random-seed",
		gtk_builder_get_object(builder, "random-seed-adjustment"), "value",
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(self, "random-seed-set",
		gtk_builder_get_object(builder, "random-seed-set-button"), "active",
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(self, "random-seed-set",
		gtk_builder_get_object(builder, "random-seed-button"), "sensitive",
		G_BINDING_SYNC_CREATE);
	g_object_bind_property(self, "transcript-columns",
		gtk_builder_get_object(builder, "columns-adjustment"), "value",
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(self, "undo-slots",
		gtk_builder_get_object(builder, "undo-adjustment"), "value",
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

	/* Make sure the widget is returned with only one reference */
	g_object_ref_sink(G_OBJECT(retval));
	g_object_unref(builder);
	return GTK_WIDGET(retval);
}
