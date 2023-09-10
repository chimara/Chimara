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
glulxercise_runner = executable('glulxercise-runner', 'glulxercise-runner.c',
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
stream_test = shared_module('stream', 'unit/stream.c', 'unit/glkunit.c',
    name_prefix: '', include_directories: [top_include, '../libchimara'],
    link_args: plugin_link_args, link_depends: plugin_link_depends)

test_env = environment()
test_env.set('NO_AT_BRIDGE', '1')
test_env.set('G_DEBUG', 'fatal-warnings')
test_env.set('SOURCE_DIR', meson.current_source_dir())

test('datetime', glkunit_runner, args: [datetime_test], suite: 'unit',
    protocol: 'tap', env: test_env)
test('stream', glkunit_runner, args: [stream_test], suite: 'unit',
    protocol: 'tap', env: test_env)

glulxercise_tests = [
    'abbrevtest',
    'abbrevtest-e',
    'chinesedicttest',
    'datetimetest',
    'dictflagtest',
    'extbinaryfile',
    'externalfile',
    'floatconsttest',
    'floatparsetest',
    'glulxercise',
    'graphwintest',
    'imagetest',
    'inputeventtest',
    'memcopytest',
    'memheaptest',
    'memstreamtest',
    'resstreamtest',
    'startsavetest',
    'unicasetest',
    'unicodetest',
    'unidicttest',
    'unisourcetest',
    'unisourcetestb',
    'windowtest',
]

foreach t : glulxercise_tests
    path = files('glulxercise/@0@.regtest'.format(t))
    test(t, glulxercise_runner, args: [path], suite: 'glulxercise',
        protocol: 'tap', env: test_env, depends: [glulxe, glulxercise_runner])
endforeach

test('cssparse', cssparse, protocol: 'tap', env: test_env)
