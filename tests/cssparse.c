#include <glib.h>
#include <gtk/gtk.h>

#include <libchimara/chimara-glk.h>

static GtkWidget *glk;

static void
test_font_size_one_digit(void) {
    chimara_glk_set_css_from_string(CHIMARA_GLK(glk), "buffer { font-size: 8; }");
}

static void
test_font_size_no_unit(void) {
    chimara_glk_set_css_from_string(CHIMARA_GLK(glk), "buffer { font-size: 12; }");
}

static void
test_font_size_unit_pt(void) {
    chimara_glk_set_css_from_string(CHIMARA_GLK(glk), "buffer { font-size: 11pt; }");
}

static void
test_font_size_unit_other(void) {
    g_test_expect_message("Chimara", G_LOG_LEVEL_WARNING, "*px*");
    chimara_glk_set_css_from_string(CHIMARA_GLK(glk), "buffer { font-size: 11px; }");
    g_test_assert_expected_messages();
}

int
main(int argc, char *argv[])
{
    gtk_test_init(&argc, &argv);

    glk = chimara_glk_new();

    g_test_add_func("/css/font-size/one-digit", test_font_size_one_digit);
    g_test_add_func("/css/font-size/no-unit", test_font_size_no_unit);
    g_test_add_func("/css/font-size/unit-pt", test_font_size_unit_pt);
    g_test_add_func("/css/font-size/unit-other", test_font_size_unit_other);

    return g_test_run();
}
