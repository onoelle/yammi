/****************************************************************************
** Yammi - main.cpp
**
**
*****************************************************************************/

#include <kapplication.h>
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
//{ "d <dir>", I18N_NOOP("specifies location of .yammi dir (defaults to user home)"), 0 },
			{ "databasedir <dir>", I18N_NOOP("Specifies the directory of yammi data (defaults to ~/.yammi/)"), 0 } };
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
	KApplication app;
	YammiGui *yammi = new YammiGui();
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
	else {
		//the configuration file could not be opened, most likely we are starting for the first time
		QString msg( i18n( "Yammi - Yet Another Music Manager I...\n\n\n \
It looks like you are starting Yammi for the first time...\n\n\
Welcome to convenient music organization!\n\n\
Please edit the settings (Settings -> Configure Yammi...)\n\
to adjust your personal configuration and options\
(especially the path to your media files).\n\
Then perform a database update (Database -> Scan Harddisk...)\n\
to scan your harddisk for media files.\n\n\
Have fun using Yammi...\n\n\
Check out Yammi's website for new versions and other info:\n\
http://yammi.sourceforge.net " ) );
		KMessageBox::information( 0L, msg );
	}
	args->clear();  
	// gYammiGui has WDestructiveClose flag by default, so it will delete itself.
	return app.exec();
}
