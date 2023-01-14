executable('test-multisession', 'test-multisession.c',
    c_args: '-DBUILDDIR="@0@"'.format(meson.current_build_dir()),
    include_directories: top_include, link_with: libchimara,
    dependencies: [gmodule, gtk])
executable('glulxercise', 'glulxercise.c',
    c_args: '-DPACKAGE_SRC_DIR="@0@"'.format(meson.current_source_dir()),
    include_directories: top_include, link_with: libchimara,
    dependencies: [gmodule, gtk])
executable('plugin-loader', 'plugin-loader.c',
    include_directories: top_include, link_with: libchimara,
    dependencies: [gmodule, gtk])
executable('test-close', 'test-close.c',
    c_args: '-DPACKAGE_SRC_DIR="@0@"'.format(meson.current_source_dir()),
    include_directories: top_include, link_with: libchimara,
    dependencies: [gmodule, gtk])
executable('csstest', 'csstest.c',
    c_args: '-DBUILDDIR="@0@"'.format(meson.current_build_dir()),
    include_directories: top_include, link_with: libchimara,
    dependencies: [gmodule, gtk])
glkunit_runner = executable('glkunit-runner', 'glkunit-runner.c',
    include_directories: top_include, link_with: libchimara,
    dependencies: [gmodule, gtk])
cssparse = executable('cssparse', 'cssparse.c',
    include_directories: top_include, link_with: libchimara,
    dependencies: [gmodule, gtk])

shared_module('first', 'first.c', name_prefix: '',
    include_directories: top_include,
    link_args: plugin_link_args, link_depends: plugin_link_depends)
shared_module('model', 'model.c', name_prefix: '',
    include_directories: top_include,
    link_args: plugin_link_args, link_depends: plugin_link_depends)
shared_module('gridtest', 'gridtest.c', name_prefix: '',
    include_directories: top_include,
    link_args: plugin_link_args, link_depends: plugin_link_depends)
shared_module('splittest', 'splittest.c', name_prefix: '',
    include_directories: top_include,
    link_args: plugin_link_args, link_depends: plugin_link_depends)
shared_module('multiwin', 'multiwin.c', name_prefix: '',
    include_directories: top_include,
    link_args: plugin_link_args, link_depends: plugin_link_depends)
shared_module('styletest', 'styletest.c', name_prefix: '',
    include_directories: top_include,
    link_args: plugin_link_args, link_depends: plugin_link_depends)
shared_module('test-userstyle', 'test-userstyle.c', name_prefix: '',
    include_directories: top_include,
    link_args: plugin_link_args, link_depends: plugin_link_depends)
shared_module('soundtest', 'soundtest.c', name_prefix: '',
    include_directories: top_include,
    link_args: plugin_link_args, link_depends: plugin_link_depends)
shared_module('fileio', 'fileio.c', name_prefix: '',
    include_directories: top_include, dependencies: glib,
    link_args: plugin_link_args, link_depends: plugin_link_depends)
datetime_test = shared_module('datetime', 'unit/datetime.c', 'unit/glkunit.c',
    name_prefix: '', include_directories: [top_include, '../libchimara'],
    link_args: plugin_link_args, link_depends: plugin_link_depends)

test_env = environment()
test_env.set('NO_AT_BRIDGE', '1')

test('datetime', glkunit_runner, args: [datetime_test], protocol: 'tap')
test('cssparse', cssparse, protocol: 'tap', env: test_env)
