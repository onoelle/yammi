
TEMPLATE += app
QT += xml qt3support
CONFIG += qt debug uic3

LIBS += -ltag -lxine

MOC_DIR = .moc
OBJECTS_DIR = .obj
UI_DIR = .ui

QMAKE_CXX=ccache g++
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
    yammimodel.h

#The following line was changed from FORMS to FORMS3 by qt3to4
FORMS3 = \
    ApplyToAllBase.ui \
    ConsistencyCheckDialogBase.ui \
    DeleteDialog.ui \
    PreferencesDialogBase.ui \
    SongInfoDialog.ui \
    UpdateDatabaseDialogBase.ui
