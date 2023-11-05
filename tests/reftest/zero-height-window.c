#include <stdbool.h>
#include <stddef.h>

#include "glk.h"

static void
ready_for_snapshot(winid_t win)
{
    glk_tick();
    glk_request_timer_events(100);
    event_t ev;
    while (true) {
        glk_select(&ev);
        switch (ev.type) {
            case evtype_Timer:
                glk_request_char_event(win);
                break;
            case evtype_CharInput:
                return;
        }
    }
}

void
glk_main(void)
{
    winid_t win1 = glk_window_open(NULL, 0, 0, wintype_TextBuffer, 1);
    winid_t win2 = glk_window_open(win1, winmethod_Above | winmethod_Proportional | winmethod_Border, 0, wintype_TextBuffer, 2);
    ready_for_snapshot(win1);
    glk_window_close(win2, NULL);
    glk_window_close(win1, NULL);
}
