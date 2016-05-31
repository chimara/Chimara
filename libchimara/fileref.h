#ifndef FILEREF_H
#define FILEREF_H

#include <glib.h>

#include "glk.h"
#include "gi_dispa.h"

struct glk_fileref_struct
{
	/*< private >*/
	glui32 magic, rock;
	gidispatch_rock_t disprock;
	/* Pointer to the list node in the global fileref list that contains this
	fileref */
	GList* fileref_list;
	/* Fileref parameters */
	gchar *filename; /* Always stored in the default filename encoding, not
		UTF8 or Latin-1 */
	char *basename; /* Name from which real filename was derived */
	glui32 orig_filemode; /* Used to check if the user gets a fileref in read
		mode and then tries to open it in write mode */
	glui32 usage;
};

G_GNUC_INTERNAL frefid_t fileref_new(char *filename, char *basename, glui32 rock, glui32 usage, glui32 orig_filemode);

#endif
