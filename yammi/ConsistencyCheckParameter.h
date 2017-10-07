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

#include <QString>


class ConsistencyCheckParameter {
public:
    static const int YAMMI2TAGS = 0;
    static const int TAGS2YAMMI = 1;

    ConsistencyCheckParameter();
    void setDefaults();
    void setAllDisabled();

    void resetStatistics();



    bool checkForExistence;
    bool updateNonExisting;
    bool checkTags;
    bool correctTags;
    int correctTagsConfirmed;
    int correctTagsDirection;          // 0 = yammi => file tags, 1 = file tags => yammi
    
    /** Ignore case in filenames (if keeping them consistent */
    bool ignoreCaseInFilenames;    
    bool checkFilenames;
    QString filenamePattern;
    int correctFilenamesConfirmed;
    bool correctFilenames;
    bool checkDirectories;
    QString directoryPattern;
    int correctDirectoriesConfirmed;
    bool correctDirectories;
    bool deleteEmptyDirectories;
    bool checkDoubles;

    int nonExisting;
    int nonExistingUpdated;
    int nonExistingDeleted;
    int dirtyTags;
    int tagsCorrected;
    int dirtyFilenames;
    int filenamesCorrected;
    int dirtyDirectories;
    int directoriesCorrected;
    int doublesFound;
};

#endif // CONSISTENCYCHECKPARAMETER_H
