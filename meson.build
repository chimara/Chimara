project('chimara', 'c', 'cpp', version: '0.9.4', license: 'BSD-3-clause',
    meson_version: '>= 0.50.0')

libname = meson.project_name()
version = meson.project_version()

pkgconfig = import('pkgconfig')
gnome = import('gnome')
i18n = import('i18n')

cc = meson.get_compiler('c')

### LIBRARY VERSION INFO #######################################################

lt_current = 0  # Increment if interface changes
lt_revision = 0 # Increment if source changes; set 0 if interface changes
lt_age = 0      # Increment if interfaces added; set 0 if removed
so_version = '@0@.@1@.@2@'.format(lt_current - lt_age, lt_age, lt_revision)

darwin_current = lt_current - lt_age + 1
darwin_age = lt_revision
darwin_versions = [darwin_current, '@0@.@1@'.format(darwin_current, darwin_age)]

### DEFINE VARIABLES ###########################################################

datadir = get_option('datadir')
plugindir = get_option('plugindir')
if plugindir == 'default'
    plugindir = get_option('libdir') / libname
endif
pkglibexecdir = get_option('libexecdir') / libname
pkgdocsdir = datadir / 'doc' / libname

prefix = get_option('prefix')
abs_datadir = prefix / datadir
abs_localedir = abs_datadir / 'locale'
abs_plugindir = prefix / plugindir

add_project_arguments('-DLOCALEDIR="@0@"'.format(abs_localedir),
    '-DPACKAGE_LOCALE_DIR="@0@"'.format(abs_localedir),
    language: 'c')

top_include = include_directories('.')

### CHECK FOR DEPENDENCIES #####################################################

awk = find_program('awk', required: get_option('nitfol'))
perl = find_program('perl',
    required: get_option('gtk_doc') or get_option('nitfol'))
bison = find_program('bison', 'byacc', 'yacc', required: get_option('nitfol'))
texinfo = find_program('makeinfo', required: false)

libm = cc.find_library('m', required: false)

glib = dependency('glib-2.0', version: '>=2.44')
gthread = dependency('gthread-2.0')
gmodule = dependency('gmodule-2.0')
gobject = dependency('gobject-2.0')
gio = dependency('gio-2.0')
pango = dependency('pango')
gtk = dependency('gtk+-3.0', version: '>=3.14')
gstreamer = dependency('gstreamer-1.0', required: get_option('sound'))

gstinspect = find_program('gst-inspect-1.0', required: get_option('sound'))
if get_option('sound').enabled()
    foreach plugin : ['giostreamsrc', 'typefind', 'audioconvert', 'volume',
        'oggdemux', 'vorbisdec', 'autoaudiosink', 'aiffparse', 'modplug']
        message('Checking GStreamer element @0@'.format(plugin))
        result = run_command(gstinspect, '--exists', plugin, check: false)
        if result.returncode() != 0
            error('''One or more required GStreamer elements were not found.
    You will have to install your system's "base", "good", and "bad" plugins
    packages. Try looking for names such as: gstreamer1-plugins-base,
    gstreamer1-plugins-good, gstreamer1-plugins-bad-free,
    gstreamer1-plugins-bad-free-extras, gstreamer1.0-plugins-base,
    gstreamer1.0-plugins-good, gstreamer1.0-plugins-bad''')
        endif
    endforeach
endif

vapigen = find_program('vapigen', required: get_option('vapi'))

### CONFIGURE ##################################################################

config = configuration_data()

config.set_quoted('GETTEXT_PACKAGE', libname)
config.set_quoted('PACKAGE_VERSION', version)

config.set10('HAVE_SOUND', get_option('sound').enabled())

configure_file(output: 'config.h', configuration: config)

### BUILD AND INSTALL ##########################################################

symbol_map = join_paths(meson.current_source_dir(), 'plugin.map')
plugin_link_depends = []
plugin_link_args = []
version_script_arg = '-Wl,--version-script,@0@'.format(symbol_map)
if cc.has_link_argument(version_script_arg)
    plugin_link_depends += symbol_map
    plugin_link_args += [version_script_arg]
endif

subdir('libchimara')
if get_option('bocfel')
    subdir('interpreters/bocfel')
endif
if get_option('frotz')
    subdir('interpreters/frotz')
endif
if get_option('git')
    subdir('interpreters/git')
endif
if get_option('glulxe')
    subdir('interpreters/glulxe')
endif
if get_option('nitfol')
    subdir('interpreters/nitfol')
endif
if get_option('player')
    subdir('player')
endif
subdir('tests')
if get_option('gtk_doc')
    subdir('docs/reference')
endif
subdir('po')

pkgconfig.generate(libchimara, filebase: 'chimara', name: 'Chimara',
    description: 'Multi-session Glk library, with interactive fiction plugins',
    url: 'http://no.public.website.yet', requires: [glib, gtk],
    requires_private: [gthread, gmodule], subdirs: 'chimara')

install_data('AUTHORS', 'COPYING', 'NEWS', 'README',
    install_dir: datadir / 'doc' / libname)

# Script adapted from GNOME Notes's build system
meson.add_install_script('build-aux/meson_post_install.py', abs_datadir)
