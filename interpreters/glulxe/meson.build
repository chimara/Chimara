glulxe_extraflags = cc.get_supported_arguments(['-Wno-strict-aliasing',
    '-Wno-uninitialized', '-Wno-unused'])
glulxe = shared_module('glulxe', 'accel.c', 'debugger.c', 'exec.c', 'files.c',
    'float.c', 'funcs.c', 'gestalt.c', 'glkop.c', 'heap.c', 'main.c',
    'operand.c', 'osdepend.c', 'profile.c', 'search.c', 'serial.c', 'string.c',
    'unixautosave.c', 'unixstrt.c', 'vm.c',
    name_prefix: '',
    c_args: ['-DOS_UNIX', '-DUNIX_RAND_GETRANDOM', glulxe_extraflags],
    include_directories: '../../libchimara',
    link_args: plugin_link_args, link_depends: plugin_link_depends,
    install: true, install_dir: plugindir)

glulxdump_extraflags = cc.get_supported_arguments(['-Wno-maybe-uninitialized'])
executable('glulxdump', 'glulxdump.c', include_directories: '../../libchimara',
    c_args: glulxdump_extraflags)

install_data('LICENSE', 'README.md', install_dir: pkgdocsdir / 'glulxe')
install_data('profile-analyze.py', install_dir: pkglibexecdir,
    install_mode: 'rwxr-xr-x')
