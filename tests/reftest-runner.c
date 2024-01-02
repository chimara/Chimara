#include "config.h"

#include <stdbool.h>

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <libchimara/chimara-glk.h>

/* in reftest-compare.c */
extern cairo_surface_t *reftest_compare_surfaces(cairo_surface_t *, cairo_surface_t *);

static const char *css = "\
* {\
    all: unset;\
}\
window{\
    background-color: green;\
}\
scrolledwindow {\
    border: 10px red solid;\
    margin: 5px;\
    padding: 5px;\
    background-color: blue;\
}";

static gboolean write_mode = FALSE;
static char **rest_args = NULL;
static GFile *source_dir = NULL;
static GFile *build_dir = NULL;

static const GOptionEntry test_args[] = {
    { "write", 'w', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &write_mode, "Write reference PNGs" },
    { G_OPTION_REMAINING, 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_FILENAME_ARRAY, &rest_args, "SOURCEDIR BUILDDIR" },
    { NULL }
};

static void
parse_command_line(int *argc, char ***argv)
{
    g_autoptr(GError) error = NULL;

    g_autoptr(GOptionContext) context = g_option_context_new("SOURCEDIR BUILDDIR - run reftests");
    g_option_context_add_main_entries(context, test_args, NULL);

    if (!g_option_context_parse(context, argc, argv, &error))
        g_error("option parsing failed: %s", error->message);

    gtk_init(argc, argv);

    if (g_strv_length(rest_args) < 2)
        g_error("Need source dir and build dir");
}

typedef struct {
    GtkWidget *window;
    GtkWidget *glk;
    const char *test_name;
    bool success : 1;
} TestData;

static cairo_surface_t *
load_image(const char *test_name, const char *ext)
{
    g_autofree char *png_name = g_strconcat(test_name, ext, NULL);
    g_autoptr(GFile) png = g_file_get_child(source_dir, png_name);
    g_autofree char *path = g_file_get_path(png);

    cairo_surface_t *image = cairo_image_surface_create_from_png(path);
    cairo_status_t ok = cairo_surface_status(image);
    if (ok != CAIRO_STATUS_SUCCESS)
        g_error("Error reading reftest file: %s", cairo_status_to_string(ok));

    return image;
}

static void
save_image(GFile *dir, const char *test_name, const char *ext, cairo_surface_t *image)
{
    g_autofree char *png_name = g_strconcat(test_name, ext, NULL);
    g_autoptr(GFile) png = g_file_get_child(dir, png_name);
    g_autofree char *path = g_file_get_path(png);

    cairo_status_t ok = cairo_surface_write_to_png(image, path);
    if (ok != CAIRO_STATUS_SUCCESS)
        g_error("Error writing reftest file: %s", cairo_status_to_string(ok));

    g_print("# wrote %s\n", path);
}

static void
on_glk_ready(ChimaraGlk *glk, TestData *data)
{
    cairo_surface_t *real_image = gtk_offscreen_window_get_surface(GTK_OFFSCREEN_WINDOW(data->window));

    g_autofree char *png_name = g_strdup_printf("%s.png", data->test_name);
    g_autoptr(GFile) png = g_file_get_child(source_dir, png_name);
    g_autofree char *path = g_file_get_path(png);

    if (write_mode) {
        save_image(source_dir, data->test_name, ".png", real_image);
    } else {
        cairo_surface_t *reference_image = load_image(data->test_name, ".png");

        cairo_surface_t *diff_image = reftest_compare_surfaces(real_image, reference_image);

        if (diff_image) {
            save_image(build_dir, data->test_name, ".actual.png", real_image);
            save_image(build_dir, data->test_name, ".diff.png", diff_image);
            cairo_surface_destroy(diff_image);
        } else {
            data->success = true;
        }

        cairo_surface_destroy(reference_image);
    }

    gtk_main_quit();
}

static char *
run_reftest(const char *test_name)
{
    g_autoptr(GError) error = NULL;

    TestData data;
    data.window = gtk_offscreen_window_new();
    data.glk = chimara_glk_new();
    data.test_name = test_name;
    data.success = false;

    gtk_widget_set_size_request(data.glk, 400, 400);
    chimara_glk_set_spacing(CHIMARA_GLK(data.glk), 5);
    gtk_container_add(GTK_CONTAINER(data.window), data.glk);

    g_autoptr(GtkCssProvider) border_provider = gtk_css_provider_new();
    if (!gtk_css_provider_load_from_data(border_provider, css, -1, &error)) {
        gtk_widget_destroy(data.window);
        return g_strdup_printf("CSS error: %s", error->message);
    }

    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(border_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_widget_show_all(data.window);

    g_signal_connect(data.glk, "waiting", G_CALLBACK(on_glk_ready), &data);

    g_autofree char *plugin_name = g_strdup_printf("%s.so", test_name);
    g_autoptr(GFile) plugin = g_file_get_child(build_dir, plugin_name);
    if (!chimara_glk_run_file(CHIMARA_GLK(data.glk), plugin, 0, NULL, &error)) {
        gtk_widget_destroy(data.window);
        return g_strdup_printf("Error starting Glk program: %s", error->message);
    }

    gtk_main();

    chimara_glk_stop(CHIMARA_GLK(data.glk));
    chimara_glk_wait(CHIMARA_GLK(data.glk));

    gtk_widget_destroy(data.window);

    if (!data.success)
        return g_strdup("Test didn't match reference image");

    return NULL;
}

int
main(int argc, char *argv[])
{
    GError *error = NULL;

    /* Use cairo image surface for rendering, to avoid GPU scaling */
    g_setenv("GDK_RENDERING", "image", FALSE);

    parse_command_line(&argc, &argv);

    source_dir = g_file_new_for_commandline_arg(rest_args[0]);
    build_dir = g_file_new_for_commandline_arg(rest_args[1]);
    g_strfreev(rest_args);

    g_autoptr(GFileEnumerator) dir = g_file_enumerate_children(source_dir, "standard::*",
        G_FILE_QUERY_INFO_NONE, NULL, &error);
    if (!dir) {
        g_print("Bail out! Can't open source dir: %s\n", error->message);
        return 77;
    }

    size_t total = 0;

    while (true) {
        GFile *file;
        if (!g_file_enumerator_iterate(dir, NULL, &file, NULL, &error)) {
            g_print("Bail out! Can't read source dir: %s\n", error->message);
            return 77;
        }
        if (!file)
            break;

        g_autofree char *basename = g_file_get_basename(file);
        if (!g_str_has_suffix(basename, ".c"))
            continue;

        total++;

        g_autofree char *root = g_strndup(basename, strlen(basename) - 2);
        g_autofree char *fail_reason = run_reftest(root);
        if (fail_reason)
            g_print("not ok %zu %s - %s\n", total, root, fail_reason);
        else
            g_print("ok %zu %s\n", total, root);
    }

    g_print("1..%zu\n", total);

    g_object_unref(source_dir);
    g_object_unref(build_dir);

    return 0;
}
