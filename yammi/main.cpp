/****************************************************************************
** Yammi - main.cpp
**
**
*****************************************************************************/

#include <time.h>

#include <QApplication>
#include <QDebug>
#include <QDir>
#ifdef USE_QDBUS
#include <QtDBus>
#endif
#include <QTranslator>

#include "config.h"
#include "prefs.h"
#include "yammigui.h"


static const char version[] = VERSION;

// global pointer to YammiGui
YammiGui* gYammiGui;

/**
 * main method, starts the whole application
 */
int main( int argc, char **argv )
{
	srand(time(NULL));

    QApplication app(argc, argv);
    //KAboutData about("yammi", QT_TR_NOOP("Yammi"), version, QT_TR_NOOP("Yammi - Yet Another Music Manager I ..."), KAboutData::License_GPL, "(C) 2001-2005 by Oliver Noelle", "", "http://yammi.sourceforge.net", "yammi-developer@lists.sourceforge.net");
    app.setOrganizationName("yammi");
    app.setOrganizationDomain("yammi.sourceforge.net");
    app.setApplicationName("yammi");

    /* set the search path, so the Qt resource system could find the icons and pictures */
    QDir::addSearchPath("icons", QCoreApplication::applicationDirPath() + "/icons");
    QDir::addSearchPath("icons", QDir::currentPath() + "/icons");
    QDir::addSearchPath("icons", QDir::currentPath() + "/../src/icons");

    QDir::addSearchPath("translations", QCoreApplication::applicationDirPath() + "/translations");
    QDir::addSearchPath("translations", QDir::currentPath() + "/translations");
    QDir::addSearchPath("translations", QDir::currentPath() + "/../src/translations");

    QString directory = "translations:";
    QString filename = QString("yammi_%1").arg(QLocale::system().name().toLower());
#if QT_VERSION < QT_VERSION_CHECK(4, 8, 2)
    /* in squeeze loading with the resource path "translations:yammi_de_de" is not working
       whereas in wheezy it is working. Therefore here get the path to the resource for ourselve.
    */
    QFileInfo fi(directory + "yammi_de.qm");
    directory = fi.absolutePath();
    qDebug() << "using workaround for older Qt versions: directory:" << directory << " filename:" << filename;
#endif
    QTranslator* translator = new QTranslator();
    if (translator->load(filename, directory)) {
        app.installTranslator(translator);
    } else {
        qDebug() << "translation not found: directory:" << directory << " filename:" << filename;
    }

    YammiGui* yammi = new YammiGui();
	if(!yammi->isValidState()) {
        qDebug() << "shutting down now...";
		return 1;
	}

#ifdef USE_QDBUS
    if (!QDBusConnection::sessionBus().isConnected()) {
        qCritical() << "Cannot connect to the D-Bus session bus.\n"
                "To start it, run:\n"
                "\teval `dbus-launch --auto-syntax`";
    } else {
        if (!QDBusConnection::sessionBus().registerService(yammi->config()->getDBusService())) {
            qCritical() << qPrintable(QDBusConnection::sessionBus().lastError().message());
        } else {
            QDBusConnection::sessionBus().registerObject(yammi->config()->getDBusPath(), yammi, QDBusConnection::ExportScriptableSlots);
        }
    }
#endif

    QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
	yammi->show();
	
    // give yammi a chance for a first draw
    app.processEvents(QEventLoop::ExcludeUserInputEvents);
	
    yammi->loadDatabase();

    // gYammiGui has WDestructiveClose flag by default, so it will delete itself.
	return app.exec();
}
