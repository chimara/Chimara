/* This is a Cfunctions (version 0.24) generated header file.
   Cfunctions is a free program for extracting headers from C files.
   Get Cfunctions from `http://www.hayamasa.demon.co.uk/cfunctions'. */

/* This file was generated with:
`cfunctions -i graphics.c' */
#ifndef CFH_GRAPHICS_H
#define CFH_GRAPHICS_H

/* From `graphics.c': */

#ifdef GLK_MODULE_IMAGE
glui32 wrap_glk_image_draw (winid_t win , glui32 image , glsi32 val1 , glsi32 val2 );
glui32 wrap_glk_image_draw_scaled (winid_t win , glui32 image , glsi32 val1 , glsi32 val2 , glui32 width , glui32 height );
glui32 wrap_glk_image_get_info (glui32 image , glui32 *width , glui32 *height );

#endif

#endif /* CFH_GRAPHICS_H */
