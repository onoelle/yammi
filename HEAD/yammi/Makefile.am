####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = yammi

## INCLUDES were found outside kdevelop specific part

yammi_SOURCES = UpdateDatabaseDialogBase.ui updatedatabasedialog.cpp yammimodel.cpp YammiGuiBase.ui yammigui.cpp WorkDialogBase.ui songlistitem.cpp SongInfoDialog.ui songinfo.cpp songentrytimestamp.cpp songentrystring.cpp songentryint2.cpp songentryint.cpp songentry.cpp song.cpp prefs.cpp PreferencesDialogBase.ui preferencesdialog.cpp mylistview.cpp mylist.cpp mydatetime.cpp mp3tag.cpp mp3_layer.cpp main.cpp fuzzsrch.cpp foldersorted.cpp foldermedia.cpp foldergroups.cpp foldercategories.cpp folder.cpp DeleteDialog.ui 
SOURCES = updatedatabasedialog.cpp yammimodel.cpp yammigui.cpp songlistitem.cpp songinfo.cpp songentrytimestamp.cpp songentrystring.cpp songentryint2.cpp songentryint.cpp songentry.cpp song.cpp prefs.cpp preferencesdialog.cpp mylistview.cpp mylist.cpp mydatetime.cpp mp3tag.cpp mp3_layer.cpp main.cpp fuzzsrch.cpp foldersorted.cpp foldermedia.cpp foldergroups.cpp foldercategories.cpp folder.cpp 
INTERFACES = UpdateDatabaseDialogBase.ui YammiGuiBase.ui WorkDialogBase.ui SongInfoDialog.ui PreferencesDialogBase.ui DeleteDialog.ui 
TRANSLATIONS = 
KDE_OPTIONS = qtonly
yammi_LDADD   = -lxmms -lid3 $(LIB_QT) $(LIBSOCKET)

SUBDIRS = docs 

EXTRA_DIST = lo32-app-yammi.png lo16-app-yammi.png clearPlaylist.xpm defaultControlClick.xpm defaultDoubleClick.xpm defaultMiddleClick.xpm defaultShiftClick.xpm DeleteDialog.ui dequeueAll.xpm dequeueSong.xpm enqueue.xpm enqueueasnext.xpm filesave.xpm folder.cpp folder.h foldercategories.cpp foldercategories.h foldergroups.cpp foldergroups.h foldermedia.cpp foldermedia.h foldersorted.cpp foldersorted.h fuzzsrch.cpp fuzzsrch.h in.xpm main.cpp mp3_layer.cpp mp3_layer.h mp3tag.cpp mp3tag.h mydatetime.cpp mydatetime.h mylist.cpp mylist.h mylistview.cpp mylistview.h notin.xpm pause.xpm playnow.xpm preferencesdialog.cpp preferencesdialog.h PreferencesDialogBase.ui prefs.cpp prefs.h prelisten.xpm skipbackward.xpm skipbackwardim.xpm skipforward.xpm skipforwardim.xpm song.cpp song.h songentry.cpp songentry.h songentryint.cpp songentryint.h songentryint2.cpp songentryint2.h songentrystring.cpp songentrystring.h songentrytimestamp.cpp songentrytimestamp.h songinfo.cpp songinfo.h songinfo.xpm SongInfoDialog.ui songlistitem.cpp songlistitem.h stop.xpm stopPrelisten.xpm WorkDialogBase.ui yammigui.cpp yammigui.h YammiGuiBase.ui yammimodel.cpp yammimodel.h playnowim.xpm yammiicon.xpm glib.h glibconfig.h xmmsctrl.h updatedatabasedialog.cpp updatedatabasedialog.h UpdateDatabaseDialogBase.ui 

####### kdevelop will overwrite this part!!! (end)############


# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

yammi_METASOURCES = AUTO

# the library search path. 
yammi_LDFLAGS = $(all_libraries) 

# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(yammi_METASOURCES)

