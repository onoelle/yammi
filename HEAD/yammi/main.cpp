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
    if(arg1=="-h" || arg1=="--help") {
      cout << "Yammi - Yet Another Media Manager I...\n";
      cout << "For information visit http://yammi.sourceforge.net\n\n";
      cout << "usage: yammi [baseDir]\n\n";
      cout << "baseDir determines where Yammi looks for the .yammi directory to store all its settings and database\n";
      cout << "if not given, it will be the user's home directory (which should be fine)\n";
      return 0;
    }
  }
  YammiApplication application( argc, argv );
        QTranslator translator( 0 );
        translator.load( QString("yammi_de"), "/home/oliver/.yammi/" );
//        translator.load( QString("yammi_") + QTextCodec::locale(), "." );
        application.installTranslator( &translator );
	// initialize gui
  YammiGui gui(arg1);
 application.setMainWidget( &gui );
	gui.show();
	// enter event loop
	return application.exec();
}
