# This spec file was generated using Kpp
# If you find any problems with this spec file please report
# the error to ian geiser <geiseri@msoe.edu>
Summary:   An mp3 collection manager and search engine.
Name:      yammi
Version:   0.8.2
Release:   1
Copyright: GPL
Vendor:    Oliver Nölle <oli.noelle@web.de>
Url:       http://yammi.sourceforge.net
Icon:      yammi.xpm
Packager:  Oliver Nölle <oli.noelle@web.de>
Group:     Applications/Multimedia
Source:    yammi-0.8.2.tar.gz
BuildRoot: %{_tmppath}/%{name}-buildroot


%description
Yet Another Music Manager I...Fastest and easiest access to your whole song collection!

%prep
%setup
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" ./configure \
                 --build=i386-linux --host=i386-linux --target=i386-linux \
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
