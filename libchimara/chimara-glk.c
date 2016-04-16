/* licensing and copyright information here */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <gtk/gtk.h>
#include <config.h>
#include <glib/gi18n-lib.h>
#include <gmodule.h>
#include <pango/pango.h>
#include <gio/gio.h>
#include "chimara-glk.h"
#include "chimara-glk-private.h"
#include "chimara-marshallers.h"
#include "glk.h"
#include "abort.h"
#include "stream.h"
#include "window.h"
#include "glkstart.h"
#include "glkunix.h"
#include "init.h"
#include "magic.h"
#include "style.h"

#define CHIMARA_GLK_MIN_WIDTH 0
#define CHIMARA_GLK_MIN_HEIGHT 0

/**
 * SECTION:chimara-glk
 * @short_description: Widget which executes a Glk program
 * @stability: Unstable
 * 
 * The #ChimaraGlk widget opens and runs a Glk program. The program must be
 * compiled as a plugin module, with a function `glk_main()` that the Glk
 * library can hook into.
 *
 * On Linux systems, this is a file with a name like `plugin.so`.
 * For portability, you can use libtool and automake:
 * |[<!--language="automake"-->
 * pkglib_LTLIBRARIES = plugin.la
 * plugin_la_SOURCES = plugin.c foo.c bar.c
 * plugin_la_LDFLAGS = -module -shared -avoid-version -export-symbols-regex "^glk_main$$"
 * ]|
 * This will produce `plugin.la` which is a text file containing the correct
 * plugin file to open (see the relevant section of the
 * <ulink 
 * url="http://www.gnu.org/software/libtool/manual/html_node/Finding-the-dlname.html">
 * Libtool manual</ulink>).
 *
 * You need to initialize GDK threading in any program you use a #ChimaraGlk
 * widget in.
 * This means calling gdk_threads_init() at the beginning of your program,
 * <emphasis>before</emphasis> the call to gtk_init().
 * In addition to this, you must also protect your call to gtk_main() by calling
 * gdk_threads_enter() right before it, and gdk_threads_leave() right after it.
 *
 * The following sample program shows how to initialize and construct a simple 
 * GTK window that runs a Glk program:
 * |[<!--language="C"-->
 * #include <glib.h>
 * #include <gtk/gtk.h>
 * #include <libchimara/chimara-glk.h>
 *
 * int
 * main(int argc, char *argv[])
 * {
 *     GtkWidget *window, *glk;
 *     GError *error = NULL;
 *     gchar *plugin_argv[] = { "plugin.so", "-option" };
 *
 *     // Initialize threads and GTK
 *     gdk_threads_init();
 *     gtk_init(&argc, &argv);
 *
 *     // Construct the window and its contents. We quit the GTK main loop
 *     // when the window's close button is clicked.
 *     window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
 *     g_signal_connect(window, "delete-event", G_CALLBACK(gtk_main_quit), NULL);
 *     glk = chimara_glk_new();
 *     gtk_container_add(GTK_CONTAINER(window), glk);
 *     gtk_widget_show_all(window);
 *
 *     // Add a reference to the ChimaraGlk widget, since we want it to
 *     // persist after the window's delete-event -- otherwise it will be destroyed
 *     // with the window.
 *     g_object_ref(glk);
 *
 *     // Start the Glk program in a separate thread
 *     if(!chimara_glk_run(CHIMARA_GLK(glk), "./plugin.so", 2, plugin_argv, &error))
 *         g_error("Error starting Glk library: %s\n", error->message);
 *
 *     // Start the GTK main loop
 *     gdk_threads_enter();
 *     gtk_main();
 *     gdk_threads_leave();
 *
 *     // After the GTK main loop exits, signal the Glk program to shut down if
 *     // it is still running, and wait for it to exit.
 *     chimara_glk_stop(CHIMARA_GLK(glk));
 *     chimara_glk_wait(CHIMARA_GLK(glk));
 *     g_object_unref(glk);
 *
 *     return 0;
 * }
 * ]|
 */

typedef void (* glk_main_t) (void);
typedef int (* glkunix_startup_code_t) (glkunix_startup_t*);

enum {
    PROP_0,
    PROP_INTERACTIVE,
    PROP_PROTECT,
	PROP_SPACING,
	PROP_PROGRAM_NAME,
	PROP_PROGRAM_INFO,
	PROP_STORY_NAME,
	PROP_RUNNING
};

enum {
	STOPPED,
	STARTED,
	WAITING,
	CHAR_INPUT,
	LINE_INPUT,
	TEXT_BUFFER_OUTPUT,
	ILIAD_SCREEN_UPDATE,

	LAST_SIGNAL
};

static guint chimara_glk_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE(ChimaraGlk, chimara_glk, GTK_TYPE_CONTAINER);

static void
chimara_glk_init(ChimaraGlk *self)
{
	chimara_init(); /* This is a library entry point */

    gtk_widget_set_has_window(GTK_WIDGET(self), FALSE);

    ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(self);
    
    priv->self = self;
    priv->interactive = TRUE;
	priv->styles = g_new0(StyleSet,1);
	priv->glk_styles = g_new0(StyleSet,1);
	priv->final_message = g_strdup("[ The game has finished ]");
    priv->event_queue = g_queue_new();
	priv->char_input_queue = g_async_queue_new_full(g_free);
	priv->line_input_queue = g_async_queue_new_full(g_free);

	g_mutex_init(&priv->event_lock);
	g_mutex_init(&priv->abort_lock);
	g_mutex_init(&priv->shutdown_lock);
	g_mutex_init(&priv->arrange_lock);
	g_mutex_init(&priv->resource_lock);

	g_cond_init(&priv->event_queue_not_empty);
	g_cond_init(&priv->event_queue_not_full);
	g_cond_init(&priv->shutdown_key_pressed);
	g_cond_init(&priv->rearranged);
	g_cond_init(&priv->resource_loaded);
	g_cond_init(&priv->resource_info_available);

	style_init(self);
}

static void
chimara_glk_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
    ChimaraGlk *glk = CHIMARA_GLK(object);
    
    switch(prop_id) 
    {
        case PROP_INTERACTIVE:
            chimara_glk_set_interactive( glk, g_value_get_boolean(value) );
            break;
        case PROP_PROTECT:
            chimara_glk_set_protect( glk, g_value_get_boolean(value) );
            break;
		case PROP_SPACING:
			chimara_glk_set_spacing( glk, g_value_get_uint(value) );
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
		case PROP_SPACING:
			g_value_set_uint(value, priv->spacing);
			break;
		case PROP_PROGRAM_NAME:
			g_value_set_string(value, priv->program_name);
			break;
		case PROP_PROGRAM_INFO:
			g_value_set_string(value, priv->program_info);
			break;
		case PROP_STORY_NAME:
			g_value_set_string(value, priv->story_name);
			break;
		case PROP_RUNNING:
			g_value_set_boolean(value, priv->running);
			break;
		default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
chimara_glk_finalize(GObject *object)
{
    ChimaraGlk *self = CHIMARA_GLK(object);
	CHIMARA_GLK_USE_PRIVATE(self, priv);
	priv->after_finalize = TRUE;

	/* Free widget properties */
	g_free(priv->final_message);
	/* Free styles */
	g_hash_table_destroy(priv->styles->text_buffer);
	g_hash_table_destroy(priv->styles->text_grid);
	g_hash_table_destroy(priv->glk_styles->text_buffer);
	g_hash_table_destroy(priv->glk_styles->text_grid);

    /* Free the event queue */
    g_mutex_lock(&priv->event_lock);
	g_queue_foreach(priv->event_queue, (GFunc)g_free, NULL);
	g_queue_free(priv->event_queue);
	g_cond_clear(&priv->event_queue_not_empty);
	g_cond_clear(&priv->event_queue_not_full);
	priv->event_queue = NULL;
	g_mutex_unlock(&priv->event_lock);
	g_mutex_clear(&priv->event_lock);

    /* Free the abort signaling mechanism */
	g_mutex_lock(&priv->abort_lock);
	/* Make sure no other thread is busy with this */
	g_mutex_unlock(&priv->abort_lock);
	g_mutex_clear(&priv->abort_lock);

	/* Free the shutdown keypress signaling mechanism */
	g_mutex_lock(&priv->shutdown_lock);
	g_cond_clear(&priv->shutdown_key_pressed);
	g_mutex_unlock(&priv->shutdown_lock);
	g_mutex_clear(&priv->shutdown_lock);

	/* Free the window arrangement signaling */
	g_mutex_lock(&priv->arrange_lock);
	g_cond_clear(&priv->rearranged);
	g_mutex_unlock(&priv->arrange_lock);
	g_mutex_clear(&priv->arrange_lock);

	g_mutex_lock(&priv->resource_lock);
	g_cond_clear(&priv->resource_loaded);
	g_cond_clear(&priv->resource_info_available);
	g_mutex_unlock(&priv->resource_lock);
	g_mutex_clear(&priv->resource_lock);
	g_slist_foreach(priv->image_cache, (GFunc)clear_image_cache, NULL);
	g_slist_free(priv->image_cache);
	/* Unref input queues (this should destroy them since any Glk thread has stopped by now */
	g_async_queue_unref(priv->char_input_queue);
	g_async_queue_unref(priv->line_input_queue);
	/* Destroy callback data if ownership retained */
	if(priv->resource_load_callback_destroy_data)
		priv->resource_load_callback_destroy_data(priv->resource_load_callback_data);
	
	/* Free other stuff */
	g_free(priv->current_dir);
	g_free(priv->program_name);
	g_free(priv->program_info);
	g_free(priv->story_name);
	g_free(priv->styles);
	g_free(priv->glk_styles);

	/* Chain up to parent */
    G_OBJECT_CLASS(chimara_glk_parent_class)->finalize(object);
}

/* Implementation of get_request_mode(): Always request constant size */
static GtkSizeRequestMode
chimara_glk_get_request_mode(GtkWidget *widget)
{
	return GTK_SIZE_REQUEST_CONSTANT_SIZE;
}

/* Minimal implementation of width request. Allocation in Glk is
strictly top-down, so we just request our current size by returning 1. */
static void
chimara_glk_get_preferred_width(GtkWidget *widget, int *minimal, int *natural)
{
    g_return_if_fail(widget || CHIMARA_IS_GLK(widget));
    g_return_if_fail(minimal);
    g_return_if_fail(natural);

    *minimal = *natural = 1;
}

/* Minimal implementation of height request. Allocation in Glk is
strictly top-down, so we just request our current size by returning 1. */
static void
chimara_glk_get_preferred_height(GtkWidget *widget, int *minimal, int *natural)
{
    g_return_if_fail(widget || CHIMARA_IS_GLK(widget));
    g_return_if_fail(minimal);
    g_return_if_fail(natural);

    *minimal = *natural = 1;
}

/* Recursively give the Glk windows their allocated space. Returns a window
 containing all children of this window that must be redrawn, or NULL if there 
 are no children that require redrawing. */
static winid_t
allocate_recurse(winid_t win, GtkAllocation *allocation, guint spacing)
{
	if(win->type == wintype_Pair)
	{
		glui32 division = win->split_method & winmethod_DivisionMask;
		glui32 direction = win->split_method & winmethod_DirMask;
		unsigned border = ((win->split_method & winmethod_BorderMask) == winmethod_NoBorder)? 0 : spacing;

		/* If the space gets too small to honor the spacing property, then just 
		 ignore spacing in this window and below. */
		if( (border > allocation->width && (direction == winmethod_Left || direction == winmethod_Right))
		   || (border > allocation->height && (direction == winmethod_Above || direction == winmethod_Below)) )
			border = 0;
		
		GtkAllocation child1, child2;
		child1.x = allocation->x;
		child1.y = allocation->y;
		
		if(division == winmethod_Fixed)
		{
			/* If the key window has been closed, then default to 0; otherwise
			 use the key window to determine the size */
			switch(direction)
			{
				case winmethod_Left:
					child1.width = win->key_window? 
						CLAMP(win->constraint_size * win->key_window->unit_width, 0, allocation->width - border) 
						: 0;
					break;
				case winmethod_Right:
					child2.width = win->key_window? 
						CLAMP(win->constraint_size * win->key_window->unit_width, 0, allocation->width - border)
						: 0;
					break;
				case winmethod_Above:
					child1.height = win->key_window? 
						CLAMP(win->constraint_size * win->key_window->unit_height, 0, allocation->height - border)
						: 0;
					break;
				case winmethod_Below:
					child2.height = win->key_window?
						CLAMP(win->constraint_size * win->key_window->unit_height, 0, allocation->height - border)
						: 0;
					break;
			}
		}
		else /* proportional */
		{
			gdouble fraction = win->constraint_size / 100.0;
			switch(direction)
			{
				case winmethod_Left:
					child1.width = MAX(0, (gint)ceil(fraction * (allocation->width - border)) );
					break;
				case winmethod_Right:
					child2.width = MAX(0, (gint)ceil(fraction * (allocation->width - border)) );
					break;
				case winmethod_Above:
					child1.height = MAX(0, (gint)ceil(fraction * (allocation->height - border)) );
					break;
				case winmethod_Below:
					child2.height = MAX(0, (gint)ceil(fraction * (allocation->height - border)) );
					break;
			}
		}
		
		/* Fill in the rest of the size requisitions according to the child specified above */
		switch(direction)
		{
			case winmethod_Left:
				child2.width = MAX(0, allocation->width - border - child1.width);
				child2.x = child1.x + child1.width + border;
				child2.y = child1.y;
				child1.height = child2.height = allocation->height;
				break;
			case winmethod_Right:
				child1.width = MAX(0, allocation->width - border - child2.width);
				child2.x = child1.x + child1.width + border;
				child2.y = child1.y;
				child1.height = child2.height = allocation->height;
				break;
			case winmethod_Above:
				child2.height = MAX(0, allocation->height - border - child1.height);
				child2.x = child1.x;
				child2.y = child1.y + child1.height + border;
				child1.width = child2.width = allocation->width;
				break;
			case winmethod_Below:
				child1.height = MAX(0, allocation->height - border - child2.height);
				child2.x = child1.x;
				child2.y = child1.y + child1.height + border;
				child1.width = child2.width = allocation->width;
				break;
		}
		
		/* Recurse */
		winid_t arrange1 = allocate_recurse(win->window_node->children->data, &child1, spacing);
		winid_t arrange2 = allocate_recurse(win->window_node->children->next->data, &child2, spacing);
		if(arrange1 == NULL)
			return arrange2;
		if(arrange2 == NULL)
			return arrange1;
		return win;
	}
	
	else if(win->type == wintype_TextGrid)
	{
		/* Pass the size allocation on to the framing widget */
		gtk_widget_size_allocate(win->frame, allocation);
		/* It says in the spec that when a text grid window is resized smaller,
		 the bottom or right area is thrown away; when it is resized larger, the
		 bottom or right area is filled with blanks. */
		GtkAllocation widget_allocation;
		gtk_widget_get_allocation(win->widget, &widget_allocation);
		glui32 new_width = (glui32)(widget_allocation.width / win->unit_width);
		glui32 new_height = (glui32)(widget_allocation.height / win->unit_height);

		if(new_width != win->width || new_height != win->height)
		{
			// Window has changed size, trim or expand the textbuffer if necessary.
			GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
			GtkTextIter start, end;

			// Add or remove lines
			if(new_height == 0) {
				gtk_text_buffer_get_start_iter(buffer, &start);
				gtk_text_buffer_get_end_iter(buffer, &end);
				gtk_text_buffer_delete(buffer, &start, &end);
			}
			else if(new_height < win->height)
			{
				// Remove surplus lines
				gtk_text_buffer_get_end_iter(buffer, &end);
				gtk_text_buffer_get_iter_at_line(buffer, &start, new_height-1);
				gtk_text_iter_forward_to_line_end(&start);
				gtk_text_buffer_delete(buffer, &start, &end);

			}
			else if(new_height > win->height)
			{
				// Add extra lines
				gint lines_to_add = new_height - win->height;
				gtk_text_buffer_get_end_iter(buffer, &end);
				start = end;

				gchar *blanks = g_strnfill(win->width, ' ');
				gchar **blanklines = g_new0(gchar *, lines_to_add + 1);
				int count;
				for(count = 0; count < lines_to_add; count++)
					blanklines[count] = blanks;
				blanklines[lines_to_add] = NULL;
				gchar *vertical_blanks = g_strjoinv("\n", blanklines);
				g_free(blanklines); 
				g_free(blanks);

				if(win->height > 0) 
					gtk_text_buffer_insert(buffer, &end, "\n", 1);

				gtk_text_buffer_insert(buffer, &end, vertical_blanks, -1);
			}

			// Trim or expand lines
			if(new_width < win->width) {
				gtk_text_buffer_get_start_iter(buffer, &start);
				end = start;

				gint line;
				for(line = 0; line <= new_height; line++) {
					// Trim the line
					gtk_text_iter_forward_cursor_positions(&start, new_width);
					gtk_text_iter_forward_to_line_end(&end);
					gtk_text_buffer_delete(buffer, &start, &end);
					gtk_text_iter_forward_line(&start);
					end = start;
				}
			} else if(new_width > win->width) {
				gint chars_to_add = new_width - win->width;
				gchar *horizontal_blanks = g_strnfill(chars_to_add, ' ');

				gtk_text_buffer_get_start_iter(buffer, &start);
				end = start;

				gint line;
				for(line = 0; line <= new_height; line++) {
					gtk_text_iter_forward_to_line_end(&start);
					end = start;
					gint start_offset = gtk_text_iter_get_offset(&start);
					gtk_text_buffer_insert(buffer, &end, horizontal_blanks, -1);
					gtk_text_buffer_get_iter_at_offset(buffer, &start, start_offset);
					gtk_text_iter_forward_line(&start);
					end = start;
				}

				g_free(horizontal_blanks);
			}
		}
	
		gboolean arrange = !(win->width == new_width && win->height == new_height);
		win->width = new_width;
		win->height = new_height;
		return arrange? win : NULL;
	}
	
	/* For non-pair, non-text-grid windows, just give them the size */
	gtk_widget_size_allocate(win->frame, allocation);
	return NULL;
}

/* Overrides gtk_widget_size_allocate */
static void
chimara_glk_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
    g_return_if_fail(widget);
    g_return_if_fail(allocation);
    g_return_if_fail(CHIMARA_IS_GLK(widget));
    
    ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(widget);
    
    gtk_widget_set_allocation(widget, allocation);

    if(priv->root_window) {
		GtkAllocation child = *allocation;
		winid_t arrange = allocate_recurse(priv->root_window->data, &child, priv->spacing);

		/* arrange points to a window that contains all text grid and graphics
		 windows which have been resized */
		g_mutex_lock(&priv->arrange_lock);
		if(!priv->ignore_next_arrange_event)
		{
			if(arrange)
				event_throw(CHIMARA_GLK(widget), evtype_Arrange, arrange == priv->root_window->data? NULL : arrange, 0, 0);
		}
		else
			priv->ignore_next_arrange_event = FALSE;
		priv->needs_rearrange = FALSE;
		g_cond_signal(&priv->rearranged);
		g_mutex_unlock(&priv->arrange_lock);
	}
}

/* Recursively invoke callback() on the GtkWidget of each non-pair window in the tree */
static void
forall_recurse(winid_t win, GtkCallback callback, gpointer callback_data)
{
	if(win->type == wintype_Pair)
	{
		forall_recurse(win->window_node->children->data, callback, callback_data);
		forall_recurse(win->window_node->children->next->data, callback, callback_data);
	}
	else
		(*callback)(win->frame, callback_data);
}

/* Overrides gtk_container_forall */
static void
chimara_glk_forall(GtkContainer *container, gboolean include_internals, GtkCallback callback, gpointer callback_data)
{
    g_return_if_fail(container);
    g_return_if_fail(CHIMARA_IS_GLK(container));
    
    ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(container);
    
	/* All the children are "internal" */
	if(!include_internals)
		return;
	
    if(priv->root_window)
		forall_recurse(priv->root_window->data, callback, callback_data);
}

static void
chimara_glk_stopped(ChimaraGlk *self)
{
    CHIMARA_GLK_USE_PRIVATE(self, priv);
    priv->running = FALSE;
    priv->program_name = NULL;
    g_object_notify(G_OBJECT(self), "program-name");
    priv->program_info = NULL;
    g_object_notify(G_OBJECT(self), "program-info");
    priv->story_name = NULL;
    g_object_notify(G_OBJECT(self), "story-name");
}

static void
chimara_glk_started(ChimaraGlk *self)
{
	CHIMARA_GLK_USE_PRIVATE(self, priv);
	priv->running = TRUE;
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
    widget_class->get_request_mode = chimara_glk_get_request_mode;
    widget_class->get_preferred_width = chimara_glk_get_preferred_width;
    widget_class->get_preferred_height = chimara_glk_get_preferred_height;
    widget_class->size_allocate = chimara_glk_size_allocate;

    GtkContainerClass *container_class = GTK_CONTAINER_CLASS(klass);
    container_class->forall = chimara_glk_forall;
    /* Automatically handle the GtkContainer:border-width property */
    gtk_container_class_handle_border_width(container_class);

    /* Signals */
    klass->stopped = chimara_glk_stopped;
    klass->started = chimara_glk_started;

    /**
     * ChimaraGlk::stopped:
     * @glk: The widget that received the signal
     *
     * Emitted when the a Glk program finishes executing in the widget, whether
     * it ended normally, or was interrupted.
     */ 
    chimara_glk_signals[STOPPED] = g_signal_new("stopped", 
        G_OBJECT_CLASS_TYPE(klass), G_SIGNAL_RUN_FIRST, 
        /* FIXME: Should be G_SIGNAL_RUN_CLEANUP but that segfaults??! */
        G_STRUCT_OFFSET(ChimaraGlkClass, stopped), NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	/**
	 * ChimaraGlk::started:
	 * @glk: The widget that received the signal
	 *
	 * Emitted when a Glk program starts executing in the widget.
	 */
	chimara_glk_signals[STARTED] = g_signal_new ("started",
		G_OBJECT_CLASS_TYPE(klass), G_SIGNAL_RUN_FIRST,
		G_STRUCT_OFFSET(ChimaraGlkClass, started), NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	/**
	 * ChimaraGlk::waiting:
	 * @glk: The widget that received the signal
	 * 
	 * Emitted when glk_select() is called by the Glk program and the event
	 * queue is empty, which means that the widget is waiting for input.
	 */
	chimara_glk_signals[WAITING] = g_signal_new("waiting",
		G_OBJECT_CLASS_TYPE(klass), 0,
		G_STRUCT_OFFSET(ChimaraGlkClass, waiting), NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	/**
	 * ChimaraGlk::char-input:
	 * @self: The widget that received the signal
	 * @window_rock: The rock value of the window that received character input
	 * (see [Rocks][chimara-Rocks])
	 * @window_id_string: A string value uniquely identifying the window that
	 * received character input
	 * @keysym: The key that was typed, in the form of a key symbol from
	 * `gdk/gdkkeysyms.h`
	 *
	 * Emitted when a Glk window receives character input.
	 * The @window_rock can be used to identify the window.
	 * However, rock values in Glk are allowed to be identical for different
	 * windows, so Chimara also provides a string value with which the window
	 * can be uniquely identified.
	 */
	chimara_glk_signals[CHAR_INPUT] = g_signal_new("char-input",
		G_OBJECT_CLASS_TYPE(klass), 0,
		G_STRUCT_OFFSET(ChimaraGlkClass, char_input), NULL, NULL,
		_chimara_marshal_VOID__UINT_STRING_UINT,
		G_TYPE_NONE, 3, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_UINT);
	/**
	 * ChimaraGlk::line-input:
	 * @self: The widget that received the signal
	 * @window_rock: The rock value of the window that received line input (see
	 * [Rocks][chimara-Rocks])
	 * @window_id_string: A string value uniquely identifying the window that
	 * received the input
	 * @text: The text that was typed
	 *
	 * Emitted when a Glk window receives line input.
	 * The @window_rock can be used to identify the window.
	 * However, rock values in Glk are allowed to be identical for different
	 * windows, so Chimara also provides a string value with which the window
	 * can be uniquely identified.
	 */
	chimara_glk_signals[LINE_INPUT] = g_signal_new("line-input",
		G_OBJECT_CLASS_TYPE(klass), 0,
		G_STRUCT_OFFSET(ChimaraGlkClass, line_input), NULL, NULL,
		_chimara_marshal_VOID__UINT_STRING_STRING,
		G_TYPE_NONE, 3, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING);
	/**
	 * ChimaraGlk::text-buffer-output:
	 * @self: The widget that received the signal
	 * @window_rock: The rock value of the window that was printed to (see
	 * [Rocks][chimara-Rocks])
	 * @window_id_string: A string value uniquely identifying the window that
	 * was printed to
	 *
	 * Emitted when text is printed to a text buffer window.
	 * The @window_rock can be used to identify the window.
	 * However, rock values in Glk are allowed to be identical for different
	 * windows, so Chimara also provides a string value with which the window
	 * can be uniquely identified.
	 */
	chimara_glk_signals[TEXT_BUFFER_OUTPUT] = g_signal_new("text-buffer-output",
		G_OBJECT_CLASS_TYPE(klass), 0,
		G_STRUCT_OFFSET(ChimaraGlkClass, text_buffer_output), NULL, NULL,
		_chimara_marshal_VOID__UINT_STRING_STRING,
		G_TYPE_NONE, 3, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING);
	/**
	 * ChimaraGlk::iliad-screen-update:
	 * @self: The widget that received the signal
	 * @typing: Whether to perform a typing or full screen update
	 *
	 * Iliad specific signal which is emitted whenever the screen needs to be updated.
	 * Since iliad screen updates are very slow, updating should only be done when
	 * necessary.
	 */
	chimara_glk_signals[ILIAD_SCREEN_UPDATE] = g_signal_new("iliad-screen-update",
		G_OBJECT_CLASS_TYPE(klass), 0,
		G_STRUCT_OFFSET(ChimaraGlkClass, iliad_screen_update), NULL, NULL,
		_chimara_marshal_VOID__BOOLEAN,
		G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

    /* Properties */
    /**
     * ChimaraGlk:interactive:
     *
     * Sets whether the widget is interactive. A Glk widget is normally 
     * interactive, but in non-interactive mode, keyboard and mouse input are 
     * ignored and the Glk program is controlled by 
     * chimara_glk_feed_char_input() and chimara_glk_feed_line_input(). 
	 * “More” prompts when a lot of text is printed to a text buffer are also
	 * disabled.
	 * This is typically used when you wish to control an interpreter program by
	 * feeding it a predefined list of commands.
     */
    g_object_class_install_property( object_class, PROP_INTERACTIVE, 
		g_param_spec_boolean("interactive", _("Interactive"),
        _("Whether user input is expected in the Glk program"),
        TRUE,
        G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_LAX_VALIDATION | G_PARAM_STATIC_STRINGS) );

	/**
     * ChimaraGlk:protect:
     *
     * Sets whether the Glk program is allowed to do file operations. In protect
     * mode, all file operations will fail.
     */
    g_object_class_install_property(object_class, PROP_PROTECT, 
		g_param_spec_boolean("protect", _("Protected"),
        _("Whether the Glk program is barred from doing file operations"),
        FALSE,
        G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_LAX_VALIDATION | G_PARAM_STATIC_STRINGS) );

	/**
	 * ChimaraGlk:spacing:
	 *
	 * The amount of space between the Glk windows. This space forms a visible
	 * border between windows; however, if you open a window using the
	 * %winmethod_NoBorder flag, there will be no spacing between it and its
	 * sibling window, no matter what the value of this property is.
	 */
	g_object_class_install_property(object_class, PROP_SPACING,
		g_param_spec_uint("spacing", _("Spacing"),
		_("The amount of space between Glk windows"),
		0, G_MAXUINT, 0,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_LAX_VALIDATION | G_PARAM_STATIC_STRINGS) );
	
	/**
	 * ChimaraGlk:program-name:
	 *
	 * The name of the currently running Glk program. You cannot set this 
	 * property yourself. It is set to the filename of the plugin when you call
	 * chimara_glk_run(), but the plugin can change it by calling 
	 * garglk_set_program_name(). To find out when this information changes,
	 * for example to put the program name in the title bar of a window, connect
	 * to the `::notify::program-name` signal.
	 */
	g_object_class_install_property(object_class, PROP_PROGRAM_NAME,
		g_param_spec_string("program-name", _("Program name"),
		_("Name of the currently running program"),
		NULL,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS) );
		
	/**
	 * ChimaraGlk:program-info:
	 *
	 * Information about the currently running Glk program. You cannot set this
	 * property yourself. The plugin can change it by calling
	 * garglk_set_program_info(). See also #ChimaraGlk:program-name.
	 */
	g_object_class_install_property(object_class, PROP_PROGRAM_INFO,
		g_param_spec_string("program-info", _("Program info"),
		_("Information about the currently running program"),
		NULL,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS) );
	
	/**
	 * ChimaraGlk:story-name:
	 *
	 * The name of the story currently running in the Glk interpreter. You
	 * cannot set this property yourself. It is set to the story filename when
	 * you call chimara_if_run_game(), but the plugin can change it by calling
	 * garglk_set_story_name().
	 *
	 * Strictly speaking, this should be a property of #ChimaraIF, but it is
	 * legal for any Glk program to call garglk_set_story_name(), even if it is
	 * not an interpreter and does not load story files.
	 */
	g_object_class_install_property(object_class, PROP_STORY_NAME,
		g_param_spec_string("story-name", _("Story name"),
		_("Name of the story currently loaded in the interpreter"),
		NULL,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS) );
	
	/**
	 * ChimaraGlk:running:
	 *
	 * Whether this Glk widget is currently running a game or not.
	 */
	g_object_class_install_property(object_class, PROP_RUNNING,
		g_param_spec_boolean("running", _("Running"),
		_("Whether there is a program currently running"),
		FALSE,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS) );

	/* Private data */
    g_type_class_add_private(klass, sizeof(ChimaraGlkPrivate));
}

/* PUBLIC FUNCTIONS */

/**
 * chimara_error_quark:
 *
 * The error domain for errors from Chimara widgets.
 *
 * Returns: The string “chimara-error-quark” as a #GQuark.
 */
GQuark
chimara_error_quark(void)
{
	chimara_init(); /* This is a library entry point */
	return g_quark_from_static_string("chimara-error-quark");
}

/**
 * chimara_glk_new:
 *
 * Creates and initializes a new #ChimaraGlk widget.
 *
 * Return value: a #ChimaraGlk widget, with a floating reference.
 */
GtkWidget *
chimara_glk_new(void)
{
	/* This is a library entry point; initialize the library */
	chimara_init();

    return GTK_WIDGET(g_object_new(CHIMARA_TYPE_GLK, NULL));
}

/**
 * chimara_glk_set_interactive:
 * @glk: a #ChimaraGlk widget
 * @interactive: whether the widget should expect user input
 *
 * Sets the #ChimaraGlk:interactive property of @glk. 
 */
void 
chimara_glk_set_interactive(ChimaraGlk *glk, gboolean interactive)
{
    g_return_if_fail(glk || CHIMARA_IS_GLK(glk));
    
    ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(glk);
    priv->interactive = interactive;
    g_object_notify(G_OBJECT(glk), "interactive");
}

/**
 * chimara_glk_get_interactive:
 * @glk: a #ChimaraGlk widget
 *
 * Returns whether @glk is interactive (expecting user input). See 
 * #ChimaraGlk:interactive.
 *
 * Return value: %TRUE if @glk is interactive.
 */
gboolean 
chimara_glk_get_interactive(ChimaraGlk *glk)
{
    g_return_val_if_fail(glk || CHIMARA_IS_GLK(glk), FALSE);
    
    ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(glk);
    return priv->interactive;
}

/**
 * chimara_glk_set_protect:
 * @glk: a #ChimaraGlk widget
 * @protect: whether the widget should allow the Glk program to do file 
 * operations
 *
 * Sets the #ChimaraGlk:protect property of @glk. In protect mode, the Glk 
 * program is not allowed to do file operations.
 */
void 
chimara_glk_set_protect(ChimaraGlk *glk, gboolean protect)
{
    g_return_if_fail(glk || CHIMARA_IS_GLK(glk));
    
    ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(glk);
    priv->protect = protect;
    g_object_notify(G_OBJECT(glk), "protect");
}

/**
 * chimara_glk_get_protect:
 * @glk: a #ChimaraGlk widget
 *
 * Returns whether @glk is in protect mode (banned from doing file operations).
 * See #ChimaraGlk:protect.
 *
 * Return value: %TRUE if @glk is in protect mode.
 */
gboolean 
chimara_glk_get_protect(ChimaraGlk *glk)
{
    g_return_val_if_fail(glk || CHIMARA_IS_GLK(glk), FALSE);
    
    ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(glk);
    return priv->protect;
}

/**
 * chimara_glk_set_css_to_default:
 * @glk: a #ChimaraGlk widget
 *
 * Resets the styles for text buffer and text grid windows to their defaults.
 * <para><warning>
 *   This function is not implemented yet.
 * </warning></para>
 */
void
chimara_glk_set_css_to_default(ChimaraGlk *glk)
{
	reset_default_styles(glk);
}

/**
 * chimara_glk_set_css_from_file:
 * @glk: a #ChimaraGlk widget
 * @filename: path to a CSS file, or %NULL
 * @error: location to store a #GError, or %NULL
 *
 * Sets the styles for text buffer and text grid windows according to the CSS
 * file @filename. Note that the styles are set cumulatively on top of whatever
 * the styles are at the time this function is called; to reset the styles to
 * their defaults, use chimara_glk_set_css_to_default().
 *
 * Returns: %TRUE on success, %FALSE if an error occurred, in which case @error
 * will be set.
 */
gboolean 
chimara_glk_set_css_from_file(ChimaraGlk *glk, const gchar *filename, GError **error)
{
	g_return_val_if_fail(glk || CHIMARA_IS_GLK(glk), FALSE);
	g_return_val_if_fail(filename, FALSE);
	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

	int fd = open(filename, O_RDONLY);
	if(fd == -1) {
		if(error)
			*error = g_error_new(G_IO_ERROR, g_io_error_from_errno(errno), 
				_("Error opening file \"%s\": %s"), filename, g_strerror(errno));
		return FALSE;
	}

	GScanner *scanner = create_css_file_scanner();
	g_scanner_input_file(scanner, fd);
	scanner->input_name = filename;
	scan_css_file(scanner, glk);

	if(close(fd) == -1) {
		if(error)
			*error = g_error_new(G_IO_ERROR, g_io_error_from_errno(errno),
				_("Error closing file \"%s\": %s"), filename, g_strerror(errno));
		return FALSE;
	}
	return TRUE;
}

/**
 * chimara_glk_set_css_from_string:
 * @glk: a #ChimaraGlk widget
 * @css: a string containing CSS code
 *
 * Sets the styles for text buffer and text grid windows according to the CSS
 * code @css. Note that the styles are set cumulatively on top of whatever the 
 * styles are at the time this function is called; to reset the styles to their
 * defaults, use chimara_glk_set_css_to_default().
 */
void 
chimara_glk_set_css_from_string(ChimaraGlk *glk, const gchar *css)
{
	g_return_if_fail(glk || CHIMARA_IS_GLK(glk));
	g_return_if_fail(css || *css);
	
	GScanner *scanner = create_css_file_scanner();
	g_scanner_input_text(scanner, css, strlen(css));
	scanner->input_name = "<string>";
	scan_css_file(scanner, glk);
}

/**
 * chimara_glk_set_spacing:
 * @glk: a #ChimaraGlk widget
 * @spacing: the number of pixels to put between Glk windows
 *
 * Sets the #ChimaraGlk:spacing property of @glk, which is the border width in
 * pixels between Glk windows.
 */
void 
chimara_glk_set_spacing(ChimaraGlk *glk, guint spacing)
{
	g_return_if_fail( glk || CHIMARA_IS_GLK(glk) );
	
	ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(glk);
	priv->spacing = spacing;
	g_object_notify(G_OBJECT(glk), "spacing");
}

/**
 * chimara_glk_get_spacing:
 * @glk: a #ChimaraGlk widget
 *
 * Gets the value set by chimara_glk_set_spacing().
 *
 * Return value: pixels of spacing between Glk windows
 */
guint 
chimara_glk_get_spacing(ChimaraGlk *glk)
{
	g_return_val_if_fail(glk || CHIMARA_IS_GLK(glk), 0);
	
	ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(glk);
	return priv->spacing;
}

struct StartupData {
	glk_main_t glk_main;
	glkunix_startup_code_t glkunix_startup_code;
	glkunix_startup_t args;
	ChimaraGlkPrivate *glk_data;
};

static void
free_startup_data(struct StartupData *startup)
{
	int i = 0;
	while(i < startup->args.argc)
		g_free(startup->args.argv[i++]);
	g_free(startup->args.argv);
	g_slice_free(struct StartupData, startup);
}

/* glk_enter() is the actual function called in the new thread in which
glk_main() runs. Takes ownership of @startup and will free it. */
static gpointer
glk_enter(struct StartupData *startup)
{
	extern GPrivate glk_data_key;
	g_private_set(&glk_data_key, startup->glk_data);

	/* Acquire the Glk thread's references to the input queues */
	g_async_queue_ref(startup->glk_data->char_input_queue);
	g_async_queue_ref(startup->glk_data->line_input_queue);

	/* Run startup function */
	if(startup->glkunix_startup_code) {
		startup->glk_data->in_startup = TRUE;
		int result = startup->glkunix_startup_code(&startup->args);
		startup->glk_data->in_startup = FALSE;

		if(!result) {
			free_startup_data(startup);
			return NULL;
		}
	}

	/* Run main function */
	glk_main_t glk_main = startup->glk_main;

	g_signal_emit_by_name(startup->glk_data->self, "started");
	glk_main();
	free_startup_data(startup);
	glk_exit(); /* Run shutdown code in glk_exit() even if glk_main() returns normally */
	g_assert_not_reached(); /* because glk_exit() calls g_thread_exit() */
	return NULL;
}

/**
 * chimara_glk_run:
 * @glk: a #ChimaraGlk widget
 * @plugin: path to a plugin module compiled with `glk.h`
 * @argc: Number of command line arguments in @argv
 * @argv: Array of command line arguments to pass to the plugin
 * @error: location to store a #GError, or %NULL
 *
 * Opens a Glk program compiled as a plugin. Sorts out its command line
 * arguments from #glkunix_arguments, calls its startup function
 * glkunix_startup_code(), and then calls its main function glk_main() in
 * a separate thread. On failure, returns %FALSE and sets @error.
 *
 * The plugin must at least export a glk_main() function; #glkunix_arguments and
 * glkunix_startup_code() are optional.
 *
 * Return value: %TRUE if the Glk program was started successfully.
 */
gboolean
chimara_glk_run(ChimaraGlk *glk, const gchar *plugin, int argc, char *argv[], GError **error)
{
    g_return_val_if_fail(glk || CHIMARA_IS_GLK(glk), FALSE);
    g_return_val_if_fail(plugin, FALSE);
	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
	
	if(chimara_glk_get_running(glk)) {
		g_set_error(error, CHIMARA_ERROR, CHIMARA_PLUGIN_ALREADY_RUNNING, _("There was already a plugin running."));
		return FALSE;
	}
    
    ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(glk);

	struct StartupData *startup = g_slice_new0(struct StartupData);

    g_assert( g_module_supported() );
	/* If there is already a module loaded, free it first -- you see, we want to
	 * keep modules loaded as long as possible to avoid crashes in stack unwinding */
	chimara_glk_unload_plugin(glk);
	/* Open the module to run */
    priv->program = g_module_open(plugin, G_MODULE_BIND_LAZY);
    
    if(!priv->program)
    {
    	g_set_error(error, CHIMARA_ERROR, CHIMARA_LOAD_MODULE_ERROR, _("Error opening module: %s"), g_module_error());
        return FALSE;
    }
    if( !g_module_symbol(priv->program, "glk_main", (gpointer *) &startup->glk_main) )
    {
    	g_set_error(error, CHIMARA_ERROR, CHIMARA_NO_GLK_MAIN, _("Error finding glk_main(): %s"), g_module_error());
        return FALSE;
    }

    if( g_module_symbol(priv->program, "glkunix_startup_code", (gpointer *) &startup->glkunix_startup_code) )
    {
		glkunix_argumentlist_t *glkunix_arguments;

		if( !(g_module_symbol(priv->program, "glkunix_arguments", (gpointer *) &glkunix_arguments) 
			  && parse_command_line(glkunix_arguments, argc, argv, &startup->args)) )
		{
			/* arguments could not be parsed, so create data ourselves */
			startup->args.argc = 1;
			startup->args.argv = g_new0(gchar *, 1);
		}

		/* Set the program invocation name */
		startup->args.argv[0] = g_strdup(plugin);
    }
	startup->glk_data = priv;
	
	/* Set the program name */
	priv->program_name = g_path_get_basename(plugin);
	g_object_notify(G_OBJECT(glk), "program-name");

	/* Set Glk styles to defaults */
	style_reset_glk(glk);

    /* Run in a separate thread */
	priv->thread = g_thread_try_new("glk", (GThreadFunc)glk_enter, startup, error);

	return !(priv->thread == NULL);
}

/**
 * chimara_glk_run_file:
 * @self: a #ChimaraGlk widget
 * @plugin_file: a #GFile pointing to a plugin module compiled with `glk.h`
 * @argc: Number of command line arguments in @argv
 * @argv: Array of command line arguments to pass to the plugin
 * @error: location to store a #GError, or %NULL
 *
 * Opens a Glk program compiled as a plugin, from a #GFile. See
 * chimara_glk_run() for details.
 *
 * Return value: %TRUE if the Glk program was started successfully.
 */
gboolean
chimara_glk_run_file(ChimaraGlk *self, GFile *plugin_file, int argc, char *argv[], GError **error)
{
	g_return_val_if_fail(self || CHIMARA_IS_GLK(self), FALSE);
	g_return_val_if_fail(plugin_file || G_IS_FILE(plugin_file), FALSE);
	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

	char *path = g_file_get_path(plugin_file);
	gboolean retval = chimara_glk_run(self, path, argc, argv, error);
	g_free(path);

	return retval;
}

/**
 * chimara_glk_stop:
 * @glk: a #ChimaraGlk widget
 *
 * Signals the Glk program running in @glk to abort. Note that if the program is
 * caught in an infinite loop in which glk_tick() is not called, this may not
 * work.
 *
 * This function does nothing if no Glk program is running.
 */
void
chimara_glk_stop(ChimaraGlk *glk)
{
    g_return_if_fail(glk || CHIMARA_IS_GLK(glk));
    CHIMARA_GLK_USE_PRIVATE(glk, priv);

    /* Don't do anything if not running a program */
    if(!priv->running)
    	return;
    
	if(!priv->after_finalize) {
		g_mutex_lock(&priv->abort_lock);
		priv->abort_signalled = TRUE;
		g_mutex_unlock(&priv->abort_lock);
		/* Stop blocking on the event queue condition */
		event_throw(glk, evtype_Abort, NULL, 0, 0);
		/* Stop blocking on the shutdown key press condition */
		g_mutex_lock(&priv->shutdown_lock);
		g_cond_signal(&priv->shutdown_key_pressed);
		g_mutex_unlock(&priv->shutdown_lock);
	}
}

/**
 * chimara_glk_wait:
 * @glk: a #ChimaraGlk widget
 *
 * Holds up the main thread and waits for the Glk program running in @glk to 
 * finish.
 *
 * This function does nothing if no Glk program is running.
 */
void
chimara_glk_wait(ChimaraGlk *glk)
{
    g_return_if_fail(glk || CHIMARA_IS_GLK(glk));
    CHIMARA_GLK_USE_PRIVATE(glk, priv);
    /* Don't do anything if not running a program */
    if(!priv->running)
    	return;
	/* Unlock GDK mutex, because the Glk program might need to use it for shutdown */
	gdk_threads_leave();
    g_thread_join(priv->thread);
	gdk_threads_enter();
}

/**
 * chimara_glk_unload_plugin:
 * @glk: a #ChimaraGlk widget
 *
 * The plugin containing the Glk program is unloaded as late as possible before
 * loading a new plugin, in order to prevent crashes while printing stack
 * backtraces during debugging. Sometimes this behavior is not desirable. This
 * function forces @glk to unload the plugin running in it.
 *
 * This function does nothing if there is no plugin loaded.
 */
void
chimara_glk_unload_plugin(ChimaraGlk *glk)
{
	g_return_if_fail(glk || CHIMARA_IS_GLK(glk));
    CHIMARA_GLK_USE_PRIVATE(glk, priv);
	if( priv->program && !g_module_close(priv->program) )
		g_warning( "Error closing module :%s", g_module_error() );
	priv->program = NULL;
}

/**
 * chimara_glk_get_running:
 * @glk: a #ChimaraGlk widget
 * 
 * Use this function to tell whether a program is currently running in the
 * widget.
 * 
 * Returns: %TRUE if @glk is executing a Glk program, %FALSE otherwise.
 */
gboolean
chimara_glk_get_running(ChimaraGlk *glk)
{
	g_return_val_if_fail(glk || CHIMARA_IS_GLK(glk), FALSE);
	CHIMARA_GLK_USE_PRIVATE(glk, priv);
	return priv->running;
}

/**
 * chimara_glk_feed_char_input:
 * @glk: a #ChimaraGlk widget
 * @keyval: a key symbol as defined in `gdk/gdkkeysyms.h`
 * 
 * Pretend that a key was pressed in the Glk program as a response to a 
 * character input request. You can call this function even when no window has
 * requested character input, in which case the key will be saved for the 
 * following window that requests character input. This has the disadvantage 
 * that if more than one window has requested character input, it is arbitrary 
 * which one gets the key press.
 */
void 
chimara_glk_feed_char_input(ChimaraGlk *glk, guint keyval)
{
	g_return_if_fail(glk || CHIMARA_IS_GLK(glk));
	CHIMARA_GLK_USE_PRIVATE(glk, priv);
	g_async_queue_push(priv->char_input_queue, GUINT_TO_POINTER(keyval));
	event_throw(glk, evtype_ForcedCharInput, NULL, 0, 0);
}

/**
 * chimara_glk_feed_line_input:
 * @glk: a #ChimaraGlk widget
 * @text: text to pass to the next line input request
 * 
 * Pretend that @text was typed in the Glk program as a response to a line input
 * request. @text does not need to end with a newline. You can call this 
 * function even when no window has requested line input, in which case the text
 * will be saved for the following window that requests line input. This has the 
 * disadvantage that if more than one window has requested line input, it is
 * arbitrary which one gets the text.
 */
void 
chimara_glk_feed_line_input(ChimaraGlk *glk, const gchar *text)
{
	g_return_if_fail(glk || CHIMARA_IS_GLK(glk));
	g_return_if_fail(text);
	CHIMARA_GLK_USE_PRIVATE(glk, priv);
	g_async_queue_push(priv->line_input_queue, g_strdup(text));
	event_throw(glk, evtype_ForcedLineInput, NULL, 0, 0);
}

/**
 * chimara_glk_is_char_input_pending:
 * @glk: a #ChimaraGlk widget
 *
 * Use this function to tell if character input forced by 
 * chimara_glk_feed_char_input() has been passed to an input request or not.
 *
 * Returns: %TRUE if forced character input is pending, %FALSE otherwise.
 */
gboolean
chimara_glk_is_char_input_pending(ChimaraGlk *glk)
{
	g_return_val_if_fail(glk || CHIMARA_IS_GLK(glk), FALSE);
	CHIMARA_GLK_USE_PRIVATE(glk, priv);
	return g_async_queue_length(priv->char_input_queue) > 0;
}

/**
 * chimara_glk_is_line_input_pending:
 * @glk: a #ChimaraGlk widget
 *
 * Use this function to tell if line input forced by 
 * chimara_glk_feed_line_input() has been passed to an input request or not.
 *
 * Returns: %TRUE if forced line input is pending, %FALSE otherwise.
 */
gboolean
chimara_glk_is_line_input_pending(ChimaraGlk *glk)
{
	g_return_val_if_fail(glk || CHIMARA_IS_GLK(glk), FALSE);
	CHIMARA_GLK_USE_PRIVATE(glk, priv);
	return g_async_queue_length(priv->line_input_queue) > 0;
}

/**
 * chimara_glk_get_tag:
 * @glk: a #ChimaraGlk widget
 * @window: The type of window to retrieve the tag for
 * @name: The name of the tag to retrieve
 *
 * Use this function to get a #GtkTextTag so style properties can be changed.
 * See also chimara_glk_set_css_from_string().
 *
 * The layout of the text in Chimara is controlled by two sets of tags: one set
 * describing the style in text buffers and one for text grids. See also the
 * Glk specification for the difference between the two. The main narrative of
 * a game is usually rendered in text buffers, whereas text grids are mostly
 * used for status bars and in game menus.
 *
 * The following tag names are supported:
 *
 * - normal
 * - emphasized
 * - preformatted
 * - header
 * - subheader
 * - alert
 * - note
 * - block-quote
 * - input
 * - user1
 * - user2
 * - hyperlink
 * - pager
 *
 * Returns: (transfer none): The #GtkTextTag corresponding to @name in the
 * styles of @window.
 */
GtkTextTag *
chimara_glk_get_tag(ChimaraGlk *glk, ChimaraGlkWindowType window, const gchar *name)
{
	CHIMARA_GLK_USE_PRIVATE(glk, priv);

	switch(window) {
	case CHIMARA_GLK_TEXT_BUFFER:
		return GTK_TEXT_TAG( g_hash_table_lookup(priv->styles->text_buffer, name) );
		break;
	case CHIMARA_GLK_TEXT_GRID:
		return GTK_TEXT_TAG( g_hash_table_lookup(priv->styles->text_grid, name) );
		break;
	default:
		ILLEGAL_PARAM("Unknown window type: %u", window);
		return NULL;
	}
}

/**
 * chimara_glk_get_tag_names:
 * @glk: a #ChimaraGlk widget
 * @num_tags: Return location for the number of tag names retrieved.
 *
 * Retrieves the possible tag names to use in chimara_glk_get_tag().
 *
 * Returns: (transfer none) (array length=num_tags) (element-type utf8):
 * Array of strings containing the tag names. This array is owned by Chimara,
 * do not free it.
 */
const gchar **
chimara_glk_get_tag_names(ChimaraGlk *glk, unsigned int *num_tags)
{
	g_return_val_if_fail(num_tags != NULL, NULL);

	*num_tags = CHIMARA_NUM_STYLES;
	return style_get_tag_names();
}

/**
 * chimara_glk_set_resource_load_callback:
 * @glk: a #ChimaraGlk widget
 * @func: a function to call for loading resources, or %NULL
 * @user_data: user data to pass to @func, or %NULL
 * @destroy_user_data: a function to call for freeing @user_data, or %NULL
 *
 * Sometimes it is preferable to load image and sound resources from somewhere
 * else than a Blorb file, for example while developing a game. Section 14 of
 * the <ulink url="http://eblong.com/zarf/blorb/blorb.html#s14">Blorb
 * specification</ulink> allows for this possibility. This function sets @func
 * to be called when the Glk program requests loading an image or sound without
 * a Blorb resource map having been loaded, optionally passing @user_data as an 
 * extra parameter.
 *
 * Note that @func is only called if no Blorb resource map has been set; having
 * a resource map in place overrides this function.
 *
 * If you pass non-%NULL for @destroy_user_data, then @glk takes ownership of
 * @user_data. When it is not needed anymore, it will be freed by calling
 * @destroy_user_data on it. If you wish to retain ownership of @user_data, pass
 * %NULL for @destroy_user_data.
 *
 * To deactivate the callback, call this function with @func set to %NULL.
 */
void
chimara_glk_set_resource_load_callback(ChimaraGlk *glk, ChimaraResourceLoadFunc func, gpointer user_data, GDestroyNotify destroy_user_data)
{
	CHIMARA_GLK_USE_PRIVATE(glk, priv);

	if(priv->resource_load_callback == func
		&& priv->resource_load_callback_data == user_data
		&& priv->resource_load_callback_destroy_data == destroy_user_data)
		return;

	if(priv->resource_load_callback_destroy_data)
		priv->resource_load_callback_destroy_data(priv->resource_load_callback_data);

	priv->resource_load_callback = func;
	priv->resource_load_callback_data = user_data;
	priv->resource_load_callback_destroy_data = destroy_user_data;
}
