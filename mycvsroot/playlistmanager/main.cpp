/****************************************************************************
** Yammi - main.cpp
**
**
*****************************************************************************/

#include <qapplication.h>
#include "yammigui.h"

/**
 * main method, starts the whole application
 */
int main( int argc, char **argv )
{
	QApplication application( argc, argv );

	YammiGui gui;						// starts gui
	gui.resize( 1024, 468 );
	gui.setCaption( "Yammi 0.1 - 12-2001 by Brian O. Nölle" );
	application.setMainWidget( &gui );
	gui.show();
	return application.exec();
}
