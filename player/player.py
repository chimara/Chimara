#!/usr/bin/env python

import sys
import os.path
import argparse
from gi.repository import Gdk
Gdk.threads_init()
from gi.repository import GObject, GLib, Gdk, Gio, Gtk, Chimara
import config

if config.ENABLE_NLS:
    import gettext
    gettext.install(config.GETTEXT_PACKAGE, config.PACKAGE_LOCALE_DIR,
        unicode=True, codeset='UTF-8')
else:
    _ = lambda x: x


class Player(GObject.GObject):
    __gtype_name__ = 'ChimaraPlayer'

    def __init__(self, graphics_file=None):
        super(Player, self).__init__()

        # Initialize settings file; it can be overridden by a "chimara-config"
        # file in the current directory
        if os.path.exists('chimara-config'):
            keyfile = 'chimara-config'
        else:
            keyfile = os.path.expanduser('~/.chimara/config')
        try:
            # This only works on my custom-built gobject-introspection; opened
            # bug #682702
            backend = Gio.keyfile_settings_backend_new(keyfile,
                "/org/chimara-if/player/", None)
        except AttributeError:
            backend = None
        self.prefs_settings = Gio.Settings('org.chimara-if.player.preferences',
            backend=backend)
        self.state_settings = Gio.Settings('org.chimara-if.player.state',
            backend=backend)

        builder = Gtk.Builder()
        try:
            builder.add_from_file(os.path.join(config.PACKAGE_DATA_DIR,
                'chimara.ui'))
        except GLib.GError:
            if config.DEBUG:
                builder.add_from_file(os.path.join(config.PACKAGE_SRC_DIR,
                    'chimara.ui'))
            else:
                raise
        self.window = builder.get_object('chimara')
        self.aboutwindow = builder.get_object('aboutwindow')
        self.prefswindow = builder.get_object('prefswindow')
        actiongroup = builder.get_object('actiongroup')

        # Set the default value of the "View/Toolbar" menu item upon creation
        # of a new window to the "show-toolbar-default" setting, but bind the
        # setting one-way only - we don't want toolbars to disappear suddenly
        toolbar_action = builder.get_object('toolbar')
        toolbar_action.active = \
            self.state_settings.get_boolean('show-toolbar-default')
        self.state_settings.bind('show-toolbar-default', toolbar_action,
            'active', Gio.SettingsBindFlags.SET)

        filt = Gtk.RecentFilter()
        # TODO: Use mimetypes and construct the filter dynamically depending on
        # what plugins are installed
        for pattern in ['*.z[1-8]', '*.[zg]lb', '*.[zg]blorb', '*.ulx', '*.blb',
            '*.blorb']:
            filt.add_pattern(pattern)
        recent = builder.get_object('recent')
        recent.add_filter(filt)

        uimanager = Gtk.UIManager()
        try:
            uimanager.add_ui_from_file(os.path.join(config.PACKAGE_DATA_DIR,
                'chimara.menus'))
        except GLib.GError:
            if config.DEBUG:
                uimanager.add_ui_from_file(os.path.join(config.PACKAGE_SRC_DIR,
                    'chimara.menus'))
            else:
                raise
        uimanager.insert_action_group(actiongroup)
        menubar = uimanager.get_widget('/menubar')
        toolbar = uimanager.get_widget('/toolbar')
        toolbar.no_show_all = True
        if toolbar_action.active:
            toolbar.show()
        else:
            toolbar.hide()

        # Connect the accelerators
        accels = uimanager.get_accel_group()
        self.window.add_accel_group(accels)

        self.glk = Chimara.IF(ignore_errors=True,
            # interpreter_number=Chimara.IFZmachineVersion.TANDY_COLOR,
            graphics_file=graphics_file)
        css_file = _maybe(self.prefs_settings.get_value('css-file'))
        if css_file is None:
            css_file = 'style.css'
        self.glk.set_css_from_file(css_file)

        # DON'T UNCOMMENT THIS your eyes will burn
        # but it is a good test of programmatically altering just one style
        # self.glk.set_css_from_string("buffer{font-family: 'Comic Sans MS';}")

        vbox = builder.get_object('vbox')
        vbox.pack_end(self.glk, True, True, 0)
        vbox.pack_start(menubar, False, False, 0)
        vbox.pack_start(toolbar, False, False, 0)

        builder.connect_signals(self)
        self.glk.connect('notify::program-name', self.change_window_title)
        self.glk.connect('notify::story-name', self.change_window_title)

        # Create preferences window
        # TODO

    def change_window_title(self, glk, pspec, data=None):
        if glk.props.program_name is None:
            title = "Chimara"
        elif glk.props.story_name is None:
            title = "{interp} - Chimara".format(interp=glk.props.program_name)
        else:
            title = "{interp} - {story} - Chimara".format(
                interp=glk.props.program_name,
                story=glk.props.story_name)
        self.window.props.title = title

    def on_open_activate(self, action, data=None):
        if not self.confirm_open_new_game():
            return

        dialog = Gtk.FileChooserDialog(_('Open Game'), self.window,
            Gtk.FileChooserAction.OPEN,
            (Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL,
            Gtk.STOCK_OPEN, Gtk.ResponseType.ACCEPT))

        # Get last opened path
        path = _maybe(self.state_settings.get_value('last-open-path'))
        if path is not None:
            dialog.set_current_folder(path)

        response = dialog.run()
        dialog.hide()
        if response != Gtk.ResponseType.ACCEPT:
            return

        gamefile = dialog.get_file()
        self.search_for_graphics_file(gamefile.get_path())
        try:
            self.glk.run_game_file(gamefile)
        except GLib.Error as e:
            error_dialog(self.window, _('Could not open game file {filename}: {errmsg}').format(filename=gamefile.get_path(), errmsg=e.message))
            return

        path = dialog.get_current_folder()
        if path is not None:
            self.state_settings.last_open_path = path

        # Add file to recent files list
        manager = Gtk.RecentManager.get_default()
        uri = gamefile.get_uri()
        manager.add_item(uri)

        dialog.destroy()

    def on_recent_item_activated(self, chooser, data=None):
        if not self.confirm_open_new_game():
            return

        uri = chooser.get_current_uri()
        gamefile = Gio.file_new_for_uri(uri)

        self.search_for_graphics_file(gamefile.get_path())
        try:
            self.glk.run_game_file(gamefile)
        except GLib.Error as e:
            error_dialog(self.window,
                _('Could not open game file {filename}: {errmsg}').format(
                    filename=gamefile.get_basename(),
                    errmsg=e.message))
            return

        # Add file to recent files list again, this updates it to most recently
        # used
        manager = Gtk.RecentManager.get_default()
        manager.add_item(uri)

    def on_stop_activate(self, *args):
        self.glk.stop()

    def on_quit_chimara_activate(self, *args):
        Gtk.main_quit()

    def on_copy_activate(self, *args):
        focus = self.window.get_focus()
        # Call "copy clipboard" on any widget that defines it
        if (isinstance(focus, Gtk.Label)
            or isinstance(focus, Gtk.Entry)
            or isinstance(focus, Gtk.TextView)):
            focus.emit('copy-clipboard')

    def on_paste_activate(self, *args):
        focus = self.window.get_focus()
        # Call "paste clipboard" on any widget that defines it
        if isinstance(focus, Gtk.Entry) or isinstance(focus, Gtk.TextView):
            focus.emit('paste-clipboard')

    def on_preferences_activate(self, *args):
        self.prefswindow.present()

    def on_toolbar_toggled(self, action, *args):
        if action.get_active():
            self.toolbar.show()
        else:
            self.toolbar.hide()

    def on_undo_activate(self, *args):
        self.glk.feed_line_input('undo')

    def on_save_activate(self, *args):
        self.glk.feed_line_input('save')

    def on_restore_activate(self, *args):
        self.glk.feed_line_input('restore')

    def on_restart_activate(self, *args):
        self.glk.feed_line_input('restart')

    def on_quit_activate(self, *args):
        self.glk.feed_line_input('quit')

    def on_about_activate(self, *args):
        self.aboutwindow.set_version(config.PACKAGE_VERSION)
        self.aboutwindow.present()

    def on_window_delete_event(self, *args):
        Gtk.main_quit()
        return True

    def confirm_open_new_game(self):
        """
        If a game is running in the Glk widget, warn the user that they will
        quit the currently running game if they open a new one. Returns True if
        no game was running. Returns False if the user cancelled. Returns True
        and shuts down the running game if the user wishes to continue.
        """
        if not self.glk.props.running:
            return True

        dialog = Gtk.MessageDialog(self.window,
            Gtk.DialogFlags.MODAL | Gtk.DialogFlags.DESTROY_WITH_PARENT,
            Gtk.MessageType.WARNING, Gtk.ButtonsType.CANCEL,
            _("Are you sure you want to open a new game?"))
        dialog.format_secondary_text(
            _("If you open a new game, you will quit the one you are "
            "currently playing."))
        dialog.add_button(Gtk.STOCK_OPEN, Gtk.ResponseType.OK)
        response = dialog.run()
        dialog.hide()

        if response != Gtk.ResponseType.OK:
            return False

        self.glk.stop()
        self.glk.wait()
        return True

    def search_for_graphics_file(self, filename):
        """Internal function: See if there is a corresponding graphics file"""

        # First get the name of the story file
        base = os.path.basename(filename)
        base_noext = os.path.splitext(base)[0]

        # Check in the stored resource path, if set
        resource_path = _maybe(self.prefs_settings.get_value('resource-path'))

        # Otherwise check in the current directory
        if resource_path is None:
            resource_path = os.path.dirname(filename)

        blorbfile = os.path.join(resource_path, base_noext + '.blb')
        if os.path.exists(blorbfile):
            self.glk.graphics_file = blorbfile

    # Various signal handlers for GtkBuilder file
    def gtk_widget_hide(self, widget, *args):
        return Gtk.Widget.hide(widget)

    def gtk_widget_hide_on_delete(self, widget, *args):
        return Gtk.Widget.hide_on_delete(widget)

    def dummy_handler(self, *args):
        pass

    on_resource_file_set = dummy_handler
    on_interpreter_cell_changed = dummy_handler
    on_toggle_underline = dummy_handler
    on_toggle_italic = dummy_handler
    on_toggle_bold = dummy_handler
    on_toggle_justify = dummy_handler
    on_toggle_right = dummy_handler
    on_toggle_center = dummy_handler
    on_toggle_left = dummy_handler
    on_background_color_set = dummy_handler
    on_foreground_color_set = dummy_handler
    on_font_set = dummy_handler
    on_css_filechooser_file_set = dummy_handler


def _maybe(variant):
    """Gets a maybe value from a GVariant - not handled in PyGI"""
    v = variant.get_maybe()
    if v is None:
        return None
    return v.unpack()


def error_dialog(parent, message):
    dialog = Gtk.MessageDialog(parent, Gtk.DialogFlags.DESTROY_WITH_PARENT,
        Gtk.MessageType.ERROR, Gtk.ButtonsType.OK, message)
    dialog.run()
    dialog.destroy()

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('game_file', nargs='?', default=None,
        metavar='GAME FILE', help='the game file to load and start')
    parser.add_argument('graphics_file', nargs='?', default=None,
        metavar='GRAPHICS FILE', help='a Blorb resource file to include')
    args = parser.parse_args()

    # Create configuration dir ~/.chimara
    try:
        os.mkdir(os.path.expanduser('~/.chimara'))
    except OSError:
        # already exists
        assert os.path.isdir(os.path.expanduser('~/.chimara'))

    player = Player(graphics_file=args.graphics_file)
    player.window.show_all()

    if args.game_file is not None:
        try:
            player.glk.run_game(args.game_file)
        except GLib.Error as e:
            error_dialog(player.window,
                _("Error starting Glk library: {errmsg}").format(
                    errmsg=e.message))
            sys.exit(1)

    Gdk.threads_enter()
    Gtk.main()
    Gdk.threads_leave()

    player.glk.stop()
    player.glk.wait()

    sys.exit(0)
