import os.path
from gi.repository import Gtk, Peas, PeasGtk, Chimara
c = Chimara.Glk()

e = Peas.Engine.get_default()
e.add_search_path(os.path.expanduser('~/gtk/inst/lib/chimara'),
	os.path.expanduser('~/gtk/inst/lib/chimara/plugins'))

frotz_info = e.get_plugin_info('frotz')
e.load_plugin(frotz_info)

w = Gtk.Window()
m = PeasGtk.PluginManager(e)
m.get_view().set_selected_plugin(frotz_info)
w.add(m)
w.connect('destroy', Gtk.main_quit)
w.show_all()

Gtk.main()
