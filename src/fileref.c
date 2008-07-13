#include "fileref.h"

/* List of streams currently in existence */
static GList *fileref_list = NULL;

/**
 * glk_fileref_iterate:
 * @fref: A file reference, or #NULL.
 * @rockptr: Return location for the next window's rock, or #NULL.
 *
 * Iterates over the list of file references; if @fref is #NULL, it returns the
 * first file reference, otherwise the next file reference after @fref. If 
 * there are no more, it returns #NULL. The file reference's rock is stored in
 * @rockptr. If you don't want the rocks to be returned, you may set @rockptr 
 * to #NULL.
 *
 * The order in which file references are returned is arbitrary. The order may
 * change every time you create or destroy a file reference, invalidating the 
 * iteration.
 *
 * Returns: the next file reference, or #NULL if there are no more.
 */
frefid_t
glk_fileref_iterate(frefid_t fref, glui32 *rockptr)
{
	GList *retnode;
	
	if(fref == NULL)
		retnode = fileref_list;
	else
		retnode = fref->fileref_list->next;
	frefid_t retval = retnode? (frefid_t)retnode->data : NULL;
		
	/* Store the fileref's rock in rockptr */
	if(retval && rockptr)
		*rockptr = glk_fileref_get_rock(retval);
		
	return retval;
}

/**
 * glk_fileref_get_rock:
 * @fref: A file reference.
 * 
 * Returns the file reference @fref's rock value.
 *
 * Returns: A rock value.
 */
glui32
glk_fileref_get_rock(frefid_t fref)
{
	g_return_val_if_fail(fref != NULL, 0);
	return fref->rock;
}
