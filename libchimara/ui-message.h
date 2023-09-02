#ifndef UI_MESSAGE_H
#define UI_MESSAGE_H

#include <stdbool.h>

#include <glib.h>

#include "chimara-glk.h"
#include "glk.h"

typedef enum {
	/* PRINT_STRING:
	 * @win: a window.
	 * @strval: a string to print to the window.
	 */
	UI_MESSAGE_PRINT_STRING,
	/* CREATE_WINDOW:
	 * @win: a window with its UI-side data not filled in yet.
	 */
	UI_MESSAGE_CREATE_WINDOW,
	/* UNPARENT_WIDGET:
	 * @win: ignored. (This message should outlive the window structure.)
	 * @ptrval: a GtkWidget which will be unparented.
	 */
	UI_MESSAGE_UNPARENT_WIDGET,
	/* ARRANGE: Calls for a rearrange of all windows.
	 * @win: ignored.
	 */
	UI_MESSAGE_ARRANGE,
	/* ARRANGE_SILENTLY: Same as ARRANGE but does not send the Glk program an
	 * arrange event.
	 * @win: ignored.
	 */
	UI_MESSAGE_ARRANGE_SILENTLY,
	/* SYNC_ARRANGE: Waits for the window arrangement to become current.
	 * It only makes sense to send this message with ui_message_queue_and_await.
	 * @win: ignored.
	 */
	UI_MESSAGE_SYNC_ARRANGE,
	/* CLEAR_WINDOW: (flushes text buffer and text grids' window buffer first)
	 * @win: a text buffer, text grid, or graphics window.
	 */
	UI_MESSAGE_CLEAR_WINDOW,
	/* MOVE_CURSOR: (flushes window buffer first)
	 * @win: a text grid window.
	 * @uintval1: x position
	 * @uintval2: y position
	 */
	UI_MESSAGE_MOVE_CURSOR,
	/* GRID_NEWLINE: (flushes window buffer first)
	 * @win: a text grid window.
	 */
	UI_MESSAGE_GRID_NEWLINE,
	/* SET_STYLE: (flushes window buffer first)
	 * @win: a text grid or text buffer window.
	 * @uintval1: style to set.
	 */
	UI_MESSAGE_SET_STYLE,
	/* SET_ZCOLORS: (flushes window buffer first)
	 * @win: text grid or text buffer window.
	 * @uintval1: foreground color.
	 * @uintval2: background color.
	 */
	UI_MESSAGE_SET_ZCOLORS,
	/* SET_REVERSE_VIDEO: (flushes window buffer first)
	 * @win: text grid or text buffer window.
	 * @boolval: TRUE for reverse video.
	 */
	UI_MESSAGE_SET_REVERSE_VIDEO,
	/* SET_STYLEHINT:
	 * @win: ignored.
	 * @uintval1: "wintype" parameter of glk_stylehint_set()
	 * @uintval2: "styl" parameter
	 * @uintval3: "hint" parameter
	 * @intval: "val" parameter
	 */
	UI_MESSAGE_SET_STYLEHINT,
	/* CLEAR_STYLEHINT:
	 * @win: ignored.
	 * @uintval1: "wintype" parameter of glk_stylehint_clear()
	 * @uintval2: "styl" parameter
	 * @uintval3: "hint" parameter
	 */
	UI_MESSAGE_CLEAR_STYLEHINT,
	/* MEASURE_STYLE:
	 * @win: ignored.
	 * @uintval1: "styl" parameter of glk_style_measure()
	 * @uintval2: "hint" parameter
	 */
	UI_MESSAGE_MEASURE_STYLE,
	/* FILE_PROMPT:
	 * @win: a text grid or text buffer window.
	 * @uintval1: "usage" parameter of glk_fileref_create_by_prompt
	 * @uintval2: "fmode" parameter
	 * @strval: Glk UNIX's current directory.
	 */
	UI_MESSAGE_FILE_PROMPT,
	/* CONFIRM_FILE_OVERWRITE:
	 * @win: ignored.
	 * @strval: display name of file to be overwritten.
	 * When sent with queue_and_await(), returns 1 if confirmed, 0 if not.
	 */
	UI_MESSAGE_CONFIRM_FILE_OVERWRITE,
	/* REQUEST_CHAR_INPUT: (flush window buffer first)
	 * @win: text buffer or text grid window.
	 * @boolval: TRUE for unicode input request.
	 */
	UI_MESSAGE_REQUEST_CHAR_INPUT,
	/* CANCEL_CHAR_INPUT:
	 * @win: text buffer or text grid window with character input pending.
	 */
	UI_MESSAGE_CANCEL_CHAR_INPUT,
	/* FORCE_CHAR_INPUT:
	 * @win: text buffer or text grid window.
	 * @uintval1: key value of key to press.
	 */
	UI_MESSAGE_FORCE_CHAR_INPUT,
	/* REQUEST_LINE_INPUT: (flush window buffer first)
	 * @win: text buffer or text grid window.
	 * @uintval1: maximum length of returned input.
	 * @boolval: whether to pre-fill any text.
	 * @strval: text to pre-fill.
	 */
	UI_MESSAGE_REQUEST_LINE_INPUT,
	/* CANCEL_LINE_INPUT:
	 * @win: text buffer or text grid window with line input pending.
	 * Returns number of characters written before cancellation.
	 */
	UI_MESSAGE_CANCEL_LINE_INPUT,
	/* FORCE_LINE_INPUT:
	 * @win: text buffer or text grid window.
	 * @strval: text to enter.
	 * Returns number of characters actually written.
	 */
	UI_MESSAGE_FORCE_LINE_INPUT,
	/* SET_HYPERLINK: (flushes window buffer first)
	 * @win: text buffer or text grid window.
	 * @uintval1: hyperlink value.
	 */
	UI_MESSAGE_SET_HYPERLINK,
	/* REQUEST_HYPERLINK_INPUT: (flushes window buffer first)
	 * @win: text buffer or text grid window.
	 */
	UI_MESSAGE_REQUEST_HYPERLINK_INPUT,
	/* CANCEL_HYPERLINK_INPUT:
	 * @win: text buffer or text grid window.
	 */
	UI_MESSAGE_CANCEL_HYPERLINK_INPUT,
	/* GRAPHICS_DRAW_IMAGE:
	 * @win: graphics window.
	 * @ptrval: GdkPixbuf, msg takes ownership and transfers to UI thread.
	 * @x: x coordinate
	 * @y: y coordinate
	 */
	UI_MESSAGE_GRAPHICS_DRAW_IMAGE,
	/* GRAPHICS_FILL_RECT:
	 * @win: graphics window.
	 * @uintval1: color
	 * @uintval2: width in pixels
	 * @uintval3: height in pixels
	 * @x: x coordinate of left
	 * @y: y coordinate of top
	 */
	UI_MESSAGE_GRAPHICS_FILL_RECT,
	/* BUFFER_DRAW_IMAGE: (flushes window buffer first)
	 * @win: text buffer window.
	 * @uintval1: alignment
	 */
	UI_MESSAGE_BUFFER_DRAW_IMAGE,
	/* SHUTDOWN:
	 * @win: ignored.
	 */
	UI_MESSAGE_SHUTDOWN,
} UiMessageType;

typedef struct {
	UiMessageType type;
	winid_t win;

	/* For passing various parameters to the UI thread */
	glsi32 x, y;
	glui32 uintval1, uintval2, uintval3;
	glsi32 intval;
	char *strval;  /* must be either NULL or owned by UiMessage */
	gboolean boolval;
	gpointer ptrval;

	/* For signaling a simple response back to the Glk thread. */
	GMutex lock;
	GCond sign;
	GVariant *response;
	bool is_waiting : 1;
} UiMessage;

G_GNUC_INTERNAL UiMessage *ui_message_new(UiMessageType type, winid_t win);
G_GNUC_INTERNAL void ui_message_free(UiMessage *msg);
G_GNUC_INTERNAL void ui_message_queue(UiMessage *msg);
G_GNUC_INTERNAL gint64 ui_message_queue_and_await(UiMessage *msg);
G_GNUC_INTERNAL char *ui_message_queue_and_await_string(UiMessage *msg);
G_GNUC_INTERNAL void ui_message_perform(ChimaraGlk *glk, UiMessage *msg);

#endif /* UI_MESSAGE_H */
