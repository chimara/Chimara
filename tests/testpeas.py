from gi.repository import Gtk, Peas, PeasGtk, Chimara
c = Chimara.Glk()

e = Peas.Engine.get_default()
e.add_search_path('/Users/fliep/gtk/inst/lib/chimara', None)

for i in e.get_plugin_list():
	print i.get_name()

w = Gtk.Window()
w.add(PeasGtk.PluginManager(e))
w.connect('destroy', Gtk.main_quit)
w.show_all()

Gtk.main()
