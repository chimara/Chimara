/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * callbacks.c
 * Copyright (C) Philip en Marijn 2008 <>
 * 
 * callbacks.c is free software copyrighted by Philip en Marijn.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name ``Philip en Marijn'' nor the name of any other
 *    contributor may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 
 * callbacks.c IS PROVIDED BY Philip en Marijn ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Philip en Marijn OR ANY OTHER CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <gdk/gdkkeysyms.h>
#include <libchimara/chimara-glk.h>
#include "error.h"

void on_save(GtkAction *action, ChimaraGlk *glk) {
	GSList *widgets = gtk_action_get_proxies(action);
	GtkWindow *top = GTK_WINDOW( gtk_widget_get_toplevel(widgets->data) );
	error_dialog(top, NULL, "Not implemented yet");
}

gboolean on_window_delete_event(GtkWidget *widget, GdkEvent *event, ChimaraGlk *glk) {
	gtk_main_quit();
	return TRUE;
}

void on_quit(GtkAction *action, ChimaraGlk *glk) {
	gtk_main_quit();
}

void on_hint(GtkAction *action, ChimaraGlk *glk) {
	chimara_glk_feed_line_input(glk, "se");
	chimara_glk_feed_line_input(glk, "push cans to window");
	chimara_glk_feed_line_input(glk, "stand on cans");
	chimara_glk_feed_line_input(glk, "open window");
	chimara_glk_feed_line_input(glk, "enter window");
}

void on_press_r(GtkAction *action, ChimaraGlk *glk) {
	chimara_glk_feed_char_input(glk, GDK_R);
}

void on_press_enter(GtkAction *action, ChimaraGlk *glk) {
	chimara_glk_feed_char_input(glk, GDK_Return);
	chimara_glk_feed_char_input(glk, GDK_Return);
	chimara_glk_feed_char_input(glk, GDK_Return);
}
