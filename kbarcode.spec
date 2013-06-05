Name:          kbarcode
Version:       3.0.0b2
Summary:       A barcode and label printing application for KDE
Release:       1%{?dist}
# Only the barcode_ps file is MIT licensed
License:       GPLv2+ and MIT
URL:           http://www.kbarcode.net
Source0:       http://downloads.sourceforge.net/kbarcode/Development/%{name}_%{version}.tar.gz
Patch0:        %{name}.desktop.patch

BuildRequires: desktop-file-utils
BuildRequires: gettext
BuildRequires: kdepimlibs-devel
BuildRequires: pcre-devel

%description
KBarcode is a barcode and label printing application for Linux and KDE 4.
It can be used to print every thing from simple business cards up to complex
labels with several barcodes (e.g. article descriptions). KBarcode comes with
an easy to use WYSIWYG label designer, a setup wizard, batch import of labels
(directly from the delivery note), thousands of predefined labels, database
management tools and translations in many languages. Even printing more than
10.000 labels in one go is no problem for KBarcode. Additionally it is a simply
xbarcode replacement for the creation of barcodes. All major types of barcodes
like EAN, UPC, CODE39 and ISBN are supported.

%prep
%setup -q -n %{name}
%patch0 -p0


%build
export LDFLAGS=-lpcre
mkdir -p %{_target_platform}
pushd %{_target_platform}
%{cmake_kde4} ../
popd
make %{?_smp_mflags} -C %{_target_platform}

%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

%find_lang %{name}

%check
desktop-file-validate ${RPM_BUILD_ROOT}/%{_datadir}/applications/kde4/%{name}.desktop

%post
update-desktop-database &> /dev/null || :
update-mime-database %{_datadir}/mime &> /dev/null || :

%postun
update-desktop-database &> /dev/null || :
update-mime-database %{_datadir}/mime &> /dev/null || :

%files -f %{name}.lang
%doc COPYING COPYING.barcode_ps README TODO
%{_bindir}/%{name}
%{_datadir}/applications/kde4/%{name}.desktop
%{_datadir}/icons/hicolor/*/actions/*.png
%{_datadir}/icons/hicolor/*/apps/*.png
%{_kde4_appsdir}/%{name}/

%changelog
* Sun Apr 28 2013 Mario Blättermann <mariobl@fedoraproject.org> 3.0.0b2-1
- Initial package


