#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include <gio/gio.h>
#include <gtk/gtk.h>
#include <libchimara/chimara-glk.h>
#include <libchimara/chimara-if.h>

/* See https://eblong.com/zarf/plotex/regtest.html for the description of the
 * file format. */

typedef enum {
    REGTEST_PARSE_ERROR_MALFORMED_PARAMETER,
    REGTEST_PARSE_ERROR_STRAY_CHECK,
    REGTEST_PARSE_ERROR_UNKNOWN_CHECK,
    REGTEST_PARSE_ERROR_UNKNOWN_INPUT_MODIFIER,
    REGTEST_PARSE_ERROR_UNKNOWN_PARAMETER,
} RegtestParseError;

GQuark regtest_parse_error_quark(void);
#define REGTEST_PARSE_ERROR (regtest_parse_error_quark())
G_DEFINE_QUARK(regtest-parse-error, regtest_parse_error);

typedef enum {
    LINE_INPUT,
    CHAR_INPUT,
    TIMER_INPUT,
    HYPERLINK_INPUT,
    RESIZE_WINDOW,
    REDRAW_WINDOW,
    CONTAIN,
    MATCH,
    IMAGE,
} CheckType;

typedef enum {
    MAIN,
    STATUS,
    GRAPHICS,
} WindowCheckType;

typedef struct {
    size_t lineno;
    char *text;
    CheckType type : 4;
    WindowCheckType wintype : 2;
    bool invert : 1;
    bool vital : 1;
} Check;

static Check *
new_check(CheckType type, size_t lineno, const char *text)
{
    Check *retval = g_new0(Check, 1);
    retval->type = type;
    retval->lineno = lineno;
    retval->text = g_strdup(text);
    return retval;
}

static void
free_check(Check *check)
{
    g_free(check->text);
    g_free(check);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC(Check, free_check);

typedef struct {
    char *name;
    GPtrArray *checks;  /* type: Check */
    GSList *messages;   /* type: char* */
    char *skip_reason;
    unsigned check_ix;
    bool failed : 1;
} Test;

static Test *
new_test(const char *name)
{
    Test *retval = g_new0(Test, 1);
    retval->name = g_strdup(name);
    retval->checks = g_ptr_array_new_with_free_func((GDestroyNotify) free_check);
    retval->messages = NULL;
    return retval;
}

static void
fail_test(Test *test, char *message /* transfer full */)
{
    test->failed = true;
    test->messages = g_slist_prepend(test->messages, message);
}

static void
skip_test(Test *test, char *message /* transfer full */)
{
    test->failed = true;
    g_clear_pointer(&test->skip_reason, g_free);
    test->skip_reason = message;
}

static void
free_test(Test *test)
{
    g_free(test->name);
    g_ptr_array_free(test->checks, /* free_data = */ true);
    g_slist_free_full(test->messages, g_free);
    g_free(test->skip_reason);
    g_free(test);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC(Test, free_test);

typedef struct {
    GFile *game_file;
    GPtrArray *tests;  /* type: Test */
} TestPlan;

static void
free_test_plan(TestPlan *testplan)
{
    g_object_unref(testplan->game_file);
    g_ptr_array_free(testplan->tests, /* free_data = */ true);
    g_free(testplan);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC(TestPlan, free_test_plan);

static struct NamedKey {
    const char *specifier;
    unsigned keysym;
} named_key_table[] = {
    { "delete", GDK_KEY_BackSpace },
    { "down", GDK_KEY_Down },
    { "end", GDK_KEY_End },
    { "escape", GDK_KEY_Escape },
    { "func1", GDK_KEY_F1 },
    { "func2", GDK_KEY_F2 },
    { "func3", GDK_KEY_F3 },
    { "func4", GDK_KEY_F4 },
    { "func5", GDK_KEY_F5 },
    { "func6", GDK_KEY_F6 },
    { "func7", GDK_KEY_F7 },
    { "func8", GDK_KEY_F8 },
    { "func9", GDK_KEY_F9 },
    { "func10", GDK_KEY_F10 },
    { "func11", GDK_KEY_F11 },
    { "func12", GDK_KEY_F12 },
    { "home", GDK_KEY_Home },
    { "left", GDK_KEY_Left },
    { "pagedown", GDK_KEY_Page_Down },
    { "pageup", GDK_KEY_Page_Up },
    { "return", GDK_KEY_Return },
    { "right", GDK_KEY_Right },
    { "space", GDK_KEY_space },
    { "tab", GDK_KEY_Tab },
    { "up", GDK_KEY_Up },
    { NULL, 0 },
};

static unsigned
char_input_specifier_to_keysym(const char *specifier)
{
    long n_utf8 = g_utf8_strlen(specifier, -1);

    if (n_utf8 == 0) {
        return GDK_KEY_Return;
    }

    if (n_utf8 == 1) {
        uint32_t c = g_utf8_get_char_validated(specifier, -1);
        unsigned keysym = gdk_unicode_to_keyval(c);
        if (keysym & 0x01000000)
            g_warning("Could not find keysym for U+%04x", c);
        return keysym;
    }

    for (struct NamedKey *iter = named_key_table; iter->specifier != NULL; iter++) {
        if (strcmp(specifier, iter->specifier) == 0)
            return iter->keysym;
    }

    unsigned long code = strtoul(specifier, /* endptr = */ NULL,
        g_str_has_prefix(specifier, "0x") ? 16 : 10);
    if (code == 0 || code == ULLONG_MAX)
        return 0xffffffff;
    return gdk_unicode_to_keyval(code);
}

static size_t
consume_check_modifier(char *line, bool *invert, bool *vital, WindowCheckType *wintype)
{
    if (line[0] == '!') {
        *invert = true;
        return 1;
    }
    if (strncmp(line, "{invert}", 8) == 0) {
        *invert = true;
        return 8;
    }
    if (strncmp(line, "{vital}", 7) == 0) {
        *vital = true;
        return 7;
    }
    if (strncmp(line, "{status}", 8) == 0) {
        *wintype = STATUS;
        return 8;
    }
    if (strncmp(line, "{graphics}", 10) == 0) {
        *wintype = GRAPHICS;
        return 10;
    }
    return 0;
}

static TestPlan *
parse_regtest_file(GFile *regtest_file, GCancellable *cancel, GError **error)
{
    g_autoptr(GFileInputStream) in_str = g_file_read(regtest_file, cancel, error);
    if (!in_str)
        return NULL;
    g_autoptr(GDataInputStream) str = g_data_input_stream_new(G_INPUT_STREAM(in_str));

    g_autoptr(TestPlan) testplan = g_new0(TestPlan, 1);
    testplan->tests = g_ptr_array_new_with_free_func((GDestroyNotify) free_test);

    g_autoptr(Test) current_test = NULL;

    size_t lineno = 0;
    while (true) {
        lineno++;
        g_autofree char *orig_line = g_data_input_stream_read_line_utf8(str, /* length = */ NULL, cancel, error);
        char *line = orig_line;
        if (!line) {
            if (*error)
                return NULL;
            break;
        }

        /* Blank lines and comment lines are ignored */
        if (line[0] == '\0' || line[0] == '#')
            continue;

        if (line[0] == '*') {
            if (line[1] == '*') {
                g_auto(GStrv) set_param = g_strsplit(&line[2], ":", 2);
                if (!set_param[0] || !set_param[1]) {
                    g_set_error(error, REGTEST_PARSE_ERROR, REGTEST_PARSE_ERROR_MALFORMED_PARAMETER,
                                "%zu: Malformed test parameter line \"%s\"", lineno, line);
                    return NULL;
                }
                const char *param = g_strstrip(set_param[0]);
                const char *value = g_strstrip(set_param[1]);

                if (strcmp(param, "game") == 0) {
                    g_autoptr(GFile) parent = g_file_get_parent(regtest_file);
                    testplan->game_file = g_file_get_child(parent, value);
                    continue;
                }
                if (strcmp(param, "interpreter") == 0) {
                    continue;  /* ignore */
                }
                if (strcmp(param, "remformat") == 0) {
                    continue;  /* ignore */
                }

                g_set_error(error, REGTEST_PARSE_ERROR, REGTEST_PARSE_ERROR_UNKNOWN_PARAMETER,
                            "%zu: Unknown test parameter '%s'", lineno, param);
                return NULL;
            }

            /* Current test is finished, start a new one */
            if (current_test)
                g_ptr_array_add(testplan->tests, g_steal_pointer(&current_test));
            current_test = new_test(g_strstrip(&line[1]));

            continue;
        }

        /* Other lines besides '*' and comment lines must be part of a test */
        if (!current_test) {
            g_set_error(error, REGTEST_PARSE_ERROR, REGTEST_PARSE_ERROR_STRAY_CHECK,
                        "%zu: line must be part of a test", lineno);
            return NULL;
        }

        if (line[0] == '>') {
            if (line[1] == '{') {
                size_t modifier_len = strcspn(&line[2], "}");
                g_autofree char *input_modifier = g_new0(char, modifier_len + 1);
                strncpy(input_modifier, &line[2], modifier_len);
                char *rest = line + modifier_len + 3;  /* skip past ">{", modifier, "}" */

                if (strcmp(input_modifier, "char") == 0) {
                    g_ptr_array_add(current_test->checks, new_check(CHAR_INPUT, lineno, g_strstrip(rest)));
                    continue;
                }

                if (strcmp(input_modifier, "timer") == 0) {
                    g_ptr_array_add(current_test->checks, new_check(TIMER_INPUT, lineno, NULL));
                    continue;
                }

                if (strcmp(input_modifier, "hyperlink") == 0) {
                    g_ptr_array_add(current_test->checks, new_check(HYPERLINK_INPUT, lineno, g_strstrip(rest)));
                    continue;
                }

                if (strcmp(input_modifier, "arrange") == 0) {
                    g_ptr_array_add(current_test->checks, new_check(RESIZE_WINDOW, lineno, g_strstrip(rest)));
                    continue;
                }

                if (strcmp(input_modifier, "refresh") == 0) {
                    g_ptr_array_add(current_test->checks, new_check(REDRAW_WINDOW, lineno, NULL));
                    continue;
                }

                g_set_error(error, REGTEST_PARSE_ERROR, REGTEST_PARSE_ERROR_UNKNOWN_INPUT_MODIFIER,
                            "%zu: unsupported input modifier {%s}", lineno, input_modifier);
                return NULL;
            }

            g_ptr_array_add(current_test->checks, new_check(LINE_INPUT, lineno, g_strstrip(&line[1])));
            continue;
        }

        /* Check modifiers */
        bool invert = false;
        bool vital = false;
        WindowCheckType wintype = MAIN;
        size_t nconsumed = 0;
        while ((nconsumed = consume_check_modifier(line, &invert, &vital, &wintype)) != 0)
            line += nconsumed;

        CheckType type = CONTAIN;
        if (line[0] == '/') {
            type = MATCH;
            line++;
        }

        if (line[0] == '{') {
            size_t modifier_len = strcspn(&line[1], "}");
            g_autofree char *check_type = g_new0(char, modifier_len + 1);
            strncpy(check_type, &line[1], modifier_len);
            line += modifier_len + 2;  /* skip past "{", modifier, "}" */

            if (strncmp(check_type, "hyperlink", 9) == 0) {
                /* we can't distinguish hyperlinks, but the text should show up
                 * in the regular text stream */
                type = CONTAIN;
            } else if (strncmp(check_type, "image", 5) == 0) {
                type = IMAGE;
            } else {
                g_set_error(error, REGTEST_PARSE_ERROR, REGTEST_PARSE_ERROR_UNKNOWN_CHECK,
                        "%zu: unsupported check type {%s}", lineno, check_type);
                return NULL;
            }
        }
        /* Anything without a / or { is just a matcher */

        Check *check = new_check(type, lineno, g_strstrip(line));
        check->invert = invert;
        check->vital = vital;
        check->wintype = wintype;
        g_ptr_array_add(current_test->checks, check);
    }

    if (current_test)
        g_ptr_array_add(testplan->tests, g_steal_pointer(&current_test));

    return g_steal_pointer(&testplan);
}

static void
on_stopped(ChimaraIF *glk, Test *test)
{
    if (test->check_ix < test->checks->len)
        fail_test(test, g_strdup("Test did not complete"));
    gtk_main_quit();
}

static void
on_command(ChimaraIF *glk, const char *input, const char *response, Test *test)
{
#ifdef DEBUG
    g_autofree char *escaped = g_strescape(response, NULL);
    g_message("command(test=%s): %s -> %.30s", test->name, input, escaped);
#endif
    const Check *check;
    while (true) {
        if (test->check_ix >= test->checks->len) {
            /* Test is done */
            gtk_main_quit();
            return;
        }
        check = g_ptr_array_index(test->checks, test->check_ix++);

        if (check->wintype != MAIN) {
            skip_test(test, g_strdup_printf("%zu: could not check %s window for '%s'",
                check->lineno, check->wintype == STATUS ? "status" : "graphics", check->text));
            continue;
        }

        switch (check->type) {
        case LINE_INPUT:
            chimara_glk_feed_line_input(CHIMARA_GLK(glk), check->text);
            /* Wait for next response */
            return;

        case CHAR_INPUT: {
            unsigned key = char_input_specifier_to_keysym(check->text);
            chimara_glk_feed_char_input(CHIMARA_GLK(glk), key);
            /* Wait for next response */
            return;
        }

        case TIMER_INPUT:
            skip_test(test, g_strdup_printf("%zu: could not deliver timer input", check->lineno));
            break;

        case HYPERLINK_INPUT: {
            uint64_t linkval = g_ascii_strtoull(check->text, NULL, 10);
            if (linkval > G_MAXUINT32)
                fail_test(test, g_strdup_printf("%zu: hyperlink value %s too big", check->lineno, check->text));

            skip_test(test, g_strdup_printf("%zu: could not deliver hyperlink input for %" G_GUINT64_FORMAT, check->lineno, linkval));
            break;
        }

        case RESIZE_WINDOW: {
            char *next;
            uint64_t width = g_ascii_strtoull(check->text, &next, 10);
            uint64_t height = g_ascii_strtoull(next, NULL, 10);
            GtkWidget *win = gtk_widget_get_toplevel(GTK_WIDGET(glk));
            int current_height;
            gtk_window_get_size(GTK_WINDOW(win), NULL, &current_height);
            if (height == 0)
                height = current_height;
            gtk_window_set_default_size(GTK_WINDOW(win), width, height);
            break;
        }

        case REDRAW_WINDOW:
            gtk_widget_queue_draw(GTK_WIDGET(glk));
            break;

        case CONTAIN: {
            bool success = strstr(response, check->text) != NULL;
            if (check->invert)
                success = !success;
            if (!success)
                fail_test(test, g_strdup_printf("%zu: output must%s contain '%s'",
                    check->lineno, check->invert ? " not" : "", check->text));
            break;
        }

        case MATCH: {
            g_autoptr(GError) error = NULL;
            g_autoptr(GRegex) regex = g_regex_new(check->text, /* flags = */ 0, 0, &error);
            if (!regex) {
                fail_test(test, g_strdup_printf("%zu: invalid regex pattern '%s'", check->lineno, check->text));
                break;
            }
            bool success = g_regex_match(regex, response, /* flags = */ 0, /* match info = */ NULL);
            if (check->invert)
                success = !success;
            if (!success)
                fail_test(test, g_strdup_printf("%zu: output must%s match '%s'",
                    check->lineno, check->invert ? " not" : "", check->text));
            break;
        }

        case IMAGE:
            skip_test(test, g_strdup_printf("%zu: could not check image", check->lineno));
            break;
        }

        if (test->failed && check->vital) {
            g_print("Bail out! Vital test failed");
            gtk_main_quit();
            return;
        }
    }
}

int
main(int argc, char *argv[])
{
    g_autoptr(GError) error = NULL;

    g_print("TAP version 13\n");

    gtk_init(&argc, &argv);

    GtkWidget *glk = chimara_if_new();
    chimara_glk_set_interactive(CHIMARA_GLK(glk), FALSE);
    chimara_if_set_preferred_interpreter(CHIMARA_IF(glk), CHIMARA_IF_FORMAT_GLULX, CHIMARA_IF_INTERPRETER_GLULXE);
    chimara_if_set_preferred_interpreter(CHIMARA_IF(glk), CHIMARA_IF_FORMAT_GLULX_BLORB, CHIMARA_IF_INTERPRETER_GLULXE);
    chimara_glk_set_css_from_string(CHIMARA_GLK(glk), "buffer { font-size: 10pt; }");

    GtkWidget *win = gtk_offscreen_window_new();
    gtk_window_set_default_size(GTK_WINDOW(win), 800, 600);
    gtk_container_add(GTK_CONTAINER(win), glk);
    gtk_widget_show_all(win);

    if(argc < 2) {
        g_print("Bail out! Must provide a regtest specification\n");
        return 77;
    }

    g_autoptr(GFile) regtest_file = g_file_new_for_commandline_arg(argv[1]);
    g_autoptr(TestPlan) testplan = parse_regtest_file(regtest_file, /* cancellable = */ NULL, &error);
    if (error) {
        g_print("Bail out! Error parsing regtest specification: %s\n", error->message);
        return 77;
    }

    g_print("1..%u\n", testplan->tests->len);

    for (unsigned ix = 0; ix < testplan->tests->len; ix++) {
        Test *test = g_ptr_array_index(testplan->tests, ix);

        unsigned long stopped_id = g_signal_connect(glk, "stopped", G_CALLBACK(on_stopped), test);
        unsigned long command_id = g_signal_connect(glk, "command", G_CALLBACK(on_command), test);
        if (!chimara_if_run_game_file(CHIMARA_IF(glk), testplan->game_file, &error)) {
            g_print("Bail out! Error starting Glk library: %s\n", error->message);
            return 77;
        }

        gtk_main();
        g_signal_handler_disconnect(glk, stopped_id);
        g_signal_handler_disconnect(glk, command_id);

        chimara_glk_stop(CHIMARA_GLK(glk));
        chimara_glk_wait(CHIMARA_GLK(glk));

        g_autofree char *skip_message;
        if (test->skip_reason) {
            skip_message = g_strdup_printf(" # TODO: %s", test->skip_reason);
        } else {
            skip_message = g_strdup("");
        }
        g_print("%sok %s%s\n", test->failed ? "not " : "", test->name, skip_message);
        if (test->failed) {
            test->messages = g_slist_reverse(test->messages);
            for (GSList *iter = test->messages; iter; iter = g_slist_next(iter))
                g_print("# %s\n", (char *)iter->data);
        }
    }

    gtk_widget_destroy(win);

    return 0;
}
