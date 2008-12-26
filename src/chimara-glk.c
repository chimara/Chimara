/* licensing and copyright information here */

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <gmodule.h>
#include "chimara-glk.h"
#include "chimara-glk-private.h"
#include "glk.h"
#include "abort.h"
#include "window.h"

#define CHIMARA_GLK_MIN_WIDTH 0
#define CHIMARA_GLK_MIN_HEIGHT 0

typedef void (* glk_main_t) (void);

enum {
    PROP_0,
    PROP_INTERACTIVE,
    PROP_PROTECT
};

enum {
	STOPPED,
	STARTED,

	LAST_SIGNAL
};

static guint chimara_glk_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE(ChimaraGlk, chimara_glk, GTK_TYPE_CONTAINER);

static void
chimara_glk_init(ChimaraGlk *self)
{
    GTK_WIDGET_SET_FLAGS(GTK_WIDGET(self), GTK_NO_WINDOW);

    ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(self);
    
    priv->self = self;
    priv->interactive = TRUE;
    priv->protect = FALSE;
    priv->program = NULL;
    priv->thread = NULL;
    priv->event_queue = NULL;
    priv->event_lock = NULL;
    priv->event_queue_not_empty = NULL;
    priv->event_queue_not_full = NULL;
    priv->abort_lock = NULL;
    priv->abort_signalled = FALSE;
    priv->interrupt_handler = NULL;
    priv->root_window = NULL;
    priv->fileref_list = NULL;
    priv->current_stream = NULL;
    priv->stream_list = NULL;
}

static void
chimara_glk_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
    ChimaraGlk *glk = CHIMARA_GLK(object);
    
    switch(prop_id) 
    {
        case PROP_INTERACTIVE:
            chimara_glk_set_interactive(glk, g_value_get_boolean(value));
            break;
        case PROP_PROTECT:
            chimara_glk_set_protect(glk, g_value_get_boolean(value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
chimara_glk_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
    ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(object);
    
    switch(prop_id)
    {
        case PROP_INTERACTIVE:
            g_value_set_boolean(value, priv->interactive);
            break;
        case PROP_PROTECT:
            g_value_set_boolean(value, priv->protect);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
chimara_glk_finalize(GObject *object)
{
    ChimaraGlk *self = CHIMARA_GLK(object);
    ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(self);
    
    /* Free the event queue */
    g_mutex_lock(priv->event_lock);
	g_queue_foreach(priv->event_queue, (GFunc)g_free, NULL);
	g_queue_free(priv->event_queue);
	g_cond_free(priv->event_queue_not_empty);
	g_cond_free(priv->event_queue_not_full);
	priv->event_queue = NULL;
	g_mutex_unlock(priv->event_lock);
	g_mutex_free(priv->event_lock);
	
	/* Free the abort signalling mechanism */
	g_mutex_lock(priv->abort_lock);
	/* Make sure no other thread is busy with this */
	g_mutex_unlock(priv->abort_lock);
	g_mutex_free(priv->abort_lock);
	priv->abort_lock = NULL;

    G_OBJECT_CLASS(chimara_glk_parent_class)->finalize(object);
}

static void
chimara_glk_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
    g_return_if_fail(widget);
    g_return_if_fail(requisition);
    g_return_if_fail(CHIMARA_IS_GLK(widget));
    
    ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(widget);
    
    /* For now, just pass the size request on to the root Glk window */
    if(priv->root_window) { 
        GtkWidget *child = ((winid_t)(priv->root_window->data))->frame;
       if(GTK_WIDGET_VISIBLE(child))
            gtk_widget_size_request(child, requisition);
    } else {
        requisition->width = CHIMARA_GLK_MIN_WIDTH;
        requisition->height = CHIMARA_GLK_MIN_HEIGHT;
    }
}

static void
chimara_glk_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
    g_return_if_fail(widget);
    g_return_if_fail(allocation);
    g_return_if_fail(CHIMARA_IS_GLK(widget));
    
    ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(widget);
    
    widget->allocation = *allocation;
            
    if(priv->root_window) {
        GtkWidget *child = ((winid_t)(priv->root_window->data))->frame;
        if(GTK_WIDGET_VISIBLE(child))
            gtk_widget_size_allocate(child, allocation);
    }
}

static void
chimara_glk_forall(GtkContainer *container, gboolean include_internals,
    GtkCallback callback, gpointer callback_data)
{
    g_return_if_fail(container);
    g_return_if_fail(CHIMARA_IS_GLK(container));
    
    ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(container);
    
    if(priv->root_window) {
        GtkWidget *child = ((winid_t)(priv->root_window->data))->frame;
        (*callback)(child, callback_data);
    }
}

static void
chimara_glk_stopped(ChimaraGlk *self)
{
    ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(self);

    /* Free the plugin */
	if( priv->program && !g_module_close(priv->program) )
	    g_warning( "Error closing module: %s", g_module_error() );
}

static void
chimara_glk_started(ChimaraGlk *self)
{
	/* TODO: Add default signal handler implementation here */
}

static void
chimara_glk_class_init(ChimaraGlkClass *klass)
{
    /* Override methods of parent classes */
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->set_property = chimara_glk_set_property;
    object_class->get_property = chimara_glk_get_property;
    object_class->finalize = chimara_glk_finalize;
    
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    widget_class->size_request = chimara_glk_size_request;
    widget_class->size_allocate = chimara_glk_size_allocate;

    GtkContainerClass *container_class = GTK_CONTAINER_CLASS(klass);
    container_class->forall = chimara_glk_forall;

    /* Signals */
    klass->stopped = chimara_glk_stopped;
    klass->started = chimara_glk_started;
    chimara_glk_signals[STOPPED] = g_signal_new("stopped", 
        G_OBJECT_CLASS_TYPE(klass), 0, 
        G_STRUCT_OFFSET(ChimaraGlkClass, stopped), NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	chimara_glk_signals[STARTED] = g_signal_new ("started",
		G_OBJECT_CLASS_TYPE (klass), 0,
		G_STRUCT_OFFSET(ChimaraGlkClass, started), NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    /* Properties */
    GParamSpec *pspec;
    pspec = g_param_spec_boolean("interactive", _("Interactive"),
        _("Whether user input is expected in the Glk program"),
        TRUE,
        G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_LAX_VALIDATION |
        G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB);
    g_object_class_install_property(object_class, PROP_INTERACTIVE, pspec);
    pspec = g_param_spec_boolean("protect", _("Protected"),
        _("Whether the Glk program is barred from doing file operations"),
        FALSE,
        G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_LAX_VALIDATION |
        G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB);
    g_object_class_install_property(object_class, PROP_PROTECT, pspec);
    
    /* Private data */
    g_type_class_add_private(klass, sizeof(ChimaraGlkPrivate));
}

/* PUBLIC FUNCTIONS */

GtkWidget *
chimara_glk_new(void)
{
    ChimaraGlk *self = CHIMARA_GLK(g_object_new(CHIMARA_TYPE_GLK, NULL));
    ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(self);
    
    priv->event_queue = g_queue_new();
    priv->event_lock = g_mutex_new();
    priv->event_queue_not_empty = g_cond_new();
    priv->event_queue_not_full = g_cond_new();
    priv->abort_lock = g_mutex_new();
    
    return GTK_WIDGET(self);
}

void 
chimara_glk_set_interactive(ChimaraGlk *glk, gboolean interactive)
{
    g_return_if_fail(glk || CHIMARA_IS_GLK(glk));
    
    ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(glk);
    priv->interactive = interactive;
}

gboolean 
chimara_glk_get_interactive(ChimaraGlk *glk)
{
    g_return_val_if_fail(glk || CHIMARA_IS_GLK(glk), FALSE);
    
    ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(glk);
    return priv->interactive;
}

void 
chimara_glk_set_protect(ChimaraGlk *glk, gboolean protect)
{
    g_return_if_fail(glk || CHIMARA_IS_GLK(glk));
    
    ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(glk);
    priv->protect = protect;
}

gboolean 
chimara_glk_get_protect(ChimaraGlk *glk)
{
    g_return_val_if_fail(glk || CHIMARA_IS_GLK(glk), FALSE);
    
    ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(glk);
    return priv->protect;
}

/* glk_enter() is the actual function called in the new thread in which glk_main() runs.  */
static gpointer
glk_enter(gpointer glk_main)
{
    extern ChimaraGlkPrivate *glk_data;
    g_signal_emit_by_name(glk_data->self, "started");
	((glk_main_t)glk_main)();
	g_signal_emit_by_name(glk_data->self, "stopped");
	return NULL;
}

gboolean
chimara_glk_run(ChimaraGlk *glk, gchar *plugin, GError **error)
{
    g_return_val_if_fail(glk || CHIMARA_IS_GLK(glk), FALSE);
    g_return_val_if_fail(plugin, FALSE);
    
    ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(glk);
    
    /* Open the module to run */
    glk_main_t glk_main;
    g_assert( g_module_supported() );
    priv->program = g_module_open(plugin, G_MODULE_BIND_LAZY);
    
    if(!priv->program)
    {
        g_warning( "Error opening module: %s", g_module_error() );
        return FALSE;
    }
    if( !g_module_symbol(priv->program, "glk_main", (gpointer *) &glk_main) )
    {
        g_warning( "Error finding glk_main(): %s", g_module_error() );
        return FALSE;
    }

    extern ChimaraGlkPrivate *glk_data;
    /* Set the thread's private data */
    /* TODO: Do this with a GPrivate */
    glk_data = priv;
    
    /* Run in a separate thread */
	priv->thread = g_thread_create(glk_enter, glk_main, TRUE, error);
	
	return !(priv->thread == NULL);
}

void
chimara_glk_stop(ChimaraGlk *glk)
{
    g_return_if_fail(glk || CHIMARA_IS_GLK(glk));
    
    signal_abort();
}

void
chimara_glk_wait(ChimaraGlk *glk)
{
    ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(glk);
    g_thread_join(priv->thread);
}