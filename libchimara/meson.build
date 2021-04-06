install_headers('chimara-glk.h', 'chimara-if.h', 'garglk.h', 'gi_blorb.h',
    'gi_dispa.h', 'glk.h', 'glkstart.h',
    subdir: 'chimara/libchimara')

debug_args = []
if get_option('buildtype').startswith('debug')
    pluginsourcedir = meson.current_build_dir() / '..' / 'interpreters'
    debug_args = ['-DDEBUG', '-DPLUGINSOURCEDIR="@0@"'.format(pluginsourcedir)]
endif

chimara_marshallers = gnome.genmarshal('chimara-marshallers',
    sources: 'chimara-marshallers.txt', prefix: '_chimara_marshal',
    internal: true)
libchimara = library('chimara', 'abort.c', 'case.c', 'charset.c',
    'chimara-glk.c', 'chimara-if.c', 'datetime.c', 'dispatch.c', 'event.c',
    'fileref.c', 'garglk.c', 'gestalt.c', 'gi_blorb.c', 'gi_dispa.c', 'glk.c',
    'glkunix.c', 'graphics.c', 'hyperlink.c', 'init.c', 'input.c', 'magic.c',
    'mouse.c', 'pager.c', 'resource.c', 'schannel.c', 'stream.c', 'strio.c',
    'style.c', 'timer.c', 'ui-buffer.c', 'ui-graphics.c', 'ui-grid.c',
    'ui-message.c', 'ui-misc.c', 'ui-style.c', 'ui-textwin.c', 'ui-window.c',
    'window.c', chimara_marshallers,
    include_directories: top_include,
    c_args: [debug_args, '-DG_LOG_DOMAIN="Chimara"',
        '-DPLUGINDIR="@0@"'.format(abs_plugindir)],
    version: so_version, darwin_versions: darwin_versions,
    dependencies: [libm, glib, gthread, gmodule, gio, pango, gtk, gstreamer],
    install: true)

if get_option('introspection')
    gir = gnome.generate_gir(libchimara,
        sources: ['chimara-glk.c', 'chimara-glk.h', 'chimara-if.c',
            'chimara-if.h'],
        export_packages: 'chimara', namespace: 'Chimara', nsversion: '1.0',
        includes: ['GLib-2.0', 'GObject-2.0', 'Gtk-3.0'],
        symbol_prefix: 'chimara', identifier_prefix: 'Chimara',
        header: 'libchimara/chimara-glk.h',
        extra_args: [
            '--c-include=libchimara/chimara-if.h',
            '--sources-top-dirs=@0@'.format(meson.current_source_dir())],
        install: true)
else
    gir = [disabler(), disabler()]
endif

if get_option('vapi').enabled()
    gnome.generate_vapi('chimara', sources: gir[0],
        packages: ['glib-2.0', 'gobject-2.0', 'gtk+-3.0'],
        install: true)
endif

libchimara_dep = declare_dependency(include_directories: top_include,
    dependencies: [libm, glib, gthread, gmodule, gio, pango, gtk, gstreamer],
    link_with: libchimara)
