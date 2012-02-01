#!/usr/bin/env python

import sys
import os.path
from gi.repository import GObject, Gdk, Gio, Gtk, Chimara
import config

# FIXME: Dummy translation function, for now
_ = lambda x: x

class Player(GObject.GObject):
	__gtype_name__ = 'ChimaraPlayer'
	
	def __init__(self):
		super(Player, self).__init__()
		
		# FIXME: should use the Keyfile backend, but that's not available from
		# Python
		self.prefs_settings = Gio.Settings('org.chimara-if.player.preferences')
		self.state_settings = Gio.Settings('org.chimara-if.player.state')
		
		builder = Gtk.Builder()
		builder.add_from_file('chimara.ui')
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
		for pattern in ['*.z[1-8]', '*.[zg]lb', '*.[zg]blorb', '*.ulx', '*.blb',
			'*.blorb']:
			filt.add_pattern(pattern)
		recent = builder.get_object('recent')
		recent.add_filter(filt)
		
		uimanager = Gtk.UIManager()
		uimanager.add_ui_from_file('chimara.menus')
		uimanager.insert_action_group(actiongroup, 0)
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
		
		self.glk = Chimara.IF()
		self.glk.props.ignore_errors = True
		self.glk.set_css_from_file('style.css')
		
		vbox = builder.get_object('vbox')
		vbox.pack_end(self.glk, True, True, 0)
		vbox.pack_start(menubar, False, False, 0)
		vbox.pack_start(toolbar, False, False, 0)
		
		#builder.connect_signals(self)  # FIXME Segfaults?!
		builder.get_object('open').connect('activate', self.on_open_activate)
		builder.get_object('restore').connect('activate',
			self.on_restore_activate)
		builder.get_object('save').connect('activate', self.on_save_activate)
		builder.get_object('stop').connect('activate', self.on_stop_activate)
		builder.get_object('recent').connect('item-activated',
			self.on_recent_item_activated)
		builder.get_object('undo').connect('activate', self.on_undo_activate)
		builder.get_object('quit').connect('activate', self.on_quit_activate)
		builder.get_object('copy').connect('activate', self.on_copy_activate)
		builder.get_object('paste').connect('activate', self.on_paste_activate)
		builder.get_object('preferences').connect('activate',
			self.on_preferences_activate)
		builder.get_object('about').connect('activate', self.on_about_activate)
		toolbar_action.connect('toggled', self.on_toolbar_toggled)
		self.aboutwindow.connect('response', lambda x, *args: x.hide())
		self.aboutwindow.connect('delete-event',
			lambda x, *args: x.hide_on_delete())
		self.window.connect('delete-event', self.on_window_delete_event)
		self.prefswindow.connect('response', lambda x, *args: x.hide())
		self.prefswindow.connect('delete-event',
			lambda x, *args: x.hide_on_delete())
		# FIXME Delete to here when above bug is fixed
		
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
		if not self.confirm_open_new_game(): return
		
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
		if not self.confirm_open_new_game(): return
		
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

	def on_stop_activate(self, action, data=None):
		self.glk.stop()

	def on_quit_chimara_activate(self, action, data=None):
		Gtk.main_quit()

	def on_copy_activate(self, action, data=None):
		focus = self.window.get_focus()
		# Call "copy clipboard" on any widget that defines it
		if (isinstance(focus, Gtk.Label) 
			or isinstance(focus, Gtk.Entry) 
			or isinstance(focus, Gtk.TextView)):
			focus.emit('copy-clipboard')
	
	def on_paste_activate(self, action, data=None):
		focus = self.window.get_focus()
		# Call "paste clipboard" on any widget that defines it
		if isinstance(focus, Gtk.Entry) or isinstance(focus, Gtk.TextView):
			focus.emit('paste-clipboard')
	
	def on_preferences_activate(self, action, data=None):
		self.prefswindow.present()
	
	def on_toolbar_toggled(self, action, data=None):
		if action.get_active():
			self.toolbar.show()
		else:
			self.toolbar.hide()

	def on_undo_activate(self, action, data=None):
		self.glk.feed_line_input('undo')

	def on_save_activate(self, action, data=None):
		self.glk.feed_line_input('save')

	def on_restore_activate(self, action, data=None):
		self.glk.feed_line_input('restore')

	def on_restart_activate(self, action, data=None):
		self.glk.feed_line_input('restart')

	def on_quit_activate(self, action, data=None):
		self.glk.feed_line_input('quit')

	def on_about_activate(self, action, data=None):
		self.aboutwindow.set_version(config.PACKAGE_VERSION)
		self.aboutwindow.present()

	def on_window_delete_event(self, widget, event, data=None):
		Gtk.main_quit()
		return True
		
	def confirm_open_new_game(self):
		"""
		If a game is running in the Glk widget, warn the user that they will
		quit the currently running game if they open a new one. Returns True if
		no game 	was running. Returns False if the user cancelled. Returns True
		and shuts down the running game if the user wishes to continue.
		"""
		if not self.glk.props.running: return True

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
			glk.graphics_file = blorbfile

def _maybe(variant):
	"""Gets a maybe value from a GVariant - not handled in PyGI"""
	v = variant.get_maybe()
	if v is None: return None
	return v.unpack()

def error_dialog(parent, message):
	dialog = Gtk.MessageDialog(parent, Gtk.DialogFlags.DESTROY_WITH_PARENT,
		Gtk.MessageType.ERROR, Gtk.ButtonsType.OK, message)
	dialog.run()
	dialog.destroy()

if __name__ == '__main__':
	Gdk.threads_init()

	player = Player()
	player.window.show_all()

	if len(sys.argv) == 3:
		player.glk.props.graphics_file = sys.argv[2]
	if len(sys.argv) >= 2:
		try:
			player.glk.run_game(sys.argv[1])
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

