/****************************************************************************
** Yammi - main.cpp
**
**
*****************************************************************************/

#include <qapplication.h>
#include <qwindowsstyle.h>
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
	YammiApplication application( argc, argv );
//	QApplication::setStyle(new QWindowsStyle());
	// initialize gui
	YammiGui gui;
	application.setMainWidget( &gui );
	gui.show();
	// enter event loop
	return application.exec();
}
