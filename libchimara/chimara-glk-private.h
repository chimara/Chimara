#ifndef __CHIMARA_GLK_PRIVATE_H__
#define __CHIMARA_GLK_PRIVATE_H__

#include <glib.h>
#include <gmodule.h>

#include "chimara-glk.h"
#include "glk.h"
#include "gi_blorb.h"
#include "gi_dispa.h"

G_BEGIN_DECLS

#ifdef __has_feature
# if __has_feature(address_sanitizer)
#  define CHIMARA_ASAN_HACK 1
# endif
#else
# ifdef __SANITIZE_ADDRESS__
#  define CHIMARA_ASAN_HACK 1
# endif
#endif

typedef struct StyleSet {
	GHashTable *text_grid;
	GHashTable *text_buffer;
} StyleSet;

typedef struct _ChimaraGlkPrivate ChimaraGlkPrivate;

struct _ChimaraGlkPrivate {
    /* Pointer back to the widget itself for use in thread */
    ChimaraGlk *self;

	/* *** Widget properties *** */
    /* Whether user input is expected */
    gboolean interactive;
    /* Whether file operations are allowed */
    gboolean protect;
	/* Spacing between Glk windows */
	guint spacing;
	/* The CSS file to read style defaults from */
	gchar *css_file;
	/* Hashtable containing the current styles set by CSS and GLK */
	struct StyleSet *styles;
	struct StyleSet *glk_styles;
	/* Final message displayed when game exits */
	gchar *final_message;
	/* Image cache */
	GSList *image_cache;
	/* Size allocate flags */
	gboolean needs_rearrange;
	gboolean ignore_next_arrange_event;

	/* *** Threading data *** */
	/* Whether program is running */
	gboolean running;
	/* Whether widget has been finalized */
	gboolean after_finalize;
    /* Glk program loaded in widget */
    GModule *program;
    /* Thread in which Glk program is run */
    GThread *thread;
	/* Pipe through which to schedule updates to the UI */
	unsigned ui_message_handler_id;
	GAsyncQueue *ui_message_queue;
    /* Event queue and threading stuff */
    GQueue *event_queue;
	GMutex event_lock;
	GCond event_queue_not_empty;
	GCond event_queue_not_full;
    /* Abort mechanism */
	GMutex abort_lock;
    gboolean abort_signalled;
	/* Key press after shutdown mechanism */
	GMutex shutdown_lock;
	GCond shutdown_key_pressed;
	/* Window arrangement lock */
	GMutex arrange_lock;
	/* Input queues */
	GAsyncQueue *char_input_queue;
	GAsyncQueue *line_input_queue;
	/* Resource loading locks */
	GMutex resource_lock;
	GCond resource_loaded;
	GCond resource_info_available;
	guint32 resource_available;

	/* *** Glk library data *** */
	/* Info about current plugin */
	gchar *program_name;
	gchar *program_info;
	gchar *story_name;
    /* User-defined interrupt handler */
    void (*interrupt_handler)(void);
    /* Global tree of all windows */
    GNode *root_window;
    /* List of filerefs currently in existence */
    GList *fileref_list;
    /* Current stream */
    strid_t current_stream;
    /* List of streams currently in existence */
    GList *stream_list;
	/* List of sound channels currently in existence */
	GList *schannel_list;
	/* Current timer */
	guint timer_id;
	/* Current resource blorb map */
	giblorb_map_t *resource_map;
	/* File stream pointing to the blorb used as current resource map */
	strid_t resource_file;
	/* Optional callback for loading resource data */
	ChimaraResourceLoadFunc resource_load_callback;
	gpointer resource_load_callback_data;
	GDestroyNotify resource_load_callback_destroy_data;
	/* Callbacks for registering and unregistering dispatch objects */
	gidispatch_rock_t (*register_obj)(void *, glui32);
	void (*unregister_obj)(void *, glui32, gidispatch_rock_t);
	gidispatch_rock_t (*register_arr)(void *, glui32, char *);
	void (*unregister_arr)(void *, glui32, char *, gidispatch_rock_t);

	/* *** Platform-dependent Glk library data *** */
	/* Flag for functions to find out if they are being called from startup code */
	gboolean in_startup;
	/* "Current directory" for creating filerefs */
	gchar *current_dir;
};

G_GNUC_INTERNAL const char *chimara_glk_get_tag_name(unsigned style);
G_GNUC_INTERNAL const char *chimara_glk_get_glk_tag_name(unsigned style);

G_GNUC_INTERNAL void chimara_glk_set_story_name(ChimaraGlk *self, const char *story_name);
G_GNUC_INTERNAL void chimara_glk_push_event(ChimaraGlk *self, uint32_t type, winid_t win, uint32_t val1, uint32_t val2);
G_GNUC_INTERNAL void chimara_glk_init_textbuffer_styles(ChimaraGlk *self, ChimaraGlkWindowType wintype, GtkTextBuffer *buffer);
G_GNUC_INTERNAL GtkTextTag *chimara_glk_get_glk_tag(ChimaraGlk *self, ChimaraGlkWindowType window, const char *name);
G_GNUC_INTERNAL gboolean chimara_glk_needs_rearrange(ChimaraGlk *self);
G_GNUC_INTERNAL void chimara_glk_queue_arrange(ChimaraGlk *self, gboolean suppress_next_arrange_event);
G_GNUC_INTERNAL gboolean chimara_glk_process_queue(ChimaraGlk *self);
G_GNUC_INTERNAL void chimara_glk_drain_queue(ChimaraGlk *self);
G_GNUC_INTERNAL void chimara_glk_stop_processing_queue(ChimaraGlk *self);
G_GNUC_INTERNAL void chimara_glk_clear_shutdown(ChimaraGlk *self);

G_END_DECLS

#endif /* __CHIMARA_GLK_PRIVATE_H__ */
