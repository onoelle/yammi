
TEMPLATE += app
QT += xml qt3support
CONFIG += qt qdbus debug uic3

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
    foldercategories.cpp \
    folder.cpp \
    foldergroups.cpp \
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
