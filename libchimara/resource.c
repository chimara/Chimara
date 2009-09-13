#include "resource.h"

extern GPrivate *glk_data_key;

/**
 * giblorb_set_resource_map:
 * @file: The file stream to read the resource map from
 *
 * This function tells the library that the file is indeed the Blorby source
 * of all resource goodness. Whenever your program calls an image or sound
 * function, such as glk_image_draw(), the library will search this file for
 * the resource you request. 
 *
 * Do <emphasis>not</emphasis> close the stream after calling this function. 
 * The library is responsible for closing the stream at shutdown time.
 *
 * Returns: a Blorb error code.
 */
giblorb_err_t
giblorb_set_resource_map(strid_t file)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	giblorb_map_t *newmap; /* create map allocates memory */
	giblorb_err_t error = giblorb_create_map(file, &newmap);

	if(error != giblorb_err_None) {
		g_free(newmap);
		return error;
	}

	/* Check if there was already an existing resource map */
	if(glk_data->resource_map != NULL) {
		WARNING("Overwriting existing resource map.\n");
		giblorb_destroy_map(glk_data->resource_map);
		glk_stream_close(glk_data->resource_file, NULL);
	}

	glk_data->resource_map = newmap;
	glk_data->resource_file = file;
	return giblorb_err_None;
}

/**
 * giblorb_get_resource_map:
 * 
 * This function returns the current resource map being used. Returns %NULL
 * if giblorb_set_resource_map() has not been called yet.
 *
 * Returns: a resource map, or %NULL.
 */
giblorb_map_t*
giblorb_get_resource_map()
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	
	if(glk_data->resource_map == NULL) {
		WARNING("Resource map not set yet.\n");
	}

	return glk_data->resource_map;
}
