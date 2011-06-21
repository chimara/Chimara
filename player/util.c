/*
 * Copyright (C) 2008, 2009, 2010, 2011 Philip Chimento and Marijn van Vliet.
 * All rights reserved.
 *
 * Chimara is free software copyrighted by Philip Chimento and Marijn van Vliet.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither of the names Philip Chimento or Marijn van Vliet, nor the name of
 *    any other contributor may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <config.h>
#include "util.h"
#include "error.h"

char *
get_data_file_path(const char *filename)
{
	char *path = g_build_filename(PACKAGE_DATA_DIR, filename, NULL);
	if(g_file_test(path, G_FILE_TEST_EXISTS))
		return path;
#ifdef DEBUG
	g_free(path);
	path = g_build_filename(PACKAGE_SRC_DIR, filename, NULL);
	if(g_file_test(path, G_FILE_TEST_EXISTS))
		return path;
#endif /* DEBUG */
	g_error("Could not find data file: %s", filename);
}

GtkBuilder *
new_builder_with_objects(char **object_ids)
{
	GError *error = NULL;
	GtkBuilder *builder = gtk_builder_new();
	
	if( !gtk_builder_add_objects_from_file(builder, PACKAGE_DATA_DIR "/chimara.ui", object_ids, &error) ) {
#ifdef DEBUG
		g_error_free(error);
		error = NULL;
		if( !gtk_builder_add_objects_from_file(builder, PACKAGE_SRC_DIR "/chimara.ui", object_ids, &error) )
#endif /* DEBUG */
			goto fail;
	}
	return builder;

fail:
	error_dialog(NULL, error, _("Error while building interface: "));
	return NULL;
}

GObject *
load_object(GtkBuilder *builder, const char *name)
{
	GObject *retval;
	if( (retval = gtk_builder_get_object(builder, name)) == NULL) {
		error_dialog(NULL, NULL, "Error while getting object '%s'", name);
		g_error("Error while getting object '%s'", name);
	}
	return retval;
}

GtkUIManager *
new_ui_manager(const char *filename)
{
	GError *error = NULL;
	GtkUIManager *uimanager = gtk_ui_manager_new();
	char *path = g_build_filename(PACKAGE_DATA_DIR, filename, NULL);
	
	if( !gtk_ui_manager_add_ui_from_file(uimanager, path, &error) ) {
#ifdef DEBUG
		g_free(path);
		path = g_build_filename(PACKAGE_SRC_DIR, filename, NULL);
		g_error_free(error);
		error = NULL;
		if( !gtk_ui_manager_add_ui_from_file(uimanager, path, &error) )
#endif /* DEBUG */
			goto fail;
	}
	return uimanager;

fail:
	error_dialog(NULL, error, _("Error while building interface: "));
	return NULL;
}

