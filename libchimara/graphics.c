#include "graphics.h"
#include "chimara-glk-private.h"
#include "magic.h"

#define BUFFER_SIZE (1024)

extern GPrivate *glk_data_key;
void on_size_prepared(GdkPixbufLoader *loader, gint width, gint height, struct image_info *info);
void on_pixbuf_closed(GdkPixbufLoader *loader, gpointer data);

static gboolean image_loaded;
static gboolean size_determined;

static struct image_info*
load_image_in_cache(glui32 image, gint width, gint height)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	giblorb_err_t blorb_error = 0;
	giblorb_result_t resource;
	GError *pixbuf_error = NULL;
	guchar *buffer;

	/* Lookup the proper resource */
	blorb_error = giblorb_load_resource(glk_data->resource_map, giblorb_method_FilePos, &resource, giblorb_ID_Pict, image);
	if(blorb_error != giblorb_err_None) {
		WARNING_S( "Error loading resource", giblorb_get_error_message(blorb_error) );
		return NULL;
	}

	struct image_info *info = g_new0(struct image_info, 1);
	info->resource_number = image;

	/* Load the resource */
	GdkPixbufLoader *loader = gdk_pixbuf_loader_new();
	g_signal_connect( loader, "size-prepared", G_CALLBACK(on_size_prepared), info ); 
	g_signal_connect( loader, "closed", G_CALLBACK(on_pixbuf_closed), NULL ); 

	/* Scale image if necessary */
	if(width > 0 && height > 0) {
		gdk_pixbuf_loader_set_size(loader, width, height);
		info->scaled = TRUE;
	}

	glk_stream_set_position(glk_data->resource_file, resource.data.startpos, seekmode_Start);
	buffer = g_malloc( BUFFER_SIZE * sizeof(guchar) );

	guint32 total_read = 0;
	image_loaded = FALSE;
	while(total_read < resource.length && !image_loaded) {
		guint32 num_read = glk_get_buffer_stream(glk_data->resource_file, (char *) buffer, BUFFER_SIZE);

		if( !gdk_pixbuf_loader_write(loader, buffer, MIN(BUFFER_SIZE, num_read), &pixbuf_error) ) {
			WARNING_S("Cannot read image", pixbuf_error->message);
			giblorb_unload_chunk(glk_data->resource_map, image);
			gdk_pixbuf_loader_close(loader, &pixbuf_error);
			g_free(buffer);
			return NULL;
		}

		total_read += num_read;
	}
	gdk_pixbuf_loader_close(loader, &pixbuf_error);
	giblorb_unload_chunk(glk_data->resource_map, resource.chunknum);
	g_free(buffer);

	/* Wait for the PixbufLoader to finish loading the image */
	g_mutex_lock(glk_data->resource_lock);
	while(!image_loaded) {
		g_cond_wait(glk_data->resource_loaded, glk_data->resource_lock);
	}
	g_mutex_unlock(glk_data->resource_lock);

	/* Store the image in the cache */
	gdk_threads_enter();

	if( g_slist_length(glk_data->image_cache) >= IMAGE_CACHE_MAX_NUM ) {
		struct image_info *head = (struct image_info*) glk_data->image_cache->data;
		gdk_pixbuf_unref(head->pixbuf);
		g_free(head);
		glk_data->image_cache = g_slist_remove_link(glk_data->image_cache, glk_data->image_cache);
	}
	info->pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
	gdk_pixbuf_ref(info->pixbuf);
	info->width = gdk_pixbuf_get_width(info->pixbuf);
	info->height = gdk_pixbuf_get_height(info->pixbuf);
	glk_data->image_cache = g_slist_prepend(glk_data->image_cache, info);

	gdk_threads_leave();

	g_object_unref(loader);
	return info;
}

void
on_size_prepared(GdkPixbufLoader *loader, gint width, gint height, struct image_info *info)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);

	g_mutex_lock(glk_data->resource_lock);
	info->width = width;
	info->height = height;
	size_determined = TRUE;
	g_cond_broadcast(glk_data->resource_info_available);
	g_mutex_unlock(glk_data->resource_lock);
}

void
on_pixbuf_closed(GdkPixbufLoader *loader, gpointer data)
{
	gdk_threads_enter();

	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);

	g_mutex_lock(glk_data->resource_lock);
	image_loaded = TRUE;
	g_cond_broadcast(glk_data->resource_loaded);
	g_mutex_unlock(glk_data->resource_lock);

	gdk_threads_leave();
}


void
clear_image_cache(struct image_info *data, gpointer user_data)
{
	gdk_pixbuf_unref(data->pixbuf);
	g_free(data);
}

static struct image_info*
image_cache_find(struct image_info* to_find)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	GSList *link = glk_data->image_cache;

	gdk_threads_enter();

	/* Empty cache */
	if(link == NULL) {
		gdk_threads_leave();
		printf("Cache miss for image %d\n", to_find->resource_number);
		return NULL;
	}

	/* Iterate over the cache to find the correct image and size */
	do {
		struct image_info *info = (struct image_info*) link->data;
		if(info->resource_number == to_find->resource_number) {
			/* Check size: are we looking for a scaled version or the original one? */
			if(to_find->scaled) {
				if(info->width >= to_find->width && info->height >= to_find->height) {
					gdk_threads_leave();
					printf("Cache hit for image %d\n", to_find->resource_number);
					return info; /* Found a good enough match */
				}
			} else {
				if(!info->scaled) {
					gdk_threads_leave();
					printf("Cache hit for image %d\n", to_find->resource_number);
					return info; /* Found a match */
				}
			}
		}
	} while( (link = g_slist_next(link)) );

	gdk_threads_leave();

	printf("Cache miss for image %d\n", to_find->resource_number);
	return NULL; /* No match found */
}

glui32
glk_image_get_info(glui32 image, glui32 *width, glui32 *height)
{
	struct image_info *to_find = g_new0(struct image_info, 1);
	struct image_info *found;
	to_find->resource_number = image;
	to_find->scaled = FALSE; /* we want the original image size */

	if( !(found = image_cache_find(to_find)) ) {
		found = load_image_in_cache(image, 0, 0);
		if(found == NULL)
			return FALSE;
	}

	if(width != NULL)
		*width = found->width;
	if(width != NULL)
		*height = found->height;
	return TRUE;
}

glui32
glk_image_draw(winid_t win, glui32 image, glsi32 val1, glsi32 val2)
{
	VALID_WINDOW(win, return FALSE);
	g_return_val_if_fail(win->type == wintype_Graphics, FALSE);

	struct image_info *to_find = g_new0(struct image_info, 1);
	struct image_info *info;
	GdkPixmap *canvas;

	/* Lookup the proper resource */
	to_find->resource_number = image;
	to_find->scaled = FALSE; /* we want the original image size */

	if( !(info = image_cache_find(to_find)) ) {
		info = load_image_in_cache(image, 0, 0);
		if(info == NULL)
			return FALSE;
	}

	gdk_threads_enter();

   	gtk_image_get_pixmap( GTK_IMAGE(win->widget), &canvas, NULL );
	if(canvas == NULL) {
		WARNING("Could not get pixmap");
		return FALSE;
	}

	gdk_draw_pixbuf( GDK_DRAWABLE(canvas), NULL, GDK_PIXBUF((GdkPixbuf*)info->pixbuf), 0, 0, val1, val2, -1, -1, GDK_RGB_DITHER_NONE, 0, 0 );

	/* Update the screen */
	gtk_widget_queue_draw(win->widget);

	gdk_threads_leave();

	return TRUE;
}


glui32
glk_image_draw_scaled(winid_t win, glui32 image, glsi32 val1, glsi32 val2, glui32 width, glui32 height)
{
	VALID_WINDOW(win, return FALSE);
	g_return_val_if_fail(win->type == wintype_Graphics, FALSE);

	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	struct image_info *to_find = g_new0(struct image_info, 1);
	struct image_info *info;
	struct image_info *scaled_info;
	GdkPixmap *canvas;

	/* Lookup the proper resource */
	to_find->resource_number = image;
	to_find->scaled = TRUE; /* any image size equal or larger than requested will do */

	if( !(info = image_cache_find(to_find)) ) {
		info = load_image_in_cache(image, width, height);
		if(info == NULL)
			return FALSE;
	}

	gdk_threads_enter();

   	gtk_image_get_pixmap( GTK_IMAGE(win->widget), &canvas, NULL );
	if(canvas == NULL) {
		WARNING("Could not get pixmap");
		return FALSE;
	}

	/* Scale the image if necessary */
	if(info->width != width || info->height != height) {
		GdkPixbuf *scaled = gdk_pixbuf_scale_simple(info->pixbuf, width, height, GDK_INTERP_BILINEAR);

		/* Add the scaled image into the image cache */
		scaled_info = g_new0(struct image_info, 1);
		scaled_info->resource_number = info->resource_number;
		scaled_info->width = gdk_pixbuf_get_width(scaled);
		scaled_info->height = gdk_pixbuf_get_width(scaled);
		scaled_info->pixbuf = scaled;
		scaled_info->scaled = TRUE;
		glk_data->image_cache = g_slist_prepend(glk_data->image_cache, scaled_info);

		/* Continue working with the scaled version */
		info = scaled_info;
	}

	gdk_draw_pixbuf( GDK_DRAWABLE(canvas), NULL, info->pixbuf, 0, 0, val1, val2, -1, -1, GDK_RGB_DITHER_NONE, 0, 0 );

	/* Update the screen */
	gtk_widget_queue_draw(win->widget);

	gdk_threads_leave();

	return TRUE;
}

void
glk_window_set_background_color(winid_t win, glui32 color) {
	win->background_color = color;
}

void
glk_window_fill_rect(winid_t win, glui32 color, glsi32 left, glsi32 top, glui32 width, glui32 height)
{
	VALID_WINDOW(win, return);
	g_return_if_fail(win->type == wintype_Graphics);


	gdk_threads_enter();

	GdkPixmap *map;
	gtk_image_get_pixmap( GTK_IMAGE(win->widget), &map, NULL );
	gdk_draw_rectangle( GDK_DRAWABLE(map), win->widget->style->white_gc, TRUE, left, top, width, height);
	gtk_widget_queue_draw(win->widget);

	gdk_threads_leave();
}

void
glk_window_erase_rect(winid_t win, glsi32 left, glsi32 top, glui32 width, glui32 height)
{
	glk_window_fill_rect(win, win->background_color, left, top, width, height);
}

void glk_window_flow_break(winid_t win)
{
}

/*** Called when the graphics window is resized. Resize the backing pixmap if necessary ***/
void
on_graphics_size_allocate(GtkWidget *widget, GtkAllocation *allocation, winid_t win)
{ 
	GdkPixmap *oldmap;
	gtk_image_get_pixmap( GTK_IMAGE(widget), &oldmap, NULL );
	gint oldwidth = 0;
	gint oldheight = 0;
 
	/* Determine whether a pixmap exists with the correct size */
	gboolean needs_resize = FALSE;
	if(oldmap == NULL)
		needs_resize = TRUE;
	else {
		gdk_drawable_get_size( GDK_DRAWABLE(oldmap), &oldwidth, &oldheight );
		if(oldwidth != allocation->width || oldheight != allocation->height)
			needs_resize = TRUE;
	}

	if(needs_resize) {
		/* Create a new pixmap */
		GdkPixmap *newmap = gdk_pixmap_new(widget->window, allocation->width, allocation->height, -1);
		gdk_draw_rectangle( GDK_DRAWABLE(newmap), widget->style->white_gc, TRUE, 0, 0, allocation->width, allocation->height);

		/* Copy the contents of the old pixmap */
		if(oldmap != NULL)
			gdk_draw_drawable( GDK_DRAWABLE(newmap), widget->style->white_gc, GDK_DRAWABLE(oldmap), 0, 0, 0, 0, oldwidth, oldheight);
		
		/* Use the new pixmap */
		gtk_image_set_from_pixmap( GTK_IMAGE(widget), newmap, NULL );
		g_object_unref(newmap);
	}
}

