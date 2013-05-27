
TEMPLATE += app
QT += xml qt3support
CONFIG += qt qdbus debug

LIBS += -ltag -lxine

MOC_DIR = .moc
OBJECTS_DIR = .obj
UI_DIR = .ui

#QMAKE_CXX=ccache g++
QMAKE_CLEAN += yammi

SOURCES = \
    applytoalldialog.cpp \
    ConsistencyCheckDialog.cpp \
    ConsistencyCheckParameter.cpp \
    folder.cpp \
    foldercategories.cpp \
    foldergroups.cpp \
    foldermodel.cpp \
    foldersorted.cpp \
    fuzzsrch.cpp \
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
    folder.h \
    foldercategories.h \
    foldergroups.h \
    foldermodel.h \
    foldersorted.h \
    fuzzsrch.h \
    mediaplayer.h \
    mydatetime.h \
    mylist.h \
    mylistview.h \
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
    trackpositionslider.h \
    updatedatabasedialog.h \
    util.h \
    xine-engine.h \
    yammigui.h \
    yammimodel.h

FORMS = \
    ApplyToAllBase.ui \
    ConsistencyCheckDialogBase.ui \
    DeleteDialog.ui \
    PreferencesDialogBase.ui \
    SongInfoDialog.ui \
    UpdateDatabaseDialogBase.ui


# translations
TRANSLATIONS = \
    translations/yammi_de.ts \
    translations/yammi_es.ts \
    translations/yammi_fi.ts \
    translations/yammi_hu.ts \
    translations/yammi_it.ts \
    translations/yammi_nl.ts

updateqm.commands = lrelease-qt4 yammi.pro
updateqm.target = updateqm
QMAKE_EXTRA_TARGETS += updateqm


# installation
yammi.path = $$(HOME)/bin/yammi
yammi.files = yammi

yammi-icons.path = $${yammi.path}/icons
yammi-icons.files = icons/*

yammi-translations.path = $${yammi.path}/translations
yammi-translations.files = translations/*.qm

INSTALLS += yammi yammi-icons yammi-translations
