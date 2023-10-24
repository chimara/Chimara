#include <gdk/gdk.h>
#include <glib.h>

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

unsigned
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
