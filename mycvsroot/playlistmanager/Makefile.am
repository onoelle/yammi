####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = playlistmanager
playlistmanager_SOURCES = yammigui.cpp yammimodel.cpp folderhistory.cpp mydatetime.cpp deletedialog.cpp DeleteDialog.ui songlistitem.cpp folderunclassified.cpp folderactual.cpp prefs.cpp preferencesdialog.cpp preferencesdialogbase.cpp PreferencesDialogBase.ui foldermedia.cpp SongInfoDialog.ui songinfodialog.cpp songinfo.cpp mylist.cpp folderplaylists.cpp foldergroups.cpp folderall.cpp folder.cpp fuzzsrch.cpp mp3_layer.cpp song.cpp mp3tag.cpp main.cpp 
playlistmanager_LDADD   = -lxmms -lm -lqt -lXext -lX11 $(LIBSOCKET)

SUBDIRS = docs 

EXTRA_DIST = playlistmanager.xpm mini-playlistmanager.xpm filenew.xpm filesave.xpm fileopen.xpm main.cpp mp3tag.cpp mp3tag.h song.h song.cpp mp3_layer.cpp mp3_layer.h fuzzsrch.cpp fuzzsrch.h folder.h folder.cpp folderall.cpp folderall.h foldergroups.cpp foldergroups.h folderplaylists.cpp folderplaylists.h mylist.cpp mylist.h firstRelease.spec songinfo.cpp songinfo.h songinfodialog.cpp songinfodialog.h SongInfoDialog.ui foldermedia.cpp foldermedia.h PreferencesDialogBase.ui preferencesdialogbase.cpp preferencesdialogbase.h preferencesdialog.cpp preferencesdialog.h prefs.cpp prefs.h folderactual.cpp folderactual.h folderunclassified.cpp folderunclassified.h songlistitem.cpp songlistitem.h DeleteDialog.ui deletedialog.cpp deletedialog.h mydatetime.cpp mydatetime.h folderhistory.cpp folderhistory.h yammimodel.cpp yammimodel.h yammigui.cpp yammigui.h 

####### kdevelop will overwrite this part!!! (end)############


# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

playlistmanager_METASOURCES = AUTO

# the library search path. 
playlistmanager_LDFLAGS = $(all_libraries) 

# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(playlistmanager_METASOURCES)




