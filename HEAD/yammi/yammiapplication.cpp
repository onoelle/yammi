/***************************************************************************
                          yammiapplication.cpp  -  description
                             -------------------
    begin                : Thu Sep 26 2002
    copyright            : (C) 2002 by Oliver Nölle
    email                : yammi-developer@lists.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "yammiapplication.h"

#include "yammigui.h"

extern YammiGui* gYammiGui;

YammiApplication::YammiApplication(int argc, char **argv)
 : KApplication(argc, argv)
{
  
}

YammiApplication::~YammiApplication()
{
}


void YammiApplication::commitData(QSessionManager& sm)
{
  if(gYammiGui)
    gYammiGui->commitData(sm);
}
