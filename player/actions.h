#ifndef ACTIONS_H
#define ACTIONS_H

#include <gio/gio.h>

G_BEGIN_DECLS

void create_app_actions(GActionMap *actionmap, gpointer data);
void create_window_actions(GActionMap *actionmap, gpointer data);

G_END_DECLS

#endif /* ACTIONS_H */
