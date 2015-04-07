
%define          coriolisVersion    2.1
%define          coriolisTop        %{_prefix}
%define          githash            71e1c18
%define          revdate            20150407
	     
%define          python_sitedir     %{_lib}/%(python -c '
import os.path
import distutils.sysconfig

pathes = distutils.sysconfig.get_python_lib().split("/")
print os.path.join ( pathes[-2], pathes[-1] )
')

%if 0%{?rhel} > 5 || 0%{?fedora} > 10
%define          qt4                qt
%else
%define          qt4                qt4
%endif
	     
	     
Name:            coriolis2
Summary:         Coriolis 2 VLSI CAD System
Version:         %{coriolisVersion}
Release:         1.%{revdate}.git%{githash}%{dist}
License:         BSD/LGPL/GPL/FLUTE
Group:           Applications/Engineering
Source:          %{name}-%{version}.git%{githash}.tar.bz2
URL:             http://www-asim.lip6.fr/
Packager:        Jean-Paul Chaput <Jean-Paul.Chaput@lip6.fr>
Requires(post):  ldconfig
Requires:        boost >= 1.33.1
Requires:        %{qt4} >= 4.5.0
BuildRequires:   boost-devel >= 1.33.1
BuildRequires:   %{qt4}-devel >= 4.5.0
BuildRoot:       %{_tmppath}/root-%{name}


%description
Coriolis is the new CAD tool suite intended to replace the
physical backend flow of Alliance.

The Knik global router makes use of FLUTE, which is redistributed
under is own license. FLUTE is copyrighted by Dr. Chris C. N. Chu
from the Iowa State University <http://home.eng.iastate.edu/~cnchu>.


%package devel
Summary:         Coriolis 2 VLSI CAD Sytem - Development
Group:           Applications/Engineering
Requires:        %{name} = %{version}-%{release}
Requires:        %{qt4}-devel >= 4.5.0


%description devel
Development files for the Coriolis 2 package.


%prep
%setup -n %{name}-%{version}.git%{githash}


%build
 if [ -d %{buildroot} ]; then rm -r %{buildroot}; fi

  CORIOLIS_TOP=%{coriolisTop};  export CORIOLIS_TOP
  IMPORTEDS_TOP=%{coriolisTop}; export IMPORTEDS_TOP

# Do build & install in one step, except for documentation.
 cd coriolis
 make -f bootstrap/Makefile.package \
      DESTDIR=%{buildroot} \
      BUILD_DESTDIR=%{_builddir}/%{buildsubdir}/install.dir \
      %{_smp_mflags} build


%install
  IMPORTEDS_TOP=%{coriolisTop}; export IMPORTEDS_TOP
  CORIOLIS_TOP=%{coriolisTop};  export CORIOLIS_TOP

# Install & build documentation, in -j1.
 cd coriolis
 make -f bootstrap/Makefile.package \
      DESTDIR=%{buildroot} -f bootstrap/Makefile.package \
      BUILD_DESTDIR=%{_builddir}/%{buildsubdir}/install.dir \
      -j1 install

# Copy the license from the Debian package directory.
 cp bootstrap/debian/copyright %{buildroot}%{_datadir}/doc/coriolis2/COPYRIGHT

#%__rm -rf %{buildroot}%{coriolisTop}/share/doc/coriolis2

# Removing undistributed binaries.
#%__rm -f %{buildroot}%{coriolisTop}/bin/{cx2y,kite-text}


%clean
 if [ -d %{buildroot} ]; then rm -r %{buildroot}; fi


%post   -p /sbin/ldconfig
%postun -p /sbin/ldconfig


%files
%defattr(-,root,root,-)
%doc %{_datadir}/doc/coriolis2 
%dir %{_sysconfdir}/coriolis2
%dir %{coriolisTop}/share/coriolis2/flute-3.1
%dir %{coriolisTop}/bin
%dir %{coriolisTop}/%{_lib}
%dir %{coriolisTop}/%{python_sitedir}
%dir %{coriolisTop}/%{python_sitedir}/crlcore
%dir %{coriolisTop}/%{python_sitedir}/crlcore/helpers
%dir %{coriolisTop}/%{python_sitedir}/cumulus
%dir %{coriolisTop}/%{python_sitedir}/stratus
%{coriolisTop}/bin/*
%{coriolisTop}/%{_lib}/*.so.*
%{coriolisTop}/%{python_sitedir}/*.so
%{coriolisTop}/%{python_sitedir}/cumulus/*.py*
%{coriolisTop}/%{python_sitedir}/stratus/*.py*
%{coriolisTop}/%{python_sitedir}/crlcore/*.py*
%{coriolisTop}/%{python_sitedir}/crlcore/helpers/*.py*
#%config(noreplace) %{_sysconfdir}/ld.so.conf.d/*
%config(noreplace) %{_sysconfdir}/coriolis2/*.conf
%config(noreplace) %{_sysconfdir}/coriolis2/*.xml
%config(noreplace) %{_sysconfdir}/coriolis2/*.sh
%config(noreplace) %{_sysconfdir}/coriolis2/*.py*
%config(noreplace) %{_sysconfdir}/coriolis2/stratus.vim
%config(noreplace) %{coriolisTop}/share/coriolis2/flute-3.1/*.dat


%files devel
%defattr(-,root,root,-)
%dir %{coriolisTop}/%{_lib}
%dir %{coriolisTop}/share/cmake/Modules
%dir %{coriolisTop}/include
%dir %{coriolisTop}/include/vlsisapd
%dir %{coriolisTop}/include/vlsisapd/agds
%dir %{coriolisTop}/include/vlsisapd/cif
%dir %{coriolisTop}/include/vlsisapd/spice
%dir %{coriolisTop}/include/vlsisapd/liberty
%dir %{coriolisTop}/include/vlsisapd/bookshelf
%dir %{coriolisTop}/include/vlsisapd/configuration
#%dir %{coriolisTop}/include/vlsisapd/dtr
#%dir %{coriolisTop}/include/vlsisapd/openChams
%dir %{coriolisTop}/include/coriolis2/hurricane
%dir %{coriolisTop}/include/coriolis2/hurricane/viewer
%dir %{coriolisTop}/include/coriolis2/hurricane/isobar
%dir %{coriolisTop}/include/coriolis2
%dir %{coriolisTop}/include/coriolis2/crlcore
%dir %{coriolisTop}/include/coriolis2/nimbus
%dir %{coriolisTop}/include/coriolis2/etesian
%dir %{coriolisTop}/include/coriolis2/metis
%dir %{coriolisTop}/include/coriolis2/mauka
%dir %{coriolisTop}/include/coriolis2/knik
%dir %{coriolisTop}/include/coriolis2/katabatic
%dir %{coriolisTop}/include/coriolis2/kite
%dir %{coriolisTop}/include/coriolis2/equinox
%dir %{coriolisTop}/include/coriolis2/solstice
%dir %{coriolisTop}/include/coriolis2/unicorn
%{coriolisTop}/%{_lib}/*.so
%{coriolisTop}/share/cmake/Modules/*.cmake
%{coriolisTop}/include/vlsisapd/agds/*.h
%{coriolisTop}/include/vlsisapd/cif/*.h
%{coriolisTop}/include/vlsisapd/spice/*.h
%{coriolisTop}/include/vlsisapd/liberty/*.h
%{coriolisTop}/include/vlsisapd/bookshelf/*.h
%{coriolisTop}/include/vlsisapd/configuration/*.h
#%{coriolisTop}/include/vlsisapd/dtr/*.h
#%{coriolisTop}/include/vlsisapd/openChams/*.h
%{coriolisTop}/include/coriolis2/hurricane/*.h
%{coriolisTop}/include/coriolis2/hurricane/viewer/*.h
%{coriolisTop}/include/coriolis2/hurricane/isobar/*.h
%{coriolisTop}/include/coriolis2/crlcore/*.h
%{coriolisTop}/include/coriolis2/nimbus/*.h
%{coriolisTop}/include/coriolis2/metis/*.h
%{coriolisTop}/include/coriolis2/mauka/*.h
%{coriolisTop}/include/coriolis2/knik/*.h
%{coriolisTop}/include/coriolis2/katabatic/*.h
%{coriolisTop}/include/coriolis2/kite/*.h
%{coriolisTop}/include/coriolis2/equinox/*.h
%{coriolisTop}/include/coriolis2/solstice/*.h
%{coriolisTop}/include/coriolis2/unicorn/*.h


%changelog
* Wed Feb  2 2011  Jean-Paul.Chaput <Jean-Paul.Chaput@lip6.fr>
- Second release, all tools from Coriolis 1 have been ported.
  (nimbus, mauka, metis, cumulus, stratus1)

* Sun May 16 2010  Jean-Paul.Chaput <Jean-Paul.Chaput@lip6.fr>
- Initial packaging for svn release 1322 (alpha stage).
