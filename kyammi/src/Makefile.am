# set the include path for X, qt and KDE
INCLUDES = $(all_includes)

# these are the headers for your project


# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/yammi.pot

KDE_ICON = yammi

#########################################################################
# APPLICATION SECTION
#########################################################################
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = yammi

# the application source, library search path, and link libraries
yammi_SOURCES = main.cpp ApplyToAllBase.ui artsplayer.cpp artsplayer.h CFrameHeader.cpp CFrameHeader.h CMP3Info.cpp CMP3Info.h ConsistencyCheckDialogBase.ui ConsistencyCheckDialog.cpp ConsistencyCheckDialog.h ConsistencyCheckParameter.h CVBitRate.cpp CVBitRate.h DeleteDialog.ui dummyplayer.h foldercategories.cpp foldercategories.h folder.cpp foldergroups.cpp foldergroups.h folder.h foldermedia.cpp foldermedia.h foldersorted.cpp foldersorted.h fuzzsrch.cpp fuzzsrch.h hi16-app-yammi.png hi32-app-yammi.png lineeditshift.cpp lineeditshift.h Makefile.am Makefile.in mediaplayer.cpp mediaplayer.h mp3_layer.cpp mp3_layer.h mp3tag.cpp mp3tag.h mydatetime.cpp mydatetime.h mylist.cpp mylist.h mylistview.cpp mylistview.h noatunplayer.cpp noatunplayer.h options.h PreferencesDialogBase.ui preferencesdialog.cpp preferencesdialog.h prefs.cpp prefs.h song.cpp songentry.cpp songentry.h songentryint2.cpp songentryint2.h songentryint.cpp songentryint.h songentrystring.cpp songentrystring.h songentrytimestamp.cpp songentrytimestamp.h song.h songinfo.cpp SongInfoDialog.ui songinfo.h songlistitem.cpp songlistitem.h UpdateDatabaseDialogBase.ui updatedatabasedialog.cpp updatedatabasedialog.h UpdateDatabaseMediaDialogBase.ui updatedatabasemediadialog.cpp updatedatabasemediadialog.h xmmsctrl.h xmmsplayer.cpp xmmsplayer.h yammigui.cpp yammigui.h yammiicons.h yammimodel.cpp yammimodel.h vcedit.c vcedit.h i18n.h
yammi_LDFLAGS = $(KDE_RPATH) $(all_libraries)
yammi_LDADD = -lartskde $(LIB_KDEUI)

# this is where the desktop file will go 
shelldesktopdir = $(kde_appsdir)/Utilities


# this is where the shell's XML-GUI resource file goes
shellrcdir = $(kde_datadir)/yammi


