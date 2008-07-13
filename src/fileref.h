#ifndef FILEREF_H
#define FILEREF_H

#include <gtk/gtk.h>
#include "glk.h"

struct glk_fileref_struct
{
	glui32 rock;
	/* Pointer to the list node in the global fileref list that contains this
	fileref */
	GList* fileref_list;
	/* Fileref parameters */
	gchar *filename;
	glui32 filemode;
	glui32 usage;
};

#endif
