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

#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CHIMARA_TYPE_PLAYER            (chimara_player_get_type())
#define CHIMARA_PLAYER(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), CHIMARA_TYPE_PLAYER, ChimaraPlayer))
#define CHIMARA_PLAYER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), CHIMARA_TYPE_PLAYER, ChimaraPlayerClass))
#define CHIMARA_IS_PLAYER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), CHIMARA_TYPE_PLAYER))
#define CHIMARA_IS_PLAYER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), CHIMARA_TYPE_PLAYER))
#define CHIMARA_PLAYER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), CHIMARA_TYPE_PLAYER, ChimaraPlayerClass))

typedef struct _ChimaraPlayer {
	GtkWindow parent_instance;
	
	/* Public pointers to widgets */
	GtkWidget *glk, *toolbar;
} ChimaraPlayer;

typedef struct _ChimaraPlayerClass {
	GtkWindowClass parent_class;
} ChimaraPlayerClass;

GType chimara_player_get_type(void) G_GNUC_CONST;
GtkWidget *chimara_player_new(void);

G_END_DECLS

#endif /* __PLAYER_H__ */
