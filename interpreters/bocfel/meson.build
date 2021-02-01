shared_module('bocfel', 'blorb.c', 'branch.c', 'dict.c', 'glkstart.c', 'iff.c',
    'io.c', 'math.c', 'memory.c', 'meta.c', 'objects.c', 'osdep.c', 'process.c',
    'random.c', 'screen.c', 'sound.c', 'stack.c', 'unicode.c', 'util.c',
    'zoom.c', 'zterp.c',
    name_prefix: '', c_args: ['-DZTERP_GLK', '-DZTERP_UNIX'],
    include_directories: '../../libchimara',
    link_args: plugin_link_args, link_depends: plugin_link_depends,
    install: true, install_dir: plugindir)

install_data('BUILDING', 'COPYING.GPLv2', 'COPYING.GPLv3', 'README',
    install_dir: pkgdocsdir / 'bocfel')
