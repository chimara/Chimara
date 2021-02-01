frotz_extraflags = cc.get_supported_arguments(['-Wno-unused'])
shared_module('frotz', 'buffer.c', 'err.c', 'fastmem.c', 'files.c', 'glkmisc.c',
    'glkscreen.c', 'input.c', 'main.c', 'math.c', 'object.c', 'process.c',
    'quetzal.c', 'random.c', 'redirect.c', 'sound.c', 'stream.c', 'table.c',
    'text.c', 'variable.c',
    name_prefix: '', c_args: frotz_extraflags,
    include_directories: '../../libchimara',
    link_args: plugin_link_args, link_depends: plugin_link_depends,
    install: true, install_dir: plugindir)

install_data('AUTHORS', 'COPYING', 'README', 'TODO',
    install_dir: pkgdocsdir / 'frotz')
