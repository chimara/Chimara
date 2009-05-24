/* licensing and copyright information here */

#include <math.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <gmodule.h>
#include <pango/pango.h>
#include "chimara-glk.h"
#include "chimara-glk-private.h"
#include "glk.h"
#include "abort.h"
#include "window.h"
#include "glkstart.h"

#define CHIMARA_GLK_MIN_WIDTH 0
#define CHIMARA_GLK_MIN_HEIGHT 0

/**
 * SECTION:chimara-glk
 * @short_description: Widget which executes a Glk program
 * @stability: Unstable
 * @include: chimara/chimara-glk.h
 * 
 * The ChimaraGlk widget opens and runs a Glk program. The program must be
 * compiled as a plugin module, with a function <function>glk_main()</function>
 * that the Glk library can hook into.
 *
 * On Linux systems, this is a file with a name like 
 * <filename>plugin.so</filename>. For portability, you can use libtool and 
 * automake:
 * |[
 * pkglib_LTLIBRARIES = plugin.la
 * plugin_la_SOURCES = plugin.c foo.c bar.c
 * plugin_la_LDFLAGS = -module -shared -avoid-version -export-symbols-regex "^glk_main$$"
 * ]|
 * This will produce <filename>plugin.la</filename> which is a text file 
 * containing the correct plugin file to open (see the relevant section of the
 * <ulink 
 * url="http://www.gnu.org/software/libtool/manual/html_node/Finding-the-dlname.html">
 * Libtool manual</ulink>).
 */

typedef void (* glk_main_t) (void);
typedef void (* glkunix_startup_code_t) (glkunix_startup_t*);

enum {
    PROP_0,
    PROP_INTERACTIVE,
    PROP_PROTECT,
	PROP_DEFAULT_FONT_DESCRIPTION,
	PROP_MONOSPACE_FONT_DESCRIPTION,
	PROP_SPACING
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
	priv->default_font_desc = pango_font_description_from_string("Sans");
	priv->monospace_font_desc = pango_font_description_from_string("Monospace");
    priv->program = NULL;
    priv->thread = NULL;
    priv->event_queue = NULL;
    priv->event_lock = NULL;
    priv->event_queue_not_empty = NULL;
    priv->event_queue_not_full = NULL;
    priv->abort_lock = NULL;
    priv->abort_signalled = FALSE;
	priv->arrange_lock = NULL;
	priv->rearranged = NULL;
	priv->needs_rearrange = FALSE;
	priv->ignore_next_arrange_event = FALSE;
    priv->interrupt_handler = NULL;
    priv->root_window = NULL;
    priv->fileref_list = NULL;
    priv->current_stream = NULL;
    priv->stream_list = NULL;
	priv->timer_id = 0;
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
		case PROP_DEFAULT_FONT_DESCRIPTION:
			chimara_glk_set_default_font_description( glk, (PangoFontDescription *)g_value_get_pointer(value) );
			break;
		case PROP_MONOSPACE_FONT_DESCRIPTION:
			chimara_glk_set_monospace_font_description( glk, (PangoFontDescription *)g_value_get_pointer(value) );
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
		case PROP_DEFAULT_FONT_DESCRIPTION:
			g_value_set_pointer(value, priv->default_font_desc);
			break;
		case PROP_MONOSPACE_FONT_DESCRIPTION:
			g_value_set_pointer(value, priv->monospace_font_desc);
			break;
		case PROP_SPACING:
			g_value_set_uint(value, priv->spacing);
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

	/* Free the window arrangement signalling */
	g_mutex_lock(priv->arrange_lock);
	g_cond_free(priv->rearranged);
	g_mutex_unlock(priv->arrange_lock);
	g_mutex_free(priv->arrange_lock);
	priv->arrange_lock = NULL;
	
	/* Free private data */
	pango_font_description_free(priv->default_font_desc);
	pango_font_description_free(priv->monospace_font_desc);
	
    G_OBJECT_CLASS(chimara_glk_parent_class)->finalize(object);
}

/* Internal function: Recursively get the Glk window tree's size request */
static void
request_recurse(winid_t win, GtkRequisition *requisition, guint spacing)
{
	if(win->type == wintype_Pair)
	{
		/* Get children's size requests */
		GtkRequisition child1, child2;
		request_recurse(win->window_node->children->data, &child1, spacing);
		request_recurse(win->window_node->children->next->data, &child2, spacing);
		
		/* If the split is fixed, get the size of the fixed child */
		if((win->split_method & winmethod_DivisionMask) == winmethod_Fixed)
		{
			switch(win->split_method & winmethod_DirMask)
			{
				case winmethod_Left:
					child1.width = win->key_window?
						win->constraint_size * win->key_window->unit_width
						: 0;
					break;
				case winmethod_Right:
					child2.width = win->key_window?
						win->constraint_size * win->key_window->unit_width
						: 0;
					break;
				case winmethod_Above:
					child1.height = win->key_window?
						win->constraint_size * win->key_window->unit_height
						: 0;
					break;
				case winmethod_Below:
					child2.height = win->key_window?
						win->constraint_size * win->key_window->unit_height
						: 0;
					break;
			}
		}
		
		/* Add the children's requests */
		switch(win->split_method & winmethod_DirMask)
		{
			case winmethod_Left:
			case winmethod_Right:
				requisition->width = child1.width + child2.width + spacing;
				requisition->height = MAX(child1.height, child2.height);
				break;
			case winmethod_Above:
			case winmethod_Below:
				requisition->width = MAX(child1.width, child2.width);
				requisition->height = child1.height + child2.height + spacing;
				break;
		}
	}
	
	/* For non-pair windows, just use the size that GTK requests */
	else
		gtk_widget_size_request(win->frame, requisition);
}

/* Overrides gtk_widget_size_request */
static void
chimara_glk_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
    g_return_if_fail(widget);
    g_return_if_fail(requisition);
    g_return_if_fail(CHIMARA_IS_GLK(widget));
    
    ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(widget);
    
    /* For now, just pass the size request on to the root Glk window */
    if(priv->root_window) 
	{
		request_recurse(priv->root_window->data, requisition, priv->spacing);
		requisition->width += 2 * GTK_CONTAINER(widget)->border_width;
		requisition->height += 2 * GTK_CONTAINER(widget)->border_width;
	} 
	else 
	{
        requisition->width = CHIMARA_GLK_MIN_WIDTH + 2 * GTK_CONTAINER(widget)->border_width;
        requisition->height = CHIMARA_GLK_MIN_HEIGHT + 2 * GTK_CONTAINER(widget)->border_width;
    }
}

/* Recursively give the Glk windows their allocated space. Returns a window
 containing all children of this window that must be redrawn, or NULL if there 
 are no children that require redrawing. */
static winid_t
allocate_recurse(winid_t win, GtkAllocation *allocation, guint spacing)
{
	if(win->type == wintype_Pair)
	{
		GtkAllocation child1, child2;
		child1.x = allocation->x;
		child1.y = allocation->y;
		
		if((win->split_method & winmethod_DivisionMask) == winmethod_Fixed)
		{
			/* If the key window has been closed, then default to 0; otherwise
			 use the key window to determine the size */
			switch(win->split_method & winmethod_DirMask)
			{
				case winmethod_Left:
					child1.width = win->key_window? 
						CLAMP(win->constraint_size * win->key_window->unit_width, 0, allocation->width - spacing) 
						: 0;
					break;
				case winmethod_Right:
					child2.width = win->key_window? 
						CLAMP(win->constraint_size * win->key_window->unit_width, 0, allocation->width - spacing)
						: 0;
					break;
				case winmethod_Above:
					child1.height = win->key_window? 
						CLAMP(win->constraint_size * win->key_window->unit_height, 0, allocation->height - spacing)
						: 0;
					break;
				case winmethod_Below:
					child2.height = win->key_window?
						CLAMP(win->constraint_size * win->key_window->unit_height, 0, allocation->height - spacing)
						: 0;
					break;
			}
		}
		else /* proportional */
		{
			gdouble fraction = win->constraint_size / 100.0;
			switch(win->split_method & winmethod_DirMask)
			{
				case winmethod_Left:
					child1.width = (glui32) ceil( fraction * (allocation->width - spacing) );
					break;
				case winmethod_Right:
					child2.width = (glui32) ceil( fraction * (allocation->width - spacing) );
					break;
				case winmethod_Above:
					child1.height = (glui32) ceil( fraction * (allocation->height - spacing) );
					break;
				case winmethod_Below:
					child2.height = (glui32) ceil( fraction * (allocation->height - spacing) );
					break;
			}
		}
		
		/* Fill in the rest of the size requisitions according to the child specified above */
		switch(win->split_method & winmethod_DirMask)
		{
			case winmethod_Left:
				child2.width = allocation->width - spacing - child1.width;
				child2.x = child1.x + child1.width + spacing;
				child2.y = child1.y;
				child1.height = child2.height = allocation->height;
				break;
			case winmethod_Right:
				child1.width = allocation->width - spacing - child2.width;
				child2.x = child1.x + child1.width + spacing;
				child2.y = child1.y;
				child1.height = child2.height = allocation->height;
				break;
			case winmethod_Above:
				child2.height = allocation->height - spacing - child1.height;
				child2.x = child1.x;
				child2.y = child1.y + child1.height + spacing;
				child1.width = child2.width = allocation->width;
				break;
			case winmethod_Below:
				child1.height = allocation->height - spacing - child2.height;
				child2.x = child1.x;
				child2.y = child1.y + child1.height + spacing;
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
		glui32 newwidth = (glui32)(win->widget->allocation.width / win->unit_width);
		glui32 newheight = (glui32)(win->widget->allocation.height / win->unit_height);
		gint line;
		GtkTextBuffer *textbuffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );
		GtkTextIter start, end;
	
		for(line = 0; line < win->height; line++)
		{
			gtk_text_buffer_get_iter_at_line(textbuffer, &start, line);
			/* If this line is going to fall off the bottom, delete it */
			if(line >= newheight)
			{
				end = start;
				gtk_text_iter_forward_to_line_end(&end);
				gtk_text_iter_forward_char(&end);
				gtk_text_buffer_delete(textbuffer, &start, &end);
				break;
			}
			/* If this line is not long enough, add spaces on the end */
			if(newwidth > win->width)
			{
				gchar *spaces = g_strnfill(newwidth - win->width, ' ');
				gtk_text_iter_forward_to_line_end(&start);
				gtk_text_buffer_insert(textbuffer, &start, spaces, -1);
				g_free(spaces);
			}
			/* But if it's too long, delete characters from the end */
			else if(newwidth < win->width)
			{
				end = start;
				gtk_text_iter_forward_chars(&start, newwidth);
				gtk_text_iter_forward_to_line_end(&end);
				gtk_text_buffer_delete(textbuffer, &start, &end);
			}
			/* Note: if the widths are equal, do nothing */
		}
		/* Add blank lines if there aren't enough lines to fit the new size */
		if(newheight > win->height)
		{
			gchar *blanks = g_strnfill(win->width, ' ');
		    gchar **blanklines = g_new0(gchar *, (newheight - win->height) + 1);
		    int count;
		    for(count = 0; count < newheight - win->height; count++)
		        blanklines[count] = blanks;
		    blanklines[newheight - win->height] = NULL;
		    gchar *text = g_strjoinv("\n", blanklines);
		    g_free(blanklines); /* not g_strfreev() */
		    g_free(blanks);
		    
			gtk_text_buffer_get_end_iter(textbuffer, &start);
			gtk_text_buffer_insert(textbuffer, &start, "\n", -1);
		    gtk_text_buffer_insert(textbuffer, &start, text, -1);
		    g_free(text);
		}
	
		gboolean arrange = !(win->width == newwidth && win->height == newheight);
		win->width = newwidth;
		win->height = newheight;
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
    
    widget->allocation = *allocation;
            
    if(priv->root_window) {
		GtkAllocation child;
		child.x = allocation->x + GTK_CONTAINER(widget)->border_width;
		child.y = allocation->y + GTK_CONTAINER(widget)->border_width;
		child.width = CLAMP(allocation->width - 2 * GTK_CONTAINER(widget)->border_width, 0, allocation->width);
		child.height = CLAMP(allocation->height - 2 * GTK_CONTAINER(widget)->border_width, 0, allocation->height);
		winid_t arrange = allocate_recurse(priv->root_window->data, &child, priv->spacing);
		
		/* arrange points to a window that contains all text grid and graphics
		 windows which have been resized */
		g_mutex_lock(priv->arrange_lock);
		if(!priv->ignore_next_arrange_event)
		{
			if(arrange)
				event_throw(evtype_Arrange, arrange == priv->root_window->data? NULL : arrange, 0, 0);
		}
		else
			priv->ignore_next_arrange_event = FALSE;
		priv->needs_rearrange = FALSE;
		g_cond_signal(priv->rearranged);
		g_mutex_unlock(priv->arrange_lock);
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

/* G_PARAM_STATIC_STRINGS only appeared in GTK 2.13.0 */
#ifndef G_PARAM_STATIC_STRINGS
#define G_PARAM_STATIC_STRINGS (G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB)
#endif

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
    /**
     * ChimaraGlk::stopped:
     * @glk: The widget that received the signal
     *
     * The ::stopped signal is emitted when the a Glk program finishes
     * executing in the widget, whether it ended normally, or was interrupted.
     */ 
    chimara_glk_signals[STOPPED] = g_signal_new("stopped", 
        G_OBJECT_CLASS_TYPE(klass), 0, 
        G_STRUCT_OFFSET(ChimaraGlkClass, stopped), NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	/**
	 * ChimaraGlk::started:
	 * @glk: The widget that received the signal
	 *
	 * The ::started signal is emitted when a Glk program starts executing in
	 * the widget.
	 */
	chimara_glk_signals[STARTED] = g_signal_new ("started",
		G_OBJECT_CLASS_TYPE (klass), 0,
		G_STRUCT_OFFSET(ChimaraGlkClass, started), NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    /* Properties */
    /**
     * ChimaraGlk:interactive:
     *
     * Sets whether the widget is interactive. A Glk widget is normally 
     * interactive, but in non-interactive mode, keyboard and mouse input are 
     * ignored and the Glk program is controlled by chimara_glk_feed_text(). 
     * <quote>More</quote> prompts when a lot of text is printed to a text 
	 * buffer are also disabled. This is typically used when you wish to control
	 * an interpreter program by feeding it a predefined list of commands.
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

	/* We can't use G_PARAM_CONSTRUCT on these because then the constructor will
	 initialize them with NULL */
	/**
	 * ChimaraGlk:default-font-description:
	 * 
	 * Pointer to a #PangoFontDescription describing the default proportional 
	 * font, to be used in text buffer windows for example.
	 *
	 * Default value: font description created from the string 
	 * <quote>Sans</quote>
	 */
	g_object_class_install_property(object_class, PROP_DEFAULT_FONT_DESCRIPTION, 
		g_param_spec_pointer("default-font-description", _("Default Font"),
		_("Font description of the default proportional font"),
		G_PARAM_READWRITE | G_PARAM_LAX_VALIDATION | G_PARAM_STATIC_STRINGS) );

	/**
	 * ChimaraGlk:monospace-font-description:
	 *
	 * Pointer to a #PangoFontDescription describing the default monospace font,
	 * to be used in text grid windows and #style_Preformatted, for example.
	 *
	 * Default value: font description created from the string 
	 * <quote>Monospace</quote>
	 */
	g_object_class_install_property(object_class, PROP_MONOSPACE_FONT_DESCRIPTION, 
		g_param_spec_pointer("monospace-font-description", _("Monospace Font"),
		_("Font description of the default monospace font"),
		G_PARAM_READWRITE | G_PARAM_LAX_VALIDATION | G_PARAM_STATIC_STRINGS) );

	/**
	 * ChimaraGlk:spacing:
	 *
	 * The amount of space between the Glk windows.
	 */
	g_object_class_install_property(object_class, PROP_SPACING,
		g_param_spec_uint("spacing", _("Spacing"),
		_("The amount of space between Glk windows"),
		0, G_MAXUINT, 0,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_LAX_VALIDATION | G_PARAM_STATIC_STRINGS) );
	
    /* Private data */
    g_type_class_add_private(klass, sizeof(ChimaraGlkPrivate));
}

/* PUBLIC FUNCTIONS */

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
    ChimaraGlk *self = CHIMARA_GLK(g_object_new(CHIMARA_TYPE_GLK, NULL));
    ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(self);
    
    priv->event_queue = g_queue_new();
    priv->event_lock = g_mutex_new();
    priv->event_queue_not_empty = g_cond_new();
    priv->event_queue_not_full = g_cond_new();
    priv->abort_lock = g_mutex_new();
	priv->arrange_lock = g_mutex_new();
	priv->rearranged = g_cond_new();
    
    return GTK_WIDGET(self);
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
 * chimara_glk_set_default_font_description:
 * @glk: a #ChimaraGlk widget
 * @font: a #PangoFontDescription
 *
 * Sets @glk's default proportional font. See 
 * #ChimaraGlk:default-font-description.
 */
void 
chimara_glk_set_default_font_description(ChimaraGlk *glk, PangoFontDescription *font)
{
	g_return_if_fail(glk || CHIMARA_IS_GLK(glk));
	g_return_if_fail(font);
	
	ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(glk);
	pango_font_description_free(priv->default_font_desc);
	priv->default_font_desc = pango_font_description_copy(font);
	
	/* TODO: Apply the font description to all the windows and recalculate the sizes */
}

/**
 * chimara_glk_set_default_font_string:
 * @glk: a #ChimaraGlk widget
 * @font: string representation of a font description
 *
 * Sets @glk's default proportional font according to the string @font, which
 * must be a string in the form <quote><replaceable>FAMILY-LIST</replaceable> 
 * [<replaceable>STYLE-OPTIONS</replaceable>] 
 * [<replaceable>SIZE</replaceable>]</quote>, such as <quote>Charter,Utopia 
 * Italic 12</quote> or <quote>Sans</quote>. See 
 * #ChimaraGlk:default-font-description.
 */
void 
chimara_glk_set_default_font_string(ChimaraGlk *glk, const gchar *font)
{
	g_return_if_fail(glk || CHIMARA_IS_GLK(glk));
	g_return_if_fail(font || *font);
	
	PangoFontDescription *fontdesc = pango_font_description_from_string(font);
	g_return_if_fail(fontdesc);
	
	ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(glk);
	pango_font_description_free(priv->default_font_desc);
	priv->default_font_desc = fontdesc;
	
	/* TODO: Apply the font description to all the windows and recalculate the sizes */
}
	
/**
 * chimara_glk_get_default_font_description:
 * @glk: a #ChimaraGlk widget
 * 
 * Returns @glk's default proportional font.
 *
 * Return value: a newly-allocated #PangoFontDescription which must be freed
 * using pango_font_description_free(), or %NULL on error.
 */
PangoFontDescription *
chimara_glk_get_default_font_description(ChimaraGlk *glk)
{
	g_return_val_if_fail(glk || CHIMARA_IS_GLK(glk), NULL);
	
	ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(glk);
	return pango_font_description_copy(priv->default_font_desc);
}

/**
 * chimara_glk_set_monospace_font_description:
 * @glk: a #ChimaraGlk widget
 * @font: a #PangoFontDescription
 *
 * Sets @glk's default monospace font. See 
 * #ChimaraGlk:monospace-font-description.
 */
void 
chimara_glk_set_monospace_font_description(ChimaraGlk *glk, PangoFontDescription *font)
{
	g_return_if_fail(glk || CHIMARA_IS_GLK(glk));
	g_return_if_fail(font);
	
	ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(glk);
	pango_font_description_free(priv->monospace_font_desc);
	priv->monospace_font_desc = pango_font_description_copy(font);
	
	/* TODO: Apply the font description to all the windows and recalculate the sizes */
}

/**
 * chimara_glk_set_monospace_font_string:
 * @glk: a #ChimaraGlk widget
 * @font: string representation of a font description
 *
 * Sets @glk's default monospace font according to the string @font, which must
 * be a string in the form <quote><replaceable>FAMILY-LIST</replaceable> 
 * [<replaceable>STYLE-OPTIONS</replaceable>] 
 * [<replaceable>SIZE</replaceable>]</quote>, such as <quote>Courier 
 * Bold 12</quote> or <quote>Monospace</quote>. See 
 * #ChimaraGlk:monospace-font-description.
 */
void 
chimara_glk_set_monospace_font_string(ChimaraGlk *glk, const gchar *font)
{
	g_return_if_fail(glk || CHIMARA_IS_GLK(glk));
	g_return_if_fail(font || *font);
	
	PangoFontDescription *fontdesc = pango_font_description_from_string(font);
	g_return_if_fail(fontdesc);
	
	ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(glk);
	pango_font_description_free(priv->monospace_font_desc);
	priv->monospace_font_desc = fontdesc;
	
	/* TODO: Apply the font description to all the windows and recalculate the sizes */
}
	
/**
 * chimara_glk_get_monospace_font_description:
 * @glk: a #ChimaraGlk widget
 * 
 * Returns @glk's default monospace font.
 *
 * Return value: a newly-allocated #PangoFontDescription which must be freed
 * using pango_font_description_free(), or %NULL on error.
 */
PangoFontDescription *
chimara_glk_get_monospace_font_description(ChimaraGlk *glk)
{
	g_return_val_if_fail(glk || CHIMARA_IS_GLK(glk), NULL);
	
	ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(glk);
	return pango_font_description_copy(priv->monospace_font_desc);
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

/**
 * chimara_glk_run:
 * @glk: a #ChimaraGlk widget
 * @plugin: path to a plugin module compiled with <filename 
 * class="header">glk.h</filename>
 * @error: location to store a <link linkend="glib-GError">GError</link>, or 
 * %NULL
 *
 * Opens a Glk program compiled as a plugin and runs its glk_main() function in
 * a separate thread. On failure, returns %FALSE and sets @error.
 *
 * Return value: %TRUE if the Glk program was started successfully.
 */
gboolean
chimara_glk_run(ChimaraGlk *glk, gchar *plugin, int argc, char *argv[], GError **error)
{
    g_return_val_if_fail(glk || CHIMARA_IS_GLK(glk), FALSE);
    g_return_val_if_fail(plugin, FALSE);
    
    ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(glk);
    

    /* Open the module to run */
    glk_main_t glk_main;
	glkunix_startup_code_t glkunix_startup_code;
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

    if( g_module_symbol(priv->program, "glkunix_startup_code", (gpointer *) &glkunix_startup_code) )
    {
		glkunix_startup_t data;
		data.argc = argc;
		data.argv = argv;

		glkunix_startup_code(&data);
    }

    /* Run in a separate thread */
	priv->thread = g_thread_create(glk_enter, glk_main, TRUE, error);
	
	return !(priv->thread == NULL);
}

/**
 * chimara_glk_stop:
 * @glk: a #ChimaraGlk widget
 *
 * Signals the Glk program running in @glk to abort. Note that if the program is
 * caught in an infinite loop in which glk_tick() is not called, this may not
 * work.
 */
void
chimara_glk_stop(ChimaraGlk *glk)
{
    g_return_if_fail(glk || CHIMARA_IS_GLK(glk));
    /* TODO: check if glk is actually running a program */
    signal_abort();
}

/**
 * chimara_glk_wait:
 * @glk: a #ChimaraGlk widget
 *
 * Holds up the main thread and waits for the Glk program running in @glk to 
 * finish.
 */
void
chimara_glk_wait(ChimaraGlk *glk)
{
    g_return_if_fail(glk || CHIMARA_IS_GLK(glk));
    
    ChimaraGlkPrivate *priv = CHIMARA_GLK_PRIVATE(glk);
    g_thread_join(priv->thread);
}
