# This spec file was generated using Kpp
# If you find any problems with this spec file please report
# the error to ian geiser <geiseri@msoe.edu>
Summary:   Manages your mp3-files
Name:      playlistmanager
Version:   control=CVS
Release:   0.2
Copyright: GPL
Vendor:    Brian O.Nlle <oli.noelle@web.de>
Url:       www.informatik.uni-freiburg.de/~noelle

Packager:  Brian O.Nlle <oli.noelle@web.de>
Group:     
Source:    playlistmanager-control=CVS.tar.gz
BuildRoot: 

%description


%prep
%setup
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" ./configure \
                --help \
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
find . -type d | sed '1,2d;s,^\.,\%attr(-\,root\,root) \%dir ,' > $RPM_BUILD_DIR/file.list.playlistmanager
find . -type f | sed 's,^\.,\%attr(-\,root\,root) ,' >> $RPM_BUILD_DIR/file.list.playlistmanager
find . -type l | sed 's,^\.,\%attr(-\,root\,root) ,' >> $RPM_BUILD_DIR/file.list.playlistmanager

%clean
rm -rf $RPM_BUILD_ROOT/*
rm -rf $RPM_BUILD_DIR/playlistmanager
rm -rf ../file.list.playlistmanager


%files -f ../file.list.playlistmanager
