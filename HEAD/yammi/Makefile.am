####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = yammi

## INCLUDES were found outside kdevelop specific part

yammi_SOURCES = artsplayer.cpp CVBitRate.cpp CMP3Info.cpp CFrameHeader.cpp ApplyToAllBase.ui noatunplayer.cpp UpdateDatabaseMediaDialogBase.ui UpdateDatabaseDialogBase.ui SongInfoDialog.ui PreferencesDialogBase.ui DeleteDialog.ui ConsistencyCheckDialogBase.ui ConsistencyCheckDialog.cpp vcedit.c yammiapplication.cpp xmmsplayer.cpp mediaplayer.cpp lineeditshift.cpp updatedatabasemediadialog.cpp updatedatabasedialog.cpp yammimodel.cpp yammigui.cpp songlistitem.cpp songinfo.cpp songentrytimestamp.cpp songentrystring.cpp songentryint2.cpp songentryint.cpp songentry.cpp song.cpp prefs.cpp preferencesdialog.cpp mylistview.cpp mylist.cpp mydatetime.cpp main.cpp fuzzsrch.cpp foldersorted.cpp foldermedia.cpp foldergroups.cpp foldercategories.cpp folder.cpp 
yammi_LDADD   = -lartskde -lkparts -lkdeui -lkdecore $(LIB_QT) $(LIBSOCKET)

SUBDIRS = docs icons 

EXTRA_DIST = folder.cpp folder.h foldercategories.cpp foldercategories.h foldergroups.cpp foldergroups.h foldermedia.cpp foldermedia.h foldersorted.cpp foldersorted.h fuzzsrch.cpp fuzzsrch.h main.cpp mydatetime.cpp mydatetime.h mylist.cpp mylist.h mylistview.cpp mylistview.h preferencesdialog.cpp preferencesdialog.h prefs.cpp prefs.h song.cpp song.h songentry.cpp songentry.h songentryint.cpp songentryint.h songentryint2.cpp songentryint2.h songentrystring.cpp songentrystring.h songentrytimestamp.cpp songentrytimestamp.h songinfo.cpp songinfo.h songlistitem.cpp songlistitem.h yammigui.cpp yammigui.h yammimodel.cpp yammimodel.h glib.h glibconfig.h xmmsctrl.h updatedatabasedialog.cpp updatedatabasedialog.h options.h updatedatabasemediadialog.cpp updatedatabasemediadialog.h lineeditshift.cpp lineeditshift.h mediaplayer.cpp mediaplayer.h xmmsplayer.cpp xmmsplayer.h yammiapplication.cpp yammiapplication.h vcedit.c vcedit.h acconfig.h ConsistencyCheckDialog.cpp ConsistencyCheckDialog.h ConsistencyCheckParameter.h ConsistencyCheckDialogBase.ui DeleteDialog.ui PreferencesDialogBase.ui SongInfoDialog.ui UpdateDatabaseDialogBase.ui UpdateDatabaseMediaDialogBase.ui i18n.h noatunplayer.cpp noatunplayer.h ApplyToAllBase.ui dummyplayer.h CFrameHeader.cpp CFrameHeader.h CMP3Info.cpp CMP3Info.h CVBitRate.cpp CVBitRate.h artsplayer.cpp artsplayer.h yammiicons.h yammi.desktop yammiui.rc yammi.de.ts yammi.de.qm 

install-data-local:
	$(mkinstalldirs) $(kde_appsdir)/Applications/
	$(INSTALL_DATA) $(srcdir)/yammi.desktop $(kde_appsdir)/Applications/yammi.desktop
	$(mkinstalldirs) $(kde_datadir)/yammi/
	$(INSTALL_DATA) $(srcdir)/yammiui.rc $(kde_datadir)/yammi/yammiui.rc

uninstall-local:
	-rm -f $(kde_appsdir)/Applications/yammi.desktop
	-rm -f $(kde_datadir)/yammi/yammiui.rc

####### kdevelop will overwrite this part!!! (end)############


# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

yammi_METASOURCES = AUTO


# the library search path. 
yammi_LDFLAGS = $(all_libraries) 

# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(yammi_METASOURCES)

rcdir = $(kde_datadir)/yammi
rc_DATA = yammiui.rc

messages: rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/yammi.pot; \
	fi


