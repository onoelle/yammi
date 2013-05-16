/****************************************************************************
** Yammi - main.cpp
**
**
*****************************************************************************/

#include <QDebug>
#include <QApplication>
#include <QtDBus>

#include "config.h"
#include "yammigui.h"


static const char description[] =   QT_TR_NOOP("Yammi - Yet Another Music Manager I...");
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
    //KAboutData about("yammi", QT_TR_NOOP("Yammi"), version, description, KAboutData::License_GPL, "(C) 2001-2005 by Oliver Noelle", "", "http://yammi.sourceforge.net", "yammi-developer@lists.sourceforge.net");
    app.setOrganizationName("yammi");
    app.setOrganizationDomain("yammi.sourceforge.net");
    app.setApplicationName("yammi");

    if (!QDBusConnection::sessionBus().isConnected()) {
        fprintf(stderr, "Cannot connect to the D-Bus session bus.\n"
                "To start it, run:\n"
                "\teval `dbus-launch --auto-syntax`\n");
        return 1;
    }

    if (!QDBusConnection::sessionBus().registerService("net.sf.yammi.yammi.YammiGui")) {
        fprintf(stderr, "%s\n", qPrintable(QDBusConnection::sessionBus().lastError().message()));
        exit(1);
    }

    YammiGui* yammi = new YammiGui();
	if(!yammi->isValidState()) {
        qDebug() << "shutting down now...";
		return 1;
	}

    QDBusConnection::sessionBus().registerObject("/YammiGui", yammi, QDBusConnection::ExportScriptableSlots);

	app.setMainWidget( yammi );
	yammi->show();
	
    // give yammi a chance for a first draw
    app.processEvents(QEventLoop::ExcludeUserInputEvents);
	
    yammi->loadDatabase();

    // gYammiGui has WDestructiveClose flag by default, so it will delete itself.
	return app.exec();
}
