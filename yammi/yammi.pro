
# Features:
    !win32:DEFINES += USE_QDBUS

    contains(QT_VERSION, ^5\\.[0-9]*\\..*): DEFINES += USE_QMEDIAPLAYER

    DEFINES += USE_TAGLIB

    !win32:DEFINES += USE_XINE

    DEFINES += USE_VLC

    !win32:DEFINES += USE_ASOUND
#


# Workarounds
    # error from moc with Qt5: usr/include/c++/4.6/bits/stl_relops.:68: Parse error at "std"
    # probably only in debian, should not harm anyone else
    #QMAKE_CXX = ccache g++-4.6
    #INCLUDEPATH += /usr/include/c++/4.6/i486-linux-gnu
#


contains(QT_VERSION, ^4\\.[0-9]*\\..*): contains(DEFINES, USE_QDBUS): CONFIG += qdbus
contains(QT_VERSION, ^5\\.[0-9]*\\..*): contains(DEFINES, USE_QDBUS): QT += dbus
contains(QT_VERSION, ^5\\.[0-9]*\\..*): QT += widgets
contains(DEFINES, USE_QMEDIAPLAYER): QT += multimedia


unix {
    contains(DEFINES, USE_TAGLIB): INCLUDEPATH += /usr/include/taglib
    contains(DEFINES, USE_TAGLIB): LIBS += -ltag

    contains(DEFINES, USE_XINE): LIBS += -lxine

    # $(pkg-config --cflags libvlc)
    # $(pkg-config --libs libvlc)
    contains(DEFINES, USE_VLC): LIBS += -lvlc

    contains(DEFINES, USE_ASOUND): LIBS += -lasound

    QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
    #QMAKE_CXX = CCACHE_DIR=/tmp/yammi-ccache ccache g++
    #QMAKE_CXX = clang++
}
win32 {
    contains(DEFINES, USE_TAGLIB): INCLUDEPATH += F:\\taglib-1.8\\taglib\\Headers
    contains(DEFINES, USE_TAGLIB): LIBS += -LF:\\taglib-1.8\\taglib -ltag

    QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\\lrelease.exe
}

TEMPLATE += app
QT += xml
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
    preferencesdialog.cpp \
    prefs.cpp \
    qmediaplayer-engine.cpp \
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
    vlc-engine.cpp \
    xine-engine.cpp \
    yammigui.cpp \
    yammilcdnumber.cpp \
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
    preferencesdialog.h \
    prefs.h \
    qmediaplayer-engine.h \
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
    vlc-engine.h \
    xine-engine.h \
    yammigui.h \
    yammilcdnumber.h \
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

update-ts.commands = lupdate -locations absolute -pro yammi.pro
update-ts.target = update-ts
QMAKE_EXTRA_TARGETS += update-ts

stage-ts.commands = lupdate -locations none -pro yammi.pro && git add $${TRANSLATIONS}
stage-ts.target = stage-ts
QMAKE_EXTRA_TARGETS += stage-ts

updateqm.input = TRANSLATIONS
updateqm.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += updateqm
POST_TARGETDEPS += compiler_updateqm_make_all


# installation
unix {
    yammi.path = $$(HOME)/bin/yammi-bin
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
    # or put a cmd file near yammi.exe and add the paths like here:
    #   @echo off
    #   set PATH=%PATH%;F:\taglib-1.8\taglib;E:\Qt\4.8.4\bin
    #   .\yammi.exe
}

yammi_icons.path = $${yammi.path}/icons
yammi_icons.files = icons/*

yammi_translations.path = $${yammi.path}/translations
yammi_translations.files = $${replace(TRANSLATIONS, .ts, .qm)}

unix:yammi_symlink.path = $${yammi.path}
unix:yammi_symlink.commands = ln -sf "$${yammi.path}/yammi" "$${yammi.path}/../yammi"

unix:yammi_shortcut.path = $$(HOME)/.local/share/applications
unix:yammi_shortcut.commands = sed -e "s,/home/user,$$(HOME),g" yammi.desktop.in > $${yammi_shortcut.path}/yammi.desktop && chmod +x $${yammi_shortcut.path}/yammi.desktop

INSTALLS += yammi yammi_icons yammi_translations yammi_symlink yammi_shortcut
