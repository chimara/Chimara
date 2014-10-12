#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <gio/gio.h>
#include <gtk/gtk.h>

#include <libchimara/chimara-glk.h>

G_BEGIN_DECLS

void on_open_activate(GSimpleAction *action, GVariant *param, ChimaraGlk *glk);
void on_open_recent_activate(GSimpleAction *action, GVariant *param, ChimaraGlk *glk);
void on_recent_item_activated(GtkRecentChooser *chooser, ChimaraGlk *glk);
void on_stop_activate(GSimpleAction *action, GVariant *param, ChimaraGlk *glk);
void on_quit_chimara_activate(GSimpleAction *action, GVariant *param, GtkApplication *app);
void on_copy_activate(GSimpleAction *action, GVariant *param, GtkWindow *toplevel);
void on_paste_activate(GSimpleAction *action, GVariant *param, GtkWindow *toplevel);
void on_preferences_activate(GSimpleAction *action, GVariant *param, GtkApplication *app);
void on_undo_activate(GSimpleAction *action, GVariant *value, ChimaraGlk *glk);
void on_save_activate(GSimpleAction *action, GVariant *value, ChimaraGlk *glk);
void on_restore_activate(GSimpleAction *action, GVariant *value, ChimaraGlk *glk);
void on_restart_activate(GSimpleAction *action, GVariant *value, ChimaraGlk *glk);
void on_quit_activate(GSimpleAction *action, GVariant *value, ChimaraGlk *glk);
void on_about_activate(GtkAction *action, GVariant *value, GtkApplication *app);

G_END_DECLS

#endif /* CALLBACKS_H */
