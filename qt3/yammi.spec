# This spec file was generated using Kpp
# If you find any problems with this spec file please report
# the error to ian geiser <geiseri@msoe.edu>
Summary:   Yammi - Yet Another Music Manager
Name:      yammi
Version:   0.4
Release:   1
Copyright: GPL
Vendor:    Oliver Nölle <oli.noelle@web.de>
Url:       yammi.sourceforge.net

Packager:  Oliver Nölle <oli.noelle@web.de>
Group:     sound
Source:    yammi-0.4.tar.gz
BuildRoot: 

%description
Yet Another Music Manager- serves as a front-end for xmms- manages your mp3 collection- convenient fuzzy search- categorization of songs- history of played songs- basic support for grabbin
 encoding and burning mp3s- playlist management

%prep
%setup
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" ./configure \
                --with-qt-dir=/usr/lib/qt2 \
                $LOCALFLAGS
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
find . -type d | sed '1,2d;s,^\.,\%attr(-\,root\,root) \%dir ,' > $RPM_BUILD_DIR/file.list.yammi
find . -type f | sed 's,^\.,\%attr(-\,root\,root) ,' >> $RPM_BUILD_DIR/file.list.yammi
find . -type l | sed 's,^\.,\%attr(-\,root\,root) ,' >> $RPM_BUILD_DIR/file.list.yammi

%clean
rm -rf $RPM_BUILD_ROOT/*
rm -rf $RPM_BUILD_DIR/yammi
rm -rf ../file.list.yammi


%files -f ../file.list.yammi
