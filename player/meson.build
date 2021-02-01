player_resources = gnome.compile_resources('resources',
    'resources/org.chimara-if.player.gresource.xml',
    source_dir: 'resources')

executable('chimara', 'actions.c', 'callbacks.c', 'error.c', 'main.c',
    'preferences.c', player_resources,
    include_directories: top_include,
    link_with: libchimara, dependencies: [glib, gmodule, gtk],
    install: true)

gnome.compile_schemas(depend_files: 'org.chimara-if.gschema.xml')
install_data('org.chimara-if.gschema.xml',
    install_dir: datadir / 'glib-2.0' / 'schemas')
