#include "graphics.h"
#include "chimara-glk-private.h"
#include "magic.h"

#define BUFFER_SIZE (1024)

extern GPrivate *glk_data_key;
void on_size_prepared(GdkPixbufLoader *loader, gint width, gint height, struct image_info *info);

static gboolean size_determined;

glui32
glk_image_get_info(glui32 image, glui32 *width, glui32 *height)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	giblorb_result_t res;
	giblorb_err_t blorb_error = 0;
	GError *pixbuf_error = NULL;
	struct image_info *info = g_new0(struct image_info, 1);
	info->resource_number = image;
	guchar *buffer;

	//printf("glk_image_get_info(%d)\n", image);

	/* Lookup the proper resource */
	blorb_error = giblorb_load_resource(glk_data->resource_map, giblorb_method_FilePos, &res, giblorb_ID_Pict, image);
	if(blorb_error != giblorb_err_None) {
		WARNING_S( "Error loading resource", giblorb_get_error_message(blorb_error) );
		return FALSE;
	}

	if(width == NULL && height == NULL) {
		/* No size requested, don't bother loading the image */
		giblorb_unload_chunk(glk_data->resource_map, image);
		return TRUE;
	}

	/* Load the resource */
	GdkPixbufLoader *loader = gdk_pixbuf_loader_new();
	g_signal_connect( loader, "size-prepared", G_CALLBACK(on_size_prepared), info ); 
	glk_stream_set_position(glk_data->resource_file, res.data.startpos, seekmode_Start);
	buffer = g_malloc( BUFFER_SIZE * sizeof(guchar) );

	guint32 total_read = 0;
	size_determined = FALSE;
	while(total_read < res.length && !size_determined) {
		guint32 num_read = glk_get_buffer_stream(glk_data->resource_file, (char *) buffer, BUFFER_SIZE);

		if( !gdk_pixbuf_loader_write(loader, buffer, MIN(BUFFER_SIZE, num_read), &pixbuf_error) ) {
			WARNING_S("Cannot read image", pixbuf_error->message);
			giblorb_unload_chunk(glk_data->resource_map, image);
			gdk_pixbuf_loader_close(loader, &pixbuf_error);
			g_free(buffer);
			return FALSE;
		}

		total_read += num_read;
	}
	giblorb_unload_chunk(glk_data->resource_map, image);
	gdk_pixbuf_loader_close(loader, &pixbuf_error);
	g_free(buffer);

	/* Determine the image dimensions */
	if(!size_determined) {
		WARNING("Cannot read image size");
		g_free(info);
		return FALSE;
	}

	if(width != NULL)
		*width = info->width;
	if(height != NULL)
		*height =info->height;
	g_free(info);

	return TRUE;
}

void
on_size_prepared(GdkPixbufLoader *loader, gint width, gint height, struct image_info *info)
{
	info->width = width;
	info->height = height;
	size_determined = TRUE;
}

/*** Called when the graphics window is resized. Resize the backing pixmap if necessary ***/
void
on_graphics_size_allocate(GtkWidget *widget, GtkAllocation *allocation, winid_t win)
{
	printf("allocate to: %dx%d\n", allocation->width, allocation->height);
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
		printf("needs resize\n");
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

glui32
glk_image_draw(winid_t win, glui32 image, glsi32 val1, glsi32 val2) {
	VALID_WINDOW(win, return FALSE);
	g_return_val_if_fail(win->type == wintype_Graphics, FALSE);
	printf("Drawing image %d\n", (int)image);

	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	giblorb_result_t res;
	giblorb_err_t blorb_error = 0;
	GError *pixbuf_error = NULL;
	struct image_info *info = g_new0(struct image_info, 1);
	info->resource_number = image;
	guchar *buffer;
	GdkPixmap *canvas;

	/* Lookup the proper resource */
	blorb_error = giblorb_load_resource(glk_data->resource_map, giblorb_method_FilePos, &res, giblorb_ID_Pict, image);
	if(blorb_error != giblorb_err_None) {
		WARNING_S( "Error loading resource", giblorb_get_error_message(blorb_error) );
		return FALSE;
	}

	/* Load the resource */
	GdkPixbufLoader *loader = gdk_pixbuf_loader_new();
	glk_stream_set_position(glk_data->resource_file, res.data.startpos, seekmode_Start);
	buffer = g_malloc( BUFFER_SIZE * sizeof(guchar) );

	guint32 total_read = 0;
	while(total_read < res.length) {
		guint32 num_read = glk_get_buffer_stream(glk_data->resource_file, (char *) buffer, BUFFER_SIZE);

		if( !gdk_pixbuf_loader_write(loader, buffer, MIN(BUFFER_SIZE, num_read), &pixbuf_error) ) {
			WARNING_S("Cannot read image", pixbuf_error->message);
			giblorb_unload_chunk(glk_data->resource_map, image);
			gdk_pixbuf_loader_close(loader, &pixbuf_error);
			g_free(buffer);
			return FALSE;
		}

		total_read += num_read;
	}
	printf("Loading done\n");
	giblorb_unload_chunk(glk_data->resource_map, image);
	g_free(buffer);

   	gtk_image_get_pixmap( GTK_IMAGE(win->widget), &canvas, NULL );
	if(canvas == NULL) {
		WARNING("Could not get pixmap");
		gdk_pixbuf_loader_close(loader, &pixbuf_error);
		return FALSE;
	}

	GdkPixbuf *pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
	if(pixbuf == NULL) {
		WARNING("Could not read image");
		gdk_pixbuf_loader_close(loader, &pixbuf_error);
		return FALSE;
	}

	// TODO: FIX hang?
	gdk_draw_pixbuf( GDK_DRAWABLE(canvas), NULL, pixbuf, 0, 0, val1, val2, -1, -1, GDK_RGB_DITHER_NONE, 0, 0 );
	gdk_pixbuf_loader_close(loader, &pixbuf_error);

	/* Update the screen */
	gtk_widget_queue_draw(win->widget);

	return TRUE;
}

glui32
glk_image_draw_scaled(winid_t win, glui32 image, glsi32 val1, glsi32 val2, glui32 width, glui32 height)
{
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

	GdkPixmap *map;
	gtk_image_get_pixmap( GTK_IMAGE(win->widget), &map, NULL );
	gdk_draw_rectangle( GDK_DRAWABLE(map), win->widget->style->white_gc, TRUE, left, top, width, height);
	gtk_widget_queue_draw(win->widget);
}

void
glk_window_erase_rect(winid_t win, glsi32 left, glsi32 top, glui32 width, glui32 height)
{
	printf("erasing rect: %d %d %d %d\n", left, top, width, height);
	glk_window_fill_rect(win, win->background_color, left, top, width, height);
}

void glk_window_flow_break(winid_t win)
{
}
