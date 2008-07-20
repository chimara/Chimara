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
	gchar *filename; /* Always stored in the default filename encoding, not
		UTF8 or Latin-1 */
	glui32 orig_filemode; /* Used to check if the user gets a fileref in read
		mode and then tries to open it in write mode */
	glui32 usage;
};

#endif
