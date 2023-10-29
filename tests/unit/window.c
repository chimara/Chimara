#include <stdio.h>

#include "glk.h"
#include "glkunit.h"

static int
test_blank_window_open_close(void)
{
    winid_t win = glk_window_open(0, 0, 0, wintype_Blank, 0);
    ASSERT_NONNULL(win, "opening window should succeed");

    glk_window_close(win, /* stream result = */ NULL);

    SUCCEED;
}

static int
test_blank_window_tree_open_close(void)
{
    winid_t left = glk_window_open(0, 0, 0, wintype_Blank, 0);
    ASSERT_NONNULL(left, "opening first window should succeed");

    winid_t right = glk_window_open(left, winmethod_Right | winmethod_Proportional, 50, wintype_Blank, 0);
    ASSERT_NONNULL(right, "opening second window should succeed");

    glk_window_close(left, /* stream result = */ NULL);
    glk_window_close(right, NULL);

    SUCCEED;
}

struct TestDescription tests[] = {
    { "open and close a blank window",
        test_blank_window_open_close },
    { "open and close a tree of blank windows",
        test_blank_window_tree_open_close },
    { NULL, NULL }
};
