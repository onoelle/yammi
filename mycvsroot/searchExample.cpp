// tiny example program for fuzzsrch.cpp
// 
// 8-2000 by Oliver Nölle
//
// checks whether file "db_strassen_objekte.txt "exists and opens it
// loop while user continues to perform searches
//   print list of best matches and list of ngrams
// close file & exit

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>

#include "fuzzsrch.h"

main()
{
	FILE	*inFile;
	char	searchStr[MaximumEntryLength*5];
	char	TextBuffer[MaximumEntryLength*5];
	int 	Done = 0;
	BestMatchEntry** bme;
	NgramEntry** ngs;
	
	if((inFile = fopen("db_strassen_objekte.txt","r")) == NULL) {
		printf("Datei kann nicht geöffnet werden.\n");
		return(1);
	}
	
	FuzzySearch fs;
	while(!Done)
	{
		printf("\nSuchstring (xxx zum Beenden):");
		gets(searchStr);
		
		if(strcmp(searchStr, "xxx")) {
			
			// remember time
			struct	_timeb tstruct;
			int		milliEnd, milliStart, secEnd, secStart, span;
			_ftime(&tstruct );
			milliStart=tstruct.millitm;
			secStart=tstruct.time;
			
			printf("build class...");
			
			fs.initialize(searchStr, 2, 4);			// STEP 1
			
			// read object names line by line...
			printf("lese file...\n");
			fseek(inFile, 0L, SEEK_SET);
			while (fgets(TextBuffer, MaximumEntryLength*5 - 1, inFile)) {
				fs.checkNext(TextBuffer, 0);				// STEP 2
			}

			// measure timespan before we print out sth
			_ftime( &tstruct );
			milliEnd=tstruct.millitm;
			secEnd=tstruct.time;
			span = (secEnd-secStart)*1000+milliEnd-milliStart;
			
			bme=fs.getBestMatchesList();				// STEP 3
			ngs=fs.ngs;

			// print ngramEntries
			for(int i=1; i<=fs.ngr->noNgrams; i++) {
				printf("[%d. ngram-entry] length %d, freq %d, entropy %d, '%s'\n", i, ngs[i]->length, ngs[i]->frequency, ngs[i]->entropy, ngs[i]->str);
			}
			
			// print 30 best matches
			for(i=0; i<30 && bme[i]; i++) {
				printf("[%d], <%s>\n", bme[i]->sim, bme[i]->preparedName);		// STEP 4
			}

			fs.newOrder();								// STEP 3A

			bme=fs.getBestMatchesList();
			ngs=fs.ngs;

			// print ngramEntries
			for(i=1; i<=fs.ngr->noNgrams; i++) {
				printf("[%d. ngram-entry] length %d, freq %d, entropy %d, '%s'\n", i, ngs[i]->length, ngs[i]->frequency, ngs[i]->entropy, ngs[i]->str);
			}
			
			// print 30 best matches
			for(i=0; i<30 && bme[i]; i++) {
				printf("[%d], <%s>\n", bme[i]->sim, bme[i]->preparedName);		// STEP 4
			}
			
			printf("search for <%s> took %d ms\n", searchStr, span);
		}
		else
			Done = 1;
	}
	fclose(inFile);
	return 0;
}

