/****************************************************************************
** Yammi - main.cpp
**
**
*****************************************************************************/

#include <kuniqueapplication.h>
#include <kconfig.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <config.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include "yammigui.h"



static KCmdLineOptions options[] = { 
			{ "d", 0, 0 },
			{ "databasedir <dir>", I18N_NOOP("Specifies the database directory"), 0 } };

static const char description[] =   I18N_NOOP("Yammi - Yet Another Music Manager I...");
static const char version[] = VERSION;

// global pointer to YammiGui
YammiGui* gYammiGui;

/**
 * main method, starts the whole application
 */
int main( int argc, char **argv )
{
	QString build_opts("");
	build_opts+=i18n("- Arts  support: yes\n");
	build_opts+=i18n("- Noatun support: yes\n");
#ifdef ENABLE_XMMS
	build_opts+=i18n("- XMMS support: yes\n");
#else
	build_opts+=i18n("- XMMS support: no\n");
#endif
	
#ifdef ENABLE_OGGLIBS
	build_opts+=i18n("- ogglibs support: yes\n");
#else
	build_opts+=i18n("- ogglibs support: no\n");
#endif
#ifdef ENABLE_ID3LIB
	build_opts+=i18n("- id3lib support: yes\n");
#else
	build_opts+=i18n("- id3lib support: no\n");
#endif
  
	KAboutData about("yammi", I18N_NOOP("Yammi"), version, description,
                     KAboutData::License_GPL, "(C) 2001-2004 by Oliver Noelle", build_opts, "http://yammi.sourceforge.net", "yammi-developer@lists.sourceforge.net");

	KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions( options );
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
	app.processEvents( );
  
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
