####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = yammi

## INCLUDES were found outside kdevelop specific part

yammi_SOURCES = UpdateDatabaseMediaDialogBase.ui UpdateDatabaseDialogBase.ui SongInfoDialog.ui PreferencesDialogBase.ui DeleteDialog.ui ConsistencyCheckDialogBase.ui ConsistencyCheckDialog.cpp vcedit.c yammiapplication.cpp xmmsplayer.cpp mediaplayer.cpp lineeditshift.cpp updatedatabasemediadialog.cpp updatedatabasedialog.cpp yammimodel.cpp yammigui.cpp songlistitem.cpp songinfo.cpp songentrytimestamp.cpp songentrystring.cpp songentryint2.cpp songentryint.cpp songentry.cpp song.cpp prefs.cpp preferencesdialog.cpp mylistview.cpp mylist.cpp mydatetime.cpp main.cpp fuzzsrch.cpp foldersorted.cpp foldermedia.cpp foldergroups.cpp foldercategories.cpp folder.cpp 
SOURCES = mp3info/CVBitRate.cpp mp3info/CMP3Info.cpp mp3info/CFrameHeader.cpp ConsistencyCheckDialog.cpp vcedit.c yammiapplication.cpp xmmsplayer.cpp mediaplayer.cpp lineeditshift.cpp updatedatabasemediadialog.cpp updatedatabasedialog.cpp yammimodel.cpp yammigui.cpp songlistitem.cpp songinfo.cpp songentrytimestamp.cpp songentrystring.cpp songentryint2.cpp songentryint.cpp songentry.cpp song.cpp prefs.cpp preferencesdialog.cpp mylistview.cpp mylist.cpp mydatetime.cpp main.cpp fuzzsrch.cpp foldersorted.cpp foldermedia.cpp foldergroups.cpp foldercategories.cpp folder.cpp 
INTERFACES = UpdateDatabaseMediaDialogBase.ui UpdateDatabaseDialogBase.ui SongInfoDialog.ui PreferencesDialogBase.ui DeleteDialog.ui ConsistencyCheckDialogBase.ui 
TRANSLATIONS = 
KDE_OPTIONS = qtonly
yammi_LDADD   = ./mp3info/libmp3info.a -lxmms $(LIB_QT) $(LIBSOCKET)

SUBDIRS = mp3info 

EXTRA_DIST = lo32-app-yammi.png lo16-app-yammi.png folder.cpp folder.h foldercategories.cpp foldercategories.h foldergroups.cpp foldergroups.h foldermedia.cpp foldermedia.h foldersorted.cpp foldersorted.h fuzzsrch.cpp fuzzsrch.h main.cpp mydatetime.cpp mydatetime.h mylist.cpp mylist.h mylistview.cpp mylistview.h preferencesdialog.cpp preferencesdialog.h prefs.cpp prefs.h song.cpp song.h songentry.cpp songentry.h songentryint.cpp songentryint.h songentryint2.cpp songentryint2.h songentrystring.cpp songentrystring.h songentrytimestamp.cpp songentrytimestamp.h songinfo.cpp songinfo.h songlistitem.cpp songlistitem.h yammigui.cpp yammigui.h yammimodel.cpp yammimodel.h glib.h glibconfig.h xmmsctrl.h updatedatabasedialog.cpp updatedatabasedialog.h options.h updatedatabasemediadialog.cpp updatedatabasemediadialog.h lineeditshift.cpp lineeditshift.h mediaplayer.cpp mediaplayer.h xmmsplayer.cpp xmmsplayer.h yammiapplication.cpp yammiapplication.h vcedit.c vcedit.h acconfig.h ConsistencyCheckDialog.cpp ConsistencyCheckDialog.h ConsistencyCheckParameter.h ConsistencyCheckDialogBase.ui DeleteDialog.ui PreferencesDialogBase.ui SongInfoDialog.ui UpdateDatabaseDialogBase.ui UpdateDatabaseMediaDialogBase.ui 

####### kdevelop will overwrite this part!!! (end)############


# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

yammi_METASOURCES = AUTO

# the library search path. 
yammi_LDFLAGS = $(all_libraries) 

# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(yammi_METASOURCES)

