# Automatically generate version.h
git_version = configuration_data({
    'GIT_MAJOR': 1,
    'GIT_MINOR': 3,
    'GIT_PATCH': 5,
})
git_version_h = configure_file(configuration: git_version, output: 'version.h')

git_extraflags = cc.get_supported_arguments(['-Wno-unused'])
shared_module('git', 'accel.c', 'compiler.c', 'gestalt.c', 'git.c',
    'git_unix.c', 'glkop.c', 'heap.c', 'memory.c', 'opcodes.c', 'operands.c',
    'peephole.c', 'savefile.c', 'saveundo.c', 'search.c', 'terp.c',
    git_version_h,
    name_prefix: '', c_args: git_extraflags,
    include_directories: '../../libchimara',
    link_args: plugin_link_args, link_depends: plugin_link_depends,
    install: true, install_dir: plugindir)

install_data('README.txt', install_dir: pkgdocsdir / 'git')
