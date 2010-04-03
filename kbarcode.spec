%define name kbarcode
%define version 2.0.6
Summary: A barcode and label printing application for KDE
Name: %{name}
Version: %{version}
Release: 1
License: GPL
Vendor: Dominik Seichter <domseichter@web.de>
Url: http://www.kbarcode.net
Packager: Dominik Seichter <domseichter@web.de>
Group: kde/utilities
Source: %{name}-%{version}.tar.gz
BuildRoot: /var/tmp/%{name}-%{version}

%description
KBarcode is a barcode and label printing application for Linux and KDE 3. It can be used to print every thing from simple business cards up to complex labels with several barcodes (e.g. article descriptions). KBarcode comes with an easy to use WYSIWYG label designer, a setup wizard, batch import of labels (directly from the delivery note), thousands of predefined labels, database managment tools and translations in many languages. Even printing more than 10.000 labels in one go is no problem for KBarcode. Additionally it is a simply xbarcode replacement for the creation of barcodes. All major types of barcodes like EAN, UPC, CODE39 and ISBN are supported.

%prep
%setup
./configure

%build

# Setup for parallel builds
numprocs=`egrep -c ^cpu[0-9]+ /proc/stat || :`
if [ "$numprocs" = "0" ]; then
  numprocs=1
fi

make -j$numprocs

%install
make install-strip DESTDIR=$RPM_BUILD_ROOT

cd $RPM_BUILD_ROOT
find . -type d | sed '1,2d;s,^\.,\%attr(-\,root\,root) \%dir ,' > $RPM_BUILD_DIR/file.list.kbarcode
find . -type f | sed 's,^\.,\%attr(-\,root\,root) ,' >>  $RPM_BUILD_DIR/file.list.kbarcode
find . -type l | sed 's,^\.,\%attr(-\,root\,root) ,' >>  $RPM_BUILD_DIR/file.list.kbarcode

%clean
rm -rf $RPM_BUILD_ROOT/*
rm -rf $RPM_BUILD_DIR/kbarcode-%{version}
rm -rf ../file.list.kbarcode


%files -f ../file.list.kbarcode




