/****************************************************************************
** Yammi - main.cpp
**
**
*****************************************************************************/

#include <qapplication.h>
#include <qwindowsstyle.h>
#include "yammigui.h"

// global pointer to YammiGui
YammiGui* gYammiGui;

/**
 * main method, starts the whole application
 */
int main( int argc, char **argv )
{
	QApplication application( argc, argv );
//	QApplication::setStyle(new QWindowsStyle());
	// initialize gui
	YammiGui gui;
	application.setMainWidget( &gui );
	gui.show();
/*
  QSettings settings;
  settings.insertSearchPath( QSettings::Unix, gui.getModel()->config.yammiBaseDir );
  int posx = settings.readNumEntry( "/Yammi/geometry/posx", 0 );
  int posy = settings.readNumEntry( "/Yammi/geometry/posy", 0 );
  int width = settings.readNumEntry( "/Yammi/geometry/width", 1024 );
  int height = settings.readNumEntry( "/Yammi/geometry/height", 468 );
  gui.setGeometry(QRect(posx, posy, width, height));
  */
	// enter event loop
	return application.exec();
}
