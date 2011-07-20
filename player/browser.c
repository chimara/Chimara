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

#include <glib-object.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include "browser.h"
#include "app.h"
#include "util.h"

typedef struct _ChimaraBrowserPrivate {
	GtkActionGroup *action_group;
	GtkListStore *library_model;
} ChimaraBrowserPrivate;

#define CHIMARA_BROWSER_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), CHIMARA_TYPE_BROWSER, ChimaraBrowserPrivate))
#define CHIMARA_BROWSER_USE_PRIVATE ChimaraBrowserPrivate *priv = CHIMARA_BROWSER_PRIVATE(self)

G_DEFINE_TYPE(ChimaraBrowser, chimara_browser, GTK_TYPE_WINDOW);

/* CALLBACKS */

static gboolean
on_browser_delete_event(GtkWidget *browser, GdkEvent *event)
{
	gtk_main_quit();
	return TRUE;
}

/* TYPE SYSTEM */

static void
chimara_browser_finalize(GObject *self)
{
	CHIMARA_BROWSER_USE_PRIVATE;
	g_object_unref(priv->action_group);
	g_object_unref(priv->library_model);

	/* Chain up */
	G_OBJECT_CLASS(chimara_browser_parent_class)->finalize(self);
}

static void
chimara_browser_class_init(ChimaraBrowserClass *klass)
{
	/* Override methods of parent classes */
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = chimara_browser_finalize;

	/* Private data */
	g_type_class_add_private(klass, sizeof(ChimaraBrowserPrivate));
}

static void
chimara_browser_init(ChimaraBrowser *self)
{
	CHIMARA_BROWSER_USE_PRIVATE;
	ChimaraApp *theapp = chimara_app_get();

	/* Set own properties */
	g_object_set(self,
		"title", _("Chimara"),
		NULL);

	/* Build user interface */
	char *object_ids[] = {
		"browser_group",
		"browser-box",
		"library-model",
		NULL
	};
	GtkBuilder *builder = new_builder_with_objects(object_ids);

	priv->action_group = GTK_ACTION_GROUP(load_object(builder, "browser_group"));
	g_object_ref(priv->action_group);

	GtkUIManager *uimanager = new_ui_manager("browser.menus");

	gtk_ui_manager_insert_action_group(uimanager, priv->action_group, 0);
	gtk_ui_manager_insert_action_group(uimanager, chimara_app_get_action_group(theapp), 1);
	GtkWidget *menubar = gtk_ui_manager_get_widget(uimanager, "/browser_menu");
	GtkWidget *toolbar = gtk_ui_manager_get_widget(uimanager, "/browser_toolbar");
	GtkWidget *vbox = GTK_WIDGET(load_object(builder, "browser-box"));
	gtk_container_add(GTK_CONTAINER(self), vbox);
	gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);

	priv->library_model = GTK_LIST_STORE(load_object(builder, "library-model"));
	g_object_ref(priv->library_model);

	/* Make up some fake library data */
	GdkPixbuf *fake_cover_art = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, 120, 120);
	gdk_pixbuf_fill(fake_cover_art, 0xA4000000);
	GtkTreeIter iter;
	gtk_list_store_append(priv->library_model, &iter);
	gtk_list_store_set(priv->library_model, &iter,
		0, "IFID-02343-234213D-23423",
	    1, "The Great Nose Picker",
	    2, "Hank Berriman",
	    3, 2004,
	    4, fake_cover_art,
	    -1);
	gdk_pixbuf_unref(fake_cover_art);

	g_signal_connect(self, "delete-event", G_CALLBACK(on_browser_delete_event), NULL);

	g_object_unref(uimanager);
	g_object_unref(builder);
}

/* PUBLIC FUNCTIONS */

GtkWidget *
chimara_browser_new(void)
{
	return GTK_WIDGET(g_object_new(CHIMARA_TYPE_BROWSER,
		"type", GTK_WINDOW_TOPLEVEL,
		NULL));
}

/* GLADE CALLBACKS */

void
action_add_file(GtkAction *action, ChimaraBrowser *browser)
{
}

void
action_add_watched_folder(GtkAction *action, ChimaraBrowser *browser)
{
}

void
action_remove_file(GtkAction *action, ChimaraBrowser *browser)
{
}

void
action_play(GtkAction *action, ChimaraBrowser *browser)
{
}

void
action_more_info(GtkAction *action, ChimaraBrowser *browser)
{
}
