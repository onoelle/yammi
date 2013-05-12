

#qmake-qt3 -o Makefile yammi.pro

TEMPLATE += app

CONFIG += qt debug

QMAKE = qmake-qt3
QMAKE_UIC = uic -L /usr/lib/kde3/plugins
QMAKE_CXX=ccache g++

INCLUDEPATH += /usr/include/kde

LIBS += -lartskde -lkhtml -lkdeui -L/usr/lib -ltag

MOC_DIR = .moc
OBJECTS_DIR = .obj
UI_DIR = .ui

QMAKE_CLEAN += yammi

SOURCES = \
    applytoalldialog.cpp \
    artsplayer.cpp \
    ConsistencyCheckDialog.cpp \
    ConsistencyCheckParameter.cpp \
    foldercategories.cpp \
    folder.cpp \
    foldergroups.cpp \
    foldermedia.cpp \
    foldersorted.cpp \
    fuzzsrch.cpp \
    lineeditshift.cpp \
    main.cpp \
    mediaplayer.cpp \
    mydatetime.cpp \
    mylist.cpp \
    mylistview.cpp \
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
    updatedatabasemediadialog.cpp \
    util.cpp \
    yammigui.cpp \
    yammimodel.cpp

HEADERS = \
    applytoalldialog.h \
    artsplayer.h \
    ConsistencyCheckDialog.h \
    ConsistencyCheckParameter.h \
    dummyplayer.h \
    foldercategories.h \
    foldergroups.h \
    folder.h \
    foldermedia.h \
    foldersorted.h \
    fuzzsrch.h \
    i18n.h \
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
    updatedatabasemediadialog.h \
    util.h \
    yammidcopiface.h \
    yammigui.h \
    yammiicons.h \
    yammimodel.h

FORMS = \
    ApplyToAllBase.ui \
    ConsistencyCheckDialogBase.ui \
    DeleteDialog.ui \
    PreferencesDialogBase.ui \
    SongInfoDialog.ui \
    UpdateDatabaseDialogBase.ui \
    UpdateDatabaseMediaDialogBase.ui

SOURCES += \
    yammidcopiface_skel.cpp

#SOURCES_DCOP = yammidcopiface.h
#yammidcop.output  = ${QMAKE_FILE_BASE}_skel.cpp
#yammidcop.commands = dcopidl ${QMAKE_FILE_NAME} > ${QMAKE_FILE_BASE}.kidl && dcopidl2cpp --c++-suffix cpp --no-signals --no-stub ${QMAKE_FILE_BASE}.kidl
##yammidcop.depends =
#yammidcop.input = SOURCES_DCOP

yammidcopiface_skel.cpp.target  = yammidcopiface_skel.cpp
yammidcopiface_skel.cpp.commands = dcopidl yammidcopiface.h > yammidcopiface.kidl && dcopidl2cpp --c++-suffix cpp --no-signals --no-stub yammidcopiface.kidl
yammidcopiface_skel.cpp.depends = yammidcopiface.h
QMAKE_EXTRA_UNIX_TARGETS += yammidcopiface_skel.cpp
PRE_TARGETDEPS += yammidcopiface_skel.cpp

QMAKE_CLEAN += *.kidl *_skel.cpp
