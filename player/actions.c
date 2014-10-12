#include <gio/gio.h>
#include <glib/gi18n.h>

#include "callbacks.h"

typedef void (*ActionCallback)(GSimpleAction *action, GVariant *param, gpointer data);

void
create_app_actions(GActionMap *actionmap, gpointer data)
{
	const GActionEntry actions[] = {
		{ "about", (ActionCallback)on_about_activate },
		{ "preferences", (ActionCallback)on_preferences_activate },
		{ "quit", (ActionCallback)on_quit_chimara_activate },
	};
	g_action_map_add_action_entries(actionmap, actions, G_N_ELEMENTS(actions), data);
}

void
create_window_actions(GActionMap *actionmap, gpointer data)
{
	const GActionEntry actions[] = {
		{ "copy", (ActionCallback)on_copy_activate },
		{ "paste", (ActionCallback)on_paste_activate },
		{ "open", (ActionCallback)on_open_activate },
		{ "open-recent", (ActionCallback)on_open_recent_activate },
		{ "stop", (ActionCallback)on_stop_activate },
		{ "undo", (ActionCallback)on_undo_activate },
		{ "save", (ActionCallback)on_save_activate },
		{ "restore", (ActionCallback)on_restore_activate },
		{ "restart", (ActionCallback)on_restart_activate },
		{ "quit", (ActionCallback)on_quit_activate },
	};
	g_action_map_add_action_entries(actionmap, actions, G_N_ELEMENTS(actions), data);
}

