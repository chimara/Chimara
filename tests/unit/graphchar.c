#include <stdio.h>

#include "glk.h"
#include "glkunit.h"

static int
test_graphics_char_input_supported(void)
{
    glui32 result = glk_gestalt(gestalt_GraphicsCharInput, 0);
    if (!result)
        BAIL_OUT("Graphics char input is not supported");
    SUCCEED;
}

static int
test_simple_graphics_char_input_works(void)
{
    winid_t win = glk_window_open(0, 0, 0, wintype_Graphics, 0);
    ASSERT_NONNULL(win, "opening window should succeed");

    glk_request_char_event(win);

    event_t ev;
    glk_select(&ev);
    ASSERT_SAME(ev.win, win);
    ASSERT_EQUAL(evtype_CharInput, ev.type);
    ASSERT_EQUAL(keycode_Return, ev.val1);
    ASSERT_EQUAL(0, ev.val2);

    glk_window_close(win, /* stream result = */ NULL);

    SUCCEED;
}

static int
test_unicode_graphics_char_input_works(void)
{
    winid_t win = glk_window_open(0, 0, 0, wintype_Graphics, 0);
    ASSERT_NONNULL(win, "opening window should succeed");

    glk_request_char_event_uni(win);

    event_t ev;
    glk_select(&ev);
    ASSERT_SAME(ev.win, win);
    ASSERT_EQUAL(evtype_CharInput, ev.type);
    ASSERT_EQUAL(0xc4, ev.val1);  /* expects Ä */
    ASSERT_EQUAL(0, ev.val2);

    glk_window_close(win, /* stream result = */ NULL);

    SUCCEED;
}

struct TestDescription tests[] = {
    { "graphics character input is supported",
        test_graphics_char_input_supported },
    { "simple graphics character input works",
        test_simple_graphics_char_input_works },
    { "unicode graphics character input works",
        test_unicode_graphics_char_input_works },
    { NULL, NULL }
};
