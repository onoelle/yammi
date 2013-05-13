/****************************************************************************
** Yammi - main.cpp
**
**
*****************************************************************************/

#include <kuniqueapplication.h>
#include <kconfig.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <config.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <qeventloop.h>

#include "yammigui.h"




static KCmdLineOptions options[] = { 
			{ "d", 0, 0 },
                        { "databasedir <dir>", QT_TR_NOOP("Specifies the database directory"), 0 } };

static const char description[] =   QT_TR_NOOP("Yammi - Yet Another Music Manager I...");
static const char version[] = VERSION;

// global pointer to YammiGui
YammiGui* gYammiGui;

/**
 * main method, starts the whole application
 */
int main( int argc, char **argv )
{
        KAboutData about("yammi", QT_TR_NOOP("Yammi"), version, description,
                     KAboutData::License_GPL, "(C) 2001-2005 by Oliver Noelle", "", "http://yammi.sourceforge.net", "yammi-developer@lists.sourceforge.net");

	KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions( options );
    KUniqueApplication::addCmdLineOptions();
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	srand(time(NULL));
	KUniqueApplication app;
	YammiGui* yammi = new YammiGui();
	if(!yammi->isValidState()) {
		kdDebug() << "shutting down now...\n";
		return 1;
	}
	app.setMainWidget( yammi );
	yammi->show();
	
    // give yammi a chance for a first draw
    app.eventLoop()->processEvents(QEventLoop::ExcludeUserInput);
	
    KConfig *cfg = app.config( );
	if( cfg->getConfigState( ) == KConfig::ReadOnly || cfg->getConfigState( ) == KConfig::ReadWrite)
	{
		QString databaseDir( args->getOption("databasedir") );
		yammi->loadDatabase(databaseDir);
	}
	args->clear();  
	// gYammiGui has WDestructiveClose flag by default, so it will delete itself.
	return app.exec();
}
