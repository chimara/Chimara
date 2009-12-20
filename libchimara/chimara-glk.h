/* Copyright / licensing information here. */

#ifndef __CHIMARA_GLK_H__
#define __CHIMARA_GLK_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <pango/pango.h>

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
typedef struct _ChimaraGlk {
	GtkContainer parent_instance;
    
	/*< public >*/
} ChimaraGlk;

typedef struct _ChimaraGlkClass {
	GtkContainerClass parent_class;
	/* Signals */
	void(* stopped) (ChimaraGlk *self);
	void(* started) (ChimaraGlk *self);
	void(* waiting) (ChimaraGlk *self);
	void(* char_input) (ChimaraGlk *self, guint32 window_rock, guint keysym);
	void(* line_input) (ChimaraGlk *self, guint32 window_rock, gchar *text);
	void(* text_buffer_output) (ChimaraGlk *self, guint32 window_rock, gchar *text);
} ChimaraGlkClass;

/**
 * ChimaraError:
 * 
 * Error codes returned by #ChimaraGlk widgets and subclasses.
 * <variablelist>
 * <varlistentry>
 *   <term>CHIMARA_LOAD_MODULE_ERROR</term>
 *   <listitem><para>There was an error opening the plugin containing the Glk
 *   program. The error message from <link 
 *   linkend="g-module-error">g_module_error()</link> is appended to the <link
 *   linkend="GError">GError</link> message.</para></listitem>
 * </varlistentry>
 * <varlistentry>
 *   <term>CHIMARA_NO_GLK_MAIN</term>
 *   <listitem><para>The plugin containing the Glk program did not export a 
 *   glk_main() function.</para></listitem>
 * </varlistentry>
 * <varlistentry>
 *   <term>CHIMARA_PLUGIN_NOT_FOUND</term>
 *   <listitem><para>An appropriate interpreter plugin for the autodetected
 *   game file type could not be found.</para></listitem>
 * </varlistentry>
 * <varlistentry>
 *   <term>CHIMARA_PLUGIN_ALREADY_RUNNING</term>
 *   <listitem><para>A plugin was opened while there was already another plugin
 *   running in the widget.</para></listitem>
 * </varlistentry>
 * </variablelist>
 */
typedef enum _ChimaraError {
	CHIMARA_LOAD_MODULE_ERROR,
	CHIMARA_NO_GLK_MAIN,
	CHIMARA_PLUGIN_NOT_FOUND,
	CHIMARA_PLUGIN_ALREADY_RUNNING
} ChimaraError;

/**
 * CHIMARA_ERROR:
 *
 * The domain of errors raised by Chimara widgets.
 */
#define CHIMARA_ERROR chimara_error_quark()

GQuark chimara_error_quark(void);
GType chimara_glk_get_type(void) G_GNUC_CONST;
GtkWidget *chimara_glk_new(void);
void chimara_glk_set_interactive(ChimaraGlk *glk, gboolean interactive);
gboolean chimara_glk_get_interactive(ChimaraGlk *glk);
void chimara_glk_set_protect(ChimaraGlk *glk, gboolean protect);
gboolean chimara_glk_get_protect(ChimaraGlk *glk);
void chimara_glk_set_default_font_description(ChimaraGlk *glk, PangoFontDescription *font);
void chimara_glk_set_default_font_string(ChimaraGlk *glk, const gchar *font);
PangoFontDescription *chimara_glk_get_default_font_description(ChimaraGlk *glk);
void chimara_glk_set_monospace_font_description(ChimaraGlk *glk, PangoFontDescription *font);
void chimara_glk_set_monospace_font_string(ChimaraGlk *glk, const gchar *font);
PangoFontDescription *chimara_glk_get_monospace_font_description(ChimaraGlk *glk);
void chimara_glk_set_spacing(ChimaraGlk *glk, guint spacing);
guint chimara_glk_get_spacing(ChimaraGlk *glk);
gboolean chimara_glk_run(ChimaraGlk *glk, const gchar *plugin, int argc, char *argv[], GError **error);
void chimara_glk_stop(ChimaraGlk *glk);
void chimara_glk_wait(ChimaraGlk *glk);
gboolean chimara_glk_get_running(ChimaraGlk *glk);
void chimara_glk_feed_char_input(ChimaraGlk *glk, guint32 keycode);
void chimara_glk_feed_line_input(ChimaraGlk *glk, const gchar *text);

G_END_DECLS

#endif /* __CHIMARA_GLK_H__ */
