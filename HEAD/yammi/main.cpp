/****************************************************************************
** Yammi - main.cpp
**
**
*****************************************************************************/

#include <qapplication.h>
#include <qwindowsstyle.h>
#include <qtranslator.h>
#include <qtextcodec.h>
#include "yammigui.h"
#include "yammiapplication.h"


//#include "options.h"
//#ifdef ENABLE_NOATUN
//#include <kcmdlineargs.h>
//#endif

// global pointer to YammiGui
YammiGui* gYammiGui;
/**
 * main method, starts the whole application
 */
int main( int argc, char **argv )
{
//#ifdef ENABLE_NOATUN
//  KCmdLineArgs::init(argc, argv, "Yammi", "", "");
//#endif
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
}
