/* Copyright / licensing information here. */

#ifndef __CHIMARA_GLK_H__
#define __CHIMARA_GLK_H__

#include <stdint.h>

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CHIMARA_TYPE_GLK            (chimara_glk_get_type())
#define CHIMARA_GLK(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), CHIMARA_TYPE_GLK, ChimaraGlk))
#define CHIMARA_GLK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), CHIMARA_TYPE_GLK, ChimaraGlkClass))
#define CHIMARA_IS_GLK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), CHIMARA_TYPE_GLK))
#define CHIMARA_IS_GLK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), CHIMARA_TYPE_GLK))
#define CHIMARA_GLK_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), CHIMARA_TYPE_GLK, ChimaraGlkClass))

/**
 * ChimaraGlk:
 * 
 * This structure contains no public members.
 */
typedef struct {
	GtkContainer parent_instance;
    
	/*< public >*/
} ChimaraGlk;

typedef struct {
	GtkContainerClass parent_class;
	/* Signals */
	void(* stopped) (ChimaraGlk *self);
	void(* started) (ChimaraGlk *self);
	void(* waiting) (ChimaraGlk *self);
	void(* char_input) (ChimaraGlk *self, guint32 window_rock, char *string_id, unsigned keysym);
	void(* line_input) (ChimaraGlk *self, guint32 window_rock, char *string_id, char *text);
	void(* text_buffer_output) (ChimaraGlk *self, guint32 window_rock, char *string_id, char *text);
	void(* iliad_screen_update) (ChimaraGlk *self, gboolean typing);
} ChimaraGlkClass;

/**
 * ChimaraGlkWindowType:
 * @CHIMARA_GLK_TEXT_BUFFER: The styles for text buffer windows.
 * @CHIMARA_GLK_TEXT_GRID: The styles for text grid windows.
 *
 * Specifies the type of windows for which to retrieve the style tag with
 * chimara_glk_get_tag().
 */
typedef enum {
	CHIMARA_GLK_TEXT_BUFFER,
	CHIMARA_GLK_TEXT_GRID
} ChimaraGlkWindowType;

/**
 * ChimaraError:
 * @CHIMARA_LOAD_MODULE_ERROR: There was an error opening the plugin containing
 * the Glk program.
 * The error message from g_module_error() is appended to the #GError message.
 * @CHIMARA_NO_GLK_MAIN: The plugin containing the Glk program did not export a 
 * glk_main() function.
 * @CHIMARA_PLUGIN_NOT_FOUND: An appropriate interpreter plugin for the 
 * autodetected game file type could not be found.
 * @CHIMARA_PLUGIN_ALREADY_RUNNING: A plugin was opened while there was already
 * another plugin running in the widget.
 * 
 * Error codes returned by #ChimaraGlk widgets and subclasses.
 */
typedef enum _ChimaraError {
	CHIMARA_LOAD_MODULE_ERROR,
	CHIMARA_NO_GLK_MAIN,
	CHIMARA_PLUGIN_NOT_FOUND,
	CHIMARA_PLUGIN_ALREADY_RUNNING
} ChimaraError;

/**
 * ChimaraResourceType:
 * @CHIMARA_RESOURCE_SOUND: A sound file.
 * @CHIMARA_RESOURCE_IMAGE: An image file.
 *
 * The type of resource that the Glk program is requesting, passed to a
 * #ChimaraResourceLoadFunc.
 */
typedef enum _ChimaraResourceType {
	CHIMARA_RESOURCE_SOUND,
	CHIMARA_RESOURCE_IMAGE
} ChimaraResourceType;

/**
 * ChimaraResourceLoadFunc:
 * @usage: A #ChimaraResourceType constant.
 * @resnum: The resource number to look for.
 * @user_data: A pointer to provide to the callback.
 *
 * The type of function passed to chimara_glk_set_resource_load_callback(). It
 * takes a #ChimaraResourceType constant, @usage, to indicate what sort of 
 * resource to look for; @resnum is the resource number to look for, and
 * @user_data is the user data provided along with the callback. The function
 * must return an allocated string containing the filename where the resource
 * can be found.
 */
typedef gchar * (*ChimaraResourceLoadFunc)(ChimaraResourceType usage, guint32 resnum, gpointer user_data);

/**
 * CHIMARA_ERROR:
 *
 * The domain of errors raised by Chimara widgets.
 */
#define CHIMARA_ERROR chimara_error_quark()

GQuark chimara_error_quark(void);
GType chimara_glk_get_type(void) G_GNUC_CONST;
GtkWidget *chimara_glk_new(void);
void chimara_glk_set_interactive(ChimaraGlk *self, gboolean interactive);
gboolean chimara_glk_get_interactive(ChimaraGlk *self);
void chimara_glk_set_protect(ChimaraGlk *self, gboolean protect);
gboolean chimara_glk_get_protect(ChimaraGlk *self);
void chimara_glk_set_css_to_default(ChimaraGlk *glk);
gboolean chimara_glk_set_css_from_file(ChimaraGlk *glk, const gchar *filename, GError **error);
void chimara_glk_set_css_from_string(ChimaraGlk *glk, const gchar *css);
void chimara_glk_set_spacing(ChimaraGlk *self, guint spacing);
guint chimara_glk_get_spacing(ChimaraGlk *self);
gboolean chimara_glk_run(ChimaraGlk *self, const gchar *plugin, int argc, char *argv[], GError **error);
gboolean chimara_glk_run_file(ChimaraGlk *self, GFile *plugin_file, int argc, char *argv[], GError **error);
void chimara_glk_stop(ChimaraGlk *self);
void chimara_glk_wait(ChimaraGlk *self);
void chimara_glk_unload_plugin(ChimaraGlk *self);
gboolean chimara_glk_get_running(ChimaraGlk *self);
void chimara_glk_feed_char_input(ChimaraGlk *self, uint32_t keyval);
void chimara_glk_feed_line_input(ChimaraGlk *self, const char *text);
gboolean chimara_glk_is_char_input_pending(ChimaraGlk *self);
gboolean chimara_glk_is_line_input_pending(ChimaraGlk *self);
GtkTextTag *chimara_glk_get_tag(ChimaraGlk *self, ChimaraGlkWindowType window, const char *name);
const char * const *chimara_glk_get_tag_names(ChimaraGlk *glk, unsigned *num_tags);
void chimara_glk_set_resource_load_callback(ChimaraGlk *self, ChimaraResourceLoadFunc func, void *user_data, GDestroyNotify destroy_user_data);

G_END_DECLS

#endif /* __CHIMARA_GLK_H__ */
