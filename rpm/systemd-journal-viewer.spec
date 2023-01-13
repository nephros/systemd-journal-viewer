Name:       systemd-journal-viewer

%{!?qtc_qmake:%define qtc_qmake %qmake}
%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}
%{?qtc_builddir:%define _builddir %qtc_builddir}
Summary:    GUI for viewing systemd logs
Version:    0.5.6
Release:    1
Group:      Qt/Qt
# The license is unspecified in the upstream code, repo, and .spec file.
# However, the author (coderus) states:
#
# > hi, so, about license stuff. unfortunately I'm not a licence expert,
# > i write my code under WTFPL,
# Source: https://github.com/sailfishos-patches/patchmanager/discussions/57#discussioncomment-1419413
#
# and
#
# > my stuff with no license choice is WTFPL,
#
# Source: https://forum.sailfishos.org/t/journal-viewer-update-for-aarch-64-possible/9924/19
# so lets use that here:
License:    WTFPL

URL:        https://openrepos.net/content/coderus/journal-viewer
Source0:    %{name}-%{version}.tar.bz2
Requires:   sailfishsilica-qt5 >= 0.10.9
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  desktop-file-utils
BuildRequires:  pkgconfig(libsystemd)
BuildRequires:  pkgconfig(systemd)
%systemd_requires

%description
%{summary}.

%if "%{?vendor}" == "chum"
PackageName: Journal Viewer
Type: desktop-application
DeveloperName: CODeRUS
PackagerName: nephros
Categories:
 - System
 - Utility
Custom:
  Repo: https://github.com/CODeRUS/systemd-journal-viewer
  PackagingRepo: https://github.com/nephros/systemd-journal-viewer
Screenshots:
 - https://openrepos.net/sites/default/files/packages/13234/screenshot-snimokekrana20170111001.png
 - https://openrepos.net/sites/default/files/packages/13234/screenshot-snimokekrana20170116001.png
Url:
  Homepage: https://openrepos.net/content/coderus/journal-viewer
  Bugtracker: https://github.com/piggz/harbour-advanced-camera/issues
  Donations:
    - https://noyb.eu/en/donations-other-support-options
    - https://my.fsfe.org/donate
    - https://supporters.eff.org/donate/join-4
    - https://openrepos.net/donate
%endif

%prep
%setup -q -n %{name}-%{version}

%build

%qtc_qmake5

%qtc_make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%qmake5_install

desktop-file-install --delete-original       \
  --dir %{buildroot}%{_datadir}/applications             \
   %{buildroot}%{_datadir}/applications/*.desktop

%post
systemctl try-restart systemd-journald.service >/dev/null 2>&1 || :
%systemd_user_post systemd-journal-daemon.service

%preun
%systemd_user_preun systemd-journal-daemon.service

%postun
%systemd_user_postun systemd-journal-daemon.service

%files
%attr(4755, root, root) %{_bindir}/systemd-journal-daemon
%attr(0755, root, root) %{_bindir}/systemd-journal-viewer
%defattr(-,root,root,-)
%{_sysconfdir}/systemd/journald.conf.d/*.conf
%{_datadir}/dbus-1/services/org.coderus.systemd.journal.service
%{_userunitdir}/systemd-journal-daemon.service
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
