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

#ifndef __APP_H__
#define __APP_H__

#include <glib-object.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CHIMARA_TYPE_APP            (chimara_app_get_type())
#define CHIMARA_APP(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), CHIMARA_TYPE_APP, ChimaraApp))
#define CHIMARA_APP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), CHIMARA_TYPE_APP, ChimaraAppClass))
#define CHIMARA_IS_APP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), CHIMARA_TYPE_APP))
#define CHIMARA_IS_APP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), CHIMARA_TYPE_APP))
#define CHIMARA_APP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), CHIMARA_TYPE_APP, ChimaraAppClass))

typedef struct _ChimaraApp {
	GObject parent_instance;
	
	/* Public pointers */
	GtkWidget *browser_window;
	GtkWidget *aboutwindow;
	GtkWidget *prefswindow;
	/* Public settings */
	GSettings *prefs_settings;
	GSettings *state_settings;
} ChimaraApp;

typedef struct _ChimaraAppClass {
	GObjectClass parent_class;
} ChimaraAppClass;

GType chimara_app_get_type(void) G_GNUC_CONST;
ChimaraApp *chimara_app_get(void);
GtkActionGroup *chimara_app_get_action_group(ChimaraApp *self);

G_END_DECLS

#endif /* __APP_H__ */
