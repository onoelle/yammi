/***************************************************************************
                          ConsistencyCheckParameter.h  -  description
                             -------------------
    begin                : Sat Dec 7 2002
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

#ifndef CONSISTENCYCHECKPARAMETER_H
#define CONSISTENCYCHECKPARAMETER_H

class ConsistencyCheckParameter
{
public:
  bool checkForExistence;
  bool updateNonExisting;
  bool checkTags;
  bool correctTags;
  int correctTagsConfirmed;
  bool checkFilenames;
  int correctFilenamesConfirmed;
  bool correctFilenames;
  bool checkDoubles;

  int nonExisting;
  int nonExistingUpdated;
  int nonExistingDeleted;
  int dirtyTags;
  int tagsCorrected;
  int dirtyFilenames;
  int filenamesCorrected;
  int doublesFound;
};

#endif // CONSISTENCYCHECKPARAMETER_H
