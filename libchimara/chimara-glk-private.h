#ifndef __CHIMARA_GLK_PRIVATE_H__
#define __CHIMARA_GLK_PRIVATE_H__

#include <glib.h>
#include <gmodule.h>
#include <pango/pango.h>
#include "glk.h"
#include "gi_blorb.h"
#include "chimara-glk.h"

G_BEGIN_DECLS

typedef struct _ChimaraGlkPrivate ChimaraGlkPrivate;

struct _ChimaraGlkPrivate {
    /* Pointer back to the widget itself for use in thread */
    ChimaraGlk *self;
    /* Whether user input is expected */
    gboolean interactive;
    /* Whether file operations are allowed */
    gboolean protect;
	/* Font description of proportional font */
	PangoFontDescription *default_font_desc;
	/* Font description of monospace font */
	PangoFontDescription *monospace_font_desc;
	/* Spacing between Glk windows */
	guint spacing;
    /* Glk program loaded in widget */
    GModule *program;
    /* Thread in which Glk program is run */
    GThread *thread;
    /* Event queue and threading stuff */
    GQueue *event_queue;
    GMutex *event_lock;
    GCond *event_queue_not_empty;
    GCond *event_queue_not_full;
    /* Abort mechanism */
    GMutex *abort_lock;
    gboolean abort_signalled;
	/* Window arrangement locks */
	GMutex *arrange_lock;
	GCond *rearranged;
	gboolean needs_rearrange;
	gboolean ignore_next_arrange_event;
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
	/* Current timer */
	guint timer_id;
	/* Current resource blorb map */
	giblorb_map_t *resource_map;
	/* File stream pointing to the blorb used as current resource map */
	strid_t resource_file;
};

#define CHIMARA_GLK_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE((obj), CHIMARA_TYPE_GLK, ChimaraGlkPrivate))
	
G_END_DECLS

#endif /* __CHIMARA_GLK_PRIVATE_H__ */
