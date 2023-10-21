shared_module('bocfel', 'blorb.cpp', 'branch.cpp', 'dict.cpp', 'glkstart.cpp',
    'iff.cpp', 'io.cpp', 'mathop.cpp', 'memory.cpp', 'meta.cpp', 'objects.cpp',
    'osdep.cpp', 'patches.cpp', 'process.cpp', 'random.cpp', 'screen.cpp',
    'sound.cpp', 'stack.cpp', 'stash.cpp', 'unicode.cpp', 'util.cpp',
    'zoom.cpp', 'zterp.cpp',
    name_prefix: '',
    cpp_args: ['-DZTERP_GLK', '-DZTERP_GLK_BLORB', '-DZTERP_GLK_UNIX',
        '-DZTERP_UNIX', '-DZTERP_GLK_TICK', '-Wno-sign-compare'],
    include_directories: '../../libchimara',
    link_args: plugin_link_args, link_depends: plugin_link_depends,
    install: true, install_dir: plugindir)

install_data('COPYING.GPLv2', 'COPYING.GPLv3',
    install_dir: pkgdocsdir / 'bocfel')
