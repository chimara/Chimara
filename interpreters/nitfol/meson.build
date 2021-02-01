nitfol_graphics = 'graphics.c'
# nitfol_graphics = 'no_graph.c'
nitfol_blorb = 'blorb.c'
# nitfol_blorb = 'no_blorb.c'
nitfol_sound = 'sound.c'
# nitfol_sound = 'no_snd.c'

opt2glkc = files('opt2glkc.pl')
nitfol_startunix = configure_file(command: [perl, opt2glkc, '-unix', '@INPUT@'],
    input: 'nitfol.opt', output: 'startunix.c')

copying_awk = files('copying.awk')
copying = files('COPYING')
nitfol_copying = custom_target('copying.c',
    command: [awk, '-f', copying_awk, copying], capture: true,
    output: 'copying.c')

bison_gen = generator(bison,
    output: ['@BASENAME@.tab.c', '@BASENAME@.tab.h'],
    arguments: ['@INPUT@', '--defines=@OUTPUT1@', '--output=@OUTPUT0@'])

nitfol_inform = bison_gen.process('inform.y')

y2help = files('y2help.pl')
inform_y = files('inform.y')
nitfol_dbg_help = custom_target('dbg_help.h',
    command: [perl, y2help, inform_y],
    output: ['dbg_help.h', 'dbg_help.texi'])

nitfol_extraflags = cc.get_supported_arguments(['-Wno-maybe-uninitialized',
    '-Wno-stringop-truncation', '-Wno-unused'])
shared_module('nitfol', 'automap.c', 'debug.c', 'decode.c', 'errmesg.c',
    'globals.c', 'hash.c', 'iff.c', 'infix.c', 'init.c', 'io.c', 'main.c',
    'objects.c', 'op_call.c', 'op_jmp.c', 'oplist.c', 'op_math.c', 'op_save.c',
    'op_table.c', 'op_v6.c', 'portfunc.c', 'quetzal.c', 'solve.c', 'stack.c',
    'struct.c', 'tokenise.c', 'undo.c', 'z_io.c', 'zscii.c', nitfol_graphics,
    nitfol_blorb, nitfol_sound, nitfol_copying, nitfol_startunix, nitfol_inform,
    name_prefix: '',
    c_args: ['-DSMART_TOKENISER', '-DUSE_INLINE', nitfol_extraflags],
    include_directories: '../../libchimara',
    link_args: plugin_link_args, link_depends: plugin_link_depends,
    install: true, install_dir: plugindir)

nitfol_opt = files('nitfol.opt')
nitfol_texi = files('nitfol.texi')
nitfol_options_info = custom_target('options.texi',
    command: [perl, opt2glkc, '-info', nitfol_opt], output: 'options.texi')
nitfol_info = custom_target('nitfol.info', input: nitfol_texi,
    command: [texinfo, '@INPUT@', '-I', meson.current_build_dir(),
        '-o', '@OUTPUT@'], output: 'nitfol.info',
    depends: [nitfol_dbg_help, nitfol_options_info],
    install: true, install_dir: get_option('infodir'))

# nitfol_manpage = custom_target('nitfol.6',
#     command: [perl, opt2glkc, '-man', nitfol_opt], output: 'nitfol.6',
#     install: true, install_dir: get_option('mandir') / 'man6')

install_data('ChangeLog', 'COPYING', 'README',
    install_dir: pkgdocsdir / 'nitfol')
