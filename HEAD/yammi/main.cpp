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

//#include <qapplication.h>
#include <qwindowsstyle.h>
#include <qtranslator.h>
#include <qtextcodec.h>
#include "yammigui.h"
#include "yammiapplication.h"

static const char description[] =   I18N_NOOP("Yammi ");

static KCmdLineOptions options[] =
{
	KCmdLineLastOption
};

static const char version[] = "1.1";

// global pointer to YammiGui
YammiGui* gYammiGui;

/**
 * main method, starts the whole application
 */
int main( int argc, char **argv )
{

 	KAboutData about("Yammi", I18N_NOOP("yammi"), version, description,
                     KAboutData::License_GPL, "(C) 2003 Yammi", 0, 0, "yammi@sourceforge.net");

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

/*
  QString arg1="";
  if(argc>=2) {
    // argv[0] is the command name
    arg1=QString(argv[1]);
    if(argc>2 || arg1=="-h" || arg1=="--help") {
      cout << "Yammi - Yet Another Media Manager I...\n";
      cout << "For information visit http://yammi.sourceforge.net\n\n";
      cout << "usage: yammi [baseDir]\n\n";
      cout << "baseDir determines where Yammi looks for the .yammi directory to store all its settings and database\n";
      cout << "if not given, it will be the user's home directory (which should be fine)\n";
      return 0;
    }
  }
  YammiApplication application( argc, argv );
	QDir d;
	if(arg1 == "") {
		// default case: take directory ".yammi" in user's home dir as base dir
		d = QDir::home();  // now points to home directory
	} else {
		// user specified a base dir
		d = QDir(arg1);
		if(!d.exists()) {      
			qDebug("Direcotry %s doesn't exist, taking home direcory.", arg1.latin1());
			d = QDir::home();  // now points to home directory
		}
	}
  QString baseDir=d.absPath();
  QTranslator translator( 0 );
  QString filename=QString("yammi_")+QTextCodec::locale();
  QString yammiBaseDir=baseDir+"/.yammi";
  cout << "trying to load translation file " << filename << " in directory " << yammiBaseDir << "\n";
  bool success=translator.load(filename , yammiBaseDir);
  if(!success) {
    cout << "translation file not found, using english...\n";
  }
  application.installTranslator( &translator );
  
	// initialize gui
  YammiGui gui(baseDir);
  application.setMainWidget( &gui );
	gui.show();
	// enter event loop
	return application.exec();
#endif
  */
}
