/****************************************************************************
** Yammi - main.cpp
**
**
*****************************************************************************/

#include <qapplication.h>
#include <qwindowsstyle.h>
#include "yammigui.h"
#include "yammiapplication.h"

// global pointer to YammiGui
YammiGui* gYammiGui;

/**
 * main method, starts the whole application
 */
int main( int argc, char **argv )
{
	YammiApplication application( argc, argv );
//	QApplication::setStyle(new QWindowsStyle());
	// initialize gui
	YammiGui gui;
	application.setMainWidget( &gui );
	gui.show();
	// enter event loop
	return application.exec();
}