/****************************************************************************
** Yammi - main.cpp
**
**
*****************************************************************************/

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kdebug.h>

#include <qwindowsstyle.h>
#include <qtranslator.h>
#include <qtextcodec.h>
#include "yammigui.h"
#include "yammiapplication.h"



static KCmdLineOptions options[] =
{
	KCmdLineLastOption
};

static const char description[] =   I18N_NOOP("Yammi - Yet Another Music Manager I...");
static const char version[] = "1.1";

// global pointer to YammiGui
YammiGui* gYammiGui;

/**
 * main method, starts the whole application
 */
int main( int argc, char **argv )
{
	QString build_opts = "";
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
                     KAboutData::License_GPL, "(C) 2001-2003 by Oliver Nölle", build_opts + "\n\n\nhave fun...", "http://yammi.sf.net", "yammi-developer@lists.sourceforge.net");

	KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions( options );
	KApplication app;
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	QDir d = QDir::home( );
  QString baseDir=d.absPath();

  gYammiGui = new YammiGui(baseDir);
  app.setMainWidget( gYammiGui );
  gYammiGui->show();
  args->clear();

	// gYammiGui has WDestructiveClose flag by default, so it will delete itself.
	return app.exec();
}
