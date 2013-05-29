
unix {
    CONFIG += qdbus
    INCLUDEPATH += /usr/include/taglib
    LIBS += -ltag -lxine
    SOURCES += xine-engine.cpp
    HEADERS += xine-engine.h
    DEFINES += USE_XINE USE_QDBUS
    QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease-qt4
    #QMAKE_CXX = ccache g++
    #QMAKE_CXX = clang++
}
win32 {
    taglib.path = F:\\taglib-1.8\\taglib
    INCLUDEPATH += $${taglib.path}\\Headers
    LIBS += -L$${taglib.path} -ltag
    QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\\lrelease.exe
}

TEMPLATE += app
QT += xml phonon
CONFIG += qt debug

MOC_DIR = .moc
OBJECTS_DIR = .obj
UI_DIR = .ui

SOURCES += \
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
    phonon-engine.cpp \
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
    yammigui.cpp \
    yammimodel.cpp

HEADERS += \
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
    phonon-engine.h \
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

updateqm.input = TRANSLATIONS
updateqm.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += updateqm
POST_TARGETDEPS += compiler_updateqm_make_all


# installation
unix {
    yammi.path = $$(HOME)/bin/yammi
    yammi.files = yammi
}
win32 {
    yammi.path = ./bin/yammi
    yammi.files = Debug/yammi.exe
    # also copy at least these dlls into folder ./bin/yammi/:
    #   F:\taglib-1.8\taglib\libtag.dll
    #   E:\Qt\4.8.4\bin\mingwm10.dll
    #   E:\Qt\4.8.4\bin\libgcc_s_dw2-1.dll
    #   E:\Qt\4.8.4\bin\QtCored4.dll
    #   E:\Qt\4.8.4\bin\QtGuid4.dll
    #   E:\Qt\4.8.4\bin\QtXmld4.dll
    #   E:\Qt\4.8.4\bin\phonond4.dll
    # or put a cmd file near yammi.exe and add the paths like here:
    #   @echo off
    #   set PATH=%PATH%;F:\taglib-1.8\taglib;E:\Qt\4.8.4\bin
    #   .\yammi.exe
}

yammi_icons.path = $${yammi.path}/icons
yammi_icons.files = icons/*

yammi_translations.path = $${yammi.path}/translations
yammi_translations.files = $${replace(TRANSLATIONS, .ts, .qm)}

INSTALLS += yammi yammi_icons yammi_translations
