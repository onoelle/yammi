/****************************************************************************
** Yammi - main.cpp
**
**
*****************************************************************************/

#include <qapplication.h>
#include "yammigui.h"

// global pointer to YammiGui
YammiGui* gYammiGui;

/**
 * main method, starts the whole application
 */
int main( int argc, char **argv )
{
	QApplication application( argc, argv );

	YammiGui gui;						// starts gui
	gui.resize( 1024, 468 );
	gui.setCaption( "Yammi 0.4 - 3-2002 by Oliver Nölle" );
	application.setMainWidget( &gui );
	gui.show();
	return application.exec();
}
