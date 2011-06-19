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