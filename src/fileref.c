#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include "fileref.h"
#include "error.h"

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

/* Internal function: create a fileref using the given parameters. */
static frefid_t
fileref_new(gchar *filename, glui32 rock, glui32 usage, glui32 orig_filemode)
{
	g_return_val_if_fail(filename != NULL, NULL);

	frefid_t f = g_new0(struct glk_fileref_struct, 1);
	f->rock = rock;
	f->filename = g_strdup(filename);
	f->usage = usage;
	f->orig_filemode = orig_filemode;
	
	/* Add it to the global fileref list */
	fileref_list = g_list_prepend(fileref_list, f);
	f->fileref_list = fileref_list;
	
	return f;
}

/**
 * glk_fileref_create_temp:
 * @usage: Bitfield with one or more of the #fileusage_ constants.
 * @rock: The new fileref's rock value.
 *
 * Creates a reference to a temporary file. It is always a new file (one which
 * does not yet exist). The file (once created) will be somewhere out of the
 * player's way.
 *
 * A temporary file should never be used for long-term storage. It may be
 * deleted automatically when the program exits, or at some later time, say
 * when the machine is turned off or rebooted. You do not have to worry about
 * deleting it yourself.
 *
 * Returns: A new fileref, or #NULL if the fileref creation failed.
 */ 
frefid_t
glk_fileref_create_temp(glui32 usage, glui32 rock)
{
	/* Get a temp file */
	GError *error = NULL;
	gchar *filename = NULL;
	gint handle = g_file_open_tmp("glkXXXXXX", &filename, &error);
	if(handle == -1)
	{
		error_dialog(NULL, error, "Error creating temporary file: ");
		if(filename)
			g_free(filename);
		return NULL;
	}
	if(close(handle) == -1) /* There is no g_close()? */
	{
		error_dialog(NULL, NULL, "Error closing temporary file.");
		if(filename)
			g_free(filename);
		return NULL;
	}
	
	frefid_t f = fileref_new(filename, rock, usage, filemode_Write);
	g_free(filename);
	return f;
}

/**
 * glk_fileref_create_by_prompt:
 * @usage: Bitfield with one or more of the #fileusage_ constants.
 * @fmode: File mode, contolling the dialog's behavior.
 * @rock: The new fileref's rock value.
 *
 * Creates a reference to a file by opening a file chooser dialog. If @fmode is
 * #filemode_Read, then the file must already exist and the user will be asked
 * to select from existing files. If @fmode is #filemode_Write, then the file
 * should not exist; if the user selects an existing file, he or she will be
 * warned that it will be replaced. If @fmode is #filemode_ReadWrite, then the
 * file may or may not exist; if it already exists, the user will be warned
 * that it will be modified. The @fmode argument should generally match the
 * @fmode which will be used to open the file.
 *
 * Returns: A new fileref, or #NULL if the fileref creation failed or the
 * dialog was canceled.
 */
frefid_t
glk_fileref_create_by_prompt(glui32 usage, glui32 fmode, glui32 rock)
{
	/* TODO: Remember current working directory and last used filename
	for each usage */
	GtkWidget *chooser;

	gdk_threads_enter();

	switch(fmode)
	{
		case filemode_Read:
			chooser = gtk_file_chooser_dialog_new("Select a file to open", NULL,
				GTK_FILE_CHOOSER_ACTION_OPEN,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				NULL);
			gtk_file_chooser_set_action(GTK_FILE_CHOOSER(chooser),
				GTK_FILE_CHOOSER_ACTION_OPEN);
			break;
		case filemode_Write:
			chooser = gtk_file_chooser_dialog_new("Select a file to save to", NULL,
				GTK_FILE_CHOOSER_ACTION_SAVE,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				NULL);
			gtk_file_chooser_set_action(GTK_FILE_CHOOSER(chooser),
				GTK_FILE_CHOOSER_ACTION_SAVE);
			gtk_file_chooser_set_do_overwrite_confirmation(
				GTK_FILE_CHOOSER(chooser), TRUE);
			break;
		case filemode_ReadWrite:
		case filemode_WriteAppend:
			chooser = gtk_file_chooser_dialog_new("Select a file to save to", NULL,
				GTK_FILE_CHOOSER_ACTION_SAVE,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				NULL);
			gtk_file_chooser_set_action(GTK_FILE_CHOOSER(chooser),
				GTK_FILE_CHOOSER_ACTION_SAVE);
			break;
		default:
			g_warning("glk_fileref_create_by_prompt: Unsupported mode");
			gdk_threads_leave();
			return NULL;
	}
	
	if(gtk_dialog_run( GTK_DIALOG(chooser) ) != GTK_RESPONSE_ACCEPT)
	{
		gtk_widget_destroy(chooser);
		gdk_threads_leave();
		return NULL;
	}
	gchar *filename = 
		gtk_file_chooser_get_filename( GTK_FILE_CHOOSER(chooser) );
	frefid_t f = fileref_new(filename, rock, usage, fmode);
	g_free(filename);
	gtk_widget_destroy(chooser);

	gdk_threads_leave();
	return f;
}

/**
 * glk_fileref_create_by_name:
 * @usage: Bitfield with one or more of the #fileusage_ constants.
 * @name: A filename.
 * @rock: The new fileref's rock value.
 *
 * This creates a reference to a file with a specific name. The file will be
 * in the same directory as your program, and visible to the player.
 *
 * Returns: A new fileref, or #NULL if the fileref creation failed. 
 */
frefid_t
glk_fileref_create_by_name(glui32 usage, char *name, glui32 rock)
{
	g_return_val_if_fail(name != NULL && strlen(name) > 0, NULL);

	/* Find out what encoding filenames are in */
	const gchar **charsets; /* Do not free */
	g_get_filename_charsets(&charsets);

	/* Convert name to that encoding */
	GError *error = NULL;
	gchar *osname = g_convert(name, -1, charsets[0], "ISO-8859-1", NULL, NULL,
		&error);
	if(osname == NULL)
	{
		error_dialog(NULL, error, "Error during latin1->filename conversion: ");
		return NULL;
	}

	/* Do any string-munging here to remove illegal characters from filename.
	On ext3, the only illegal characters are '/' and '\0'. TODO: Should this
	function be allowed to reference files in other directories, or should we
	disallow '/'? */

	frefid_t f = fileref_new(osname, rock, usage, filemode_ReadWrite);
	g_free(osname);
	return f;
}

/**
 * glk_fileref_create_from_fileref:
 * @usage: Bitfield with one or more of the #fileusage_ constants.
 * @fref: Fileref to copy.
 * @rock: The new fileref's rock value.
 *
 * This copies an existing file reference @fref, but changes the usage. (The
 * original @fref is not modified.)
 *
 * If you write to a file in text mode and then read from it in binary mode,
 * the results are platform-dependent.
 *
 * Returns: A new fileref, or #NULL if the fileref creation failed. 
 */
frefid_t
glk_fileref_create_from_fileref(glui32 usage, frefid_t fref, glui32 rock)
{
	return fileref_new(fref->filename, rock, usage, fref->orig_filemode);
}

/**
 * glk_fileref_destroy:
 * @fref: Fileref to destroy.
 * 
 * Destroys a fileref which you have created. This does not affect the disk
 * file.
 *
 * It is legal to destroy a fileref after opening a file with it (while the
 * file is still open.) The fileref is only used for the opening operation,
 * not for accessing the file stream.
 */
void
glk_fileref_destroy(frefid_t fref)
{
	fileref_list = g_list_delete_link(fileref_list, fref->fileref_list);
	if(fref->filename)
		g_free(fref->filename);
	g_free(fref);
}

/**
 * glk_fileref_delete_file:
 * @fref: A refrence to the file to delete.
 *
 * Deletes the file referred to by @fref. Does not destroy @fref itself.
 */
void
glk_fileref_delete_file(frefid_t fref)
{
	if( glk_fileref_does_file_exist(fref) )
		if(g_unlink(fref->filename) == -1)
			error_dialog(NULL, NULL, "Error deleting file %s", fref->filename);
}

/**
 * glk_fileref_does_file_exist:
 * @fref: A fileref to check.
 *
 * Checks whether the file referred to by @fref exists.
 *
 * Returns: #TRUE (1) if @fref refers to an existing file, #FALSE (0) if not.
 */
glui32
glk_fileref_does_file_exist(frefid_t fref)
{
	if( g_file_test(fref->filename, G_FILE_TEST_EXISTS) )
		return 1;
	return 0;
}

