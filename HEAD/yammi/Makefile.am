####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = yammi

## INCLUDES were found outside kdevelop specific part

yammi_SOURCES = UpdateDatabaseDialogBase.ui updatedatabasedialog.cpp yammimodel.cpp YammiGuiBase.ui yammigui.cpp WorkDialogBase.ui songlistitem.cpp SongInfoDialog.ui songinfo.cpp songentrytimestamp.cpp songentrystring.cpp songentryint2.cpp songentryint.cpp songentry.cpp song.cpp prefs.cpp PreferencesDialogBase.ui preferencesdialog.cpp mylistview.cpp mylist.cpp mydatetime.cpp main.cpp fuzzsrch.cpp foldersorted.cpp foldermedia.cpp foldergroups.cpp foldercategories.cpp folder.cpp DeleteDialog.ui 
SOURCES = mp3info/CVBitRate.cpp mp3info/CMP3Info.cpp mp3info/CFrameHeader.cpp updatedatabasedialog.cpp yammimodel.cpp yammigui.cpp songlistitem.cpp songinfo.cpp songentrytimestamp.cpp songentrystring.cpp songentryint2.cpp songentryint.cpp songentry.cpp song.cpp prefs.cpp preferencesdialog.cpp mylistview.cpp mylist.cpp mydatetime.cpp main.cpp fuzzsrch.cpp foldersorted.cpp foldermedia.cpp foldergroups.cpp foldercategories.cpp folder.cpp 
INTERFACES = UpdateDatabaseDialogBase.ui YammiGuiBase.ui WorkDialogBase.ui SongInfoDialog.ui PreferencesDialogBase.ui DeleteDialog.ui 
TRANSLATIONS = 
KDE_OPTIONS = qtonly
yammi_LDADD   = ./mp3info/libmp3info.a -lxmms -lid3 -lvorbis -lvorbisfile $(LIB_QT) $(LIBSOCKET)

SUBDIRS = docs icons mp3info 

EXTRA_DIST = lo32-app-yammi.png lo16-app-yammi.png DeleteDialog.ui folder.cpp folder.h foldercategories.cpp foldercategories.h foldergroups.cpp foldergroups.h foldermedia.cpp foldermedia.h foldersorted.cpp foldersorted.h fuzzsrch.cpp fuzzsrch.h main.cpp mydatetime.cpp mydatetime.h mylist.cpp mylist.h mylistview.cpp mylistview.h preferencesdialog.cpp preferencesdialog.h PreferencesDialogBase.ui prefs.cpp prefs.h song.cpp song.h songentry.cpp songentry.h songentryint.cpp songentryint.h songentryint2.cpp songentryint2.h songentrystring.cpp songentrystring.h songentrytimestamp.cpp songentrytimestamp.h songinfo.cpp songinfo.h SongInfoDialog.ui songlistitem.cpp songlistitem.h WorkDialogBase.ui yammigui.cpp yammigui.h YammiGuiBase.ui yammimodel.cpp yammimodel.h glib.h glibconfig.h xmmsctrl.h updatedatabasedialog.cpp updatedatabasedialog.h UpdateDatabaseDialogBase.ui options.h 

####### kdevelop will overwrite this part!!! (end)############


# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

yammi_METASOURCES = AUTO

# the library search path. 
yammi_LDFLAGS = $(all_libraries) 

# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(yammi_METASOURCES)

