/***************************************************************************
 *   Copyright (C) 2004 by Oliver Nölle                                    *
 *   oli.noelle@web.de                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "ConsistencyCheckParameter.h"
 
ConsistencyCheckParameter::ConsistencyCheckParameter()
{
	setDefaults();
	resetStatistics();
}

void ConsistencyCheckParameter::setDefaults()
{
	checkDirectories=true;
	checkDoubles=true;
	checkFilenames=true;
	checkForExistence=true;
	checkTags=true;
	correctDirectories=false;
	correctFilenames=false;
	correctTags=false;
	deleteEmptyDirectories=false;
	ignoreCaseInFilenames=false;
	updateNonExisting=false;
	filenamePattern = "{artist} - {title}.{suffix}";
	directoryPattern = "{artist}/{album}";
}	
	
void ConsistencyCheckParameter::resetStatistics()
{
	dirtyTags=0;
    dirtyFilenames=0;
    dirtyDirectories=0;
    doublesFound=0;
    filenamesCorrected=0;
    directoriesCorrected=0;
    nonExisting=0;
    nonExistingDeleted=0;
    nonExistingUpdated=0;
    tagsCorrected=0;
    correctTagsConfirmed=-1;
    correctFilenamesConfirmed=-1;
    correctDirectoriesConfirmed=-1;
}
