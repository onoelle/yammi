

#qmake-qt3 -o Makefile yammi.pro

TEMPLATE += app

CONFIG += qt debug

QMAKE = qmake-qt3
QMAKE_UIC = uic -L /usr/lib/kde3/plugins
QMAKE_CXX=ccache g++

INCLUDEPATH += /usr/include/kde

LIBS += -lkdeui -L/usr/lib -ltag -lxine

MOC_DIR = .moc
OBJECTS_DIR = .obj
UI_DIR = .ui

QMAKE_CLEAN += yammi

SOURCES = \
    applytoalldialog.cpp \
    ConsistencyCheckDialog.cpp \
    ConsistencyCheckParameter.cpp \
    foldercategories.cpp \
    folder.cpp \
    foldergroups.cpp \
    foldersorted.cpp \
    fuzzsrch.cpp \
    lineeditshift.cpp \
    main.cpp \
    mediaplayer.cpp \
    mydatetime.cpp \
    mylist.cpp \
    mylistview.cpp \
    options.cpp \
    preferencesdialog.cpp \
    prefs.cpp \
    searchthread.cpp \
    song.cpp \
    songentry.cpp \
    songentryint2.cpp \
    songentryint.cpp \
    songentrystring.cpp \
    songentrytimestamp.cpp \
    songinfo.cpp \
    songlistitem.cpp \
    trackpositionslider.cpp \
    updatedatabasedialog.cpp \
    util.cpp \
    xine-engine.cpp \
    yammigui.cpp \
    yammimodel.cpp

HEADERS = \
    applytoalldialog.h \
    ConsistencyCheckDialog.h \
    ConsistencyCheckParameter.h \
    dummyplayer.h \
    foldercategories.h \
    foldergroups.h \
    folder.h \
    foldersorted.h \
    fuzzsrch.h \
    lineeditshift.h \
    mediaplayer.h \
    mydatetime.h \
    mylist.h \
    mylistview.h \
    options.h \
    preferencesdialog.h \
    prefs.h \
    searchthread.h \
    songentry.h \
    songentryint2.h \
    songentryint.h \
    songentrystring.h \
    songentrytimestamp.h \
    song.h \
    songinfo.h \
    songlistitem.h \
    trackpositionslider.h \
    updatedatabasedialog.h \
    util.h \
    xine-engine.h \
    yammigui.h \
    yammiicons.h \
    yammimodel.h

FORMS = \
    ApplyToAllBase.ui \
    ConsistencyCheckDialogBase.ui \
    DeleteDialog.ui \
    PreferencesDialogBase.ui \
    SongInfoDialog.ui \
    UpdateDatabaseDialogBase.ui
