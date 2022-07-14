# Spec file for chimara, chimara-devel, and chimara-player
#

Name:           chimara
Version:        0.9.3
Release:        1%{?dist}
Summary:        A GTK+ widget implementation of the Glk library
URL:            http://chimara.github.io/Chimara/
License:        BSD

Group:          Development/Libraries
Source:         %{name}-%{version}.tar.xz

Requires(post): info
Requires(preun): info

BuildRequires:  meson >= 0.50
BuildRequires:  bison
# byacc is allowed instead of bison, but stoopid RPM doesn't let you specify alternative pkgs
BuildRequires:  gettext perl-interpreter pkgconfig texinfo
BuildRequires:  gtk-doc >= 1.20
BuildRequires:  glib2-devel >= 2.44
BuildRequires:  gtk3-devel >= 3.14
BuildRequires:  pango-devel
BuildRequires:  gstreamer1-devel
BuildRequires:  gstreamer1-plugins-base
BuildRequires:  gstreamer1-plugins-good
BuildRequires:  gstreamer1-plugins-bad-free
BuildRequires:  gstreamer1-plugins-bad-free-extras

%description
A GTK widget that loads and runs Glk programs as plugins. Glk is an
input/output specification specifically designed for interactive fiction.

%package        devel
Summary:        Development files for %{name}
Group:          Development/Libraries
Requires:       %{name} = %{version}-%{release}
Requires:       python2

%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.

%package        player
Summary:        The default IF player using %{name}
Group:          Amusements/Games
Requires:       %{name} = %{version}-%{release}

%description    player
The %{name}-player package contains the default interactive fiction player 
using %{name}.

# wat, definition of meson macro has builddir and srcdir swapped?!
%global _vpath_srcdir %{name}-%{version}

%prep
%autosetup -c

%build
%set_build_flags
%{shrink:%{__meson} --buildtype=plain --prefix=%{_prefix} --libdir=%{_libdir}
   --libexecdir=%{_libexecdir} --bindir=%{_bindir} --sbindir=%{_sbindir}
   --includedir=%{_includedir} --datadir=%{_datadir} --mandir=%{_mandir}
   --infodir=%{_infodir} --localedir=%{_datadir}/locale
   --sysconfdir=%{_sysconfdir} --localstatedir=%{_localstatedir}
   --sharedstatedir=%{_sharedstatedir} --wrap-mode=%{__meson_wrap_mode}
   --auto-features=%{__meson_auto_features} -Dgtk_doc=true -Dvapi=enabled
   %{_vpath_builddir} %{_vpath_srcdir}}
%meson_build

%install
%meson_install

%check
%meson_test

%post
/sbin/ldconfig
/sbin/install-info %{_infodir}/nitfol.info || :

%postun player
if [ $1 -eq 0 ] ; then
  glib-compile-schemas %{_datadir}/glib-2.0/schemas &> /dev/null || :
fi

%preun
if [ $1 = 0 ] ; then
  /sbin/install-info --delete %{_infodir}/nitfol.info || :
fi

%postun -p /sbin/ldconfig

%posttrans player
glib-compile-schemas %{_datadir}/glib-2.0/schemas &> /dev/null || :

%files
%defattr(-,root,root,-)
%doc %{_datadir}/doc/chimara/bocfel/*
%doc %{_datadir}/doc/chimara/frotz/*
%doc %{_datadir}/doc/chimara/git/*
%doc %{_datadir}/doc/chimara/glulxe/*
%doc %{_datadir}/doc/chimara/nitfol/*
%doc %{_infodir}/*.info*
%{_libdir}/libchimara.so.*
%{_libdir}/chimara/*.so
%{_libdir}/girepository-1.0/Chimara-1.0.typelib

%files devel
%defattr(-,root,root,-)
%doc %{_datadir}/gtk-doc/html/*
%{_includedir}/chimara/libchimara/*.h
%{_libdir}/libchimara.so
%{_libdir}/pkgconfig/*.pc
%{_datadir}/gir-1.0/Chimara-1.0.gir
%{_datadir}/vala/vapi/chimara.*
%{_libexecdir}/chimara/profile-analyze.py

%files player
%defattr(-,root,root,-)
%doc %{_datadir}/doc/chimara/README 
%doc %{_datadir}/doc/chimara/COPYING 
%doc %{_datadir}/doc/chimara/AUTHORS 
%doc %{_datadir}/doc/chimara/NEWS
%{_bindir}/chimara
%{_datadir}/glib-2.0/schemas/org.chimara-if.gschema.xml

%changelog
* Wed Jul 13 2022 Philip Chimento <philip.chimento@gmail.com> - 0.9.3-1
- Update release to 0.9.3.
* Sat Apr 2 2022 Philip Chimento <philip.chimento@gmail.com> - 0.9.2-1
- Update release to 0.9.2.
* Mon Feb 1 2021 Philip Chimento <philip.chimento@gmail.com> - 0.9.1-1
- Changed build system to Meson.
* Mon Mar 7 2011 P. F. Chimento <philip.chimento@gmail.com>
- Added glib-compile-schemas invocations.
* Fri Dec 4 2009 P. F. Chimento <philip.chimento@gmail.com>
- 0.1-1
- Mended rpmlint warnings and errors.
- Moved player data files to player package.
* Mon Nov 30 2009 W. M. van Vliet <w.m.vanvliet@student.utwente.nl>
- Added Chimara player to the package.
* Wed Nov 25 2009 P. F. Chimento <philip.chimento@gmail.com>
- Created specfile.
